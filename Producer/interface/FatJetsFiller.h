#ifndef PandaProd_Producer_FatJetsFiller_h
#define PandaProd_Producer_FatJetsFiller_h

#include "JetsFiller.h"

#include "DataFormats/BTauReco/interface/JetTag.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "PandaUtilities/Substructure/interface/HEPTopTaggerWrapperV2.h"
#include "PandaUtilities/Substructure/interface/EnergyCorrelations.h"

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
  void setRefs(ObjectMapStore const&) override;

 protected:
  void fillDetails_(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

  typedef edm::ValueMap<float> FloatMap;

  NamedToken<JetView> subjetsToken_;
  NamedToken<reco::JetTagCollection> btagsToken_;
  NamedToken<FloatMap> qglToken_;
  std::string njettinessTag_;
  std::string sdKinematicsTag_;

  fastjet::GhostedAreaSpec activeArea_;
  fastjet::AreaDefinition areaDef_;
  fastjet::JetDefinition* jetDefCA_{0};
  fastjet::contrib::SoftDrop* softdrop_{0};
  fastjet::contrib::Njettiness* tau_{0};
  fastjet::HEPTopTaggerV2* htt_{0};
  ECFNManager* ecfnManager_{0};

  bool computeSubstructure_{false};
};

#endif
