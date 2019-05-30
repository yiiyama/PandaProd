import FWCore.ParameterSet.Config as cms

from PhysicsTools.JetMCAlgos.HadronAndPartonSelector_cfi import selectedHadronsAndPartons
from PhysicsTools.JetMCAlgos.GenHFHadronMatcher_cff import matchGenBHadron
from PhysicsTools.JetMCAlgos.GenHFHadronMatcher_cff import matchGenCHadron
from PhysicsTools.JetMCAlgos.AK4PFJetsMCFlavourInfos_cfi import ak4JetFlavourInfos

def setupGenJetFlavor(process, do_merge):
    process.load('PhysicsTools.JetMCAlgos.HadronAndPartonSelector_cfi')

    # Input particle collection for matching to gen jets (partons + leptons) 
    # MUST use use proper input jet collection: the jets to which hadrons should be associated
    # rParam and jetAlgorithm MUST match those used for jets to be associated with hadrons
    # More details on the tool: https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideBTagMCTools#New_jet_flavour_definition

    process.selectedHadronsAndPartons.particles = 'mergedGenParticles' if do_merge else 'prunedGenParticles'

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
    
    # Begin GenHFHadronMatcher subsequences
    # Adapted from PhysicsTools/JetMCAlgos/test/matchGenHFHadrons.py
    # Supplies PDG ID to real name resolution of MC particles
    process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
    
    process.ak4MatchGenBHadron = matchGenBHadron.clone(
        genParticles = process.selectedHadronsAndPartons.particles,
        jetFlavourInfos = "ak4GenJetFlavourInfos"
    )
    process.ak4MatchGenCHadron = matchGenCHadron.clone(
        genParticles = process.selectedHadronsAndPartons.particles,
        jetFlavourInfos = "ak4GenJetFlavourInfos"
    )
    process.ak8MatchGenBHadron = matchGenBHadron.clone(
        genParticles = process.selectedHadronsAndPartons.particles,
        jetFlavourInfos = "ak8GenJetFlavourInfos"
    )
    process.ak8MatchGenCHadron = matchGenCHadron.clone(
        genParticles = process.selectedHadronsAndPartons.particles,
        jetFlavourInfos = "ak8GenJetFlavourInfos"
    )
    process.ca15MatchGenBHadron = matchGenBHadron.clone(
        genParticles = process.selectedHadronsAndPartons.particles,
        jetFlavourInfos = "ca15GenJetFlavourInfos"
    )
    process.ca15MatchGenCHadron = matchGenCHadron.clone(
        genParticles = process.selectedHadronsAndPartons.particles,
        jetFlavourInfos = "ca15GenJetFlavourInfos"
    )
    #End GenHFHadronMatcher subsequences

    genJetFlavorSequence = cms.Sequence(
        process.selectedHadronsAndPartons +
        process.ak4GenJetFlavourInfos +
        process.ak8GenJetFlavourInfos +
        process.ca15GenJetFlavourInfos + 
        process.ak4MatchGenBHadron +
        process.ak4MatchGenCHadron +
        process.ak8MatchGenBHadron +
        process.ak8MatchGenCHadron +
        process.ca15MatchGenBHadron +
        process.ca15MatchGenCHadron
    )

    return genJetFlavorSequence
