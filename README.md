# PandaProd
Package for production of PandA from CMSSW

Installation

```bash
    cd $CMSSW_BASE/src
    eval `scram runtime -sh`
    
    git clone https://github.com/PandaPhysics/PandaTree
    git clone https://github.com/PandaPhysics/PandaProd

    # Don't do the following if you want to use 92x, but it is necessary for 80x:
    ./PandaProd/Producer/cfg/setuprel.sh

    scram b -j12
```

Run

    cmsRun PandaProd/Producer/cfg/prod.py [options]
