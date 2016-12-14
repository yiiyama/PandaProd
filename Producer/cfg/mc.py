from FWCore.ParameterSet.VarParsing import VarParsing

options =VarParsing('analysis')
options.register('globaltag', default = '', mult = VarParsing.multiplicity.singleton, info = 'Global tag')
options.register('lumilist', default = '', mult = VarParsing.multiplicity.singleton, info = 'Good lumi list JSON')
options.register('isData', default = False, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.bool, info = 'True if running on Data, False if running on MC')
options.register('useTrigger', default = True, mult = VarParsing.multiplicity.singleton, mytype = VarParsing.varType.bool, info = 'Fill trigger information')

options.parseArguments()

import FWCore.ParameterSet.Config as cms

process = cms.Process('NTUPLES')

process.load('FWCore.MessageService.MessageLogger_cfi')
# If you run over many samples and you save the log, remember to reduce
# the size of the output by prescaling the report of the event number
process.MessageLogger.cerr.FwkReport.reportEvery = 5000
process.MessageLogger.categories.append('PandaProducer')
process.MessageLogger.cerr.PandaProducer = cms.untracked.PSet(limit = cms.untracked.int32(10000))

#process.load('Configuration.StandardSequences.Services_cff')

############
## SOURCE ##
############

### INPUT FILES
if len(options.inputFiles) == 0:
    if options.isData:
       inputFiles = [
           'file:/tmp/6CA25B7B-CD46-E611-BDEC-02163E01354A.root'
       ]
    else:
       inputFiles = [
           'file:/data/t3home000/snarayan/test/tt_8011.root'
       ]
else:
    inputFiles = options.inputFiles

process.source = cms.Source('PoolSource',
    skipEvents = cms.untracked.uint32(0),
    fileNames = cms.untracked.vstring(inputFiles)
)

### NUMBER OF EVENTS
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))

### LUMI MASK
if options.lumilist != '':
    process.source.lumisToProcess = LumiList.LumiList(filename = options.lumilist).getVLuminosityBlockRange()

##############
## SERVICES ##
##############

process.load('Configuration.Geometry.GeometryIdeal_cff') 
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')

#mc https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFrontierConditions#Global_Tags_for_Run2_MC_Producti
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
if options.globaltag == '':
    if options.isData:
        # sept reprocessing
        process.GlobalTag.globaltag = '80X_dataRun2_2016SeptRepro_v3'
    else:
        ## tranche IV v6 ... is this correct?
        process.GlobalTag.globaltag = '80X_mcRun2_asymptotic_2016_miniAODv2' # for 8011 MC? 
        # process.GlobalTag.globaltag = '80X_mcRun2_asymptotic_2016_TrancheIV_v6'
else:
    process.GlobalTag.globaltag = options.globaltag

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

import PandaProd.Producer.utils.egmidconf as egmidconf

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

egmidconf.electronCombIsoEA = 'RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_25ns.txt'
egmidconf.electronEcalIsoEA = 'PandaProd/Producer/data/effAreaElectrons_HLT_ecalPFClusterIso.txt'
egmidconf.electronHcalIsoEA = 'PandaProd/Producer/data/effAreaElectrons_HLT_hcalPFClusterIso.txt'
egmidconf.electronId = 'egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-'
egmidconf.photonId = 'egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-'
egmidconf.photonEA = 'RecoEgamma/PhotonIdentification/data/Spring15/effAreaPhotons_cone03_'

### PUPPI

# Imports puppiSequence
process.load('PandaProd.Producer.utils.puppi_cff')

### RECLUSTER PUPPI JET
# Copied from PhysicsTools.PatAlgos.slimming.miniAOD_tools
from RecoJets.JetProducers.ak4PFJetsPuppi_cfi import ak4PFJetsPuppi
process.ak4PFJetsPuppi = ak4PFJetsPuppi.clone(
    src = cms.InputTag('puppiForMET'),
    doAreaFastjet = cms.bool(True)
)

from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import patJetCorrFactors

process.puppiJetCorrFactors = patJetCorrFactors.clone(
    src = cms.InputTag('ak4PFJetsPuppi'),
    payload = cms.string('AK4PFPuppi'),
    levels = cms.vstring(*jecLevels),
    primaryVertices = cms.InputTag('offlineSlimmedPrimaryVertices')
)

