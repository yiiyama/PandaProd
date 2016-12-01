#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "PandaTree/Objects/interface/Event.h"
#include "PandaTree/Objects/interface/Run.h"

#include "../interface/FillerBase.h"
#include "../interface/ObjectMap.h"

#include "TFile.h"
#include "TTree.h"

#include <vector>
#include <utility>

class PandaProducer : public edm::EDAnalyzer {
public:
  explicit PandaProducer(edm::ParameterSet const&);
  ~PandaProducer();

private:
  void analyze(edm::Event const&, edm::EventSetup const&) override;
  void beginRun(edm::Run const&, edm::EventSetup const&) override;
  void beginJob() override;
  void endJob() override;

  std::vector<FillerBase*> fillers_;
  ObjectMapStore objectMaps_;

  TFile* outputFile_{0};
  TTree* eventTree_{0};
  TTree* runTree_{0};
  panda::Event outEvent_;
  panda::Run outRun_;
};

PandaProducer::PandaProducer(edm::ParameterSet const& _cfg)
{
  auto&& coll(consumesCollector());

  auto& fillersCfg(_cfg.getUntrackedParameterSet("fillers"));

  for (auto& fillerName : fillersCfg.getParameterNames()) {
    auto className(fillersCfg.getUntrackedParameterSet(fillerName).getUntrackedParameter<std::string>("filler"));
    try {
      fillers_.push_back(FillerFactoryStore::singleton()->makeFiller(className + "Filler", fillerName, _cfg, coll));
    }
    catch (std::exception& ex) {
      edm::LogError("FIllMitTree") << "Configuration error in " << fillerName << " " << className << "::" << className << "()";
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
  outEvent_.init();
  objectMaps_.clear();

  outEvent_.runNumber = _event.id().run();
  outEvent_.lumiNumber = _event.luminosityBlock();
  outEvent_.eventNumber = _event.id().event();

  for (auto* filler : fillers_)
    filler->fill(outEvent_, _event, _setup, objectMaps_);

  for (auto* filler : fillers_)
    filler->setRefs(objectMaps_);

  eventTree_->Fill();
}

void
PandaProducer::beginRun(edm::Run const& _run, edm::EventSetup const& _setup)
{
  outRun_.init();

  outRun_.run = _run.run();

  for (auto* filler : fillers_)
    filler->fillRun(outRun_, _run, _setup);

  runTree_->Fill();
}

void 
PandaProducer::beginJob()
{
  outputFile_ = TFile::Open("panda.root", "recreate");
  eventTree_ = new TTree("events", "");
  runTree_ = new TTree("runs", "");
  outEvent_.book(*eventTree_);
  outRun_.book(*runTree_);

  for (auto* filler : fillers_)
    filler->addOutput(*outputFile_);
}

void 
PandaProducer::endJob()
{
  outputFile_->cd();
  outputFile_->Write();
  delete outputFile_;
}

DEFINE_FWK_MODULE(PandaProducer);
