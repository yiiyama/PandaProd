#!/bin/bash

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

$CMSSW_BASE/PandaProd/Producer/scripts/install-pkg cms-met fromCMSSW_8_0_20_postICHEPfilter RecoMET/METFilters
$CMSSW_BASE/PandaProd/Producer/scripts/install-pkg ahinzmann METRecipe_8020_Spring16 CommonTools/PileupAlgos PhysicsTools/PatAlgos PhysicsTools/PatUtils RecoMET/METAlgorithms RecoMET/METAlgorithms
