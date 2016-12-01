#ifndef PandaProd_Producer_HLTFiller_h
#define PandaProd_Producer_HLTFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

class HLTFiller : public FillerBase {
 public:
  HLTFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~HLTFiller();

  void addOutput(TFile&) override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;
  void fillRun(panda::Run&, edm::Run const&, edm::EventSetup const&) override;

 private:
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken_;
  HLTConfigProvider hltConfig_;
  unsigned currentMenu_{};
  std::map<std::string, unsigned> menuMap_{};

  TTree* hltTree_{0};
  std::vector<TString>* paths_{0};
};

#endif
