#ifndef PandaProd_Producer_GenJetsFiller_h
#define PandaProd_Producer_GenJetsFiller_h

#include "FillerBase.h"

#include "DataFormats/JetReco/interface/GenJet.h"

class GenJetsFiller : public FillerBase {
 public:
  GenJetsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~GenJetsFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  typedef edm::View<reco::GenJet> GenJetView;

  NamedToken<GenJetView> genJetsToken_;

  double minPt_{15.};
};

#endif
