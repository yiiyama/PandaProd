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

  void addOutput(TFile&) override;
  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;
  void setRefs(ObjectMapStore const&) override;

 protected:
  typedef edm::View<reco::Photon> PhotonView;
  typedef edm::View<reco::GsfElectron> GsfElectronView;
  typedef edm::ValueMap<bool> BoolMap;
  typedef edm::ValueMap<float> FloatMap;

  NamedToken<PhotonView> photonsToken_;
  NamedToken<EcalRecHitCollection> ebHitsToken_;
  NamedToken<EcalRecHitCollection> eeHitsToken_;
  //  NamedToken<GsfElectronView> electronsToken_;
  NamedToken<BoolMap> looseIdToken_;
  NamedToken<BoolMap> mediumIdToken_;
  NamedToken<BoolMap> tightIdToken_;
  NamedToken<FloatMap> chIsoToken_;
  NamedToken<FloatMap> nhIsoToken_;
  NamedToken<FloatMap> phIsoToken_;
  NamedToken<FloatMap> wchIsoToken_;
  NamedToken<double> rhoToken_;
  NamedToken<pat::TriggerObjectStandAloneCollection> triggerObjectsToken_;

  EffectiveAreas chIsoEA_;
  EffectiveAreas nhIsoEA_;
  EffectiveAreas phIsoEA_;

  TFormula chIsoLeakage_[2];
  TFormula nhIsoLeakage_[2];
  TFormula phIsoLeakage_[2];

  VString l1Filters_;
  VString hltFilters_;
  double minPt_;
  double maxEta_;
};

#endif
