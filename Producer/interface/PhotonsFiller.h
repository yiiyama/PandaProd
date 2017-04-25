#ifndef PandaProd_Producer_PhotonsFiller_h
#define PandaProd_Producer_PhotonsFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"

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
  NamedToken<PhotonView> smearedPhotonsToken_;
  NamedToken<PhotonView> regressionPhotonsToken_;
  NamedToken<PhotonView> gsUnfixedPhotonsToken_; // temporary for 03Feb2017 Re-MINIAOD
  NamedToken<reco::CandidateView> pfCandidatesToken_;
  NamedToken<EcalRecHitCollection> ebHitsToken_;
  NamedToken<EcalRecHitCollection> eeHitsToken_;
  NamedToken<BoolMap> looseIdToken_;
  NamedToken<BoolMap> mediumIdToken_;
  NamedToken<BoolMap> tightIdToken_;
  NamedToken<FloatMap> chIsoToken_;
  NamedToken<FloatMap> nhIsoToken_;
  NamedToken<FloatMap> phIsoToken_;
  NamedToken<FloatMap> chIsoMaxToken_;
  NamedToken<double> rhoToken_;
  NamedToken<pat::PackedGenParticleCollection> genParticlesToken_;

  EffectiveAreas chIsoEA_;
  EffectiveAreas nhIsoEA_;
  EffectiveAreas phIsoEA_;

  TFormula chIsoLeakage_[2];
  TFormula nhIsoLeakage_[2];
  TFormula phIsoLeakage_[2];

  std::string superClustersFillerName_;

  std::set<std::string> triggerObjects_[panda::Photon::nTriggerObjects];
  double minPt_;
  double maxEta_;
};

#endif
