#!/bin/bash

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

INSTALL=$CMSSW_BASE/src/PandaProd/Producer/scripts/install-pkg

$INSTALL cms-met fromCMSSW_8_0_20_postICHEPfilter RecoMET/METFilters
$INSTALL ahinzmann METRecipe_8020_Spring16 CommonTools/PileupAlgos PhysicsTools/PatAlgos PhysicsTools/PatUtils RecoMET/METAlgorithms RecoMET/METAlgorithms
