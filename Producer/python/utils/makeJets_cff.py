import FWCore.ParameterSet.Config as cms

from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets
from RecoJets.JetProducers.QGTagger_cfi import QGTagger
from PhysicsTools.PatAlgos.producersLayer1.jetProducer_cfi import patJets
from PhysicsTools.PatAlgos.selectionLayer1.jetSelector_cfi import selectedPatJets
from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import patJetCorrFactors
from PhysicsTools.PatAlgos.slimming.slimmedJets_cfi import slimmedJets

from PandaProd.Producer.utils.addattr import AddAttr
from PandaProd.Producer.utils.setupBTag import initBTag, setupBTag
from PhysicsTools.PatAlgos.mcMatchLayer0.jetMatch_cfi import patJetGenJetMatch
from PhysicsTools.JetMCAlgos.HadronAndPartonSelector_cfi import algoSelectedHadronsAndPartons
from PhysicsTools.JetMCAlgos.AK4PFJetsMCFlavourInfos_cfi import algoAk4JetFlavourInfos
from PhysicsTools.JetMCAlgos.GenHFHadronMatcher_cff import algoMatchGenBHadron
from PhysicsTools.JetMCAlgos.GenHFHadronMatcher_cff import algoMatchGenCHadron


pfSource = 'packedPFCandidates'
pvSource = 'offlineSlimmedPrimaryVertices'
electrons = 'slimmedElectrons'
muons = 'slimmedMuons'
taus = 'slimmedTaus'
photons = 'slimmedPhotons'
genJets = 'slimmedGenJets'
genParticleCollection = 'prunedGenParticles'

def makeJets(process, isData, label, candidates, suffix):
    """
    Light-weight version of pat addJetCollection.
    @labels: e.g. 'AK4PFPuppi'
    """

    sequence = cms.Sequence()

    addattr = AddAttr(process, sequence, suffix)

    jets = addattr('ak4PFJets',
        ak4PFJets.clone(
            src = candidates,
            doAreaFastjet = True
        )
    )

    jecLevels= ['L1FastJet',  'L2Relative', 'L3Absolute']
    if isData:
        jecLevels.append('L2L3Residual')

    jetCorrFactors = addattr('jetCorrFactors',
        patJetCorrFactors.clone(
            src = jets,
            payload = label,
            levels = jecLevels,
            primaryVertices = pvSource
        )
    )

    # btag should always use standard PF collection
    sequence += initBTag(process, '', pfSource, pvSource)

    sequence += setupBTag(
        process,
        jetCollection = jets,
        suffix = suffix,
        vsuffix = '',
        muons = muons,
        electrons = electrons,
        tags = [
            'pfCombinedInclusiveSecondaryVertexV2BJetTags',
            'pfCombinedMVAV2BJetTags',
            'pfDeepCSVJetTags',
            'pfDeepCMVAJetTags'
        ]
    )

    qgTagger = addattr('QGTagger',
        QGTagger.clone(
            srcJets = jets
        )
    )

    if not isData:
        genJetMatch = addattr('genJetMatch',
            patJetGenJetMatch.clone(
                src = jets,
                maxDeltaR = 0.4,
                matched = genJets
            )
        )
        # Begin GenHFHadronMatcher subsequences
        # Adapted from PhysicsTools/JetMCAlgos/test/matchGenHFHadrons.py

        # Supplies PDG ID to real name resolution of MC particles
        process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
        
        # Ghost particle collection used for Hadron-Jet association 
        # MUST use proper input particle collection
        selectedHadronsAndPartons = addattr('selectedHadronsAndPartons', 
            algoSelectedHadronsAndPartons.clone(
                particles = genParticleCollection
            )
        )
        
        # Input particle collection for matching to gen jets (partons + leptons) 
        # MUST use use proper input jet collection: the jets to which hadrons should be associated
        # rParam and jetAlgorithm MUST match those used for jets to be associated with hadrons
        # More details on the tool: https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideBTagMCTools#New_jet_flavour_definition
        genJetFlavourInfos = addattr('genJetFlavourInfos',
            algoAk4JetFlavourInfos.clone(
                jets = genJets,
            )
        )
        
        # Plugin for analysing B hadrons
        # MUST use the same particle collection as in selectedHadronsAndPartons
        matchGenBHadron = addattr('matchGenBHadron',
            algoMatchGenBHadron.clone(
                genParticles = genParticleCollection,
                jetFlavourInfos = "genJetFlavourInfos"
            )
        )
        
        # Plugin for analysing C hadrons
        # MUST use the same particle collection as in selectedHadronsAndPartons
        matchGenCHadron = addattr('matchGenCHadron',
            algoMatchGenCHadron.clone(
                genParticles = genParticleCollection,
                jetFlavourInfos = "genJetFlavourInfos"
            )
        )
        #End GenHFHadronMatcher subsequences


    allPatJets = addattr('patJets',
        patJets.clone(
            jetSource = jets,
            addJetCorrFactors = True,
            jetCorrFactorsSource = [jetCorrFactors],
            addBTagInfo = True,
            discriminatorSources = [
                cms.InputTag('pfCombinedInclusiveSecondaryVertexV2BJetTags' + suffix),
                cms.InputTag('pfCombinedMVAV2BJetTags' + suffix),
                ] + \
                sum([[cms.InputTag('pfDeepCSVJetTags' + suffix, 'prob' + prob),
                      cms.InputTag('pfDeepCMVAJetTags' + suffix, 'prob' + prob)]
#                     for prob in ['udsg', 'b', 'c', 'bb', 'cc']],
                     for prob in ['udsg', 'b', 'c', 'bb']],
                    []),
            addAssociatedTracks = False,
            addJetCharge = False,
            addGenPartonMatch = False,
            addGenJetMatch = (not isData),
            getJetMCFlavour = False,
            addJetFlavourInfo = False
        )
    )

    addattr.last.userData.userFloats.src = [qgTagger.getModuleLabel() + ':qgLikelihood']
    addattr.last.userData.userFloats.labelPostfixesToStrip = cms.vstring(suffix)

    if not isData:
        addattr.last.genJetMatch = genJetMatch

    selectedJets = addattr('selectedJets',
        selectedPatJets.clone(
            src = allPatJets,
            cut = 'pt > 15'
        )
    )

    addattr('slimmedJets',
        slimmedJets.clone(
            src = selectedJets,
            rekeyDaughters = '0'
        )
    )

    return sequence
