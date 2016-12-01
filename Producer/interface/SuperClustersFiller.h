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

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  typedef edm::View<reco::SuperCluster> SuperClusterView;

  edm::EDGetTokenT<SuperClusterView> superClustersToken_;
  edm::EDGetTokenT<EcalRecHitCollection> ebHitsToken_;
  edm::EDGetTokenT<EcalRecHitCollection> eeHitsToken_;
};

#endif
