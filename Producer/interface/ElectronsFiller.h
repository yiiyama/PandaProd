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

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;
  void setRefs(ObjectMapStore const&) override;

 protected:
  typedef edm::View<reco::Photon> PhotonView;
  typedef edm::View<reco::GsfElectron> GsfElectronView;
  typedef edm::ValueMap<bool> BoolMap;
  typedef edm::ValueMap<float> FloatMap;

  NamedToken<GsfElectronView> electronsToken_;
  NamedToken<PhotonView> photonsToken_;
  NamedToken<BoolMap> vetoIdToken_;
  NamedToken<BoolMap> looseIdToken_;
  NamedToken<BoolMap> mediumIdToken_;
  NamedToken<BoolMap> tightIdToken_;
  NamedToken<FloatMap> phCHIsoToken_;
  NamedToken<FloatMap> phNHIsoToken_;
  NamedToken<FloatMap> phPhIsoToken_;
  NamedToken<FloatMap> ecalIsoToken_;
  NamedToken<FloatMap> hcalIsoToken_;
  NamedToken<double> rhoToken_;
  NamedToken<double> rhoCentralCaloToken_;
  NamedToken<pat::TriggerObjectStandAloneCollection> triggerObjectsToken_;

  EffectiveAreas combIsoEA_;
  EffectiveAreas ecalIsoEA_;
  EffectiveAreas hcalIsoEA_;
  EffectiveAreas phCHIsoEA_;
  EffectiveAreas phNHIsoEA_;
  EffectiveAreas phPhIsoEA_;

  VString hltFilters_;
  double minPt_{-1.};
  double maxEta_{10.};
};

#endif
