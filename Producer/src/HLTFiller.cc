#include "../interface/HLTFiller.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/PatCandidates/interface/MET.h"

HLTFiller::HLTFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(triggerResultsToken_, _cfg, _coll, "triggerResults");
}

HLTFiller::~HLTFiller()
{
  delete paths_;
}

void
HLTFiller::addOutput(TFile& _outputFile)
{
  TDirectory::TContext(&_outputFile);
  hltTree_ = new TTree("hlt", "HLT");
  menu_ = new TString;
  paths_ = new std::vector<TString>;
  hltTree_->Branch("menu", "TString", &menu_);
  hltTree_->Branch("paths", "std::vector<TString>", &paths_, 32000, 0);
}

void
HLTFiller::fillBeginRun(panda::Run& _outRun, edm::Run const& _inRun, edm::EventSetup const& _setup)
{
  bool configChanged(false);
  if (!hltConfig_.init(_inRun, _setup, "HLT", configChanged)) {
    throw edm::Exception(edm::errors::Configuration, "HLTFiller")
      << "Cannot access hlt config";
  }

  TString menu(hltConfig_.tableName());

  if (!configChanged) {
    if (menu == *menu_)
      return;
    else
      throw edm::Exception(edm::errors::Configuration, "HLTFiller")
        << "HLTConfigProvider claims nothing is changed, but the menu name did.";
  }

  *menu_ = menu;
  auto menuItr(menuMap_.find(menu));

  if (menuItr != menuMap_.end()) {
    _outRun.hltMenu = menuItr->second;
    hltTree_->GetEntry(_outRun.hltMenu);
    return;
  }

  _outRun.hltMenu = menuMap_.size();
  menuMap_.emplace(menu, _outRun.hltMenu);

  paths_->clear();
  for (unsigned iP(0); iP != hltConfig_.size(); ++iP)
    paths_->push_back(hltConfig_.triggerName(iP));

  hltTree_->Fill();
}

void
HLTFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inTriggerResults(getProduct_(_inEvent, triggerResultsToken_));

  auto& outHLT(_outEvent.triggers);

  for (unsigned iF(0); iF != inTriggerResults.size(); ++iF) {
    if (inTriggerResults.accept(iF))
      outHLT.set(iF);
  }
}

DEFINE_TREEFILLER(HLTFiller);
