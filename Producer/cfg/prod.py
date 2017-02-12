from FWCore.ParameterSet.VarParsing import VarParsing

options =VarParsing('analysis')
options.register('globaltag', default = '', mult = VarParsing.multiplicity.singleton, info = 'Global tag')
options.register('lumilist', default = '', mult = VarParsing.multiplicity.singleton, info = 'Good lumi list JSON')
options.register('isData', default = False, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.bool, info = 'True if running on Data, False if running on MC')
options.register('useTrigger', default = True, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.bool, info = 'Fill trigger information')
options._tags.pop('numEvent%d')
options._tagOrder.remove('numEvent%d')

options.parseArguments()

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
    fileNames = cms.untracked.vstring(options.inputFiles)
)

### NUMBER OF EVENTS
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))

### LUMI MASK
if options.lumilist != '':
    import FWCore.PythonUtilities.LumiList as LumiList
    process.source.lumisToProcess = LumiList.LumiList(filename = options.lumilist).getVLuminosityBlockRange()

##############
## SERVICES ##
##############

process.load('Configuration.Geometry.GeometryIdeal_cff') 
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
if options.globaltag == '':
    if options.isData:
        # sept reprocessing
        process.GlobalTag.globaltag = '80X_dataRun2_2016SeptRepro_v7'
    else:
        process.GlobalTag.globaltag = '80X_mcRun2_asymptotic_2016_TrancheIV_v8'
else:
    process.GlobalTag.globaltag = options.globaltag

process.RandomNumberGeneratorService.panda = cms.PSet(
    initialSeed = cms.untracked.uint32(1234567),
    engineName = cms.untracked.string('TRandom3')
)

#############################
## RECO SEQUENCE AND SKIMS ##
#############################

jecLevels= ['L1FastJet',  'L2Relative', 'L3Absolute']
if options.isData:
    jecLevels.append('L2L3Residual')

### MONOX FILTER
process.load('PandaProd.Filters.MonoXFilter_cfi')
process.MonoXFilter.taggingMode = True

### EGAMMA ID
# Loads photonIDValueMapProducer, egmPhotonIDs, and egmGsfElectronIDs

from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupAllVIDIdsInModule, setupVIDElectronSelection, setupVIDPhotonSelection, switchOnVIDPhotonIdProducer, switchOnVIDElectronIdProducer, DataFormat
switchOnVIDPhotonIdProducer(process, DataFormat.MiniAOD)
switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)
setupAllVIDIdsInModule(process, 'RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring15_25ns_V1_cff', setupVIDPhotonSelection)
setupAllVIDIdsInModule(process, 'RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring16_V2p2_cff', setupVIDPhotonSelection)
setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff', setupVIDElectronSelection)
setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronHLTPreselecition_Summer16_V1_cff', setupVIDElectronSelection)

process.load('PandaProd.Auxiliary.WorstIsolationProducer_cfi')

egmIdSequence = cms.Sequence(
    process.photonIDValueMapProducer +
    process.egmPhotonIDs +
    process.egmGsfElectronIDs +
    process.worstIsolationProducer
)

### PUPPI

process.load('PandaProd.Producer.utils.puppi_cff')
from PandaProd.Producer.utils.puppi_cff import puppiSequence

### QG TAGGING

from RecoJets.JetProducers.QGTagger_cfi import QGTagger

process.QGTagger = QGTagger.clone(
    srcJets = 'slimmedJets',
    jetsLabel = cms.string('QGL_AK4PFchs')
)

jetTaggingSequence = cms.Sequence(
    process.QGTagger
)

### MET

from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
# process.fullPatMetSequence
runMetCorAndUncFromMiniAOD(process, isData = options.isData)
# process.fullPatMetSequencePuppi
runMetCorAndUncFromMiniAOD(
    process,
    isData = options.isData,
    metType = 'Puppi',
    pfCandColl = 'puppiForMET',
    recoMetFromPFCs = True,
    jetFlavor = 'AK4PFPuppi',
    postfix = 'Puppi'
)

### FAT JETS

from PandaProd.Producer.utils.makeFatJets_cff import initFatJets, makeFatJets

# pfCHS set up here
fatJetInitSequence = initFatJets(process, options.isData, ['AK8', 'CA15'])

ak8CHSSequence = makeFatJets(
    process,
    isData = options.isData,
    label = 'AK8PFchs',
    candidates = 'pfCHS'
)

ak8PuppiSequence = makeFatJets(
    process,
    isData = options.isData,
    label = 'AK8PFPuppi',
    candidates = 'puppi'
)

ca15CHSSequence = makeFatJets(
    process,
    isData = options.isData,
    label = 'CA15PFchs',
    candidates = 'pfCHS'
)

ca15PuppiSequence = makeFatJets(
    process,
    isData = options.isData,
    label = 'CA15PFPuppi',
    candidates = 'puppi'
)

from PandaProd.Producer.utils.setupBTag import initBTag, setupDoubleBTag
initBTag(process, '', 'packedPFCandidates', 'offlineSlimmedPrimaryVertices')
ak8CHSDoubleBTagSequence = setupDoubleBTag(process, 'packedPatJetsAK8PFchs', 'AK8PFchs', '', 'ak8')
ak8PuppiDoubleBTagSequence = setupDoubleBTag(process, 'packedPatJetsAK8PFPuppi', 'AK8PFPuppi', '', 'ak8')
ca15CHSDoubleBTagSequence = setupDoubleBTag(process, 'packedPatJetsCA15PFchs', 'CA15PFchs', '', 'ca15')
ca15PuppiDoubleBTagSequence = setupDoubleBTag(process, 'packedPatJetsCA15PFPuppi', 'CA15PFPuppi', '', 'ca15')

fatJetSequence = cms.Sequence(
    fatJetInitSequence +
    ak8CHSSequence +
    ak8PuppiSequence +
    ca15CHSSequence +
    ca15PuppiSequence +
    ak8CHSDoubleBTagSequence +
    ak8PuppiDoubleBTagSequence +
    ca15CHSDoubleBTagSequence +
    ca15PuppiDoubleBTagSequence
)

### RECO PATH

process.reco = cms.Path(
    process.MonoXFilter +
    egmIdSequence +
    puppiSequence +
    jetTaggingSequence +
#    process.fullPatMetSequence +
#    process.fullPatMetSequencePuppi +
    fatJetSequence
)

#############
## NTULPES ##
#############

process.load('PandaProd.Producer.panda_cfi')
process.panda.isRealData = options.isData
process.panda.useTrigger = options.useTrigger
#process.panda.SelectEvents = ['reco'] # no skim
if options.isData:
    process.panda.fillers.partons.enabled = False
    process.panda.fillers.genParticles.enabled = False
    process.panda.fillers.genJets.enabled = False
if not options.useTrigger:
    process.panda.fillers.hlt.enabled = False

process.panda.outputFile = options.outputFile

process.ntuples = cms.EndPath(process.panda)

process.schedule = cms.Schedule(process.reco, process.ntuples)
