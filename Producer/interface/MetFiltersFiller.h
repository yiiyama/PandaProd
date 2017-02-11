#ifndef PandaProd_Producer_MetFiltersFiller_h
#define PandaProd_Producer_MetFiltersFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/TriggerResults.h"

class MetFiltersFiller : public FillerBase {
 public:
  MetFiltersFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~MetFiltersFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  NamedToken<edm::TriggerResults> filterResultsToken_;
};

#endif
