P#!/bin/bash

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

INSTALL=$CMSSW_BASE/src/PandaProd/Producer/scripts/install-pkg

$INSTALL cms-met METRecipe_8020 PhysicsTools/PatAlgos PhysicsTools/PatUtils RecoMET/METAlgorithms #CommonTools/PileupAlgos
# Loosen the dR matching between GS-fixed and nonfixed photons
$INSTALL cms-met METRecipe_80X_part2 PhysicsTools/PatUtils RecoMET/METProducers
# Check out the METFilters in the release and patch it
$INSTALL cms-sw CMSSW_8_0_X RecoMET/METFilters
sed -i 's/badGlobalMuonTagger\.clone/badGlobalMuonTaggerMAOD.clone/' $CMSSW_BASE/src/RecoMET/METFilters/python/badGlobalMuonTaggersMiniAOD_cff.py
# Should have asked cms-met to pull this branch but it kind of got lost
# Contains a hacked version of PuppiPhoton that can run puppi on muon-filtered PF
$INSTALL yiiyama puppiphoton_filteredpf CommonTools/PileupAlgos
$INSTALL ikrav egm_id_80X_v3_photons PhysicsTools/SelectorUtils RecoEgamma/PhotonIdentification RecoEgamma/EgammaIsolationAlgos
