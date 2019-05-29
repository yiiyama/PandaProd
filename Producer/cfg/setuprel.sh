#!/bin/bash

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

CWD=`pwd`
SRC=$CMSSW_BASE/src
INSTALL=$CMSSW_BASE/src/PandaProd/Producer/scripts/install-pkg

# EGM add EGammaPostRecoTools
# https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPostRecoRecipes
#$INSTALL cms-egamma EgammaPostRecoTools RecoEgamma/EgammaTools
$INSTALL cms-egamma EgammaPostRecoTools

# Deep double B tagger from FNAL
#$INSTALL jmduarte double-b-rebased-94x  RecoBTag/Combined RecoBTag/Configuration RecoBTag/DeepFlavour DataFormats/BTauReco PhysicsTools/PatAlgos
#$INSTALL DylanHsu double-b-rebased-94x  RecoBTag/Combined RecoBTag/Configuration RecoBTag/DeepFlavour DataFormats/BTauReco PhysicsTools/PatAlgos

# Last contribtion to PhysicsTools/PatAlgos from cms-met/METRecipe94x was on Nov 6, 2017.
# It was merged into CMSSW as commit 8a9523901e3fc5cec4c6200ed2160e0f45baca7d, so this should be safe
#cp $CMSSW_RELEASE_BASE/src/PhysicsTools/PatAlgos/plugins/* $SRC/PhysicsTools/PatAlgos/plugins/.

#cp $CMSSW_RELEASE_BASE/src/PhysicsTools/PatAlgos/interface/MuonMvaEstimator.h $SRC/PhysicsTools/PatAlgos/interface/.
#cp $CMSSW_RELEASE_BASE/src/PhysicsTools/PatAlgos/src/MuonMvaEstimator.cc $SRC/PhysicsTools/PatAlgos/src/.
#cp $CMSSW_RELEASE_BASE/src/PhysicsTools/PatAlgos/interface/SoftMuonMvaEstimator.h $SRC/PhysicsTools/PatAlgos/interface/.
#cp $CMSSW_RELEASE_BASE/src/PhysicsTools/PatAlgos/src/SoftMuonMvaEstimator.cc $SRC/PhysicsTools/PatAlgos/src/.


## clone to a temp area and move the contents to avoid including .git
#TEMP=$(mktemp -d)
#git clone https://github.com/jmduarte/RecoBTag-Combined.git $TEMP/data -b deepdoubleb_v0
#mkdir -p $SRC/RecoBTag/Combined/data
#mv $TEMP/data/DeepDoubleB $SRC/RecoBTag/Combined/data/
#rm -rf $TEMP
