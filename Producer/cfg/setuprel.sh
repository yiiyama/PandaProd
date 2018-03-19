#!/bin/bash

CWD=`pwd`
SRC=$CMSSW_BASE/src

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

#INSTALL=$CMSSW_BASE/src/PandaProd/Producer/scripts/install-pkg
INSTALL=/home/yiiyama/cms/cmssw/dev/CMSSW_9_4_4/src/PandaProd/Producer/scripts/install-pkg

# cut-based electron and photon ID
$INSTALL lsoffi CMSSW_9_4_0_pre3_TnP RecoEgamma/ElectronIdentification RecoEgamma/PhotonIdentification
# MVA electron ID
# RecoEgamma/ElectronIdentification overlaps with the line above but most of the files added above are not contained in this branch and thus will not be overwritten
# Using -x exclusion list to avoid overwriting lsoffi - so dumb!
$INSTALL guitargeek ElectronID_MVA2017_940pre3 RecoEgamma/EgammaTools RecoEgamma/ElectronIdentification -x RecoEgamma/ElectronIdentification/python/Identification/cutBasedElectronID_tools.py

# MVA weights for ID
git clone -b CMSSW_9_4_0_pre3_TnP https://github.com/lsoffi/RecoEgamma-ElectronIdentification.git electronid
cp -r electronid/* $SRC/RecoEgamma/ElectronIdentification/data/
rm -rf electronid
git clone -b CMSSW_9_4_0_pre3_TnP https://github.com/lsoffi/RecoEgamma-PhotonIdentification.git photonid
cp -r photonid/* $SRC/RecoEgamma/PhotonIdentification/data/
rm -rf photonid

# electron and photon calibration
# POG-recommended recipe for 94X is
#$INSTALL cms-egamma EGM_94X_v1 EgammaAnalysis/ElectronTools
# But we want to be able to run on 80X input as well. So we check out the branch for 80X and make it 94X-compilable
# This hack is not validated yet - we need to compare smearing results made with straight 80X and straight 94X with this hybrid
install-pkg cms-egamma EGM_gain_v1 EgammaAnalysis/ElectronTools
grep -r numberOfHits $SRC/EgammaAnalysis/ElectronTools | awk '{print $1}' | sed 's/\(.*\):/\1/' | while read path; do sed -i 's/numberOfHits/numberOfAllHits/' $path; done
sed -i '/#include <TFile.h>/a #include <iostream>' $SRC/EgammaAnalysis/ElectronTools/plugins/GBRForestGetterFromDB.cc
grep -r auto_ptr $SRC/EgammaAnalysis/ElectronTools | awk '{print $1}' | sed 's/\(.*\):/\1/' | while read path; do sed -i -e 's/auto_ptr/unique_ptr/' -e 's/\.put(\([^,)]*\)\(.*\)/.put(std::move(\1)\2/' $path; done

# MVA weights
cd $SRC/EgammaAnalysis/ElectronTools/data
git clone https://github.com/ECALELFS/ScalesSmearings.git
cd ScalesSmearings/
git checkout Run2017_17Nov2017_v1
cd $CWD

# fixing some POG bugs
$INSTALL cms-sw CMSSW_9_4_X PhysicsTools/PatUtils
# simple indentation error?
sed -i 's/    \( *addToProcessAndTask(cleanedPFCandCollection.*\)/\1/' $SRC/PhysicsTools/PatUtils/python/tools/muonRecoMitigation.py
# CHS PF candidates are never added to the sequence
sed -i '/setattr(process,"pfNoPileUpJME"+postfix,pfCHS)/a\                task.add(pfCHS)' $SRC/PhysicsTools/PatUtils/python/tools/runMETCorrectionsAndUncertainties.py
sed -i '/setattr(process,"pfNoPileUpJME"+postfix,pfCHS)/a\                patMetModuleSequence += pfCHS' $SRC/PhysicsTools/PatUtils/python/tools/runMETCorrectionsAndUncertainties.py
