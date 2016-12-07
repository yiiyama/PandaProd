import FWCore.ParameterSet.Config as cms

import PandaProd.Producer.utils.egmidconf as egmidconf

panda = cms.EDAnalyzer('PandaProducer',
    isRealData = cms.untracked.bool(False),
    useTrigger = cms.untracked.bool(True),
    SelectEvents = cms.untracked.vstring(),
    fillers = cms.untracked.PSet(
        common = cms.untracked.PSet(
            triggerObjects = cms.untracked.string('selectedPatTrigger')
        ),
        chsAK4Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Jets'),
            jets = cms.untracked.string('slimmedJets::NTUPLES'),
            genJets = cms.untracked.string('ak4GenJetsNoNu::NTUPLES'),
            R = cms.untracked.double(0.4),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        puppiAK4Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Jets'),
            jets = cms.untracked.string('selectedPatJetsPuppi::NTUPLES'),
            genJets = cms.untracked.string('ak4GenJetsNoNu::NTUPLES'),
            R = cms.untracked.double(0.4),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        chsAK8Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsPFchsAK8::NTUPLES'),
            genJets = cms.untracked.string('genJetsNoNuAK8::NTUPLES'),
            R = cms.untracked.double(0.8),
            subjets = cms.untracked.string('patJetsSoftDropSubjetsPFchsAK8:NTUPLES'),
            btags = cms.untracked.string(''),
            qgl = cms.untracked.string('chsAK8SubQGTag::NTUPLES'),
            computeSubstructure = cms.untracked.bool(False),
            fillConstituents = cms.untracked.bool(False),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        puppiAK8Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsPFpuppiAK8::NTUPLES'),
            genJets = cms.untracked.string('genJetsNoNuAK8::NTUPLES'),
            R = cms.untracked.double(0.8),
            subjets = cms.untracked.string('patJetsSoftDropSubjetsPFpuppiAK8:NTUPLES'),
            qgl = cms.untracked.string('puppiAK8SubQGTag::NTUPLES'),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        chsCA15Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsPFchsCA15::NTUPLES'),
            genJets = cms.untracked.string('genJetsNoNuCA15::NTUPLES'),
            R = cms.untracked.double(1.5),
            subjets = cms.untracked.string('patJetsSoftDropSubjetsPFchsCA15:NTUPLES'),
            qgl = cms.untracked.string('chsCA15SubQGTag::NTUPLES'),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        puppiCA15Jets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('FatJets'),
            jets = cms.untracked.string('packedPatJetsPFpuppiCA15::NTUPLES'),
            genJets = cms.untracked.string('genJetsNoNuCA15::NTUPLES'),
            R = cms.untracked.double(1.5),
            subjets = cms.untracked.string('patJetsSoftDropSubjetsPFpuppiCA15:NTUPLES'),
            qgl = cms.untracked.string('puppiCA15SubQGTag::NTUPLES'),
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        electrons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Electrons'),
            electrons = cms.untracked.string('slimmedElectrons'),
            vetoId = cms.untracked.string(egmidconf.electronId + 'veto'),
            looseId = cms.untracked.string(egmidconf.electronId + 'loose'),
            mediumId = cms.untracked.string(egmidconf.electronId + 'medium'),
            tightId = cms.untracked.string(egmidconf.electronId + 'tight'),
            combIsoEA = cms.untracked.FileInPath(egmidconf.electronCombIsoEA),
            ecalIsoEA = cms.untracked.FileInPath(egmidconf.electronEcalIsoEA),
            hcalIsoEA = cms.untracked.FileInPath(egmidconf.electronHcalIsoEA),
            ecalIso = cms.untracked.string('electronEcalPFClusterIsolationProducer'),
            hcalIso = cms.untracked.string('electronHcalPFClusterIsolationProducer'),
            hltFilters = cms.untracked.vstring(
                'dummy',
                'dummy',
                'hltEG120HEFilter',
                'hltEG135HEFilter',
                'hltEG165HE10Filter',
                'hltEG175HEFilter',
                'hltEG22R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG36R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG50R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG75R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG90R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG120R9Id90HE10Iso40EBOnlyTrackIsoFilter'
            ),
            minPt = cms.untracked.double(-1.),
            maxEta = cms.untracked.double(10.)
        ),
        muons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Muons'),
            muons = cms.untracked.string('slimmedMuons'),
            hltFilters = cms.untracked.vstring(
                'hltL3crIsoL1sMu18L1f0L2f10QL3f20QL3trkIsoFiltered0p09',
                'hltL3fL1sMu18L1f0Tkf20QL3trkIsoFiltered0p09',
                'hltL3crIsoL1sMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p09',
                'hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p09'
            ),
            minPt = cms.untracked.double(-1.),
            maxEta = cms.untracked.double(10.)
        ),
        taus = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Taus'),
            taus = cms.untracked.string('slimmedTaus'),
            minPt = cms.untracked.double(-1.),
            maxEta = cms.untracked.double(10.)
        ),
        photons = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Photons'),
            photons = cms.untracked.string('slimmedPhotons'),
            looseId = cms.untracked.string(egmidconf.photonId + 'loose'),
            mediumId = cms.untracked.string(egmidconf.photonId + 'medium'),
            tightId = cms.untracked.string(egmidconf.photonId + 'tight'),
            chIso = cms.untracked.string('photonIDValueMapProducer:phoChargedIsolation'),
            nhIso = cms.untracked.string('photonIDValueMapProducer:phoNeutralHadronIsolation'),
            phIso = cms.untracked.string('photonIDValueMapProducer:phoPhotonIsolation'),
            wchIso = cms.untracked.string('photonIDValueMapProducer:phoWorstChargedIsolation'),
            chIsoEA = cms.untracked.FileInPath(egmidconf.photonEA + 'pfChargedHadrons_25ns_NULLcorrection.txt'),
            nhIsoEA = cms.untracked.FileInPath(egmidconf.photonEA + 'pfNeutralHadrons_25ns_90percentBased.txt'),
            phIsoEA = cms.untracked.FileInPath(egmidconf.photonEA + 'pfPhotons_25ns_90percentBased.txt'),
            chIsoLeakage = cms.untracked.PSet(EB = cms.untracked.string(''), EE = cms.untracked.string('')),
            nhIsoLeakage = cms.untracked.PSet(
                EB = cms.untracked.string('0.014 * x + 0.000019 * x * x'),
                EE = cms.untracked.string('0.0139 * x + 0.000025 * x * x')
            ),
            phIsoLeakage = cms.untracked.PSet(
                EB = cms.untracked.string('0.0053 * x'),
                EE = cms.untracked.string('0.0034 * x')
            ),
            ebHits = cms.untracked.string('reducedEgamma:reducedEBRecHits'),
            eeHits = cms.untracked.string('reducedEgamma:reducedEERecHits'),
            l1Filters = cms.untracked.vstring(
                'hltL1sSingleEG34IorSingleEG40',
                'hltL1sSingleEG40IorSingleJet200',
                'hltL1sSingleEG34IorSingleEG40IorSingleJet200',
                'hltL1sSingleEG24',
                'hltL1sSingleEG30',
                'hltL1sSingleEG40'
            ),
            hltFilters = cms.untracked.vstring(
                'hltEG120HEFilter',
                'hltEG135HEFilter',
                'hltEG165HE10Filter',
                'hltEG175HEFilter',
                'hltEG22R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG36R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG50R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG75R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG90R9Id90HE10Iso40EBOnlyTrackIsoFilter',
                'hltEG120R9Id90HE10Iso40EBOnlyTrackIsoFilter'
            )
        ),
        pfCandidates = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('PFCands'),
            candidates = cms.untracked.string('packedPFCandidates')
        ),
        genParticles = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenParticles'),
            genParticles = cms.untracked.string('prunedGenParticles'),
            pdgIds = cms.untracked.vstring('6', '11', '13', '15', '21-25', '1000000-'),
            minPt = cms.untracked.double(-1.)
        ),
        genJets = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('GenJets'),
            genJets = cms.untracked.string('slimmedGenJets'),
            minPt = cms.untracked.double(-1.)
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
        met = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Met'),
            met = cms.untracked.string('slimmedMETs::NTUPLES'),
            noHFMet = cms.untracked.string('slimmedMETsNoHF::NTUPLES')
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
            generalFilters = cms.untracked.string('TriggerResults::RECO'),
            badTrack = cms.untracked.string('BadChargedCandidateSummer16Filter'),
            badMuonTrack = cms.untracked.string('BadPFMuonSummer16Filter')
        ),
        hlt = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('HLT'),
            triggerResults = cms.untracked.string('TriggerResults::HLT')
        ),
        weights = cms.untracked.PSet(
            enabled = cms.untracked.bool(True),
            filler = cms.untracked.string('Weights'),
            genEventInfo = cms.untracked.string('generator'),
            lheEvent = cms.untracked.string('externalLHEProducer'),
            lheRun = cms.untracked.string('externalLHEProducer')
        )
    )
)
