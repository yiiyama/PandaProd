#!/bin/bash

recursive-mv() {
# mv fails if the top directory exists already
# go recursively into subdirectories to complete the move

  SRC=$1
  TARG=$2
  for F in $(ls $SRC)
  do
    mv $SRC/$F $TARG/ 2>/dev/null || recursive-mv $SRC/$F $TARG/$F
  done
}

install-pkg() {
  GITGROUP=$1
  REF=$2
  shift 2
  PKGS="$@"

  mkdir tmp
  cd tmp
  git clone -n https://github.com/$GITGROUP/cmssw.git
  cd cmssw
  git fetch origin
  git checkout $REF $PKGS

  recursive-mv $PWD $CMSSW_BASE/src

  cd ../..
  rm -rf tmp
}

# To figure out which packages a specific repository branch updates with respect to your base workspace, go to github
# and find the commit where the developper branched off. Then
#  scram p CMSSW CMSSW_{release}
#  cd CMSSW_{release}/src
#  cmsenv
#  git cms-init
#  git remote add <name> <url>
#  git fetch <name>
#  git checkout -t <name>/<branch>
#  git diff --name-only {branch-off commit}..HEAD
#
# You should delete the CMSSW area once done. git cms-init pulls in so many files that are unnecessary.

install-pkg cms-met origin/fromCMSSW_8_0_20_postICHEPfilter RecoMET/METFilters
install-pkg emanueledimarco origin/ecal_smear_fix_80X EgammaAnalysis/ElectronTools
git clone -b ICHEP2016_v2 https://github.com/ECALELFS/ScalesSmearings.git $CMSSW_BASE/src/EgammaAnalysis/ElectronTools/data/ScalesSmearings
