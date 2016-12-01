#ifndef PandaProd_Producer_TausFiller_h
#define PandaProd_Producer_TausFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/TauReco/interface/BaseTau.h"

class TausFiller : public FillerBase {
 public:
  TausFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~TausFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  typedef edm::View<reco::BaseTau> TauView;

  edm::EDGetTokenT<TauView> tausToken_;

  double minPt_;
  double maxEta_;
};

#endif
