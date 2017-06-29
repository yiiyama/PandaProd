#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Common/interface/Handle.h"

#include "PandaTree/Objects/interface/Event.h"

#include "../interface/FillerBase.h"
#include "../interface/ObjectMap.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include <vector>
#include <utility>
#include <chrono>

typedef std::chrono::steady_clock SClock;
double toMS(SClock::duration const& interval)
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(interval).count() * 1.e-6;
}

class PandaProducer : public edm::EDAnalyzer {
public:
  explicit PandaProducer(edm::ParameterSet const&);
  ~PandaProducer();

private:
  void analyze(edm::Event const&, edm::EventSetup const&) override;
  void beginRun(edm::Run const&, edm::EventSetup const&) override;
  void endRun(edm::Run const&, edm::EventSetup const&) override;
  void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  void beginJob() override;
  void endJob() override;

  std::vector<FillerBase*> fillers_;
  ObjectMapStore objectMaps_;

  VString selectEvents_;
  edm::EDGetTokenT<edm::TriggerResults> skimResultsToken_;

  TFile* outputFile_{0};
  TTree* eventTree_{0};
  TTree* runTree_{0};
  TTree* lumiSummaryTree_{0};
  TH1D* eventCounter_{0};
  panda::Event outEvent_;

  unsigned nEventsInLumi_;

  std::string outputName_;
  bool useTrigger_;
  unsigned printLevel_;

  std::vector<SClock::duration> timers_;
  SClock::time_point lastAnalyze_; //! Time point of last return from analyze()
  unsigned long long nEvents_;
};

PandaProducer::PandaProducer(edm::ParameterSet const& _cfg) :
  selectEvents_(_cfg.getUntrackedParameter<VString>("SelectEvents")),
  skimResultsToken_(consumes<edm::TriggerResults>(edm::InputTag("TriggerResults"))), // no process name -> pick up the trigger results from the current process
  outEvent_(),
  nEventsInLumi_(0),
  outputName_(_cfg.getUntrackedParameter<std::string>("outputFile", "panda.root")),
  useTrigger_(_cfg.getUntrackedParameter<bool>("useTrigger", true)),
  printLevel_(_cfg.getUntrackedParameter<unsigned>("printLevel", 0)),
  timers_(),
  lastAnalyze_(),
  nEvents_(0)
{
  auto&& coll(consumesCollector());

  auto& fillersCfg(_cfg.getUntrackedParameterSet("fillers"));

  SClock::time_point start;

  for (auto& fillerName : fillersCfg.getParameterNames()) {
    if (fillerName == "common")
      continue;

    auto& fillerPSet(fillersCfg.getUntrackedParameterSet(fillerName));
    try {
      auto className(fillerPSet.getUntrackedParameter<std::string>("filler") + "Filler");

      if (printLevel_ >= 1) {
        std::cout << "[PandaProducer::PandaProducer] " 
          << "Constructing " << className << "::" << fillerName << std::endl;

        if (printLevel_ >= 3)
          start = SClock::now();
      }

      auto* filler(FillerFactoryStore::singleton()->makeFiller(className, fillerName, _cfg, coll));
      fillers_.push_back(filler);

      if (filler->enabled())
        filler->setObjectMap(objectMaps_[fillerName]);

      if (printLevel_ >= 1) {
        timers_.push_back(SClock::duration::zero());

        if (printLevel_ >= 3)
          std::cout << "Initializing " << fillerName << " took " << toMS(SClock::now() - start) << " ms." << std::endl;
      }
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer::PandaProducer] " 
        << "Configuration error in " << fillerName << ":"
                                     << ex.what() << std::endl;
      throw;
    }
  }

  if (printLevel_ >= 1) {
    // timer for the CMSSW execution outside of this module
    timers_.push_back(SClock::duration::zero());
  }

  // The lambda function inside will be called by CMSSW Framework whenever a new product is registered
  callWhenNewProductsRegistered([this](edm::BranchDescription const& branchDescription) {
      auto&& coll(this->consumesCollector());
      for (auto* filler : this->fillers_)
        filler->notifyNewProduct(branchDescription, coll);
    });
}

PandaProducer::~PandaProducer()
{
  for (auto* filler : fillers_)
    delete filler;
}

