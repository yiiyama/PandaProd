#ifndef SCMIPHaloTagger_h
#define SCMIPHaloTagger_h

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

class SCMIPHaloTagger {
public:
  SCMIPHaloTagger(){}

  virtual ~SCMIPHaloTagger(){}

  void setup(const edm::ParameterSet& conf);

  void fillMIPVariables(const reco::SuperCluster*, 
                        EcalRecHitCollection const&,
                        reco::Photon::MIPVariables& mipId);

protected:
  //get the seed crystal index
  void getSeedHighestE(const reco::SuperCluster*,
                       EcalRecHitCollection const&,
                       int &seedIEta,
                       int &seedIPhi,
                       double &seedE);

  //Isolation parameters variables as input
  double yRangeFit_;
  double xRangeFit_;
  double residualWidthEnergy_;
  double haloDiscThreshold_;
};

#endif
