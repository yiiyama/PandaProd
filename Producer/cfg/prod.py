from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')
options.register('config', default = '', mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.string, info = 'Single-switch config. Values: Prompt17, Summer16')
options.register('globaltag', default = '', mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.string, info = 'Global tag')
options.register('connect', default = '', mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.string, info = 'Globaltag connect')
options.register('lumilist', default = '', mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.string, info = 'Good lumi list JSON')
options.register('isData', default = False, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.bool, info = 'True if running on Data, False if running on MC')
options.register('useTrigger', default = True, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.bool, info = 'Fill trigger information')
options.register('printLevel', default = 0, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.int, info = 'Debug level of the ntuplizer')
options.register('skipEvents', default = 0, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.int, info = 'Skip first events')
options._tags.pop('numEvent%d')
options._tagOrder.remove('numEvent%d')

options.parseArguments()

# EGM object energy smearing type to apply
egmSmearingType = 'Moriond2017_JEC'

# Global tags
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions

if options.config == 'Prompt2017':
    options.isData = True
    options.globaltag = '92X_dataRun2_Prompt_v6'

elif options.config:
    raise RuntimeError('Unknown config ' + options.config)

import FWCore.ParameterSet.Config as cms

process = cms.Process('NTUPLES')

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

if options.isData:
    process.load('Configuration.Geometry.GeometryRecoDB_cff') 
else:
    process.load('Configuration.Geometry.GeometrySimDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.GlobalTag.globaltag = options.globaltag

process.RandomNumberGeneratorService.panda = cms.PSet(
    initialSeed = cms.untracked.uint32(1234567),
    engineName = cms.untracked.string('TRandom3')
)
process.RandomNumberGeneratorService.smearedElectrons = cms.PSet(
    initialSeed = cms.untracked.uint32(89101112),
    engineName = cms.untracked.string('TRandom3')
)
process.RandomNumberGeneratorService.smearedPhotons = cms.PSet(
    initialSeed = cms.untracked.uint32(13141516),
    engineName = cms.untracked.string('TRandom3')
)

#############################
## RECO SEQUENCE AND SKIMS ##
#############################

if options.isData:
    ### EGAMMA SCALING
    # https://twiki.cern.ch/twiki/bin/view/CMS/EGMSmearer
    # Configurations in ECALELFS repo don't work out-of-the-box for us; downloaded into PandaProd.

    # Will use the same modules as EGM smearing (internally the modules switch between scaling (data)
    # and smearing (MC)). Scaling parameters are run-dependent and not defined for 2017 data yet.

    egmCorrectionSequence = cms.Sequence()

else:
    ### EGAMMA SMEARING
    # https://twiki.cern.ch/twiki/bin/view/CMS/EGMSmearer
    # Configurations in ECALELFS repo don't work out-of-the-box for us; downloaded into PandaProd.
    
    import PandaProd.Producer.utils.egmidconf as egmidconf
    
    from PandaProd.Producer.utils.calibratedEgamma_cfi import calibratedPatElectrons, calibratedPatPhotons
    process.smearedElectrons = calibratedPatElectrons.clone(
        electrons = 'slimmedElectrons',
        isMC = (not options.isData),
        correctionFile = egmidconf.electronSmearingData[egmSmearingType]
    )
    process.smearedPhotons = calibratedPatPhotons.clone(
        photons = 'slimmedPhotons',
        isMC = (not options.isData),
        correctionFile = egmidconf.photonSmearingData[egmSmearingType]
    )   
    
    egmCorrectionSequence = cms.Sequence(
        process.smearedElectrons +
        process.smearedPhotons
    )


### Vanilla MET
# this is the most basic MET one can find
# even if we override with various types of MET later on, create this so we have a consistent calo MET
# https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription

from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD

runMetCorAndUncFromMiniAOD(
    process,
    isData = options.isData,
) 
metSequence = cms.Sequence(
    process.fullPatMetSequence
)

### PUPPI

# Original EDProducer to very simply make puppi candidates out of packed candidates (as input to puppi jets below)
process.load('PandaProd.Auxiliary.PuppiCandidatesProducer_cfi')

puppiSequence = cms.Sequence(process.puppi)

### EGAMMA ID
# https://twiki.cern.ch/twiki/bin/view/CMS/EgammaIDRecipesRun2
# https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2
# https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2

from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupAllVIDIdsInModule, setupVIDElectronSelection, setupVIDPhotonSelection, switchOnVIDElectronIdProducer, switchOnVIDPhotonIdProducer, DataFormat
# Loads egmGsfElectronIDs
switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)
setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff', setupVIDElectronSelection)
setupAllVIDIdsInModule(process, 'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronHLTPreselecition_Summer16_V1_cff', setupVIDElectronSelection)

