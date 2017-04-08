import FWCore.ParameterSet.Config as cms

import PandaProd.Producer.utils.egmidconf as egmidconf

panda = cms.EDAnalyzer('PandaProducer',
    isRealData = cms.untracked.bool(False),
    outputFile = cms.untracked.string('panda.root'),
    useTrigger = cms.untracked.bool(True),
    SelectEvents = cms.untracked.vstring(),
    printLevel = cms.untracked.uint32(0),
    fillers = cms.untracked.PSet(
        common = cms.untracked.PSet(
            triggerObjects = cms.untracked.string('selectedPatTrigger'),
            genEventInfo = cms.untracked.string('generator'),
            lheEvent = cms.untracked.string('externalLHEProducer'),
            lheRun = cms.untracked.string('externalLHEProducer'),
            finalStateParticles = cms.untracked.string('packedGenParticles'),
            genParticles = cms.untracked.string('prunedGenParticles'),
            pfCandidates = cms.untracked.string('packedPFCandidates'),
            ebHits = cms.untracked.string('reducedEgamma:reducedEBRecHits'),
            eeHits = cms.untracked.string('reducedEgamma:reducedEERecHits')
        ),
        chsAK4Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Jets'),
            jets = cms.untracked.string('slimmedJets'),
            genJets = cms.untracked.string('slimmedGenJets'),
            pandaGenJets = cms.untracked.string('ak4GenJets'),
            jec = cms.untracked.string('AK4PFchs'),
            jer = cms.untracked.string('AK4PFchs'),
            csv = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            puid = cms.untracked.string('pileupJetId:fullDiscriminant'),
            qgl = cms.untracked.string('QGTagger:qgLikelihood'),
            R = cms.untracked.double(0.4),
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
            R = cms.untracked.double(0.4),
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
            doubleBTag = cms.untracked.string('pfBoostedDoubleSVTagInfosAK8PFchs'),
            njettiness = cms.untracked.string('NjettinessAK8PFchs'),
            sdKinematics = cms.untracked.string('sdKinematicsAK8PFchs'),
            subjetBtag = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            subjetQGL = cms.untracked.string('subQGTagAK8PFchs:qgLikelihood'),
            doubleBTagWeights = cms.untracked.FileInPath('PandaProd/Utilities/data/BoostedSVDoubleCA15_withSubjet_v4.weights.xml'),
            computeSubstructure = cms.untracked.string('recoil'),
            recoil = cms.untracked.string('MonoXFilter:categories'),
            fillConstituents = cms.untracked.bool(False),
            minPt = cms.untracked.double(180.),
            maxEta = cms.untracked.double(4.7)
        ),
        puppiAK8Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsAK8PFPuppi'),
            genJets = cms.untracked.string('genJetsNoNuAK8'),
            pandaGenJets = cms.untracked.string('ak8GenJets'),
            jec = cms.untracked.string('AK8PFPuppi'),
            je = cms.untracked.string(''),
            R = cms.untracked.double(0.8),
            subjets = cms.untracked.string('patSubjetsAK8PFPuppi'),
            doubleBTag = cms.untracked.string('pfBoostedDoubleSVTagInfosAK8PFPuppi'),
            njettiness = cms.untracked.string('NjettinessAK8PFPuppi'),
            sdKinematics = cms.untracked.string('sdKinematicsAK8PFPuppi'),
            subjetBtag = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            subjetQGL = cms.untracked.string('subQGTagAK8PFPuppi:qgLikelihood'),
            doubleBTagWeights = cms.untracked.FileInPath('PandaProd/Utilities/data/BoostedSVDoubleCA15_withSubjet_v4.weights.xml'),
            computeSubstructure = cms.untracked.string('recoil'),
            recoil = cms.untracked.string('MonoXFilter:categories'),
            fillConstituents = cms.untracked.bool(False),
            minPt = cms.untracked.double(180.),
            maxEta = cms.untracked.double(4.7)
        ),
        chsCA15Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsCA15PFchs'),
            genJets = cms.untracked.string('genJetsNoNuCA15'),
            pandaGenJets = cms.untracked.string('ca15GenJets'),
            jec = cms.untracked.string('AK8PFchs'),
            jer = cms.untracked.string(''),
            R = cms.untracked.double(1.5),
            subjets = cms.untracked.string('patSubjetsCA15PFchs'),
            doubleBTag = cms.untracked.string('pfBoostedDoubleSVTagInfosCA15PFchs'),
            njettiness = cms.untracked.string('NjettinessCA15PFchs'),
            sdKinematics = cms.untracked.string('sdKinematicsCA15PFchs'),
            subjetBtag = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            subjetQGL = cms.untracked.string('subQGTagCA15PFchs:qgLikelihood'),
            doubleBTagWeights = cms.untracked.FileInPath('PandaProd/Utilities/data/BoostedSVDoubleCA15_withSubjet_v4.weights.xml'),
            computeSubstructure = cms.untracked.string('recoil'),
            recoil = cms.untracked.string('MonoXFilter:categories'),
            fillConstituents = cms.untracked.bool(False),
            minPt = cms.untracked.double(100.),
            maxEta = cms.untracked.double(4.7)
        ),
        puppiCA15Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsCA15PFPuppi'),
            genJets = cms.untracked.string('genJetsNoNuCA15'),
            pandaGenJets = cms.untracked.string('ca15GenJets'),
            jec = cms.untracked.string('AK8PFPuppi'),
            jer = cms.untracked.string(''),
            R = cms.untracked.double(1.5),
            subjets = cms.untracked.string('patSubjetsCA15PFPuppi'),
            doubleBTag = cms.untracked.string('pfBoostedDoubleSVTagInfosCA15PFPuppi'),
            njettiness = cms.untracked.string('NjettinessCA15PFPuppi'),
            sdKinematics = cms.untracked.string('sdKinematicsCA15PFPuppi'),
            subjetBtag = cms.untracked.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'),
            subjetQGL = cms.untracked.string('subQGTagCA15PFPuppi:qgLikelihood'),
            doubleBTagWeights = cms.untracked.FileInPath('PandaProd/Utilities/data/BoostedSVDoubleCA15_withSubjet_v4.weights.xml'),
            computeSubstructure = cms.untracked.string('recoil'),
            recoil = cms.untracked.string('MonoXFilter:categories'),
            fillConstituents = cms.untracked.bool(False),
            minPt = cms.untracked.double(100.),
            maxEta = cms.untracked.double(4.7)
        ),
        electrons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Electrons'),
            electrons = cms.untracked.string('slimmedElectrons'),
            rawElectrons = cms.untracked.string('slimmedElectrons::@skipCurrentProcess'),
            regressionElectrons = cms.untracked.string('correctedElectrons'),
            vetoId = cms.untracked.string(egmidconf.electronVetoId),
            looseId = cms.untracked.string(egmidconf.electronLooseId),
            mediumId = cms.untracked.string(egmidconf.electronMediumId),
            tightId = cms.untracked.string(egmidconf.electronTightId),
            hltId = cms.untracked.string(egmidconf.electronHLTId),
            combIsoEA = cms.untracked.FileInPath(egmidconf.electronCombIsoEA),
            ecalIsoEA = cms.untracked.FileInPath(egmidconf.electronEcalIsoEA),
            hcalIsoEA = cms.untracked.FileInPath(egmidconf.electronHcalIsoEA),
            triggerObjects = cms.untracked.PSet(
                El23El12FirstLeg = cms.untracked.vstring('hltEle23Ele12CaloIdLTrackIdLIsoVLHcalIsoLeg1Filter'),
                El23El12SecondLeg = cms.untracked.vstring('hltEle23Ele12CaloIdLTrackIdLIsoVLTrackIsoLeg2Filter'),
                El25Tight = cms.untracked.vstring('hltEle25WPTightGsfTrackIsoFilter'),
                El27Loose = cms.untracked.vstring('hltEle27noerWPLooseGsfTrackIsoFilter'),
                El27Tight = cms.untracked.vstring('hltEle27WPTightGsfTrackIsoFilter'),
                El120Ph = cms.untracked.vstring('hltEG120HEFilter'),
                El135Ph = cms.untracked.vstring('hltEG135HEFilter'),
                El165HE10Ph = cms.untracked.vstring('hltEG165HE10Filter'),
                El175Ph = cms.untracked.vstring('hltEG175HEFilter'),
                El22EBR9IsoPh = cms.untracked.vstring('hltEG22R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                El36EBR9IsoPh = cms.untracked.vstring('hltEG36R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                El50EBR9IsoPh = cms.untracked.vstring('hltEG50R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                El75EBR9IsoPh = cms.untracked.vstring('hltEG75R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                El90EBR9IsoPh = cms.untracked.vstring('hltEG90R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                El120EBR9IsoPh = cms.untracked.vstring('hltEG120R9Id90HE10Iso40EBOnlyTrackIsoFilter')
            ),
            minPt = cms.untracked.double(5.),
            maxEta = cms.untracked.double(10.)
        ),
        muons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Muons'),
            muons = cms.untracked.string('slimmedMuons'),
            triggerObjects = cms.untracked.PSet(
                Mu17Mu8FirstLeg = cms.untracked.vstring('hltL3fL1sDoubleMu114L1f0L2f10L3Filtered17'),
                Mu17Mu8SecondLeg = cms.untracked.vstring('hltDiMuonGlb17Trk8RelTrkIsoFiltered0p4'),
                IsoMu22er = cms.untracked.vstring('hltL3crIsoL1sSingleMu20erL1f0L2f10QL3f22QL3trkIsoFiltered0p09'),
                IsoTkMu22er = cms.untracked.vstring('hltL3fL1sMu20erL1f0Tkf22QL3trkIsoFiltered0p09'),
                IsoMu24 = cms.untracked.vstring('hltL3crIsoL1sMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p09'),
                IsoTkMu24 = cms.untracked.vstring('hltL3fL1sMu22L1f0Tkf24QL3trkIsoFiltered0p09')
            ),
            minPt = cms.untracked.double(5.),
            maxEta = cms.untracked.double(10.)
        ),
        taus = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Taus'),
            taus = cms.untracked.string('slimmedTaus'),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(10.)
        ),
        photons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Photons'),
            photons = cms.untracked.string('slimmedPhotons'),
            rawPhotons = cms.untracked.string('slimmedPhotons::@skipCurrentProcess'),
            regressionPhotons = cms.untracked.string('correctedPhotons'),
            looseId = cms.untracked.string(egmidconf.photonLooseId),
            mediumId = cms.untracked.string(egmidconf.photonMediumId),
            tightId = cms.untracked.string(egmidconf.photonTightId),
            chIso = cms.untracked.string('photonIDValueMapProducer:phoChargedIsolation'),
            nhIso = cms.untracked.string('photonIDValueMapProducer:phoNeutralHadronIsolation'),
            phIso = cms.untracked.string('photonIDValueMapProducer:phoPhotonIsolation'),
            chIsoMax = cms.untracked.string('worstIsolationProducer'),
            chIsoEA = cms.untracked.FileInPath(egmidconf.photonCHIsoEA),
            nhIsoEA = cms.untracked.FileInPath(egmidconf.photonNHIsoEA),
            phIsoEA = cms.untracked.FileInPath(egmidconf.photonPhIsoEA),
            chIsoLeakage = cms.untracked.PSet(EB = cms.untracked.string(''), EE = cms.untracked.string('')),
            nhIsoLeakage = cms.untracked.PSet(
                EB = cms.untracked.string('0.014 * x + 0.000019 * x * x'),
                EE = cms.untracked.string('0.0139 * x + 0.000025 * x * x')
            ),
            phIsoLeakage = cms.untracked.PSet(
                EB = cms.untracked.string('0.0053 * x'),
                EE = cms.untracked.string('0.0034 * x')
            ),
            triggerObjects = cms.untracked.PSet(
                SEG34IorSEG40 = cms.untracked.vstring('hltL1sSingleEG34IorSingleEG40'),
                SEG40IorSJet200 = cms.untracked.vstring('hltL1sSingleEG40IorSingleJet200'),
                SEG34IorSEG40IorSJet200 = cms.untracked.vstring('hltL1sSingleEG34IorSingleEG40IorSingleJet200'),
                SEG24 = cms.untracked.vstring('hltL1sSingleEG24'),
                SEG30 = cms.untracked.vstring('hltL1sSingleEG30'),
                SEG40 = cms.untracked.vstring('hltL1sSingleEG40'),
                Ph120 = cms.untracked.vstring('hltEG120HEFilter'),
                Ph135 = cms.untracked.vstring('hltEG135HEFilter'),
                Ph165HE10 = cms.untracked.vstring('hltEG165HE10Filter'),
                Ph175 = cms.untracked.vstring('hltEG175HEFilter'),
                Ph22EBR9Iso = cms.untracked.vstring('hltEG22R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                Ph36EBR9Iso = cms.untracked.vstring('hltEG36R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                Ph50EBR9Iso = cms.untracked.vstring('hltEG50R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                Ph75EBR9Iso = cms.untracked.vstring('hltEG75R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                Ph90EBR9Iso = cms.untracked.vstring('hltEG90R9Id90HE10Iso40EBOnlyTrackIsoFilter'),
                Ph120EBR9Iso = cms.untracked.vstring('hltEG120R9Id90HE10Iso40EBOnlyTrackIsoFilter')
            ),
            minPt = cms.untracked.double(10.)
        ),
        pfCandidates = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('PFCands'),
            puppi = cms.untracked.string('puppi'),
            puppiNoLep = cms.untracked.string('puppiForMET')
        ),
        partons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Partons')
        ),
        genParticles = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenParticles'),
            pdgIds = cms.untracked.vstring('-6', '11-16', '21-39', '1000000-'),
            minPt = cms.untracked.double(-1.)
        ),
        ak4GenJets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenJets'),
            genJets = cms.untracked.string('slimmedGenJets'),
            flavor = cms.untracked.string('ak4GenJetFlavourInfos'),
            minPt = cms.untracked.double(15.)
        ),
        ak8GenJets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenJets'),
            genJets = cms.untracked.string('genJetsNoNuAK8'),
            flavor = cms.untracked.string('ak8GenJetFlavourInfos'),
            minPt = cms.untracked.double(150.)
        ),
        ca15GenJets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenJets'),
            genJets = cms.untracked.string('genJetsNoNuCA15'),
            flavor = cms.untracked.string('ca15GenJetFlavourInfos'),
            minPt = cms.untracked.double(100.)
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
            vertices = cms.untracked.string('offlineSlimmedPrimaryVertices'),
            puSummaries = cms.untracked.string('slimmedAddPileupInfo')
        ),
        pfMet = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Met'),
            met = cms.untracked.string('slimmedMETs'),
            noHFMet = cms.untracked.string('slimmedMETsNoHF'),
            fillOthers = cms.untracked.bool(True)
        ),
        puppiMet = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Met'),
            met = cms.untracked.string('slimmedMETsPuppi')
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
            signalWeights = cms.untracked.bool(True)
        ),
        recoil = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Recoil'),
            categories = cms.untracked.string('MonoXFilter:categories'),
            max = cms.untracked.string('MonoXFilter:max')
        )
    )
)
