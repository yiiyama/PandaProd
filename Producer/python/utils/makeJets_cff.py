import FWCore.ParameterSet.Config as cms

from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets
from PhysicsTools.PatAlgos.producersLayer1.jetProducer_cfi import patJets
from PhysicsTools.PatAlgos.selectionLayer1.jetSelector_cfi import selectedPatJets
from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import patJetCorrFactors
from PhysicsTools.PatAlgos.slimming.slimmedJets_cfi import slimmedJets

from PandaProd.Producer.utils.addattr import AddAttr
from PandaProd.Producer.utils.setupBTag import initBTag, setupBTag
from PhysicsTools.PatAlgos.mcMatchLayer0.jetMatch_cfi import patJetGenJetMatch

pfSource = 'packedPFCandidates'
pvSource = 'offlineSlimmedPrimaryVertices'
electrons = 'slimmedElectrons'
muons = 'slimmedMuons'
taus = 'slimmedTaus'
photons = 'slimmedPhotons'
genJets = 'slimmedGenJets'

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

    if not isData:
        genJetMatch = addattr('genJetMatch',
            patJetGenJetMatch.clone(
                src = jets,
                maxDeltaR = 0.4,
                matched = genJets
            )
        )

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
                     for prob in ['udsg', 'b', 'c', 'bb', 'cc']],
                    []),
            addAssociatedTracks = False,
            addJetCharge = False,
            addGenPartonMatch = False,
            addGenJetMatch = (not isData),
            getJetMCFlavour = False,
            addJetFlavourInfo = False
        )
    )

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
