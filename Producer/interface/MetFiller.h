#ifndef PandaProd_Producer_MetFiller_h
#define PandaProd_Producer_MetFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/METReco/interface/MET.h"

#include <functional>

class MetFiller : public FillerBase {
 public:
  MetFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~MetFiller() {}

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  typedef edm::View<reco::MET> METView;

  NamedToken<METView> metToken_;

  typedef std::function<panda::RecoMet&(panda::Event&)> OutputSelector;

  OutputSelector outputSelector_{};
};

#endif
