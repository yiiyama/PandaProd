#ifndef PandaProd_Producer_MuonsFiller_h
#define PandaProd_Producer_MuonsFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

class MuonsFiller : public FillerBase {
 public:
  MuonsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~MuonsFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  typedef edm::View<reco::Muon> MuonView;

  edm::EDGetTokenT<MuonView> muonsToken_;
  edm::EDGetTokenT<reco::VertexCollection> verticesToken_;
  edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjectsToken_;

  bool useTrigger_;
  VString hltFilters_;
  double minPt_{-1.};
  double maxEta_{10.};
};

#endif