void
PandaProducer::analyze(edm::Event const& _event, edm::EventSetup const& _setup)
{
  eventCounter_->Fill(0.5);
  
  if (printLevel_ >= 1) {
    if (nEvents_ == 0) {
      if (printLevel_ >= 3)
        std::cout << "[PandaProducer::analyze] "
                  << "First event; CMSSW step time unknown" << std::endl;

    }
    else {
      auto dt(SClock::now() - lastAnalyze_);
      if (printLevel_ >= 3)
        std::cout << "[PandaProducer::analyze] "
                  << "Previous (CMSSW) step took " << toMS(dt) << " ms" << std::endl;

      timers_.back() += dt;
    }
  }

  ++nEvents_;
  ++nEventsInLumi_;

  SClock::time_point start;

  // Fill "all events" information
  for (unsigned iF(0); iF != fillers_.size(); ++iF) {
    auto* filler(fillers_[iF]);

    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ >= 1) {
        start = SClock::now();

        if (printLevel_ >= 2)
          std::cout << "[PandaProducer::analyze] " 
                    << "Calling " << filler->getName() << "->fillAll()" << std::endl;
      }

      filler->fillAll(_event, _setup);

      if (printLevel_ >= 1) {
        auto dt(SClock::now() - start);

        if (printLevel_ >= 3) {
          std::cout << "[PandaProducer::analyze] " 
                    << "Step " << filler->getName() << "->fillAll() took " << toMS(dt) << " ms" << std::endl;
        }
        
        timers_[iF] += dt;
      }
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer::analyze] " 
        << "Error in " << filler->getName() << "::fillAll()" << std::endl;
      throw;
    }
  }

  // If path names are given, check if at least one succeeded
  if (selectEvents_.size() != 0) {
    edm::Handle<edm::TriggerResults> triggerResults;
    if(_event.getByToken(skimResultsToken_, triggerResults)){
      auto& pathNames(_event.triggerNames(*triggerResults));
      unsigned iS(0);
      for (; iS != selectEvents_.size(); ++iS) {
        unsigned iP(pathNames.triggerIndex(selectEvents_[iS]));
        if (iP != pathNames.size() && triggerResults->accept(iP))
          break;
      }
      if (iS == selectEvents_.size())
        return;
    }
  }

  eventCounter_->Fill(1.5);

  // Now fill the event
  outEvent_.init();

  for (auto& mm : objectMaps_)
    mm.second.clearMaps();

  if (useTrigger_) {

  }

  outEvent_.runNumber = _event.id().run();
  outEvent_.lumiNumber = _event.luminosityBlock();
  outEvent_.eventNumber = _event.id().event();
  outEvent_.isData = _event.isRealData();

  for (unsigned iF(0); iF != fillers_.size(); ++iF) {
    auto* filler(fillers_[iF]);

    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ >= 1) {
        if (printLevel_ >= 2)
          std::cout << "[PandaProducer::fill] " 
                    << "Calling " << filler->getName() << "->fill()" << std::endl;

        start = SClock::now();
      }

      filler->fill(outEvent_, _event, _setup);

      if (printLevel_ >= 1) {
        auto dt(SClock::now() - start);

        if (printLevel_ >= 3)
          std::cout << "[PandaProducer::analyze] " 
                    << "Step " << filler->getName() << "->fill() took " << toMS(dt) << " ms" << std::endl;

        timers_[iF] += dt;
      }
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer::fill] " 
        << "Error in " << filler->getName() << "::fill()" << std::endl;
      throw;
    }
  }

  // Set inter-branch references
  for (unsigned iF(0); iF != fillers_.size(); ++iF) {
    auto* filler(fillers_[iF]);

    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ >= 1) {
        if (printLevel_ >= 2)
          std::cout << "[PandaProducer:fill] "
                    << "Calling " << filler->getName() << "->setRefs()" << std::endl;

        start = SClock::now();
      }

      filler->setRefs(objectMaps_);

      if (printLevel_ >= 1) {
        auto dt(SClock::now() - start);

        if (printLevel_ >= 3)
          std::cout << "[PandaProducer::analyze] " 
                    << "Step " << filler->getName() << "->setRefs() took " << toMS(dt) << " ms" << std::endl;

        timers_[iF] += dt;
      }
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer:fill] " 
        << "Error in " << filler->getName() << "::setRefs()" << std::endl;
      throw;
    }
  }

  outEvent_.fill(*eventTree_);

  lastAnalyze_ = SClock::now();
}

