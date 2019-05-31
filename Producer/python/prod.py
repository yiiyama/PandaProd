from PandaProd.Producer.opts import options

if not options.config:
    raise RuntimeError('Mandatory option: config')

# Global tags
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions

# https://twiki.cern.ch/twiki/pub/CMS/PdmVAnalysisSummaryTable
if options.config == '17Jul2018':
    # re-miniaod of 2016 data 07Aug17 rereco
    options.isData = True
    options.globaltag = '94X_dataRun2_v10'
elif options.config == '31Mar2018':
    # re-miniaod of 2017 data 17Nov2017 rereco
    options.isData = True
    options.globaltag = '94X_dataRun2_v11'
elif options.config == '2018Prompt':
    # Run2018D (equivalent to 17Sep2018 rereco of ABC)
    options.isData = True
    options.globaltag = '102X_dataRun2_Prompt_v13'
elif options.config == '17Sep2018':
    # 2018 ABC data rereco. Use also for parking 05May2019 reconstruction
    options.isData = True
    options.globaltag = '102X_dataRun2_Sep2018ABC_v2'
elif options.config == 'Summer16v3':
    # 2016 MC
    options.isData = False
    options.globaltag = '102X_mcRun2_asymptotic_v6'
    options.pdfname = 'NNPDF3.0'
elif options.config == 'Fall17v2':
    # 2017 MC
    options.isData = False
    options.globaltag = '102X_mc2017_realistic_v6'
    options.pdfname = 'NNPDF3.1'
elif options.config == 'Autumn18':
    options.isData = False
    options.globaltag = '102X_upgrade2018_realistic_v18'
    options.pdfname = 'NNPDF3.1'
else:
    raise RuntimeError('Unknown config ' + options.config)

import FWCore.ParameterSet.Config as cms

process = cms.Process('NTUPLES')

process.options = cms.untracked.PSet(
    numberOfThreads = cms.untracked.uint32(1),
    numberOfStreams = cms.untracked.uint32(0)
)

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100
for cat in ['PandaProducer', 'JetPtMismatchAtLowPt', 'JetPtMismatch', 'NullTransverseMomentum', 'MissingJetConstituent']:
    process.MessageLogger.categories.append(cat)
    setattr(process.MessageLogger.cerr, cat, cms.untracked.PSet(limit = cms.untracked.int32(10)))

############
## SOURCE ##
############

