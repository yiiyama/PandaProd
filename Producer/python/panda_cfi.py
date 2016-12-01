import FWCore.ParameterSet.Config as cms

photonId = 'egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-'
photonEA = 'RecoEgamma/PhotonIdentification/data/Spring15/effAreaPhotons_cone03_'
electronId = 'egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-'

pandaProducer = cms.EDAnalyzer('PandaProducer',
    isRealData = cms.untracked.bool(False),
    useTrigger = cms.untracked.bool(True),
    photons = cms.untracked.string('slimmedPhotons'),
    electrons = cms.untracked.string('slimmedElectrons'),
    muons = cms.untracked.string('slimmedMuons'),
    taus = cms.untracked.string('slimmedTaus'),
    jets = cms.untracked.string('slimmedJets'),
    t1met = cms.untracked.string('slimmedMETs'),
    vertices = cms.untracked.string('offlineSlimmedPrimaryVertices'),
    superClusters = cms.untracked.string('reducedEgamma:reducedSuperClusters'),
    ebHits = cms.untracked.string('reducedEgamma:reducedEBRecHits'),
    eeHits = cms.untracked.string('reducedEgamma:reducedEERecHits'),
    rho = cms.untracked.string('fixedGridRhoFastjetAll'),
    triggerObjects = cms.untracked.string('selectedPatTrigger'),
    triggerResults = cms.untracked.string('TriggerResults::HLT'),
    fillers = cms.untracked.PSet(
        Jets = cms.untracked.PSet(
            minPt = cms.untracked.double(15.),
            maxEta = cms.untracked.double(4.7)
        ),
        Photons = cms.untracked.PSet(
            looseId = cms.untracked.string(photonId + 'loose'),
            mediumId = cms.untracked.string(photonId + 'medium'),
            tightId = cms.untracked.string(photonId + 'tight'),
            chIso = cms.untracked.string('photonIDValueMapProducer:phoChargedIsolation'),
            nhIso = cms.untracked.string('photonIDValueMapProducer:phoNeutralHadronIsolation'),
            phIso = cms.untracked.string('photonIDValueMapProducer:phoPhotonIsolation'),
            wchIso = cms.untracked.string('photonIDValueMapProducer:phoWorstChargedIsolation'),
            chIsoEA = cms.untracked.FileInPath(photonEA + 'pfChargedHadrons_25ns_NULLcorrection.txt'),
            nhIsoEA = cms.untracked.FileInPath(photonEA + 'pfNeutralHadrons_25ns_90percentBased.txt'),
            phIsoEA = cms.untracked.FileInPath(photonEA + 'pfPhotons_25ns_90percentBased.txt'),
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
        Electrons = cms.untracked.PSet(
            minPt = cms.untracked.double(-1.),
            maxEta = cms.untracked.double(10.),
            vetoId = cms.untracked.string(electronId + 'veto'),
            looseId = cms.untracked.string(electronId + 'loose'),
            mediumId = cms.untracked.string(electronId + 'medium'),
            tightId = cms.untracked.string(electronId + 'tight'),
            combIsoEA = cms.untracked.FileInPath('RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_25ns.txt'),
            ecalIsoEA = cms.untracked.FileInPath('PandaProd/Producer/data/effAreaElectrons_HLT_ecalPFClusterIso.txt'),
            hcalIsoEA = cms.untracked.FileInPath('PandaProd/Producer/data/effAreaElectrons_HLT_hcalPFClusterIso.txt'),
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
            )
        ),
        Muons = cms.untracked.PSet(
            hltFilters = cms.untracked.vstring(
                'hltL3crIsoL1sMu18L1f0L2f10QL3f20QL3trkIsoFiltered0p09',
                'hltL3fL1sMu18L1f0Tkf20QL3trkIsoFiltered0p09',
                'hltL3crIsoL1sMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p09',
                'hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p09'
            )
        ),
        MetFilters = cms.untracked.PSet(
            generalFilters = cms.untracked.string('TriggerResults::RECO'),
            badTrack = cms.untracked.string('BadChargedCandidateSummer16Filter'),
            badMuonTrack = cms.untracked.string('BadPFMuonSummer16Filter')
        )
    )
)
