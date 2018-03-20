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
            doubleBTag = cms.untracked.string('pfBoostedDoubleSVBJetTags'),
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
            doubleBTag = cms.untracked.string('pfBoostedDoubleSVBJetTags'),
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
            doubleBTag = cms.untracked.string('pfBoostedDoubleSVBJetTags'),
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
            hcalIsoEA = cms.untracked.string(''),
            triggerObjects = cms.untracked.PSet(
                El23El12FirstLeg = cms.untracked.vstring('hltEle23Ele12CaloIdLTrackIdLIsoVLTrackIsoLeg1Filter'),
                El23El12SecondLeg = cms.untracked.vstring('hltEle23Ele12CaloIdLTrackIdLIsoVLTrackIsoLeg2Filter'),
                El25Tight = cms.untracked.vstring('hltEle25WPTightGsfTrackIsoFilter'), # not in 2017
                El27Loose = cms.untracked.vstring('hltEle27erWPLooseGsfTrackIsoFilter', 'hltEle27noerWPLooseGsfTrackIsoFilter'), # not in 2017
                El27Tight = cms.untracked.vstring('hltEle27WPTightGsfTrackIsoFilter'),
                El35Tight = cms.untracked.vstring('hltEle35noerWPTightGsfTrackIsoFilter'), # not in 2016
                Ph165HE10 = cms.untracked.vstring('hltEG165HE10Filter'), # not in 2017
                Ph175 = cms.untracked.vstring('hltEG175HEFilter'),
                Ph200 = cms.untracked.vstring('hltEG200HEFilter'), # not in 2016
                Ph36EBR9Iso = cms.untracked.vstring('hltEG36R9Id90HE10Iso40EBOnlyTrackIsoFilter')
            )
        ),
        muons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Muons'),
            muons = cms.untracked.string('slimmedMuons'),
            triggerObjects = cms.untracked.PSet(
                Mu17Mu8FirstLeg = cms.untracked.vstring('hltL3fL1sDoubleMu114L1f0L2f10OneMuL3Filtered17', 'hltL3fL1sDoubleMu114L1f0L2f10L3Filtered17', 'hltL3fL1DoubleMu155fFiltered17'), # last one 2017
                Mu17Mu8SecondLeg = cms.untracked.vstring('hltL3pfL1sDoubleMu114ORDoubleMu125L1f0L2pf0L3PreFiltered8', 'hltL3pfL1sDoubleMu114L1f0L2pf0L3PreFiltered8', 'hltDiMuonGlb17Trk8RelTrkIsoFiltered0p4', 'hltDiMuon178RelTrkIsoFiltered0p4'), # last one 2017
                IsoMu22er = cms.untracked.vstring('hltL3crIsoL1sSingleMu20erL1f0L2f10QL3f22QL3trkIsoFiltered0p09'), # not in 2017
                IsoTkMu22er = cms.untracked.vstring('hltL3fL1sMu20erL1f0Tkf22QL3trkIsoFiltered0p09'),  # not in 2017
                IsoMu24 = cms.untracked.vstring('hltL3crIsoL1sMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p09', 'hltL3crIsoL1sSingleMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p07'), # last one 2017
                IsoTkMu24 = cms.untracked.vstring('hltL3fL1sMu22L1f0Tkf24QL3trkIsoFiltered0p09'), # not in 2017
                IsoMu27 = cms.untracked.vstring('hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p09', 'hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p07'), # last one 2017
                IsoTkMu27 = cms.untracked.vstring('hltL3fL1sMu22Or25L1f0Tkf27QL3trkIsoFiltered0p09'), # not in 2017
                Mu50 = cms.untracked.vstring('hltL3fL1sMu22Or25L1f0L2f10QL3Filtered50Q')
            )
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
            triggerObjects = cms.untracked.PSet(
                Ph165HE10Seed = cms.untracked.vstring('hltL1sSingleEG34IorSingleEG40IorSingleJet200', 'hltL1sSingleEGNonIsoOrWithJetAndTau'), # not in 2017
                Ph175Seed = cms.untracked.vstring('hltL1sSingleEG40IorSingleJet200', 'hltL1sSingleEGNonIsoOrWithJetAndTau', 'hltL1sSingleEGNonIsoOrWithJetAndTau'), # last one 2017
                Ph200Seed = cms.untracked.vstring('hltL1sSingleEGNonIsoOrWithJetAndTau'), # not in 2016
                Ph135 = cms.untracked.vstring('hltEG135HEFilter'), # not in 2017
                Ph165HE10 = cms.untracked.vstring('hltEG165HE10Filter'), # not in 2017
                Ph175 = cms.untracked.vstring('hltEG175HEFilter'),
                Ph200 = cms.untracked.vstring('hltEG200HEFilter'), # not in 2016
                Ph22EBR9Iso = cms.untracked.vstring('hltEG22R9Id90HE10Iso40EBOnlyTrackIsoFilter'), # not in 2017
                Ph36EBR9Iso = cms.untracked.vstring('hltEG36R9Id90HE10Iso40EBOnlyTrackIsoFilter'), # not in 2017
                Ph50EBR9Iso = cms.untracked.vstring('hltEG50R9Id90HE10Iso40EBOnlyTrackIsoFilter'), # not in 2017
                Ph75EBR9Iso = cms.untracked.vstring('hltEG75R9Id90HE10Iso40EBOnlyTrackIsoFilter'), # not in 2017
                Ph90EBR9Iso = cms.untracked.vstring('hltEG90R9Id90HE10Iso40EBOnlyTrackIsoFilter'), # not in 2017
                Ph120EBR9Iso = cms.untracked.vstring('hltEG120R9Id90HE10Iso40EBOnlyTrackIsoFilter') # not in 2017
            )
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
