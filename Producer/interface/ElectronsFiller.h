#ifndef PandaProd_Producer_ElectronsFiller_h
#define PandaProd_Producer_ElectronsFiller_h

#include "FillerBase.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"
#include "DataFormats/EgammaCandidates/interface/ConversionFwd.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

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
  typedef edm::View<reco::Vertex> VertexView;
  typedef edm::View<reco::Photon> PhotonView;
  typedef edm::View<reco::GsfElectron> GsfElectronView;
  typedef edm::ValueMap<float> FloatMap;

  NamedToken<GsfElectronView> electronsToken_;
  NamedToken<PhotonView> photonsToken_;
  NamedToken<reco::ConversionCollection> conversionsToken_;
  NamedToken<reco::CandidateView> pfCandidatesToken_;
  NamedToken<VertexView> verticesToken_;
  NamedToken<EcalRecHitCollection> ebHitsToken_;
  NamedToken<EcalRecHitCollection> eeHitsToken_;
  NamedToken<reco::BeamSpot> beamSpotToken_;
  NamedToken<double> rhoToken_;
  NamedToken<double> rhoCentralCaloToken_;
  // Iso tokens only used if filling from AOD
  NamedToken<FloatMap> ecalIsoToken_;
  NamedToken<FloatMap> hcalIsoToken_;

  std::string vetoIdName_;
  std::string looseIdName_;
  std::string mediumIdName_;
  std::string tightIdName_;
  std::string hltIdName_;
  std::string mvaWP90Name_;
  std::string mvaWP80Name_;
  std::string mvaWPLooseName_;
  std::string mvaIsoWP90Name_;
  std::string mvaIsoWP80Name_;
  std::string mvaIsoWPLooseName_;
  std::string mvaValuesName_;
  //std::string mvaCategoriesName_;

  EffectiveAreas combIsoEA_;
  EffectiveAreas ecalIsoEA_;
  EffectiveAreas hcalIsoEA_;
};

#endif
