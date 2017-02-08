#include "../interface/TausFiller.h"

#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/Math/interface/deltaR.h"

TausFiller::TausFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.)),
  maxEta_(getParameter_<double>(_cfg, "maxEta", 10.))
{
  getToken_(tausToken_, _cfg, _coll, "taus");
  if (!isRealData_)
    getToken_(genParticlesToken_, _cfg, _coll, "common", "genParticles");
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
      outTau.looseIsoMVA = patTau.tauID("byVLooseIsolationMVArun2v1DBnewDMwLT") > 0.5;
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

  std::vector<edm::Ptr<reco::GenParticle>> genTaus;
  if (!isRealData_) {
    auto& genParticles(getProduct_(_inEvent, genParticlesToken_));
    unsigned iG(0);
    for (auto& gen : genParticles) {
      if (std::abs(gen.pdgId()) == 15 && gen.isLastCopy())
        genTaus.emplace_back(genParticles.ptrAt(iG));
      ++iG;
    }
  } 

  auto& objectMap(objectMap_->get<reco::BaseTau, panda::Tau>());
  auto& genTauMap(objectMap_->get<reco::GenParticle, panda::Tau>());

  for (unsigned iP(0); iP != outTaus.size(); ++iP) {
    auto& outTau(outTaus[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outTau);

    if (!isRealData_) {
      for (auto& genPtr : genTaus) {
        if (reco::deltaR(*genPtr, *ptrList[idx]) < 0.3) {
          genTauMap.add(genPtr, outTau);
          break;
        }
      }
    }
  }
}

void
TausFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  if (!isRealData_) {
    auto& genTauMap(objectMap_->get<reco::GenParticle, panda::Tau>());

    auto& genMap(_objectMaps.at("genParticles").get<reco::GenParticle, panda::GenParticle>().fwdMap);

    for (auto& link : genTauMap.bwdMap) {
      auto& genPtr(link.second);
      if (genMap.find(genPtr) == genMap.end())
        continue;

      auto& outTau(*link.first);
      outTau.matchedGen = genMap.at(genPtr);
    }
  }
}

DEFINE_TREEFILLER(TausFiller);
