#-----------------------FAT JET CLUSTERING-----------------------

import FWCore.ParameterSet.Config as cms
from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets
from RecoJets.JetProducers.ak4PFJetsPuppi_cfi import ak4PFJetsPuppi
from RecoJets.JetProducers.ak4GenJets_cfi import ak4GenJets
from RecoJets.JetProducers.nJettinessAdder_cfi import Njettiness
from RecoBTag.Configuration.RecoBTag_cff import *
from RecoJets.JetProducers.QGTagger_cfi import *
from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection

def initFatJets(process, isData):
    ########################################
    ##         INITIAL SETUP              ##
    ########################################
  
    ## Load standard PAT objects
    process.load("PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff")
    process.load("PhysicsTools.PatAlgos.selectionLayer1.selectedPatCandidates_cff")
  
    sequence = cms.Sequence()

    if not isData and not hasattr(process, 'packedGenParticlesForJetsNoNu'):
        process.packedGenParticlesForJetsNoNu = cms.EDFilter("CandPtrSelector",
            src = cms.InputTag("packedGenParticles"),
            cut = cms.string("abs(pdgId) != 12 && abs(pdgId) != 14 && abs(pdgId) != 16")
        )

        sequence += proces.packedGenParticlesForJetsNoNu

    process.pfCHS = cms.EDFilter("CandPtrSelector",
        src = cms.InputTag("packedPFCandidates"),
        cut = cms.string("fromPV")
    )

    sequence += process.pfCHS
  
    return sequence

