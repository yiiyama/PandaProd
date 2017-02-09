#!/bin/bash

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

INSTALL=$CMSSW_BASE/src/PandaProd/Producer/scripts/install-pkg

$INSTALL cms-met METRecipe_8020 RecoMET/METFilters CommonTools/PileupAlgos PhysicsTools/PatAlgos PhysicsTools/PatUtils RecoMET/METAlgorithms
$INSTALL ikrav egm_id_80X_v3_photons PhysicsTools/SelectorUtils RecoEgamma/PhotonIdentification
