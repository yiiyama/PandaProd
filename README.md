# PandaProd
Package for production of PandA from CMSSW

Installation

    cd $CMSSW_BASE/src
    eval `scram runtime -sh`

    # Actually don't do this at the moment.
    # None of the recipes even work in CMSSW_9_2_X
    # ./PandaProd/Producer/cfg/setuprel.sh

    scram b -j12

Run

    cmsRun PandaProd/Producer/cfg/prod.py [options]
