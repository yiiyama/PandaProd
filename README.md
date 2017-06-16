# PandaProd
Package for production of PandA from CMSSW

Installation

 cd $CMSSW_BASE/src
 eval `scram runtime -sh`
 ./PandaProd/Producer/cfg/setuprel.sh
 scram b -j12

Run

 cmsRun PandaProd/Producer/cfg/prod.py [options]
