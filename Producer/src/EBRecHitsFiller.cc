#include "../interface/EBRecHitsFiller.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"

#include <cmath>

EBRecHitsFiller::EBRecHitsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(ebHitsToken_, _cfg, _coll, "ebHits");
}

void
EBRecHitsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back(getName());
}

void
EBRecHitsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  edm::Handle<EcalRecHitCollection> handle;
  auto& inEBRecHits(getProduct_(_inEvent, ebHitsToken_, &handle));

  auto& outEBRecHits(_outEvent.ebRecHits);
  outEBRecHits.reserve(inEBRecHits.size());

  auto& objectMap(objectMap_->get<EcalRecHit, panda::EBRecHit>());

  unsigned iRH(-1);
  for (auto& inRH : inEBRecHits) {
    ++iRH;

    auto& outRH(outEBRecHits.create_back());

    EBDetId id(inRH.detid());

    outRH.energy = inRH.energy();
    outRH.time = inRH.time();
    outRH.timeError = inRH.timeError();
    outRH.ieta = id.ieta();
    outRH.iphi = id.iphi();

    objectMap.add(edm::Ptr<EcalRecHit>(handle, iRH), outRH);
  }
}

void
EBRecHitsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  std::map<uint32_t, panda::EBRecHit*> idHitMap;

  auto& hitMap(objectMap_->get<EcalRecHit, panda::EBRecHit>().fwdMap);
  for (auto& link : hitMap)
    idHitMap[link.first->detid().rawId()] = link.second;

  auto& scMap(_objectMaps.at("superClusters").get<reco::SuperCluster, panda::SuperCluster>().fwdMap);
  auto& scftMap(_objectMaps.at("superClustersFT").get<reco::SuperCluster, panda::SuperCluster>().fwdMap);

  for (auto& link : scMap) {
    for (auto&& idfrac : link.first->hitsAndFractions()) {
      auto&& hitItr(idHitMap.find(idfrac.first.rawId()));
      if (hitItr != idHitMap.end())
        hitItr->second->superCluster.setRef(link.second);
    }
  }

  for (auto& link : scftMap) {
    for (auto&& idfrac : link.first->hitsAndFractions()) {
      auto&& hitItr(idHitMap.find(idfrac.first.rawId()));
      if (hitItr != idHitMap.end())
        hitItr->second->superClusterFT.setRef(link.second);
    }
  }
}

DEFINE_TREEFILLER(EBRecHitsFiller);
