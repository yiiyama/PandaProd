#ifndef PandaProd_Producer_JetsFiller_h
#define PandaProd_Producer_JetsFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

#include <functional>

class JetCorrectionUncertainty;

class JetsFiller : public FillerBase {
 public:
  JetsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~JetsFiller();

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;
  void setRefs(ObjectMapStore const&) override;

 protected:
  virtual void fillDetails_(panda::Event&, edm::Event const&, edm::EventSetup const&) {}

  //! Fills the training variables for b-tagging and regression
  void fillHbbVars_(panda::MicroJet& outJet, pat::Jet const& inJet);

  typedef edm::View<reco::Jet> JetView;
  typedef edm::View<reco::GenJet> GenJetView;
  typedef edm::ValueMap<float> FloatMap;

  NamedToken<JetView> jetsToken_;
  NamedToken<GenJetView> genJetsToken_;
  NamedToken<FloatMap> qglToken_;
  NamedToken<double> rhoToken_;
  std::string jecName_;
  std::string jerName_;
  std::string csvTag_;
  std::string cmvaTag_;
  std::string deepCsvTag_;
  std::string deepCmvaTag_;
  std::string puidTag_;

  JetCorrectionUncertainty* jecUncertainty_{0};

  typedef std::function<panda::JetCollection&(panda::Event&)> OutputSelector;

  OutputSelector outputSelector_{};

  std::string outGenJets_{};
  std::string constituentsLabel_{};
  //! jet radius
  double R_{0.4};
  double minPt_{15.};
  double maxEta_{4.7};

  bool fillConstituents_{false};
  unsigned subjetsOffset_{0}; // first N constituents are actually subjets (happens when fixDaughters = True in JetSubstructurePacker)
};

#endif
