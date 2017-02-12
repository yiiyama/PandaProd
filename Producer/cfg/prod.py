import os
from FWCore.ParameterSet.VarParsing import VarParsing

options =VarParsing('analysis')
options.register('conf', default = '', mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.string, info = 'Single-switch config. Values: 03Feb2017, 23Sep2016, Spring16, Summer16')
options.register('globaltag', default = '', mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.string, info = 'Global tag')
options.register('lumilist', default = '', mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.string, info = 'Good lumi list JSON')
options.register('isData', default = False, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.bool, info = 'True if running on Data, False if running on MC')
options.register('useTrigger', default = True, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.bool, info = 'Fill trigger information')
options._tags.pop('numEvent%d')
options._tagOrder.remove('numEvent%d')

options.parseArguments()

jetMETReco = True
if options.conf == '03Feb2017':
    jetMETReco = False
    options.isData = True
    options.globaltag = '80X_dataRun2_2016SeptRepro_v7'
elif options.conf == '23Sep2016':
    options.isData = True
    options.globaltag = '80X_dataRun2_2016SeptRepro_v7'
elif options.conf == 'Spring16':
    options.isData = False
    options.globaltag = '80X_mcRun2_asymptotic_2016_v3'
elif options.conf == 'Summer16':
    options.isData = False
    options.globaltag = '80X_mcRun2_asymptotic_2016_TrancheIV_v8'

if options.conf == '03Feb2017' or options.conf == '23Sep2016':
    jsonDir = '/cvmfs/cvmfs.cmsaf.mit.edu/hidsk0001/cmsprod/cms/json'
    lumilist = 'Cert_271036-284044_13TeV_23Sep2016ReReco_Collisions16_JSON.txt'

    if os.path.isdir(jsonDir):
        options.lumilist = jsonDir + '/' + lumilist
    elif os.path.exists(lumilist):
        options.lumilist = lumilist
    else:
        print 'No good lumi mask applied'

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
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

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

### MONOX FILTER

process.load('PandaProd.Filters.MonoXFilter_cfi')
process.MonoXFilter.taggingMode = True

### PUPPI

# 80X does not contain the latest & greatest PuppiPhoton; need to rerun
from PhysicsTools.PatAlgos.slimming.puppiForMET_cff import makePuppiesFromMiniAOD
# Creates process.puppiMETSequence which includes 'puppi' and 'puppiForMET' (= EDProducer('PuppiPhoton'))
# *UGLY* also runs switchOnVIDPhotonIdProducer and sets up photon id Spring16_V2p2 internally
# which loads photonIDValueMapProducer and egmPhotonIDs
makePuppiesFromMiniAOD(process, createScheduledSequence = True)

# Just renaming
puppiForMETSequence = process.puppiMETSequence

### PUPPI MET

from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
# Creates process.fullPatMetSequencePuppi
runMetCorAndUncFromMiniAOD(
    process,
    isData = options.isData,
    metType = 'Puppi',
    pfCandColl = 'puppiForMET',
    recoMetFromPFCs = True,
    jetFlavor = 'AK4PFPuppi',
    postfix = 'Puppi'
)

### EGAMMA ID

from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupAllVIDIdsInModule, setupVIDElectronSelection, switchOnVIDElectronIdProducer, DataFormat
# Loads egmGsfElectronIDs
switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)
setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff', setupVIDElectronSelection)
setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronHLTPreselecition_Summer16_V1_cff', setupVIDElectronSelection)

process.load('PandaProd.Auxiliary.WorstIsolationProducer_cfi')

egmIdSequence = cms.Sequence(
    process.photonIDValueMapProducer +
    process.egmPhotonIDs +
    process.egmGsfElectronIDs +
    process.worstIsolationProducer
)

### QG TAGGING

process.load('RecoJets.JetProducers.QGTagger_cfi')
process.QGTagger.srcJets = 'slimmedJets'

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
    puppiForMETSequence +
    process.fullPatMetSequencePuppi +
    process.QGTagger +
    fatJetSequence
)

if jetMETReco:
    ### RECLUSTER PUPPI JET

    from PandaProd.Producer.utils.makeJets_cff import makeJets

    puppiJetSequence = makeJets(process, options.isData, 'AK4PFPuppi', 'puppi', 'Puppi')

    process.reco += puppiJetSequence
    
    ### JET RE-CORRECTION

    from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import updatedPatJetCorrFactors, updatedPatJets

    jecLevels= ['L1FastJet',  'L2Relative', 'L3Absolute']
    if options.isData:
        jecLevels.append('L2L3Residual')
    
    process.updatedPatJetCorrFactors = updatedPatJetCorrFactors.clone(
        src = cms.InputTag('slimmedJets', '', cms.InputTag.skipCurrentProcess()),
        levels = cms.vstring(*jecLevels),
    )

    process.slimmedJets = updatedPatJets.clone(
        jetSource = cms.InputTag('slimmedJets', '', cms.InputTag.skipCurrentProcess()),
        addJetCorrFactors = cms.bool(True),
        jetCorrFactorsSource = cms.VInputTag(cms.InputTag("updatedPatJetCorrFactors")),
        addBTagInfo = cms.bool(False),
        addDiscriminators = cms.bool(False)
    )

    jetRecorrectionSequence = cms.Sequence(
        process.updatedPatJetCorrFactors +
        process.slimmedJets
    )

    process.reco.insert(process.reco.index(process.QGTagger), jetRecorrectionSequence)

    ### MET

    # Collection naming aligned with 03Feb2017 reminiaod

    # Creates process.fullPatMetSequenceUncorrected which includes slimmedMETsUncorrected
    runMetCorAndUncFromMiniAOD(
        process,
        isData = options.isData,
        postfix = 'Uncorrected'
    )

    # Make MET also with bad muon removal
    from PhysicsTools.PatUtils.tools.muonRecoMitigation import muonRecoMitigation

    # Creates cleanMuonsPFCandidates
    muonrecoMitigation(
        process,
        pfCandCollection = 'packedPFCandidates',
        runOnMiniAOD = True
    )

    # Creates process.fullPatMetSequenceMuEGClean which includes slimmedMETsMuEGClean
    # Postfix MuEGClean is just for convenience - there is no EG cleaning actually applied
    runMetCorAndUncFromMiniAOD(
        process,
        isData = options.isData,
        pfCandColl = 'cleanMuonsPFCandidates',
        recoMetFromPFCs = True,
        postfix = 'MuEGClean'
    )

    process.reco += process.fullPatMetSequenceUncorrected
    process.reco += process.fullPatMetSequenceMuEGClean

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

process.panda.fillers.met.met = 'slimmedMETsMuEGClean'
process.panda.fillers.metNoFix = process.panda.fillers.puppiMet.clone(
    met = 'slimmedMETsUncorrected'
)
if not jetMETReco:
    process.panda.fillers.metMuOnlyFix = process.panda.fillers.puppiMet.clone(
        met = 'slimmedMETs'
    )

process.panda.outputFile = options.outputFile

process.ntuples = cms.EndPath(process.panda)

process.schedule = cms.Schedule(process.reco, process.ntuples)
