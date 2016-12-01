#include "../interface/MuonsFiller.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

MuonsFiller::MuonsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name)
{
  auto& fillerCfg(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name));

  getToken_(muonsToken_, _cfg, _coll, "muons");
  getToken_(verticesToken_, _cfg, _coll, "vertices");

  if (useTrigger_) {
    getToken_(triggerObjectsToken_, _cfg, _coll, "triggerObjects");
    hltFilters_ = fillerCfg.getUntrackedParameter<VString>("hltFilters");
    if (hltFilters_.size() != panda::nMuonHLTObjects)
      throw edm::Exception(edm::errors::Configuration, "MuonsFiller")
        << "muonHLTFilters.size()";
  }

  minPt_ = fillerCfg.getUntrackedParameter<double>("minPt", -1.);
  maxEta_ = fillerCfg.getUntrackedParameter<double>("maxEta", 10.);
}

void
MuonsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup, ObjectMapStore& _objectMaps)
{
  auto& inMuons(getProduct_(_inEvent, muonsToken_, "muons"));
  auto& vertices(getProduct_(_inEvent, verticesToken_, "vertices"));

  std::vector<pat::TriggerObjectStandAlone const*> hltObjects[panda::nMuonHLTObjects];
  if (useTrigger_) {
    auto& triggerObjects(getProduct_(_inEvent, triggerObjectsToken_, "triggerObjects"));
    for (auto& obj : triggerObjects) {
      for (unsigned iF(0); iF != panda::nMuonHLTObjects; ++iF) {
        if (obj.hasFilterLabel(hltFilters_[iF]))
          hltObjects[iF].push_back(&obj);
      }
    }
  }

  auto& outMuons(_outEvent.muons);

  std::vector<edm::Ptr<reco::Muon>> ptrList;

  unsigned iMu(-1);
  for (auto& inMuon : inMuons) {
    ++iMu;

    if (inMuon.pt() < minPt_)
      continue;
    if (std::abs(inMuon.eta()) > maxEta_)
      continue;

    auto& outMuon(outMuons.create_back());

    fillP4(outMuon, inMuon);

    outMuon.q = inMuon.charge();

    auto& pfIso(inMuon.pfIsolationR04());

    outMuon.chiso = pfIso.sumChargedHadronPt;
    outMuon.nhiso = pfIso.sumNeutralHadronEt;
    outMuon.phoiso = pfIso.sumPhotonEt;
    outMuon.puiso = pfIso.sumPUPt;

    if (dynamic_cast<pat::Muon const*>(&inMuon)) {
      auto& patMuon(static_cast<pat::Muon const&>(inMuon));

      outMuon.loose = patMuon.isLooseMuon();
      outMuon.medium = patMuon.isMediumMuon();
      if (vertices.size() == 0)
        outMuon.tight = false;
      else
        outMuon.tight = patMuon.isTightMuon(vertices.at(0));
    }
    else {
      outMuon.loose = muon::isLooseMuon(inMuon);
      outMuon.medium = muon::isMediumMuon(inMuon);
      if (vertices.size() == 0)
        outMuon.tight = false;
      else
        outMuon.tight = muon::isTightMuon(inMuon, vertices.at(0));
    }

    if (useTrigger_) {
      for (unsigned iF(0); iF != panda::nMuonHLTObjects; ++iF) {
        for (auto* obj : hltObjects[iF]) {
          if (reco::deltaR(inMuon, *obj) < 0.3) {
            outMuon.matchHLT[iF] = true;
            break;
          }
        }
      }
    }

    ptrList.push_back(inMuons.ptrAt(iMu));
  }
  
  auto originalIndices(outMuons.sort(panda::ptGreater));

  // export panda <-> reco mapping

  auto& objectMap(_objectMaps.get<reco::Muon, panda::PMuon>("muons"));

  for (unsigned iP(0); iP != outMuons.size(); ++iP) {
    auto& outMuon(outMuons[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outMuon);
  }
}

DEFINE_TREEFILLER(MuonsFiller);