switchOnVIDPhotonIdProducer(process, DataFormat.MiniAOD)
setupAllVIDIdsInModule(process, 'RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring16_V2p2_cff', setupVIDPhotonSelection)

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
# initBTag is already being called in makeFatJets() but we call it here again to eliminate implicit dependence between parts of config
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

### GEN JET FLAVORS
if not options.isData:
    process.load('PhysicsTools.JetMCAlgos.HadronAndPartonSelector_cfi')
    from PhysicsTools.JetMCAlgos.AK4PFJetsMCFlavourInfos_cfi import ak4JetFlavourInfos

    process.selectedHadronsAndPartons.particles = 'prunedGenParticles'

    process.ak4GenJetFlavourInfos = ak4JetFlavourInfos.clone(
        jets = 'slimmedGenJets'
    )
    process.ak8GenJetFlavourInfos = ak4JetFlavourInfos.clone(
        jets = 'genJetsNoNuAK8',
        rParam = 0.8
    )
    process.ca15GenJetFlavourInfos = ak4JetFlavourInfos.clone(
        jets = 'genJetsNoNuCA15',
        jetAlgorithm = 'CambridgeAachen',
        rParam = 1.5
    )

    genJetFlavorSequence = cms.Sequence(
        process.selectedHadronsAndPartons +
        process.ak4GenJetFlavourInfos +
        process.ak8GenJetFlavourInfos +
        process.ca15GenJetFlavourInfos
    )
else:
    genJetFlavorSequence = cms.Sequence()

### MONOX FILTER

process.load('PandaProd.Filters.MonoXFilter_cfi')
process.MonoXFilter.taggingMode = True

### RECO PATH

process.reco = cms.Path(
    egmCorrectionSequence +
    egmIdSequence +
    puppiSequence +
    metSequence +
    process.MonoXFilter +
    process.QGTagger +
    fatJetSequence +
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
    del process.panda.fillers.electrons.smearedElectrons
    del process.panda.fillers.photons.smearedPhotons

if not options.useTrigger:
    process.panda.fillers.hlt.enabled = False

process.panda.outputFile = options.outputFile
process.panda.printLevel = options.printLevel

process.ntuples = cms.EndPath(process.panda)

##############
## SCHEDULE ##
##############

process.schedule = cms.Schedule(process.reco, process.ntuples)

############################
## REPLACE-ALL TYPE FIXES ##
############################

# runMetCorAnd.. adds a CaloMET module only once, adding the postfix
# However, repeated calls to the function overwrites the MET source of patCaloMet
process.patCaloMet.metSource = 'metrawCalo'

if options.connect:
    if options.connect == 'mit':
        options.connect = 'frontier://(proxyurl=http://squid.cmsaf.mit.edu:3128)(proxyurl=http://squid1.cmsaf.mit.edu:3128)(proxyurl=http://squid2.cmsaf.mit.edu:3128)(serverurl=http://cmsfrontier.cern.ch:8000/FrontierProd)/CMS_CONDITIONS'

    process.GlobalTag.connect = options.connect
    for toGet in process.GlobalTag.toGet:
        toGet.connect = options.connect
