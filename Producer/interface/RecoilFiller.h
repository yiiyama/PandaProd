#ifndef PandaProd_Producer_RecoilFiller_h
#define PandaProd_Producer_RecoilFiller_h

#include "FillerBase.h"

class RecoilFiller : public FillerBase {
 public:
  RecoilFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~RecoilFiller() {}

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  NamedToken<int> categoriesToken_;
  NamedToken<double> maxToken_;
};

#endif