def makeFatJets(process, isData, pfCandidates, algoLabel, jetRadius):
    postfix = 'PFlow'
    if pfCandidates == 'particleFlow':
        # mini aod needs a different config
        pfCandidates = 'pfCHS'
    
    if pfCandidates == 'pfCHS':
        puMethod = 'chs'
    else:
        puMethod = 'puppi'
  
    rLabel = algoLabel + str(int(jetRadius * 10.))
    customLabel = puMethod + rLabel
  
    if algoLabel == 'CA':
        jetAlgo = 'CambridgeAachen'
    else:
        jetAlgo = 'AntiKt'
  
    if jetRadius < 1.:
        sdZcut = 0.1
        sdBeta = 0.0
    else:
        sdZcut = 0.15
        sdBeta = 1.0
   
    ## Various collection names
    genParticles = 'prunedGenParticles'
    pfSource = 'packedPFCandidates'
    pvSource = 'offlineSlimmedPrimaryVertices'
    svSource = 'slimmedSecondaryVertices'
    muSource = 'slimmedMuons'
    elSource = 'slimmedElectrons'
    bTagInfos = [
        'pfImpactParameterTagInfos',
        'pfSecondaryVertexTagInfos',
        'pfInclusiveSecondaryVertexFinderTagInfos',
        'softPFMuonsTagInfos',
        'softPFElectronsTagInfos'
    ]
    ## b-tag discriminators
    bTagDiscriminators = [
        'pfCombinedSecondaryVertexV2BJetTags',
        'pfCombinedInclusiveSecondaryVertexV2BJetTags'
    ]
  
    bTagInfosSubjets = [
        'pfImpactParameterTagInfos',
        'pfSecondaryVertexTagInfos',
        'pfInclusiveSecondaryVertexFinderTagInfos',
        'softPFMuonsTagInfos',
        'softPFElectronsTagInfos'
    ]
    ## b-tag discriminators
    bTagDiscriminatorsSubjets = [
        'pfCombinedSecondaryVertexV2BJetTags',
        'pfCombinedInclusiveSecondaryVertexV2BJetTags'
    ]
  
    ## Output names
    pfJetsName = 'PFJets' + customLabel
    pfJetsSoftDropName = 'PFJetsSoftDrop' + customLabel
    genJetsNoNuName = 'genJetsNoNu' + rLabel
    genJetsNoNuSoftDropName = 'genJetsNoNuSoftDrop' + rLabel
    njettinessName = customLabel + 'Njettiness'
    sdKinematicsName = customLabel + 'SDKinematics'
    subQGTagName = customLabel + 'SubQGTag'
    subIPTagInfosName = customLabel + 'PFImpactParameterTagInfos'
    subISVFTagInfosName = customLabel + 'PFInclusiveSecondaryVertexFinderTagInfos'
    subISVTagsName = customLabel + 'PFCombinedInclusiveSecondaryVertexV2BJetTags'

    ## Output sequence
    newSeq = cms.Sequence()
  
    ########################################
    ##           REMAKE JETS              ##
    ########################################

    pfJets = ak4PFJets.clone(
        jetAlgorithm = cms.string(jetAlgo),
        rParam = cms.double(jetRadius),
        src = cms.InputTag(pfCandidates),
        jetPtMin = cms.double(180.)
    )
    setattr(process, pfJetsName, pfJets)
    newSeq += pfJets

    pfJetsSoftDrop = pfJets.clone(
        useSoftDrop = cms.bool(True),
        R0 = cms.double(jetRadius),
        zcut = cms.double(sdZcut),
        beta = cms.double(sdBeta),
        writeCompound = cms.bool(True),
        useExplicitGhosts = cms.bool(True),
        jetCollInstanceName=cms.string("SubJets"),
        jetPtMin = cms.double(180.)
    )
    setattr(process, pfJetsSoftDropName, pfJetsSoftDrop)
    newSeq += pfJetsSoftDrop
  
    if not isData:
        if not hasattr(process, genJetsNoNuName):
            addingGenJets = True
            genJetsNoNu = ak4GenJets.clone(
                jetAlgorithm = cms.string(jetAlgo),
                rParam = cms.double(jetRadius),
                src = cms.InputTag('packedGenParticlesForJetsNoNu')
            )
            setattr(process, genJetsNoNuName, genJetsNoNu)
            newSeq += genJetsNoNu
        else:
            genJetsNoNu = getattr(process, genJetsNoNuName)

        if not hasattr(process, genJetsNoNuSoftDropName):
            genJetsNoNuSoftDrop = genJetsNoNu.clone(
                R0 = cms.double(jetRadius),
                useSoftDrop = cms.bool(True),
                zcut = cms.double(sdZcut),
                beta = cms.double(sdBeta),
                writeCompound = cms.bool(True),
                useExplicitGhosts = cms.bool(True),
                jetCollInstanceName=cms.string("SubJets")
            )
            setattr(process, genJetsNoNuSoftDropName, genJetsNoNuSoftDrop)
            newSeq += genJetsNoNuSoftDrop
        else:
            genJetsNoNuSoftDrop = getattr(process, genJetsNoNuSoftDropName)

    ########################################
    ##           SUBSTRUCTURE             ##
    #######################################
  
    njettiness = Njettiness.clone(                                      
        src = cms.InputTag('PFJets' + customLabel),
        R0 = cms.double(jetRadius),
        Njets = cms.vuint32(1,2,3,4)
    )
    setattr(process, njettinessName, njettiness)
    newSeq += njettiness
  
    sdKinematics = cms.EDProducer('RecoJetDeltaRValueMapProducer',
        src = cms.InputTag(pfJetsName),
        matched = cms.InputTag(pfJetsSoftDropName),
        distMax = cms.double(1.5),
        values = cms.vstring('mass'),
        valueLabels = cms.vstring('Mass')
    )
    setattr(process, sdKinematicsName, sdKinematics)
    newSeq += sdKinematics
  
    ### subjet qg-tagging ###

    subQGTag = QGTagger.clone(
        srcJets = cms.InputTag(pfJetsSoftDropName, 'SubJets'),
        jetsLabel = cms.string('QGL_AK4PFchs')
    )
  
    setattr(process, subQGTagName, subQGTag)
    newSeq += subQGTag
  
    ### subjet b-tagging ###

    subIPTagInfos = pfImpactParameterTagInfos.clone(
        jets = cms.InputTag(pfJetsSoftDropName, 'SubJets'),
        maxDeltaR = cms.double(0.4),
        primaryVertex = cms.InputTag(pvSource),
        candidates = cms.InputTag(pfSource)
    )
    setattr(process, subIPTagInfosName, subIPTagInfos)
    newSeq += subIPTagInfos

    subISVFTagInfos = pfInclusiveSecondaryVertexFinderTagInfos.clone(
        trackIPTagInfos = cms.InputTag(subIPTagInfosName),
        extSVCollection = cms.InputTag(svSource)
    )
    setattr(process, subISVFTagInfosName, subISVFTagInfos)
    newSeq += subISVFTagInfos

    subISVTags = pfCombinedInclusiveSecondaryVertexV2BJetTags.clone(
        tagInfos = cms.VInputTag( 
            cms.InputTag(customLabel+"PFImpactParameterTagInfos"), 
            cms.InputTag(customLabel+"PFInclusiveSecondaryVertexFinderTagInfos") 
        )
    )
    setattr(process, subISVTagsName, subISVTags)
    newSeq += subISVTags
    
    bTagDiscriminators = ['None']
  
    ########################################
    ##          MAKE PAT JETS             ##
    ########################################
  
    ## MAIN JET ##
    addJetCollection(
        process,
        labelName = 'PF' + customLabel,
        jetSource = cms.InputTag(pfJetsName),
        algo = algoLabel, # needed for jet flavor clustering
        rParam = jetRadius, # needed for jet flavor clustering
        pfCandidates = cms.InputTag(pfSource),
        pvSource = cms.InputTag(pvSource),
        svSource = cms.InputTag(svSource),
        muSource = cms.InputTag(muSource),
        elSource = cms.InputTag(elSource),
        btagInfos = ['None'],
        btagDiscriminators = bTagDiscriminators,
        genJetCollection = cms.InputTag(genJetsNoNuName),
        genParticles = cms.InputTag(genParticles),
        getJetMCFlavour = False # jet flavor disabled
    )
    patJetsName = 'patJetsPF' + customLabel
    selectedPatJetsName = 'selectedPatJetsPF' + customLabel

    patJets = getattr(process, patJetsName)
    selectedPatJets = getattr(process, selectedPatJetsName)

    for tau in ['tau1', 'tau2', 'tau3', 'tau4']:
        patJets.userData.userFloats.src.append(njettinessName + ':' + tau)
    patJets.userData.userFloats.src.append(sdKinematicsName + ':Mass')
    if patJets.addBTagInfo:
        patJets.addTagInfos = True

    selectedPatJets.cut = 'abs(eta) < 2.5'

    if not isData:
        newSeq += getattr(process, 'patJetPartonMatchPF' + customLabel)
        newSeq += getattr(process, 'patJetGenJetMatchPF' + customLabel)

    newSeq += patJets
    newSeq += selectedPatJets

    ## SOFT DROP ##
    addJetCollection(
        process,
        labelName = 'SoftDropPF' + customLabel,
        jetSource = cms.InputTag(pfJetsSoftDropName),
        pfCandidates = cms.InputTag(pfSource),
        algo = algoLabel,
        rParam = jetRadius,
        btagInfos = ['None'],
        btagDiscriminators = ['None'],
        genJetCollection = cms.InputTag(genJetsNoNuName),
        genParticles = cms.InputTag(genParticles),
        getJetMCFlavour = False # jet flavor disabled
    )
    patJetsSoftDropName = 'patJetsSoftDropPF' + customLabel
    selectedPatJetsSoftDropName = 'selectedPatJetsSoftDropPF' + customLabel

    if not isData:
        newSeq += getattr(process, 'patJetPartonMatchSoftDropPF' + customLabel)
        newSeq += getattr(process, 'patJetGenJetMatchSoftDropPF' + customLabel)

    newSeq += getattr(process, patJetsSoftDropName)
    newSeq += getattr(process, selectedPatJetsSoftDropName)

    addJetCollection(
        process,
        labelName = 'SoftDropSubjetsPF' + customLabel,
        jetSource=cms.InputTag(pfJetsSoftDropName, 'SubJets'),
        algo = algoLabel,
        rParam = jetRadius,
        pfCandidates = cms.InputTag(pfSource),
        pvSource = cms.InputTag(pvSource),
        svSource = cms.InputTag(svSource),
        muSource = cms.InputTag(muSource),
        elSource = cms.InputTag(elSource),
        btagInfos = bTagInfosSubjets,
        btagDiscriminators = bTagDiscriminatorsSubjets,
        genJetCollection = cms.InputTag(genJetsNoNuSoftDropName, 'SubJets'),
        genParticles = cms.InputTag(genParticles),
        explicitJTA = True, # needed for subjet b tagging
        svClustering = True, # needed for subjet b tagging
        fatJets = cms.InputTag(pfJetsName), # needed for subjet flavor clustering
        groomedFatJets = cms.InputTag(pfJetsSoftDropName), # needed for subjet flavor clustering
        runIVF = False,
        getJetMCFlavour = False # jet flavor disabled
    )
    patSubjetsName = 'patJetsSoftDropSubjetsPF' + customLabel
    selectedPatSubjetsName = 'selectedPatJetsSoftDropSubjetsPF' + customLabel

    patSubjets = getattr(process, patSubjetsName)
    if patSubjets.addBTagInfo:
        patSubjets.addTagInfos = True
    
    if not isData:
      newSeq += getattr(process, 'patJetPartonMatchSoftDropSubjetsPF' + customLabel)
      newSeq += getattr(process, 'patJetGenJetMatchSoftDropSubjetsPF' + customLabel)

    newSeq += patSubjets
    newSeq += getattr(process, selectedPatSubjetsName)

    jetMerger = cms.EDProducer("BoostedJetMerger",    
        jetSrc = cms.InputTag(selectedPatJetsSoftDropName),
        subjetSrc = cms.InputTag(selectedPatSubjetsName)
    )
    jetMergerName = "selectedPatJetsSoftDropPFPacked" + customLabel
    setattr(process, jetMergerName, jetMerger)
    newSeq += jetMerger

    ## PACK ##
    substructurePacker = cms.EDProducer("JetSubstructurePacker",
        jetSrc = cms.InputTag('selectedPatJetsPF'+customLabel),
        distMax = cms.double(jetRadius),
        algoTags = cms.VInputTag(
            cms.InputTag(jetMergerName)
        ),
        algoLabels = cms.vstring('SoftDrop'),
        fixDaughters = cms.bool(False)
    )
    setattr(process, "packedPatJetsPF"+customLabel, substructurePacker)
    newSeq += substructurePacker

    return newSeq