void
PandaProducer::beginRun(edm::Run const& _run, edm::EventSetup const& _setup)
{
  outEvent_.run.init();

  outEvent_.run.runNumber = _run.run();

  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ >= 2)
        std::cout << "[PandaProducer::beginRun] " 
          << "Calling " << filler->getName() << "->fillBeginRun()" << std::endl;

      filler->fillBeginRun(outEvent_.run, _run, _setup);
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer::beginRun] "
        << "Error in " << filler->getName() << "::fillBeginRun()" << std::endl;
      throw;
    }
  }
}

void
PandaProducer::endRun(edm::Run const& _run, edm::EventSetup const& _setup)
{
  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ >= 2) 
        std::cout << "[PandaProducer::endRun] " 
          << "Calling " << filler->getName() << "->fillEndRun()" << std::endl;

      filler->fillEndRun(outEvent_.run, _run, _setup);
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer::endRun] "
        << "Error in " << filler->getName() << "::fillEndRun()" << std::endl;
      throw;
    }
  }

  outEvent_.run.fill(*runTree_);
}

void
PandaProducer::beginLuminosityBlock(edm::LuminosityBlock const& _lumi, edm::EventSetup const& _setup)
{
  nEventsInLumi_ = 0;
}

void
PandaProducer::endLuminosityBlock(edm::LuminosityBlock const& _lumi, edm::EventSetup const& _setup)
{
  outEvent_.runNumber = _lumi.id().run();
  outEvent_.lumiNumber = _lumi.id().luminosityBlock();
  lumiSummaryTree_->Fill();
}

void 
PandaProducer::beginJob()
{
  outputFile_ = TFile::Open(outputName_.c_str(), "recreate");
  eventTree_ = new TTree("events", "");
  runTree_ = new TTree("runs", "");
  lumiSummaryTree_ = new TTree("lumiSummary", "");

  panda::utils::BranchList eventBranches = {"runNumber", "lumiNumber", "eventNumber", "isData"};
  panda::utils::BranchList runBranches = {"runNumber"};
  for (auto* filler : fillers_) {
    if (filler->enabled())
      filler->branchNames(eventBranches, runBranches);
  }

  outEvent_.book(*eventTree_, eventBranches);
  outEvent_.run.book(*runTree_, runBranches);

  lumiSummaryTree_->Branch("runNumber", &outEvent_.runNumber, "runNumber/i");
  lumiSummaryTree_->Branch("lumiNumber", &outEvent_.lumiNumber, "lumiNumber/i");
  lumiSummaryTree_->Branch("nEvents", &nEventsInLumi_, "nEventsInLumi_/i");

  for (auto* filler : fillers_)
    filler->addOutput(*outputFile_);

  if (useTrigger_ && outputFile_->Get("hlt")) {
    outEvent_.run.hlt.create();
    auto& hltTree(*static_cast<TTree*>(outputFile_->Get("hlt")));
    hltTree.Branch("menu", "TString", &outEvent_.run.hlt.menu);
    hltTree.Branch("paths", "std::vector<TString>", &outEvent_.run.hlt.paths, 32000, 0);
    hltTree.Branch("filters", "std::vector<TString>", &outEvent_.run.hlt.filters, 32000, 0);
  }

  eventCounter_ = new TH1D("eventcounter", "", 2, 0., 2.);
  eventCounter_->SetDirectory(outputFile_);
  eventCounter_->GetXaxis()->SetBinLabel(1, "all");
  eventCounter_->GetXaxis()->SetBinLabel(2, "selected");
}

void 
PandaProducer::endJob()
{
  // writes out all outputs that are still hanging in the directory
  outputFile_->cd();
  outputFile_->Write();
  delete outputFile_;

  if (printLevel_ >= 1) {
    double total(0.);

    std::cout << "[PandaProducer::endJob] Timer summary" << std::endl;
    for (unsigned iF(0); iF != fillers_.size(); ++iF) {
      double msPerEvt(toMS(timers_[iF]) / nEvents_);
      std::cout << " " << fillers_[iF]->getName() << "  "
                << std::fixed << std::setprecision(3) << msPerEvt << " ms/evt"
                << std::endl;

      total += msPerEvt;
    }
    if (nEvents_ > 1) {
      double msPerEvt(toMS(timers_.back()) / (nEvents_ - 1));
      std::cout << " Other CMSSW  "
                << std::fixed << std::setprecision(3) << msPerEvt << " ms/evt"
                << std::endl;

      total += msPerEvt;
    }
    std::cout << std::endl << " Total  "
              << std::fixed << std::setprecision(3) << total << " ms/evt"
              << std::endl;
  }
}

DEFINE_FWK_MODULE(PandaProducer);
