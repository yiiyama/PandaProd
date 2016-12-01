#ifndef PandaProd_Producer_JetsFiller_h
#define PandaProd_Producer_JetsFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"

class JetCorrectionUncertainty;

class JetsFiller : public FillerBase {
 public:
  JetsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~JetsFiller();

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  typedef edm::View<reco::Jet> JetView;

  edm::EDGetTokenT<JetView> jetsToken_;
  edm::EDGetTokenT<reco::GenJetCollection> genJetsToken_;
  edm::EDGetTokenT<double> rhoToken_;

  JetCorrectionUncertainty* jecUncertainty_{0};

  //! anti-kT distance parameter
  double R_{0.4};
  double minPt_{15.};
  double maxEta_{4.7};
};

#endif
