#ifndef PandaProd_Producer_TausFiller_h
#define PandaProd_Producer_TausFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/TauReco/interface/BaseTau.h"

class TausFiller : public FillerBase {
 public:
  TausFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~TausFiller() {}

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  typedef edm::View<reco::BaseTau> TauView;

  NamedToken<TauView> tausToken_;

  double minPt_;
  double maxEta_;
};

#endif
