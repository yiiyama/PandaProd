#!/bin/bash

CWD=`pwd`

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

INSTALL=$CMSSW_BASE/src/PandaProd/Producer/scripts/install-pkg

# electron and photon ID
$INSTALL lsoffi CMSSW_9_4_0_pre3_TnP RecoEgamma/ElectronIdentification RecoEgamma/PhotonIdentification
# MVA weights for ID
git clone -b CMSSW_9_4_0_pre3_TnP https://github.com/lsoffi/RecoEgamma-ElectronIdentification.git electronid
cp -r electronid/* $CMSSW_BASE/src/RecoEgamma/ElectronIdentification/data/
rm -rf electronid
git clone -b CMSSW_9_4_0_pre3_TnP https://github.com/lsoffi/RecoEgamma-PhotonIdentification.git photonid
cp -r photonid/* $CMSSW_BASE/src/RecoEgamma/PhotonIdentification/data/
rm -rf photonid

# electron and photon calibration
$INSTALL cms-egamma EGM_94X_v1 EgammaAnalysis/ElectronTools
# MVA weights
cd $CMSSW_BASE/src/EgammaAnalysis/ElectronTools/data
git clone https://github.com/ECALELFS/ScalesSmearings.git
cd ScalesSmearings/
git checkout Run2017_17Nov2017_v1
cd $CWD

# fixing some POG bugs
$INSTALL cms-sw CMSSW_9_4_X PhysicsTools/PatUtils
# simple indentation error?
sed -i 's/    \( *addToProcessAndTask(cleanedPFCandCollection.*\)/\1/' ${CMSSW_BASE}/src/PhysicsTools/PatUtils/python/tools/muonRecoMitigation.py
# CHS PF candidates are never added to the sequence
sed -i '/setattr(process,"pfNoPileUpJME"+postfix,pfCHS)/a\                task.add(pfCHS)' ${CMSSW_BASE}/src/PhysicsTools/PatUtils/python/tools/runMETCorrectionsAndUncertainties.py
sed -i '/setattr(process,"pfNoPileUpJME"+postfix,pfCHS)/a\                patMetModuleSequence += pfCHS' ${CMSSW_BASE}/src/PhysicsTools/PatUtils/python/tools/runMETCorrectionsAndUncertainties.py
