#ifndef PandaProd_Producer_P4Filler_h
#define PandaProd_Producer_P4Filler_h

#include "FillerBase.h"
#include <functional>

class P4Filler : public FillerBase {
 public:
  P4Filler(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~P4Filler() {}

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  NamedToken<reco::CandidateView> sourceToken_;

  std::function<panda::ParticleCollection&(panda::Event&)> outputSelector_{};

};

#endif
