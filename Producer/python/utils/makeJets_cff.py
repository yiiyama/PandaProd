import FWCore.ParameterSet.Config as cms

from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets
from RecoJets.JetProducers.QGTagger_cfi import QGTagger
from RecoJets.JetProducers.PileupJetID_cfi import pileupJetId
from PhysicsTools.PatAlgos.producersLayer1.jetProducer_cfi import patJets
from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cfi import updatedPatJets
from PhysicsTools.PatAlgos.selectionLayer1.jetSelector_cfi import selectedPatJets
from PhysicsTools.PatAlgos.recoLayer0.jetCorrections_cff import patJetCorrFactors
from PhysicsTools.PatAlgos.slimming.slimmedJets_cfi import slimmedJets

from PandaProd.Producer.utils.addattr import AddAttr
from PandaProd.Producer.utils.setupBTag import addBTagInfo
from PhysicsTools.PatAlgos.mcMatchLayer0.jetMatch_cfi import patJetGenJetMatch


pfSource = 'packedPFCandidates'
pvSource = 'offlineSlimmedPrimaryVertices'
electrons = 'slimmedElectrons'
muons = 'slimmedMuons'
taus = 'slimmedTaus'
photons = 'slimmedPhotons'
genJets = 'slimmedGenJets'
genParticleCollection = 'prunedGenParticles'

def makeJets(process, isData, suffix, jecType='', candidates='', patModule=None, patSequence=None):
    """
    Light-weight version of pat addJetCollection.
    @param suffix: output label suffix
    @param candidates: packedPFCandidates, pfCHS, puppi, etc.
    @param jecType: e.g. 'AK4PFPuppi'
    @param patModule: if a patJets collection is already made, skip the first few steps
    @param patSequence: sequence containing patModule.
    """

    prePatJetsSequence = cms.Sequence()

    addattr = AddAttr(process, prePatJetsSequence, suffix)

    if patModule is None:
        jets = addattr('ak4PFJets',
            ak4PFJets.clone(
                src = candidates,
                doAreaFastjet = True
            )
        )
    
        jecLevels = ['L1FastJet',  'L2Relative', 'L3Absolute']
        if isData:
            jecLevels.append('L2L3Residual')
    
        jetCorrFactors = addattr('jetCorrFactors',
            patJetCorrFactors.clone(
                src = jets,
                payload = jecType,
                levels = jecLevels,
                primaryVertices = pvSource
            )
        )

        patModule = patJets.clone(
            jetSource = jets,
            addJetCorrFactors = True,
            jetCorrFactorsSource = [jetCorrFactors],
            addBTagInfo = True,
            addAssociatedTracks = False,
            addJetCharge = False,
            addGenPartonMatch = False,
            addGenJetMatch = (not isData),
            getJetMCFlavour = False,
            addJetFlavourInfo = False
        )

        setattr(process, 'patJets' + suffix, patModule)

    else:
        jets = patModule.jetSource.getModuleLabel()

    btags = [
        'pfCombinedInclusiveSecondaryVertexV2BJetTags',
        'pfCombinedMVAV2BJetTags',
        'pfDeepCSVJetTags',
        'pfDeepCMVAJetTags',
        #'pfDeepFlavourJetTags' # for some reason DeepFlavour only works with packedCandidates if the input is a pat::Jet (see below)
    ]

    bTagSequence = addBTagInfo(process, patModule, suffix=suffix, vsuffix='', tags=btags)
    prePatJetsSequence += bTagSequence

    qgTagger = addattr('QGTagger',
        QGTagger.clone(
            srcJets = jets
        )
    )

    puJetId = addattr('pileupJetId',
        pileupJetId.clone(
            jets = jets,
            inputIsCorrected = True,
            applyJec = True,
            vertexes = pvSource
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

    if patSequence is not None:
        patSequence.insert(patSequence.index(patModule), prePatJetsSequence)
        addattr.sequence = cms.Sequence()
    else:
        addattr.sequence += patModule

    patModule.userData.userInts.src = [
        puJetId.getModuleLabel() + ':fullId'
    ]
    patModule.userData.userInts.labelPostfixesToStrip = cms.vstring(suffix)

    patModule.userData.userFloats.src = [
        qgTagger.getModuleLabel() + ':qgLikelihood',
        puJetId.getModuleLabel() + ':fullDiscriminant'
    ]
    patModule.userData.userFloats.labelPostfixesToStrip = cms.vstring(suffix)

    if not isData:
        patModule.genJetMatch = genJetMatch

    # Make another pat::Jet collection just for DeepFlavour

    patJetsWithDeepFlavour = updatedPatJets.clone(
        jetSource = cms.InputTag(patModule.label()),
        addJetCorrFactors = False,
        addBTagInfo = True,
        addDiscriminators = True
    )

    setattr(process, 'patJetsWithDeepFlavour' + suffix, patJetsWithDeepFlavour)

    btags = [
        'pfDeepFlavourJetTags'
    ]

    addattr.sequence += addBTagInfo(process, patJetsWithDeepFlavour, suffix=suffix + 'WithDeepFlavour', vsuffix='', tags=btags)

    addattr.sequence += patJetsWithDeepFlavour

    selectedJets = addattr('selectedJets',
        selectedPatJets.clone(
            src = cms.InputTag('patJetsWithDeepFlavour' + suffix),
            cut = 'pt > 15'
        )
    )

    addattr('slimmedJets',
        slimmedJets.clone(
            src = selectedJets,
            rekeyDaughters = '0'
        )
    )

    return addattr.sequence
