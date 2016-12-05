#ifndef PandaProd_Producer_WeightsFiller_h
#define PandaProd_Producer_WeightsFiller_h

#include "FillerBase.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"

#include "TH1D.h"
#include "TObjString.h"

class WeightsFiller : public FillerBase {
 public:
  WeightsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~WeightsFiller() {}

  void branchNames(panda::utils::BranchList&, panda::utils::BranchList&) const override;
  void addOutput(TFile&) override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;
  void fillAll(edm::Event const&, edm::EventSetup const&) override;
  void fillEndRun(panda::Run&, edm::Run const&, edm::EventSetup const&) override;

 protected:
  void initIndices_(LHEEventProduct const&);

  NamedToken<GenEventInfoProduct> genInfoToken_;
  NamedToken<LHEEventProduct> lheEventToken_;
  NamedToken<LHERunInfoProduct> lheRunToken_;

  //! Weight ID to index mapping
  std::map<TString, unsigned> weightIndices_{};

  TH1D* hSumW_{0};

  TFile* outputFile_{0};
};

#endif
