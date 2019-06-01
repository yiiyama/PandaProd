#!/bin/bash

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

CWD=`pwd`
SRC=$CMSSW_BASE/src
INSTALL=$CMSSW_BASE/src/PandaProd/Producer/scripts/install-pkg

# EGM: Add EGammaPostRecoTools
# https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPostRecoRecipes
$INSTALL cms-egamma EgammaPostRecoTools

# EGM: 2018 preliminary energy corrections
# https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaPostRecoRecipes
# optional but speeds up the photon ID value module so things fun faster
$INSTALL cms-egamma PhotonIDValueMapSpeedup1029
# fixes the Run2018D dictionary issue, see https://github.com/cms-sw/cmssw/issues/26182
$INSTALL cms-egamma slava77-btvDictFix_10210
# check out the package otherwise code accessing it will crash
$INSTALL cms-sw $CMSSW_VERSION EgammaAnalysis/ElectronTools
# delete the data directory so and populate it from cms-egamma latest
rm -rf $SRC/EgammaAnalysis/ElectronTools/data
git clone git@github.com:cms-egamma/EgammaAnalysis-ElectronTools.git $SRC/EgammaAnalysis/ElectronTools/data
cd $SRC/EgammaAnalysis/ElectronTools/data
git checkout ScalesSmearing2018_Dev
cd -
$INSTALL cms-egamma EgammaPostRecoTools_dev

# JME: PUPPI v12 tune
$INSTALL ahinzmann puppi-v12-102X
