#ifndef PandaProd_Producer_FatJetsFiller_h
#define PandaProd_Producer_FatJetsFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/BTauReco/interface/JetTag.h"

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

class JetCorrectionUncertainty;

class FatJetsFiller : public FillerBase {
 public:
  FatJetsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~FatJetsFiller();

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;
  void setRefs(ObjectMapStore const&) override;

 private:
  typedef edm::View<reco::Jet> JetView;
  typedef edm::ValueMap<float> FloatMap;

  edm::EDGetTokenT<JetView> jetsToken_;
  edm::EDGetTokenT<JetView> subjetsToken_;
  edm::EDGetTokenT<reco::JetTagCollection> btagsToken_;
  edm::EDGetTokenT<FloatMap> qglToken_;
  edm::EDGetTokenT<reco::GenJetCollection> genJetsToken_;
  edm::EDGetTokenT<double> rhoToken_;

  JetCorrectionUncertainty* jecUncertainty_{0};

  //! anti-kT distance parameter
  double R_{0.8};
  double minPt_{180.};
  double maxEta_{2.5};
};

#endif
