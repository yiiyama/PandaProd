#ifndef PandaProd_Producer_MetFiltersFiller_h
#define PandaProd_Producer_MetFiltersFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/TriggerResults.h"

class MetFiltersFiller : public FillerBase {
 public:
  MetFiltersFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~MetFiltersFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  edm::EDGetTokenT<edm::TriggerResults> filterResultsToken_;
  edm::EDGetTokenT<bool> badTrackToken_;
  edm::EDGetTokenT<bool> badMuonTrackToken_;
};

#endif