from PandaProd.Producer.utils.setupBTag import initBTag, setupBTag

# btag on puppi should also use packedPFCandidates
puppiInclusiveVertexingSequence = initBTag(process, '', 'packedPFCandidates', 'offlineSlimmedPrimaryVertices')

puppiJetsBTagSequence = setupBTag(
    process,
    jetCollection = 'ak4PFJetsPuppi',
    suffix = 'Puppi',
    vsuffix = '',
    muons = 'slimmedMuons',
    electrons = 'slimmedElectrons',
    tags = ['pfCombinedInclusiveSecondaryVertexV2BJetTags']
)

if not options.isData:
    from PhysicsTools.PatAlgos.mcMatchLayer0.jetMatch_cfi import patJetGenJetMatch

    process.genJetMatchPuppi = patJetGenJetMatch.clone(
        src = 'ak4PFJetsPuppi',
        maxDeltaR = 0.4,
        matched = 'slimmedGenJets'
    )

from PhysicsTools.PatAlgos.producersLayer1.jetProducer_cfi import patJets

process.patJetsPuppi = patJets.clone(
    jetSource = cms.InputTag('ak4PFJetsPuppi'),
    addJetCorrFactors = True,
    jetCorrFactorsSource = [cms.InputTag('puppiJetCorrFactors')],
    addBTagInfo = True,
    discriminatorSources = [cms.InputTag('pfCombinedInclusiveSecondaryVertexV2BJetTagsPuppi')],
    addAssociatedTracks = False,
    addJetCharge = False,
    addGenPartonMatch = False,
    addGenJetMatch = (not options.isData),
    genJetMatch = 'genJetMatchPuppi',
    getJetMCFlavour = False,
    addJetFlavourInfo = False
)

from PhysicsTools.PatAlgos.selectionLayer1.jetSelector_cfi import selectedPatJets

process.selectedPatJetsPuppi = selectedPatJets.clone(
    src = 'patJetsPuppi',
    cut = 'pt > 15'
)

puppiJetSequence = cms.Sequence(
    process.ak4PFJetsPuppi +
    process.puppiJetCorrFactors +
    puppiInclusiveVertexingSequence +
    puppiJetsBTagSequence
)

if not options.isData:
    puppiJetSequence += process.genJetMatchPuppi

puppiJetSequence += cms.Sequence(
    process.patJetsPuppi +
    process.selectedPatJetsPuppi
)

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

### MET RE-CORRECTION
from PandaProd.Producer.utils.makeMET_cff import initMET, makeMET

initMetSequence = initMET(process, options.isData)

# extracts the raw pfMET from input MINIAOD and repack new corrections
pfMetSequence = makeMET(process, options.isData, 'packedPFCandidates', 'slimmedJets', 'AK4PFchs')
# compute a brand-new pfMET from puppi candidates and pack with corrections
puppiMetSequence = makeMET(process, options.isData, 'puppiForMET', 'selectedPatJetsPuppi', 'AK4PFPuppi', 'Puppi')

metSequence = cms.Sequence(
    initMetSequence +
    pfMetSequence +
    puppiMetSequence
)

### FAT JETS

from PandaProd.Producer.utils.makeFatJets_cff import initFatJets, makeFatJets

fatJetInitSequence = initFatJets(process, options.isData, ['AK8', 'CA15'])

ak8CHSSequence = makeFatJets(
    process,
    isData = options.isData,
    label = 'AK8PFchs'
)

ak8PuppiSequence = makeFatJets(
    process,
    isData = options.isData,
    label = 'AK8PFPuppi'
)

ca15CHSSequence = makeFatJets(
    process,
    isData = options.isData,
    label = 'CA15PFchs'
)

ca15PuppiSequence = makeFatJets(
    process,
    isData = options.isData,
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
    process.puppiSequence +
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
process.panda.isRealData = options.isData
process.panda.useTrigger = options.useTrigger
#process.panda.SelectEvents = ['reco'] # no skim
if options.isData:
    process.panda.fillers.partons.enabled = False
    process.panda.fillers.genParticles.enabled = False
    process.panda.fillers.genJets.enabled = False
if not options.useTrigger:
    process.panda.fillers.hlt.enabled = False

process.ntuples = cms.EndPath(process.panda)

process.schedule = cms.Schedule(process.reco, process.ntuples)
