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
}

void
HLTFiller::addOutput(TFile& _outputFile)
{
  TDirectory::TContext context(&_outputFile);
  hltTree_ = new TTree("hlt", "HLT");
}

void
HLTFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList& _runBranches) const
{
  _eventBranches.emplace_back("triggers");
  _runBranches.emplace_back("hltMenu");
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

  // _outRun.hlt is not reset at each init() call
  if (!configChanged && menu != *_outRun.hlt.menu)
    throw edm::Exception(edm::errors::Configuration, "HLTFiller")
      << "HLTConfigProvider claims nothing is changed, but the menu name did.";

  *_outRun.hlt.menu = menu;
  auto menuItr(menuMap_.find(menu));

  if (menuItr != menuMap_.end()) {
    _outRun.hltMenu = menuItr->second;
    hltTree_->GetEntry(_outRun.hltMenu);
    return;
  }

  _outRun.hltMenu = menuMap_.size();
  menuMap_.emplace(menu, _outRun.hltMenu);

  _outRun.hlt.paths->clear();
  for (unsigned iP(0); iP != hltConfig_.size(); ++iP)
    _outRun.hlt.paths->push_back(hltConfig_.triggerName(iP));

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
