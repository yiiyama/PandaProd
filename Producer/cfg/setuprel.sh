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
$INSTALL cms-egamma EGM_94X_v1 EgammaAnalysis/ElectronTools

# MVA weights
cd $SRC/EgammaAnalysis/ElectronTools/data
git clone https://github.com/ECALELFS/ScalesSmearings.git
cd ScalesSmearings/
git checkout Run2017_17Nov2017_v1
cd $CWD

# fixing some POG bugs
$INSTALL cms-sw CMSSW_9_4_X PhysicsTools/PatUtils
# CHS PF candidates are never added to the sequence
sed -i '/setattr(process,"pfNoPileUpJME"+postfix,pfCHS)/a\                task.add(pfCHS)' $SRC/PhysicsTools/PatUtils/python/tools/runMETCorrectionsAndUncertainties.py
sed -i '/setattr(process,"pfNoPileUpJME"+postfix,pfCHS)/a\                patMetModuleSequence += pfCHS' $SRC/PhysicsTools/PatUtils/python/tools/runMETCorrectionsAndUncertainties.py
