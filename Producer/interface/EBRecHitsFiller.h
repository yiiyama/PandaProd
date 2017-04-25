#ifndef PandaProd_Producer_EBRecHitsFiller_h
#define PandaProd_Producer_EBRecHitsFiller_h

#include "FillerBase.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

class EBRecHitsFiller : public FillerBase {
 public:
  EBRecHitsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~EBRecHitsFiller() {}

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;
  void setRefs(ObjectMapStore const&) override;

 protected:
  NamedToken<EcalRecHitCollection> ebHitsToken_;
};

#endif
