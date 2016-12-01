#include "../interface/TausFiller.h"

#include "DataFormats/PatCandidates/interface/Tau.h"

TausFiller::TausFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name)
{
  auto& fillerCfg(_cfg.getUntrackedParameterSet("fillers").getUntrackedParameterSet(_name));

  getToken_(tausToken_, _cfg, _coll, "taus");

  minPt_ = fillerCfg.getUntrackedParameter<double>("minPt", -1.);
  maxEta_ = fillerCfg.getUntrackedParameter<double>("maxEta", 10.);
}

void
TausFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup, ObjectMapStore& _objectMaps)
{
  auto& inTaus(getProduct_(_inEvent, tausToken_, "taus"));

  auto& outTaus(_outEvent.taus);

  std::vector<edm::Ptr<reco::BaseTau>> ptrList;

  for (auto& inTau : inTaus) {
    if (inTau.pt() < minPt_)
      continue;
    if (std::abs(inTau.eta()) > maxEta_)
      continue;

    auto& outTau(outTaus.create_back());

    fillP4(outTau, inTau);

    if (dynamic_cast<pat::Tau const*>(&inTau)) {
      auto& patTau(static_cast<pat::Tau const&>(inTau));
      outTau.decayMode = patTau.tauID("decayModeFinding") > 0.5;
      outTau.decayModeNew = patTau.tauID("decayModeFindingNewDMs") > 0.5;
      outTau.isoDeltaBetaCorr = patTau.tauID("byCombinedIsolationDeltaBetaCorrRaw3Hits");
      outTau.iso = 0.;
      for (auto&& cand : patTau.isolationGammaCands())
        outTau.iso += cand->pt();
      for (auto&& cand : patTau.isolationChargedHadrCands())
        outTau.iso += cand->pt();
      for (auto&& cand : patTau.isolationNeutrHadrCands())
        outTau.iso += cand->pt();
    }
  }

  auto originalIndices(outTaus.sort(panda::ptGreater));

  // export panda <-> reco mapping

  auto& objectMap(_objectMaps.get<reco::BaseTau, panda::PTau>("taus"));

  for (unsigned iP(0); iP != outTaus.size(); ++iP) {
    auto& outTau(outTaus[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outTau);
  }
}

DEFINE_TREEFILLER(TausFiller);
