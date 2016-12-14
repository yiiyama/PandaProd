#include "../interface/TausFiller.h"

#include "DataFormats/PatCandidates/interface/Tau.h"

TausFiller::TausFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.)),
  maxEta_(getParameter_<double>(_cfg, "maxEta", 10.))
{
  getToken_(tausToken_, _cfg, _coll, "taus");
}

void
TausFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  if (isRealData_) {
    char const* genBranches[] = {
      ".matchedGen_"
    };
    for (char const* b : genBranches)
      _eventBranches.push_back("!" + getName() + b);
  }
}

void
TausFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  auto& inTaus(getProduct_(_inEvent, tausToken_));

  auto& outTaus(_outEvent.taus);

  std::vector<edm::Ptr<reco::BaseTau>> ptrList;

  unsigned iTau(-1);
  for (auto& inTau : inTaus) {
    ++iTau;
    if (inTau.pt() < minPt_)
      continue;
    if (std::abs(inTau.eta()) > maxEta_)
      continue;

    auto& outTau(outTaus.create_back());

    fillP4(outTau, inTau);

    outTau.q = inTau.charge();

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

    ptrList.push_back(inTaus.ptrAt(iTau));
  }

  auto originalIndices(outTaus.sort(panda::ptGreater));

  // export panda <-> reco mapping

  auto& objectMap(objectMap_->get<reco::BaseTau, panda::Tau>());

  for (unsigned iP(0); iP != outTaus.size(); ++iP) {
    auto& outTau(outTaus[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outTau);
  }
}

DEFINE_TREEFILLER(TausFiller);
