#ifndef PandaProd_Producer_PhotonsFiller_h
#define PandaProd_Producer_PhotonsFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"

#include "RecoEgamma/EgammaTools/interface/EffectiveAreas.h"

#include "TFormula.h"

class PhotonsFiller : public FillerBase {
 public:
  PhotonsFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~PhotonsFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;
  void setRefs(ObjectMapStore const&) override;

 private:
  typedef edm::View<reco::Photon> PhotonView;
  typedef edm::View<reco::GsfElectron> GsfElectronView;
  typedef edm::ValueMap<bool> BoolMap;
  typedef edm::ValueMap<float> FloatMap;

  edm::EDGetTokenT<PhotonView> photonsToken_;
  edm::EDGetTokenT<EcalRecHitCollection> ebHitsToken_;
  edm::EDGetTokenT<EcalRecHitCollection> eeHitsToken_;
  //  edm::EDGetTokenT<GsfElectronView> electronsToken_;
  edm::EDGetTokenT<BoolMap> looseIdToken_;
  edm::EDGetTokenT<BoolMap> mediumIdToken_;
  edm::EDGetTokenT<BoolMap> tightIdToken_;
  edm::EDGetTokenT<FloatMap> chIsoToken_;
  edm::EDGetTokenT<FloatMap> nhIsoToken_;
  edm::EDGetTokenT<FloatMap> phIsoToken_;
  edm::EDGetTokenT<FloatMap> wchIsoToken_;
  edm::EDGetTokenT<double> rhoToken_;
  edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjectsToken_;

  EffectiveAreas chIsoEA_;
  EffectiveAreas nhIsoEA_;
  EffectiveAreas phIsoEA_;

  TFormula chIsoLeakage_[2];
  TFormula nhIsoLeakage_[2];
  TFormula phIsoLeakage_[2];

  bool useTrigger_;
  VString l1Filters_;
  VString hltFilters_;
  double minPt_;
  double maxEta_;

  std::vector<std::pair<panda::PPhoton*, edm::Ptr<reco::SuperCluster>>> scPtrs_;
};

#endif
