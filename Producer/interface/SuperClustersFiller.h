#ifndef PandaProd_Producer_SuperClustersFiller_h
#define PandaProd_Producer_SuperClustersFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"

class SuperClustersFiller : public FillerBase {
 public:
  SuperClustersFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~SuperClustersFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  typedef edm::View<reco::SuperCluster> SuperClusterView;

  NamedToken<SuperClusterView> superClustersToken_;
  NamedToken<EcalRecHitCollection> ebHitsToken_;
  NamedToken<EcalRecHitCollection> eeHitsToken_;
};

#endif
