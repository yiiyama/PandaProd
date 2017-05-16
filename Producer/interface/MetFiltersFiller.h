#ifndef PandaProd_Producer_MetFiltersFiller_h
#define PandaProd_Producer_MetFiltersFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/DetId/interface/DetIdCollection.h"

class MetFiltersFiller : public FillerBase {
 public:
  MetFiltersFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~MetFiltersFiller() {}

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  std::vector<NamedToken<edm::TriggerResults>> filterResultsTokens_;
  NamedToken<bool> badPFMuonsToken_;
  NamedToken<bool> badChargedHadronsToken_;
  NamedToken<bool> dupECALClustersToken_; // temporary for 03Feb2017 REMINIAOD
  NamedToken<DetIdCollection> unfixedECALHitsToken_; // temporary for 03Feb2017 REMINIAOD
};

#endif
