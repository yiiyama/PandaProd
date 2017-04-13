#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"

#include "PandaTree/Objects/interface/Event.h"

#include "../interface/FillerBase.h"
#include "../interface/ObjectMap.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TStopwatch.h"

#include <vector>
#include <utility>

class PandaProducer : public edm::EDAnalyzer {
public:
  explicit PandaProducer(edm::ParameterSet const&);
  ~PandaProducer();

private:
  typedef edm::View<pat::TriggerObjectStandAlone> TriggerObjectView;

  void analyze(edm::Event const&, edm::EventSetup const&) override;
  void beginRun(edm::Run const&, edm::EventSetup const&) override;
  void endRun(edm::Run const&, edm::EventSetup const&) override;
  void beginJob() override;
  void endJob() override;

  std::vector<FillerBase*> fillers_;
  ObjectMapStore objectMaps_;

  VString selectEvents_;
  edm::EDGetTokenT<edm::TriggerResults> skimResultsToken_;
  edm::EDGetTokenT<edm::TriggerResults> hltResultsToken_;
  edm::EDGetTokenT<TriggerObjectView> triggerObjectsToken_;

  TFile* outputFile_{0};
  TTree* eventTree_{0};
  TTree* runTree_{0};
  TH1D* eventCounter_{0};
  panda::Event outEvent_;

  TStopwatch *sw_{0};
  std::string outputName_;
  bool useTrigger_;
  // [[filter0, filter1, ...], ...] outer index runs over trigger objects
  std::vector<VString> triggerObjectNames_;
  unsigned printLevel_;
};

PandaProducer::PandaProducer(edm::ParameterSet const& _cfg) :
  selectEvents_(_cfg.getUntrackedParameter<VString>("SelectEvents")),
  skimResultsToken_(consumes<edm::TriggerResults>(edm::InputTag("TriggerResults"))), // no process name -> pick up the trigger results from the current process
  outputName_(_cfg.getUntrackedParameter<std::string>("outputFile", "panda.root")),
  useTrigger_(_cfg.getUntrackedParameter<bool>("useTrigger", true)),
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
        std::cout << "[PandaProducer::PandaProducer] " 
          << "Constructing " << className << "::" << fillerName << std::endl;

      auto* filler(FillerFactoryStore::singleton()->makeFiller(className, fillerName, _cfg, coll));
      fillers_.push_back(filler);

      if (filler->enabled())
        filler->setObjectMap(objectMaps_[fillerName]);
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer::PandaProducer] " 
        << "Configuration error in " << fillerName << ":"
                                     << ex.what() << std::endl;
      throw;
    }
  }

  if (useTrigger_) {
    hltResultsToken_ = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLT"));
    triggerObjectsToken_ = consumes<TriggerObjectView>(edm::InputTag(fillersCfg.getUntrackedParameterSet("common").getUntrackedParameter<std::string>("triggerObjects")));
  }

  if (printLevel_ > 2) {
    sw_ = new TStopwatch();
    sw_->Start();
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
  
  if (printLevel_ > 2) {
    std::cout << "[PandaProducer::analyze]" 
      << "Previous (CMSSW) step took " << sw_->RealTime()*1000 << " s" << std::endl;
    sw_->Start();
  }


  // Fill "all events" information
  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ > 1)
        std::cout << "[PandaProducer::analyze] " 
          << "Calling " << filler->getName() << "->fillAll()" << std::endl;

      filler->fillAll(_event, _setup);
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
    // Unpack trigger object names
    edm::Handle<edm::TriggerResults> triggerResultsHandle;
    _event.getByToken(hltResultsToken_, triggerResultsHandle);
    auto& triggerNames(_event.triggerNames(*triggerResultsHandle));

    edm::Handle<TriggerObjectView> triggerObjectsHandle;
    _event.getByToken(triggerObjectsToken_, triggerObjectsHandle);
    auto& triggerObjects(*triggerObjectsHandle);

    auto& objMap(objectMaps_["global"].get<pat::TriggerObjectStandAlone, VString>());
    triggerObjectNames_.assign(triggerObjects.size(), VString());

    unsigned iObj(0);
    for (auto& obj : triggerObjects) {
      // need to create a copy to perform the non-const action of unpacking
      pat::TriggerObjectStandAlone copy(obj);
      copy.unpackPathNames(triggerNames);
      
      for (auto& label : obj.filterLabels())
        triggerObjectNames_[iObj].push_back(label);

      // link the pat trigger object to the list of labels
      objMap.add(triggerObjects.ptrAt(iObj), triggerObjectNames_[iObj]);
      ++iObj;
    }
  }

  outEvent_.runNumber = _event.id().run();
  outEvent_.lumiNumber = _event.luminosityBlock();
  outEvent_.eventNumber = _event.id().event();
  outEvent_.isData = _event.isRealData();

  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ > 1)
        std::cout << "[PandaProducer::fill] " 
          << "Calling " << filler->getName() << "->fill()" << std::endl;

      if (printLevel_ > 2) {
        sw_->Start();
      }

      filler->fill(outEvent_, _event, _setup);

      if (printLevel_ > 2) {
        std::cout << "[PandaProducer::analyze] Step " 
          << filler->getName() << "->fill() took " << sw_->RealTime()*1000 << " s" << std::endl;
      }
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer::fill] " 
        << "Error in " << filler->getName() << "::fill()" << std::endl;
      throw;
    }
  }

  // Set inter-branch references
  for (auto* filler : fillers_) {
    if (!filler->enabled())
      continue;

    try {
      if (printLevel_ > 1)
        std::cout << "[PandaProducer:fill] "
          << "Calling " << filler->getName() << "->setRefs()" << std::endl;

      filler->setRefs(objectMaps_);
    }
    catch (std::exception& ex) {
      std::cerr << "[PandaProducer:fill] " 
        << "Error in " << filler->getName() << "::setRefs()" << std::endl;
      throw;
    }
  }

  outEvent_.fill(*eventTree_);
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
      if (printLevel_ > 1)
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
      if (printLevel_ > 1) 
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
PandaProducer::beginJob()
{
  outputFile_ = TFile::Open(outputName_.c_str(), "recreate");
  eventTree_ = new TTree("events", "");
  runTree_ = new TTree("runs", "");

  panda::utils::BranchList eventBranches = {"runNumber", "lumiNumber", "eventNumber", "isData"};
  panda::utils::BranchList runBranches = {"runNumber"};
  for (auto* filler : fillers_) {
    if (filler->enabled())
      filler->branchNames(eventBranches, runBranches);
  }

  outEvent_.book(*eventTree_, eventBranches);
  outEvent_.run.book(*runTree_, runBranches);

  for (auto* filler : fillers_) {
    filler->addOutput(*outputFile_);
  }

  if (useTrigger_ && outputFile_->Get("hlt")) {
    outEvent_.run.hlt.create();
    auto& hltTree(*static_cast<TTree*>(outputFile_->Get("hlt")));
    hltTree.Branch("menu", "TString", &outEvent_.run.hlt.menu);
    hltTree.Branch("paths", "std::vector<TString>", &outEvent_.run.hlt.paths, 32000, 0);
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
}

DEFINE_FWK_MODULE(PandaProducer);
