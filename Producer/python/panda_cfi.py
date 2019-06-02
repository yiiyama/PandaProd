import FWCore.ParameterSet.Config as cms

panda = cms.EDAnalyzer('PandaProducer',
    isRealData = cms.untracked.bool(False),
    outputFile = cms.untracked.string('panda.root'),
    useTrigger = cms.untracked.bool(True),
    SelectEvents = cms.untracked.vstring(),
    printLevel = cms.untracked.uint32(0),
    fillers = cms.untracked.PSet(
        common = cms.untracked.PSet(
            genEventInfo = cms.untracked.string('generator'),
            finalStateParticles = cms.untracked.string('packedGenParticles'),
            genParticles = cms.untracked.string('prunedGenParticles'),
#            genParticles = cms.untracked.string('mergedGenParticles'),
            pfCandidates = cms.untracked.string('packedPFCandidates'),
            vertices = cms.untracked.string('offlineSlimmedPrimaryVertices'),
            beamSpot = cms.untracked.string('offlineBeamSpot'),
            ebHits = cms.untracked.string('reducedEgamma:reducedEBRecHits'),
            eeHits = cms.untracked.string('reducedEgamma:reducedEERecHits'),
            conversions = cms.untracked.string('reducedEgamma:reducedConversions')
        ),
        chsAK4Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Jets'),
            jets = cms.untracked.string('slimmedJets'),
            pileupJets = cms.untracked.string('slimmedJets'),
            genJets = cms.untracked.string('slimmedGenJets'),
            pandaGenJets = cms.untracked.string('ak4GenJets'),
            jec = cms.untracked.string('AK4PFchs'),
            jer = cms.untracked.string('AK4PFchs'),
            csv = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            cmva = cms.untracked.string('pfCombinedMVAV2BJetTags'),
            deepCSV = cms.untracked.string('pfDeepCSVJetTags'),
            deepCMVA = cms.untracked.string('pfDeepCMVAJetTags'),
            puid = cms.untracked.string('pileupJetId:fullDiscriminant'),
            qgl = cms.untracked.string('QGTagger:qgLikelihood'),
            R = cms.untracked.double(0.4),
            fillConstituents = cms.untracked.bool(True),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        puppiAK4Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Jets'),
            jets = cms.untracked.string('slimmedJetsPuppi'),
            genJets = cms.untracked.string('slimmedGenJets'),
            pandaGenJets = cms.untracked.string('ak4GenJets'),
            jec = cms.untracked.string('AK4PFPuppi'),
            jer = cms.untracked.string(''),
            csv = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            cmva = cms.untracked.string('pfCombinedMVAV2BJetTags'),
            deepCSV = cms.untracked.string('pfDeepCSVJetTags'),
            deepCMVA = cms.untracked.string('pfDeepCMVAJetTags'),
            R = cms.untracked.double(0.4),
            fillConstituents = cms.untracked.bool(True),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        chsAK8Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsAK8PFchs'),
            genJets = cms.untracked.string('genJetsNoNuAK8'),
            pandaGenJets = cms.untracked.string('ak8GenJets'),
            jec = cms.untracked.string('AK8PFchs'),
            jer = cms.untracked.string('AK8PFchs'),
            R = cms.untracked.double(0.8),
            subjets = cms.untracked.string('patSubjetsAK8PFchs'),
            shallowBBTag = cms.untracked.string('pfBoostedDoubleSVBJetTags'),
            deepBBprobQTag = cms.untracked.string('pfDeepDoubleBJetTags:probQ'),
            deepBBprobHTag = cms.untracked.string('pfDeepDoubleBJetTags:probH'),
            njettiness = cms.untracked.string('Njettiness'),
            sdKinematics = cms.untracked.string('sdKinematics'),
            prunedKinematics = cms.untracked.string('prunedKinematics'),
            subjetBtag = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            subjetQGL = cms.untracked.string('subQGTag:qgLikelihood'),
            subjetCmva = cms.untracked.string('pfCombinedMVAV2BJetTags'),
            subjetDeepCSV = cms.untracked.string('pfDeepCSVJetTags'),
            subjetDeepCMVA = cms.untracked.string('pfDeepCMVAJetTags'),
            computeSubstructure = cms.untracked.string('never'),
            recoil = cms.untracked.string('MonoXFilter:categories'),
            fillConstituents = cms.untracked.bool(True),
            minPt = cms.untracked.double(180.),
            maxEta = cms.untracked.double(4.7)
        ),
        puppiAK8Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsAK8PFPuppi'),
            genJets = cms.untracked.string('genJetsNoNuAK8'),
            pandaGenJets = cms.untracked.string('ak8GenJets'),
            constituents = cms.untracked.string('puppi'),
            jec = cms.untracked.string('AK8PFPuppi'),
            je = cms.untracked.string(''),
            R = cms.untracked.double(0.8),
            subjets = cms.untracked.string('patSubjetsAK8PFPuppi'),
            shallowBBTag = cms.untracked.string('pfBoostedDoubleSVBJetTags'),
            deepBBprobQTag = cms.untracked.string('pfDeepDoubleBJetTags:probQ'),
            deepBBprobHTag = cms.untracked.string('pfDeepDoubleBJetTags:probH'),
            njettiness = cms.untracked.string('Njettiness'),
            sdKinematics = cms.untracked.string('sdKinematics'),
            prunedKinematics = cms.untracked.string('prunedKinematics'),
            subjetBtag = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            subjetQGL = cms.untracked.string('subQGTag:qgLikelihood'),
            subjetCmva = cms.untracked.string('pfCombinedMVAV2BJetTags'),
            subjetDeepCSV = cms.untracked.string('pfDeepCSVJetTags'),
            subjetDeepCMVA = cms.untracked.string('pfDeepCMVAJetTags'),
            computeSubstructure = cms.untracked.string('always'),
            recoil = cms.untracked.string('MonoXFilter:categories'),
            fillConstituents = cms.untracked.bool(True),
            minPt = cms.untracked.double(180.),
            maxEta = cms.untracked.double(4.7)
        ),
        puppiCA15Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsCA15PFPuppi'),
            genJets = cms.untracked.string('genJetsNoNuCA15'),
            pandaGenJets = cms.untracked.string('ca15GenJets'),
            constituents = cms.untracked.string('puppi'),
            jec = cms.untracked.string('AK8PFPuppi'),
            jer = cms.untracked.string(''),
            R = cms.untracked.double(1.5),
            subjets = cms.untracked.string('patSubjetsCA15PFPuppi'),
            shallowBBTag = cms.untracked.string('pfBoostedDoubleSVBJetTags'),
            deepBBprobQTag = cms.untracked.string('pfDeepDoubleBJetTags:probQ'),
            deepBBprobHTag = cms.untracked.string('pfDeepDoubleBJetTags:probH'),
            njettiness = cms.untracked.string('Njettiness'),
            sdKinematics = cms.untracked.string('sdKinematics'),
            prunedKinematics = cms.untracked.string('prunedKinematics'),
            subjetBtag = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            subjetQGL = cms.untracked.string('subQGTag:qgLikelihood'),
            subjetCmva = cms.untracked.string('pfCombinedMVAV2BJetTags'),
            subjetDeepCSV = cms.untracked.string('pfDeepCSVJetTags'),
            subjetDeepCMVA = cms.untracked.string('pfDeepCMVAJetTags'),
            computeSubstructure = cms.untracked.string('recoil'),
            recoil = cms.untracked.string('MonoXFilter:categories'),
            fillConstituents = cms.untracked.bool(True),
            minPt = cms.untracked.double(180.),
            maxEta = cms.untracked.double(4.7)
        ),
        electrons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Electrons'),
            electrons = cms.untracked.string('slimmedElectrons'),
            smearedElectrons = cms.untracked.string('smearedElectrons'),
            vetoId = cms.untracked.string(''),
            looseId = cms.untracked.string(''),
            mediumId = cms.untracked.string(''),
            tightId = cms.untracked.string(''),
            hltId = cms.untracked.string(''),
            mvaWP90 = cms.untracked.string(''),
            mvaWP80 = cms.untracked.string(''),
            mvaWPLoose = cms.untracked.string(''),
            mvaIsoWP90 = cms.untracked.string(''),
            mvaIsoWP80 = cms.untracked.string(''),
            mvaIsoWPLoose = cms.untracked.string(''),
            mvaValuesMap     = cms.untracked.string(''),
            mvaCategoriesMap = cms.untracked.string(''),
            combIsoEA = cms.untracked.string(''),
            ecalIsoEA = cms.untracked.string(''),
            hcalIsoEA = cms.untracked.string('')
        ),
        muons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Muons'),
            muons = cms.untracked.string('slimmedMuons'),
            rochesterCorrectionSource = cms.untracked.string('')
        ),
        taus = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Taus'),
            taus = cms.untracked.string('slimmedTaus')
        ),
        photons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Photons'),
            photons = cms.untracked.string('slimmedPhotons'),
            smearedPhotons = cms.untracked.string('smearedPhotons'),
            looseId = cms.untracked.string(''),
            mediumId = cms.untracked.string(''),
            tightId = cms.untracked.string(''),
            chIso = cms.untracked.string('photonIDValueMapProducer:phoChargedIsolation'),
            nhIso = cms.untracked.string('photonIDValueMapProducer:phoNeutralHadronIsolation'),
            phIso = cms.untracked.string('photonIDValueMapProducer:phoPhotonIsolation'),
            chIsoMax = cms.untracked.string('worstIsolationProducer'),
            chIsoEA = cms.untracked.string(''),
            nhIsoEA = cms.untracked.string(''),
            phIsoEA = cms.untracked.string(''),
            chIsoLeakage = cms.untracked.PSet(EB = cms.untracked.string(''), EE = cms.untracked.string('')),
            nhIsoLeakage = cms.untracked.PSet(
                EB = cms.untracked.string('0.0148 * x + 0.000017 * x * x'),
                EE = cms.untracked.string('0.0163 * x + 0.000014 * x * x')
            ),
            phIsoLeakage = cms.untracked.PSet(
                EB = cms.untracked.string('0.0047 * x'),
                EE = cms.untracked.string('0.0034 * x')
            ),
            doPulseFit = cms.untracked.bool(False),
            ebDigis = cms.untracked.string('selectDigi:selectedEcalEBDigiCollection'),
            eeDigis = cms.untracked.string('selectDigi:selectedEcalEEDigiCollection')

        ),
        pfCandidates = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('PFCands'),
            puppiMap = cms.untracked.string('puppi'),
            puppiInput = cms.untracked.string('packedPFCandidates'),
            useExistingWeights = cms.untracked.bool(True)
        ),
        partons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Partons')
        ),
        genParticles = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenParticles'),
            prune = cms.untracked.bool(False), # Attempted further pruning during the development of version 003, but we could not come up with an effective algorithm.
            outputMode = cms.untracked.uint32(0) # 0 -> fill normal genParticles (packed), 1 -> fill unpacked genParticlesU, 2 -> fill both
        ),
        ak4GenJets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenJets'),
            genJets = cms.untracked.string('slimmedGenJets'),
            flavor = cms.untracked.string('ak4GenJetFlavourInfos'),
            genBHadPlusMothers = cms.untracked.string('ak4MatchGenBHadron:genBHadPlusMothers'),
            genBHadIndex = cms.untracked.string('ak4MatchGenBHadron:genBHadIndex'),
            genBHadJetIndex = cms.untracked.string('ak4MatchGenBHadron:genBHadJetIndex'),
            genCHadPlusMothers = cms.untracked.string('ak4MatchGenCHadron:genCHadPlusMothers'),
            genCHadIndex = cms.untracked.string('ak4MatchGenCHadron:genCHadIndex'),
            genCHadJetIndex = cms.untracked.string('ak4MatchGenCHadron:genCHadJetIndex'),
            minPt = cms.untracked.double(15.),
        ),
        ak8GenJets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenJets'),
            genJets = cms.untracked.string('genJetsNoNuAK8'),
            flavor = cms.untracked.string('ak8GenJetFlavourInfos'),
            genBHadPlusMothers = cms.untracked.string('ak8MatchGenBHadron:genBHadPlusMothers'),
            genBHadIndex = cms.untracked.string('ak8MatchGenBHadron:genBHadIndex'),
            genBHadJetIndex = cms.untracked.string('ak8MatchGenBHadron:genBHadJetIndex'),
            genCHadPlusMothers = cms.untracked.string('ak8MatchGenCHadron:genCHadPlusMothers'),
            genCHadIndex = cms.untracked.string('ak8MatchGenCHadron:genCHadIndex'),
            genCHadJetIndex = cms.untracked.string('ak8MatchGenCHadron:genCHadJetIndex'),
            minPt = cms.untracked.double(150.),
        ),
        ca15GenJets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenJets'),
            genJets = cms.untracked.string('genJetsNoNuCA15'),
            flavor = cms.untracked.string('ca15GenJetFlavourInfos'),
            genBHadPlusMothers = cms.untracked.string('ca15MatchGenBHadron:genBHadPlusMothers'),
            genBHadIndex = cms.untracked.string('ca15MatchGenBHadron:genBHadIndex'),
            genBHadJetIndex = cms.untracked.string('ca15MatchGenBHadron:genBHadJetIndex'),
            genCHadPlusMothers = cms.untracked.string('ca15MatchGenCHadron:genCHadPlusMothers'),
            genCHadIndex = cms.untracked.string('ca15MatchGenCHadron:genCHadIndex'),
            genCHadJetIndex = cms.untracked.string('ca15MatchGenCHadron:genCHadJetIndex'),
            minPt = cms.untracked.double(100.),
        ),
        superClusters = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('SuperClusters'),
            superClusters = cms.untracked.string('reducedEgamma:reducedSuperClusters'),
            ebHits = cms.untracked.string('reducedEgamma:reducedEBRecHits'),
            eeHits = cms.untracked.string('reducedEgamma:reducedEERecHits')
        ),
        vertices = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Vertices'),
            puSummaries = cms.untracked.string('slimmedAddPileupInfo')
        ),
        pfMet = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Met'),
            met = cms.untracked.string('slimmedMETs')
        ),
        puppiMet = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Met'),
            met = cms.untracked.string('slimmedMETsPuppi')
        ),
        extraMets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('MetExtra'),
            patMet = cms.untracked.string('slimmedMETs'),
            noHFMet = cms.untracked.string('slimmedMETsNoHF'),
            types = cms.untracked.vstring('raw', 'calo', 'noMu', 'noHF', 'trk', 'neutral', 'photon', 'hf')
        ),
        rho = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Rho'),
            rho = cms.untracked.string('fixedGridRhoFastjetAll'),
            rhoCentralCalo = cms.untracked.string('fixedGridRhoFastjetCentralCalo')
        ),
        metFilters = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('MetFilters'),
            filterProcesses = cms.untracked.vstring('', 'PAT', 'RECO')
        ),
        hlt = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('HLT'),
            triggerResults = cms.untracked.string('TriggerResults::HLT')
        ),
        weights = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Weights'),
            pdfType = cms.untracked.string('')
        ),
        recoil = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Recoil'),
            categories = cms.untracked.string('MonoXFilter:categories'),
            max = cms.untracked.string('MonoXFilter:max')
        ),
        secondaryVertices = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            forceFront = cms.untracked.bool(True),
            filler = cms.untracked.string('SecondaryVertices'),
            source = cms.untracked.string('slimmedSecondaryVertices')
        )
    )
)
