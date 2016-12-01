#ifndef PandaProd_Producer_ElectronsFiller_h
#define PandaProd_Producer_ElectronsFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"

#include "RecoEgamma/EgammaTools/interface/EffectiveAreas.h"

#include <vector>
#include <utility>

class ElectronsFiller : public FillerBase {
 public:
  ElectronsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~ElectronsFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;
  void setRefs(ObjectMapStore const&) override;

 private:
  typedef edm::View<reco::Photon> PhotonView;
  typedef edm::View<reco::GsfElectron> GsfElectronView;
  typedef edm::ValueMap<bool> BoolMap;
  typedef edm::ValueMap<float> FloatMap;

  edm::EDGetTokenT<GsfElectronView> electronsToken_;
  edm::EDGetTokenT<PhotonView> photonsToken_;
  edm::EDGetTokenT<BoolMap> vetoIdToken_;
  edm::EDGetTokenT<BoolMap> looseIdToken_;
  edm::EDGetTokenT<BoolMap> mediumIdToken_;
  edm::EDGetTokenT<BoolMap> tightIdToken_;
  edm::EDGetTokenT<FloatMap> phCHIsoToken_;
  edm::EDGetTokenT<FloatMap> phNHIsoToken_;
  edm::EDGetTokenT<FloatMap> phPhIsoToken_;
  edm::EDGetTokenT<FloatMap> ecalIsoToken_;
  edm::EDGetTokenT<FloatMap> hcalIsoToken_;
  edm::EDGetTokenT<double> rhoToken_;
  edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjectsToken_;

  EffectiveAreas combIsoEA_;
  EffectiveAreas ecalIsoEA_;
  EffectiveAreas hcalIsoEA_;
  EffectiveAreas phCHIsoEA_;
  EffectiveAreas phNHIsoEA_;
  EffectiveAreas phPhIsoEA_;

  bool useTrigger_;
  VString hltFilters_;
  double minPt_{-1.};
  double maxEta_{10.};

  std::vector<std::pair<panda::PElectron*, edm::Ptr<reco::SuperCluster>>> scPtrs_;
};

#endif
