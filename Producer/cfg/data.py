import FWCore.ParameterSet.Config as cms

process = cms.Process('NTUPLES')

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

############
## SOURCE ##
############

### INPUT FILES
process.source = cms.Source('PoolSource',
    skipEvents = cms.untracked.uint32(0),
    fileNames = cms.untracked.vstring('XX-LFN-XX')
)

### NUMBER OF EVENTS
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

##############
## SERVICES ##
##############

process.load('Configuration.Geometry.GeometryIdeal_cff') 
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.GlobalTag.globaltag = '80X_dataRun2_2016SeptRepro_v3'

#############################
## RECO SEQUENCE AND SKIMS ##
#############################

jecLevels= ['L1FastJet',  'L2Relative', 'L3Absolute', 'L2L3Residual']

### MONOX FILTER
process.load('PandaProd.Filters.MonoXFilter_cfi')
process.MonoXFilter.taggingMode = True

### EGAMMA ID
# Loads photonIDValueMapProducer, egmPhotonIDs, and egmGsfElectronIDs

import PandaProd.Producer.utils.egmidconf as egmidconf
egmidconf.electronCombIsoEA = 'RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_25ns.txt'
egmidconf.electronEcalIsoEA = 'PandaProd/Producer/data/effAreaElectrons_HLT_ecalPFClusterIso.txt'
egmidconf.electronHcalIsoEA = 'PandaProd/Producer/data/effAreaElectrons_HLT_hcalPFClusterIso.txt'
egmidconf.electronId = 'egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-'
egmidconf.photonId = 'egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-'
egmidconf.photonEA = 'RecoEgamma/PhotonIdentification/data/Spring15/effAreaPhotons_cone03_'

from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupAllVIDIdsInModule, setupVIDElectronSelection, setupVIDPhotonSelection, switchOnVIDPhotonIdProducer, switchOnVIDElectronIdProducer, DataFormat
switchOnVIDPhotonIdProducer(process, DataFormat.MiniAOD)
switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)
setupAllVIDIdsInModule(process, 'RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring15_25ns_V1_cff', setupVIDPhotonSelection)
setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Spring15_25ns_V1_cff', setupVIDElectronSelection)

egmIdSequence = cms.Sequence(
    process.photonIDValueMapProducer +
    process.egmPhotonIDs +
    process.egmGsfElectronIDs
)

### PUPPI
process.load('PandaProd.Producer.utils.puppi_cff')
from PandaProd.Producer.utils.puppi_cff import puppiSequence

### RECLUSTER PUPPI JET
from PandaProd.Producer.utils.makeJets_cff import makeJets
puppiJetSequence = makeJets(process, True, 'AK4PFPuppi', 'puppiForMET', 'Puppi')

### JET RE-CORRECTION, TAGGING, AND SLIMMING
from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import updatedPatJetCorrFactors, updatedPatJets

process.updatedPatJetCorrFactors = updatedPatJetCorrFactors.clone(
    src = cms.InputTag('slimmedJets', '', cms.InputTag.skipCurrentProcess()),
    levels = cms.vstring(*jecLevels),
)

from RecoJets.JetProducers.QGTagger_cfi import QGTagger

process.slimmedJets = updatedPatJets.clone(
    jetSource = cms.InputTag('slimmedJets', '', cms.InputTag.skipCurrentProcess()),
    addJetCorrFactors = cms.bool(True),
    jetCorrFactorsSource = cms.VInputTag(cms.InputTag("updatedPatJetCorrFactors")),
    addBTagInfo = cms.bool(False),
    addDiscriminators = cms.bool(False)
)

process.QGTagger = QGTagger.clone(
    srcJets = 'slimmedJets',
    jetsLabel = cms.string('QGL_AK4PFchs')
)

jetRecorrectionSequence = cms.Sequence(
    process.updatedPatJetCorrFactors +
    process.slimmedJets +
    process.QGTagger
)

### MET
from PandaProd.Producer.utils.makeMET_cff import initMET, makeMET

initMetSequence = initMET(process, True)

# extracts the raw pfMET from input MINIAOD and repack new corrections
pfMetSequence = makeMET(process, True, 'packedPFCandidates', 'slimmedJets', 'AK4PFchs')
# compute a brand-new pfMET from puppi candidates and pack with corrections
puppiMetSequence = makeMET(process, True, 'puppiForMET', 'selectedJetsPuppi', 'AK4PFPuppi', 'Puppi')

metSequence = cms.Sequence(
    initMetSequence +
    pfMetSequence +
    puppiMetSequence
)

### FAT JETS

from PandaProd.Producer.utils.makeFatJets_cff import initFatJets, makeFatJets

fatJetInitSequence = initFatJets(process, True, ['AK8', 'CA15'])

ak8CHSSequence = makeFatJets(
    process,
    isData = True,
    label = 'AK8PFchs'
)

ak8PuppiSequence = makeFatJets(
    process,
    isData = True,
    label = 'AK8PFPuppi'
)

ca15CHSSequence = makeFatJets(
    process,
    isData = True,
    label = 'CA15PFchs'
)

ca15PuppiSequence = makeFatJets(
    process,
    isData = True,
    label = 'CA15PFPuppi'
)

fatJetSequence = cms.Sequence(
    fatJetInitSequence +
    ak8CHSSequence +
    ak8PuppiSequence +
    ca15CHSSequence +
    ca15PuppiSequence
)

### MET FILTERS
process.load('RecoMET.METFilters.BadPFMuonFilter_cfi')
process.BadPFMuonFilter.muons = cms.InputTag('slimmedMuons')
process.BadPFMuonFilter.PFCandidates = cms.InputTag('packedPFCandidates')
process.BadPFMuonFilter.taggingMode = cms.bool(True)

process.load('RecoMET.METFilters.BadChargedCandidateFilter_cfi')
process.BadChargedCandidateFilter.muons = cms.InputTag('slimmedMuons')
process.BadChargedCandidateFilter.PFCandidates = cms.InputTag('packedPFCandidates')
process.BadChargedCandidateFilter.taggingMode = cms.bool(True)

metFilterSequence = cms.Sequence(
    process.BadPFMuonFilter +
    process.BadChargedCandidateFilter
)

process.reco = cms.Path(
    process.MonoXFilter +
    egmIdSequence +
    puppiSequence +
    puppiJetSequence +
    jetRecorrectionSequence +
    metSequence +
    fatJetSequence +
    metFilterSequence
)

process.RandomNumberGeneratorService.panda = cms.PSet(
    initialSeed = cms.untracked.uint32(1234567),
    engineName = cms.untracked.string('TRandom3')
)

#############
## NTULPES ##
#############

process.load('PandaProd.Producer.panda_cfi')
process.panda.isRealData = True
process.panda.useTrigger = True
#process.panda.SelectEvents = ['reco'] # no skim
process.panda.fillers.partons.enabled = False
process.panda.fillers.genParticles.enabled = False
process.panda.fillers.genJets.enabled = False

process.panda.outputFile = 'kraken-output-file-tmp_000.root'

process.ntuples = cms.EndPath(process.panda)

process.schedule = cms.Schedule(process.reco, process.ntuples)