### INPUT FILES
process.source = cms.Source('PoolSource',
    skipEvents = cms.untracked.uint32(options.skipEvents),
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

process.load('Configuration.Geometry.GeometryRecoDB_cff') 
if not options.isData:
    process.load('Configuration.Geometry.GeometrySimDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.GlobalTag.globaltag = options.globaltag

process.RandomNumberGeneratorService.panda = cms.PSet(
    initialSeed = cms.untracked.uint32(1234567),
    engineName = cms.untracked.string('TRandom3')
)

#############################
## RECO SEQUENCE AND SKIMS ##
#############################

from PandaProd.Producer.utils.makeJets_cff import makeJets

### PUPPI V12
# https://twiki.cern.ch/twiki/bin/view/CMS/PUPPI
# This block has to come before EGM because puppiForMET_cff creates an empty egmPhotonIDs

from PhysicsTools.PatAlgos.slimming.puppiForMET_cff import makePuppiesFromMiniAOD
# Creates process.puppiMETSequence which includes 'puppi' and 'puppiForMET' (= EDProducer('PuppiPhoton'))
# By default, does not use specific photon ID for PuppiPhoton (which was the case in 80X)
makePuppiesFromMiniAOD(process, createScheduledSequence=True)

# puppi sequences created by makePuppiesFromMiniAOD reuses weights in MINIAOD by default
process.puppiNoLep.useExistingWeights = False
process.puppi.useExistingWeights = False

puppiV12Sequence = cms.Sequence(
    process.puppiMETSequence
)

### EGAMMA CORRECTIONS AND VIDS
# https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPostRecoRecipes

from RecoEgamma.EgammaTools.EgammaPostRecoTools import setupEgammaPostRecoSeq

# adds egammaPostRecoSeq
if options.config in ['17Jul2018', 'Summer16v3']:
    setupEgammaPostRecoSeq(process, runVID=True, runEnergyCorrections=False, era='2016-Legacy')
elif options.config in ['31Mar2018', 'Fall17v2']:
    setupEgammaPostRecoSeq(process, runVID=True, era='2017-Nov17ReReco')
elif options.config in ['2018Prompt', '17Sep2018', 'Autumn18']:
    setupEgammaPostRecoSeq(process, runVID=True, era='2018-Prompt')

#process.egmPhotonIDs.physicsObjectSrc

electronFillerParams = {
    'vetoId': 'cutBasedElectronID-Fall17-94X-V1-veto',
    'looseId': 'cutBasedElectronID-Fall17-94X-V1-loose',
    'mediumId': 'cutBasedElectronID-Fall17-94X-V1-medium',
    'tightId': 'cutBasedElectronID-Fall17-94X-V1-tight',
    'mvaWP90': 'mvaEleID-Fall17-noIso-V1-wp90',
    'mvaWP80': 'mvaEleID-Fall17-noIso-V1-wp80',
    'mvaWPLoose': 'mvaEleID-Fall17-noIso-V1-wpLoose',
    'mvaIsoWP90': 'mvaEleID-Fall17-iso-V1-wp90',
    'mvaIsoWP80': 'mvaEleID-Fall17-iso-V1-wp80',
    'mvaIsoWPLoose': 'mvaEleID-Fall17-iso-V1-wpLoose',
    'hltId': '', # seems like we don't have these for >= 2017?
    'mvaValues': 'ElectronMVAEstimatorRun2Spring16GeneralPurposeV1Values',
    #'mvaCategories': 'ElectronMVAEstimatorRun2Spring16GeneralPurposeV1Categories',
    'combIsoEA': 'RecoEgamma/ElectronIdentification/data/Fall17/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_92X.txt',
    'ecalIsoEA': 'RecoEgamma/ElectronIdentification/data/Summer16/effAreaElectrons_HLT_ecalPFClusterIso.txt',
    'hcalIsoEA': 'RecoEgamma/ElectronIdentification/data/Summer16/effAreaElectrons_HLT_hcalPFClusterIso.txt'
}

# Iso leakage formulas are not consistent with the ID version (leakage for Spring16 but ID is Fall17 V1)
# Kept as is for compatibility with existing 014 samples; for the next panda release we should move to Fall17 V2 anyway
photonFillerParams = {
    'looseId': 'cutBasedPhotonID-Fall17-94X-V1-loose',
    'mediumId': 'cutBasedPhotonID-Fall17-94X-V1-medium',
    'tightId': 'cutBasedPhotonID-Fall17-94X-V1-tight',
    'chIsoLeakage': {'EB': '', 'EE': ''},
    'nhIsoLeakage': {'EB': '0.0148 * x + 0.000017 * x * x', 'EE': '0.0163 * x + 0.000014 * x * x'},
    'phIsoLeakage': {'EB': '0.0047 * x', 'EE': '0.0034 * x'}
}

if options.config in ['17Jul2018', 'Summer16v3', '2018Prompt', '17Sep2018', 'Autumn18']:
    # energy scale & smearing not available for 2018 at the moment (31 May 2018, for panda 014)
    electronFillerParams['fillCorrectedPts'] = False
    photonFillerParams['fillCorrectedPts'] = False

process.load('PandaProd.Auxiliary.WorstIsolationProducer_cfi')

egmCorrectionsSequence = cms.Sequence(
    process.egammaPostRecoSeq +
    process.worstIsolationProducer
)

### Vanilla JET+MET (+ EE NOISE MITIGATION FOR 2017)
# This is the most basic MET one can find
# Even if we override with various types of MET later on, create this so we have a consistent calo MET
# While we are at it, do full jet reclustering and also add non-default btags
# https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription
 
from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD

if options.config in ['31Mar2018', 'Fall17v2']:
    fixEE2017 = True
    fixEE2017Params = {'userawPt': True, 'ptThreshold':50.0, 'minEtaThreshold':2.65, 'maxEtaThreshold': 3.139}
else:
    fixEE2017 = False
    fixEE2017Params = {}

runMetCorAndUncFromMiniAOD(
    process,
    isData=options.isData,
    pfCandColl=cms.InputTag("packedPFCandidates"),
    recoMetFromPFCs=True,
    CHS=True,
    reclusterJets=True,
    fixEE2017=fixEE2017,
    fixEE2017Params=fixEE2017Params
)

slimmedJetsSequence = makeJets(
    process,
    options.isData,
    suffix='',
    patModule=process.patJets,
    patSequence=process.patMetModuleSequence # subsequence of fullPatMetSequence that contains patMets
)

runMetCorAndUncFromMiniAOD(
    process,
    isData=options.isData,
    metType="Puppi",
    pfCandColl=cms.InputTag("puppiForMET"),
    recoMetFromPFCs=True,
    jetFlavor="AK4PFPuppi",
    postfix="Puppi",
    fixEE2017=fixEE2017,
    fixEE2017Params=fixEE2017Params
)

slimmedJetsPuppiSequence = makeJets(
    process,
    options.isData,
    suffix='Puppi',
    patModule=process.patJetsPuppi,
    patSequence=process.patMetModuleSequencePuppi # subsequence of fullPatMetSequencePuppi that contains patMetsPuppi
)

jetMETSequence = cms.Sequence(
    process.fullPatMetSequence +
    slimmedJetsSequence +
    process.fullPatMetSequencePuppi +
    slimmedJetsPuppiSequence
)

### FAT JETS

from PandaProd.Producer.utils.makeFatJets_cff import initFatJets, makeFatJets

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

ca15PuppiSequence = makeFatJets(
    process,
    isData = options.isData,
    label = 'CA15PFPuppi',
    candidates = 'puppi'
)

fatJetSequence = cms.Sequence(
    fatJetInitSequence +
    ak8CHSSequence +
    ak8PuppiSequence +
    ca15PuppiSequence
)

### MERGE GEN PARTICLES
do_merge = not options.isData and False
if do_merge:
    process.load('PandaProd.Auxiliary.MergedGenProducer_cfi')
    genMergeSequence = cms.Sequence( process.mergedGenParticles )
else:
    genMergeSequence = cms.Sequence()

### GEN JET FLAVORS
if not options.isData:
    from PandaProd.Producer.utils.setupGenJetFlavor_cff import setupGenJetFlavor
    genJetFlavorSequence = setupGenJetFlavor(process)
else:
    genJetFlavorSequence = cms.Sequence()

### MONOX FILTER

process.load('PandaProd.Filters.MonoXFilter_cfi')
process.MonoXFilter.taggingMode = True

### RECO PATH

process.reco = cms.Path(
    egmCorrectionsSequence +
    puppiV12Sequence +
    jetMETSequence +
    process.MonoXFilter +
    fatJetSequence +
    genMergeSequence + 
    genJetFlavorSequence
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
    process.panda.fillers.ak4GenJets.enabled = False
    process.panda.fillers.ak8GenJets.enabled = False
    process.panda.fillers.ca15GenJets.enabled = False
else:
    process.panda.fillers.weights.pdfType = options.pdfname
    process.panda.fillers.extraMets.types.append('gen')

if not options.useTrigger:
    process.panda.fillers.hlt.enabled = False

for name, value in electronFillerParams.items():
    setattr(process.panda.fillers.electrons, name, value)

for name, value in photonFillerParams.items():
    if type(value) is dict:
        pset = getattr(process.panda.fillers.photons, name)
        for k, v in value.items():
            setattr(pset, k, v)
    else:
        setattr(process.panda.fillers.photons, name, value)

process.panda.fillers.muons.rochesterCorrectionSource = 'PandaProd/Utilities/data/RoccoR2017v0.txt'

process.panda.outputFile = options.outputFile
process.panda.printLevel = options.printLevel

process.ntuples = cms.EndPath(process.panda)

##############
## SCHEDULE ##
##############

process.schedule = cms.Schedule(process.reco, process.ntuples)

#from FWCore.ParameterSet.Utilities import convertToUnscheduled
#process = convertToUnscheduled(process)

if options.connect:
    if options.connect == 'mit':
        options.connect = 'frontier://(proxyurl=http://squid.cmsaf.mit.edu:3128)(proxyurl=http://squid1.cmsaf.mit.edu:3128)(proxyurl=http://squid2.cmsaf.mit.edu:3128)(serverurl=http://cmsfrontier.cern.ch:8000/FrontierProd)/CMS_CONDITIONS'

    process.GlobalTag.connect = options.connect
    for toGet in process.GlobalTag.toGet:
        toGet.connect = options.connect

if options.dumpPython:
    print process.dumpPython()
