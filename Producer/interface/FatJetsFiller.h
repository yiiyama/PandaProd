#ifndef PandaProd_Producer_FatJetsFiller_h
#define PandaProd_Producer_FatJetsFiller_h

#include "JetsFiller.h"

#include "DataFormats/BTauReco/interface/JetTag.h"
#include "DataFormats/BTauReco/interface/BoostedDoubleSVTagInfo.h"
#include "PandaProd/Utilities/interface/HEPTopTaggerWrapperV2.h"
#include "PandaProd/Utilities/interface/EnergyCorrelations.h"
#include "PandaProd/Utilities/interface/BoostedBtaggingMVACalculator.h"

// fastjet
#include "fastjet/PseudoJet.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/GhostedAreaSpec.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/contrib/SoftDrop.hh"
#include "fastjet/contrib/NjettinessPlugin.hh"
#include "fastjet/contrib/MeasureDefinition.hh"
#include "fastjet/contrib/EnergyCorrelator.hh"

class FatJetsFiller : public JetsFiller {
 public:
  FatJetsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~FatJetsFiller();

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;

 protected:
  void fillDetails_(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

  NamedToken<JetView> subjetsToken_;
  NamedToken<reco::BoostedDoubleSVTagInfoCollection> doubleBTagInfoToken_;
  NamedToken<int> categoriesToken_;
  std::string njettinessTag_;
  std::string sdKinematicsTag_;
  std::string subjetBtagTag_;
  std::string subjetQGLTag_;

  fastjet::GhostedAreaSpec activeArea_;
  fastjet::AreaDefinition areaDef_;
  fastjet::JetDefinition* jetDefCA_{0};
  fastjet::contrib::SoftDrop* softdrop_{0};
  fastjet::contrib::Njettiness* tau_{0};
  fastjet::HEPTopTaggerV2* htt_{0};
  ECFNManager* ecfnManager_{0};
  panda::BoostedBtaggingMVACalculator jetBoostedBtaggingMVACalc_{};

  enum SubstructureComputeMode {
    kAlways,
    kLargeRecoil,
    kNever
  };

  SubstructureComputeMode computeSubstructure_{kNever};
};

#endif
