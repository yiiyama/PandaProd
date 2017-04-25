#include "../interface/SuperClustersFiller.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include <cmath>

SuperClustersFiller::SuperClustersFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(superClustersToken_, _cfg, _coll, "superClusters");
  getToken_(ebHitsToken_, _cfg, _coll, "ebHits");
  getToken_(eeHitsToken_, _cfg, _coll, "eeHits");
  getToken_(pfCandidatesToken_, _cfg, _coll, "common", "pfCandidates");

  mipTagger_.setup(getParameter_<edm::ParameterSet>(_cfg, "mipTagger"));
}

void
SuperClustersFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back(getName());
}

void
SuperClustersFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inSuperClusters(getProduct_(_inEvent, superClustersToken_));
  auto& ebHits(getProduct_(_inEvent, ebHitsToken_));
  auto& eeHits(getProduct_(_inEvent, eeHitsToken_));
  auto& pfCandidates(getProduct_(_inEvent, pfCandidatesToken_));

  std::vector<pat::PackedCandidate const*> chargedCandidates;
  for (auto& cand : pfCandidates) {
    if (cand.charge() != 0 && cand.vertexRef().isNonnull())
      chargedCandidates.push_back(&cand);
  }

  auto findHit([&ebHits, &eeHits](DetId const& id)->EcalRecHit const* {
      EcalRecHitCollection const* hits(0);
      if (id.subdetId() == EcalBarrel)
        hits = &ebHits;
      else
        hits = &eeHits;

      auto&& hitItr(hits->find(id));
      if (hitItr == hits->end())
        return 0;

      return &*hitItr;
    });

  noZS::EcalClusterLazyTools lazyTools(_inEvent, _setup, ebHitsToken_.second, eeHitsToken_.second);

  panda::SuperClusterCollection* outPtr(0);
  if (getName() == "superClusters")
    outPtr = &_outEvent.superClusters;
  else
    outPtr = &_outEvent.superClustersFT;

  auto& outSuperClusters(*outPtr);

  auto& objectMap(objectMap_->get<reco::SuperCluster, panda::SuperCluster>());

  for (auto& inSC : inSuperClusters) {
    auto& outSC(outSuperClusters.create_back());

    outSC.eta = inSC.eta();
    outSC.phi = inSC.phi();
    outSC.rawPt = inSC.rawEnergy() / std::cosh(outSC.eta);

    auto&& seedRef(inSC.seed());
    if (seedRef.isNonnull()) {
      auto& seed(*seedRef);
      auto&& localCov(lazyTools.localCovariances(seed));

      outSC.sieie = std::sqrt(localCov[0]);
      outSC.sipip = std::sqrt(localCov[2]);
      outSC.e2e9 = (lazyTools.eMax(seed) + lazyTools.e2nd(seed)) / lazyTools.e3x3(seed);

      outSC.emax = lazyTools.eMax(seed);
      outSC.e2nd = lazyTools.e2nd(seed);
      outSC.e4 = lazyTools.eTop(seed) + lazyTools.eRight(seed) + lazyTools.eBottom(seed) + lazyTools.eLeft(seed);

      auto* seedHit(findHit(seed.hitsAndFractions()[0].first));
      if (seedHit)
        outSC.time = seedHit->time();
    }

    outSC.timeSpan = 0.;
    for (auto& hf : inSC.hitsAndFractions()) {
      auto* hit(findHit(hf.first));
      if (!hit || hit->energy() < 1.)
        continue;

      double dt(outSC.time - hit->time());
      if (std::abs(dt) > std::abs(outSC.timeSpan))
        outSC.timeSpan = dt;
    }

    std::map<reco::VertexRef, double> trackIso;

    for (auto* cand : chargedCandidates) {
      auto vtxRef(cand->vertexRef());
      auto& vertex(*vtxRef);

      TVector3 direction;
      direction.SetPtEtaPhi(outSC.rawPt, outSC.eta, outSC.phi);
      direction -= TVector3(vertex.x(), vertex.y(), vertex.z());

      double dEta(direction.Eta() - cand->eta());
      double dPhi(TVector2::Phi_mpi_pi(direction.Phi() - cand->phi()));
      if (dEta * dEta + dPhi * dPhi < 0.09)
        trackIso[vtxRef] += cand->pt();
    }

    outSC.trackIso = -1.;
    for (auto& iso : trackIso)
      if (iso.second > outSC.trackIso)
        outSC.trackIso = iso.second;

    reco::Photon::MIPVariables mipVariables;
    mipTagger_.fillMIPVariables(&inSC, ebHits, mipVariables);

    outSC.mipEnergy = mipVariables.mipTotEnergy;
    outSC.mipChi2 = mipVariables.mipChi2;
    outSC.mipSlope = mipVariables.mipSlope;
    outSC.mipIntercept = mipVariables.mipIntercept;
    outSC.mipNhits = mipVariables.mipNhitCone;
  }

  for (unsigned iSC(0); iSC != inSuperClusters.size(); ++iSC)
    objectMap.add(inSuperClusters.ptrAt(iSC), outSuperClusters[iSC]);
}

DEFINE_TREEFILLER(SuperClustersFiller);
