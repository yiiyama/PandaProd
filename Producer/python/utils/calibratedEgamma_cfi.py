# Copied from
# https://github.com/shervin86/cmssw/blob/Moriond2017_JEC_energyScales/EgammaAnalysis/ElectronTools/python/calibratedElectronsRun2_cfi.py
# https://github.com/shervin86/cmssw/blob/Moriond2017_JEC_energyScales/EgammaAnalysis/ElectronTools/python/calibratedPhotonsRun2_cfi.py

import FWCore.ParameterSet.Config as cms
import PandaProd.Producer.utils.egmidconf as egmidconf

calibratedElectrons = cms.EDProducer("CalibratedElectronProducerRun2",

                                     # input collections
                                     electrons = cms.InputTag('gedGsfElectrons'),
                                     gbrForestName = cms.string("gedelectron_p4combination_25ns"),

                                     # data or MC corrections
                                     # if isMC is false, data corrections are applied
                                     isMC = cms.bool(False),
    
                                     # set to True to get special "fake" smearing for synchronization. Use JUST in case of synchronization
                                     isSynchronization = cms.bool(False),
                                     
                                     correctionFile = cms.string(egmidconf.electronSmearingData['Moriond2017_JEC'])
                                     )


calibratedPatElectrons = cms.EDProducer("CalibratedPatElectronProducerRun2",
                                        
                                        # input collections
                                        electrons = cms.InputTag('slimmedElectrons'),
                                        gbrForestName = cms.string("gedelectron_p4combination_25ns"),
                                        
                                        # data or MC corrections
                                        # if isMC is false, data corrections are applied
                                        isMC = cms.bool(False),
                                        
                                        # set to True to get special "fake" smearing for synchronization. Use JUST in case of synchronization
                                        isSynchronization = cms.bool(False),

                                        correctionFile = cms.string(egmidconf.electronSmearingData['Moriond2017_JEC'])
                                       )

calibratedPatPhotons = cms.EDProducer("CalibratedPatPhotonProducerRun2",

                                      # input collections
                                      photons = cms.InputTag('slimmedPhotons'),
                                      
                                      # data or MC corrections
                                      # if isMC is false, data corrections are applied
                                      isMC = cms.bool(False),
                                      
                                      # set to True to get special "fake" smearing for synchronization. Use JUST in case of synchronization
                                      isSynchronization = cms.bool(False),

                                      correctionFile = cms.string(egmidconf.photonSmearingData['Moriond2017_JEC'])
                                      )

calibratedPhotons = cms.EDProducer("CalibratedPhotonProducerRun2",

                                   # input collections
                                   photons = cms.InputTag('photons'),
                                   
                                   # data or MC corrections
                                   # if isMC is false, data corrections are applied
                                   isMC = cms.bool(False),
                                   
                                   # set to True to get special "fake" smearing for synchronization. Use JUST in case of synchronization
                                   isSynchronization = cms.bool(False),

                                   correctionFile = cms.string(egmidconf.photonSmearingData['Moriond2017_JEC'])
                                   )


