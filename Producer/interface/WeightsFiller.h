/*!
  WeightsFiller
  Fill event weights. Assumes weights with id 1-9 (LO) or 1001-1009 (NLO) are mu_R and mu_F variations mu_R = (1, 2, 0.5) x mu_F = (1, 2, 0.5)
  and those with id 11-110 (LO) or 2001-2100 (NLO) are NNPDF MC variations (only RMS interesting).
*/

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
  void getLHEWeights_(LHEEventProduct const&, double [7], float _params[]=0);

  NamedToken<GenEventInfoProduct> genInfoToken_;
  NamedToken<LHEEventProduct> lheEventToken_;
  NamedToken<LHERunInfoProduct> lheRunToken_;
  
  bool saveSignalWeights_{false};
  int nSignalWeights_ = -1;


  // these objects will be deleted automatically when the output file closes
  TH1D* hSumW_{0};
  TTree* groupTree_{0};
  TString* gcombine_{0};
  TString* gtype_{0};
  TTree* weightTree_{0};
  TString* wid_{0};
  TString* wtitle_{0};
  unsigned gid_{0};
  
  TTree* eventTree_{0};
};

#endif
