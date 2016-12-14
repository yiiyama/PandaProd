#!/bin/bash

recursive-mv() {
# mv fails if the top directory exists already
# go recursively into subdirectories to complete the move

  SRC=$1
  TARG=$2
  
  mv $SRC $TARG/ 2>/dev/null && return

  for F in $(ls $SRC)
  do
    recursive-mv $SRC/$F $TARG/$(basename $SRC)
  done
}

install-pkg() {
  ACCOUNT=$1
  REF=$2
  shift 2
  PKGS="$@"

  mkdir tmp
  cd tmp
  git clone -n https://github.com/$ACCOUNT/cmssw.git
  cd cmssw
  git fetch origin
  if git branch -r | grep origin/$REF
  then
    # this is a branch name
    git checkout -t origin/$REF
  else
    # this is a commit
    git checkout $REF 2>/dev/null
  fi

  for PKG in $PKGS
  do
    recursive-mv $PWD/$PKG $CMSSW_BASE/src
  done

  cd ../..
#  rm -rf tmp
}

# Use PandaProd/Producer/scripts/cms-git-diff to find out which packages should be checked out from each branch.

install-pkg cms-met fromCMSSW_8_0_20_postICHEPfilter RecoMET/METFilters
install-pkg ahinzmann METRecipe_8020_Spring16 CommonTools/PileupAlgos PhysicsTools/PatAlgos PhysicsTools/PatUtils RecoMET/METAlgorithms RecoMET/METAlgorithms
