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
#include "PandaTree/Objects/interface/Run.h"

#include "../interface/FillerBase.h"
#include "../interface/ObjectMap.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"

#include <vector>
#include <utility>

class PandaProducer : public edm::EDAnalyzer {
public:
  explicit PandaProducer(edm::ParameterSet const&);
  ~PandaProducer();

private:
  void analyze(edm::Event const&, edm::EventSetup const&) override;
  void beginRun(edm::Run const&, edm::EventSetup const&) override;
  void endRun(edm::Run const&, edm::EventSetup const&) override;
  void beginJob() override;
  void endJob() override;

  std::vector<FillerBase*> fillers_;
  ObjectMapStore objectMaps_;

  VString selectEvents_;
  edm::EDGetTokenT<edm::TriggerResults> skimResultsToken_;

  TFile* outputFile_{0};
  TTree* eventTree_{0};
  TTree* runTree_{0};
  TH1D* eventCounter_{0};
  panda::Event outEvent_;
  panda::Run outRun_;

  unsigned printLevel_;
};

PandaProducer::PandaProducer(edm::ParameterSet const& _cfg) :
  selectEvents_(_cfg.getUntrackedParameter<VString>("SelectEvents")),
  skimResultsToken_(consumes<edm::TriggerResults>(edm::InputTag("TriggerResults"))), // no process name -> pick up the trigger results from the current process
  printLevel_(_cfg.getUntrackedParameter<unsigned>("printLevel", 0))
{
  auto&& coll(consumesCollector());

  auto& fillersCfg(_cfg.getUntrackedParameterSet("fillers"));

  for (auto& fillerName : fillersCfg.getParameterNames()) {
    if (fillerName == "common")
      continue;

    auto& fillerPSet(fillersCfg.getUntrackedParameterSet(fillerName));
    try {
      auto className(fillerPSet.getUntrackedParameter<std::string>("filler") + "Filler");

      if (printLevel_ > 0)
        std::cerr << "Constructing " << className << "::" << fillerName << std::endl;

      auto* filler(FillerFactoryStore::singleton()->makeFiller(className, fillerName, _cfg, coll));
      fillers_.push_back(filler);

      if (filler->enabled())
        filler->setObjectMap(objectMaps_[fillerName]);
    }
    catch (std::exception& ex) {
      edm::LogError("PandaProducer") << "Configuration error in " << fillerName;
      throw;
    }
  }
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

  // Fill "all events" information
  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ > 1)
        std::cerr << "Calling " << filler->getName() << "->fillAll()" << std::endl;

      filler->fillAll(_event, _setup);
    }
    catch (std::exception& ex) {
      edm::LogError("PandaProducer") << "Error in " << filler->getName() << "::fillAll()";
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

  outEvent_.runNumber = _event.id().run();
  outEvent_.lumiNumber = _event.luminosityBlock();
  outEvent_.eventNumber = _event.id().event();

  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ > 1)
        std::cerr << "Calling " << filler->getName() << "->fill()" << std::endl;

      filler->fill(outEvent_, _event, _setup);
    }
    catch (std::exception& ex) {
      edm::LogError("PandaProducer") << "Error in " << filler->getName() << "::fill()";
      throw;
    }
  }

  // Set inter-branch references
  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ > 1)
        std::cerr << "Calling " << filler->getName() << "->setRefs()" << std::endl;

      filler->setRefs(objectMaps_);
    }
    catch (std::exception& ex) {
      edm::LogError("PandaProducer") << "Error in " << filler->getName() << "::setRefs()";
      throw;
    }
  }

  eventTree_->Fill();
}

void
PandaProducer::beginRun(edm::Run const& _run, edm::EventSetup const& _setup)
{
  outRun_.init();

  outRun_.run = _run.run();

  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ > 1)
        std::cerr << "Calling " << filler->getName() << "->fillBeginRun()" << std::endl;

      filler->fillBeginRun(outRun_, _run, _setup);
    }
    catch (std::exception& ex) {
      edm::LogError("PandaProducer") << "Error in " << filler->getName() << "::fillBeginRun()";
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
      if (printLevel_ > 1)
        std::cerr << "Calling " << filler->getName() << "->fillEndRun()" << std::endl;

      filler->fillEndRun(outRun_, _run, _setup);
    }
    catch (std::exception& ex) {
      edm::LogError("PandaProducer") << "Error in " << filler->getName() << "::fillEndRun()";
      throw;
    }
  }

  runTree_->Fill();
}

void 
PandaProducer::beginJob()
{
  outputFile_ = TFile::Open("panda.root", "recreate");
  eventTree_ = new TTree("events", "");
  runTree_ = new TTree("runs", "");

  panda::utils::BranchList eventBranches;
  panda::utils::BranchList runBranches;
  for (auto* filler : fillers_)
    filler->branchNames(eventBranches, runBranches);

  outEvent_.book(*eventTree_, eventBranches);
  outRun_.book(*runTree_, runBranches);

  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    filler->addOutput(*outputFile_);
  }

  eventCounter_ = new TH1D("eventcounter", "", 2, 0., 2.);
  eventCounter_->GetXaxis()->SetBinLabel(1, "all");
  eventCounter_->GetXaxis()->SetBinLabel(2, "selected");
}

void 
PandaProducer::endJob()
{
  outputFile_->cd();
  outputFile_->Write();
  delete outputFile_;
}

DEFINE_FWK_MODULE(PandaProducer);
