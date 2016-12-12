#include "../interface/GenParticlesFiller.h"

#include "DataFormats/Common/interface/RefToPtr.h"

GenParticlesFiller::GenParticlesFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg),
  minPt_(getParameter_<double>(_cfg, "minPt", -1.))
{
  getToken_(genParticlesToken_, _cfg, _coll, "genParticles");

  auto vPdgIds(getParameter_<VString>(_cfg, "pdgIds"));
  for (auto& idstr : vPdgIds) {
    size_t dash(idstr.find('-'));
    if (dash != std::string::npos) {
      // is a range
      if (dash == 0)
        allPdgBelow_ = std::stoi(idstr.substr(1));
      else if (dash == idstr.size() - 1)
        allPdgAbove_ = std::stoi(idstr.substr(0, dash));
      else {
        unsigned low(std::stoi(idstr.substr(0, dash)));
        unsigned high(std::stoi(idstr.substr(dash + 1)));
        for (unsigned pdg(low); pdg <= high; ++pdg)
          pdgIds_.insert(pdg);
      }
    }
    else
      pdgIds_.insert(std::stoi(idstr));
  }
}

void
GenParticlesFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  if (!enabled())
    _eventBranches.emplace_back("!" + getName());
}

void
GenParticlesFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  auto& inParticles(getProduct_(_inEvent, genParticlesToken_));

  auto& outParticles(_outEvent.genParticles);

  std::vector<edm::Ptr<reco::GenParticle>> ptrList;

  unsigned iP(-1);
  for (auto& inCand : inParticles) {
    ++iP;
    if (inCand.pt() < minPt_)
      continue;

    unsigned absId(std::abs(inCand.pdgId()));
    if (absId > allPdgBelow_ && absId < allPdgAbove_ && pdgIds_.find(absId) == pdgIds_.end())
      continue;

    // photons should be kept only if prompt and final state
    if (absId == 22 && !inCand.isPromptFinalState())
      continue;

    // only keep first or last copy
    if (!inCand.isLastCopy() && !inCand.statusFlags().isFirstCopy())
      continue;

    auto& outParticle(outParticles.create_back());

    fillP4(outParticle, inCand);

    outParticle.pdgid = inCand.pdgId();

    ptrList.push_back(inParticles.ptrAt(iP));
  }

  // sort the output electrons
  auto originalIndices(outParticles.sort(panda::ptGreater));

  // make reco <-> panda mapping
  auto& objectMap(objectMap_->get<reco::GenParticle, panda::PGenParticle>());
  
  for (unsigned iP(0); iP != outParticles.size(); ++iP) {
    auto& outParticle(outParticles[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outParticle);
  }
}

void
GenParticlesFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  auto& map(objectMap_->get<reco::GenParticle, panda::PGenParticle>());

  for (auto& link : map.bwdMap) { // panda -> edm
    auto& outChild(*link.first);
    auto& inChild(link.second);

    std::function<bool(edm::Ptr<reco::GenParticle> const&)> findMother;

    findMother = [&map, &outChild, &findMother](edm::Ptr<reco::GenParticle> const& ptr)->bool {
      for (auto&& mRef : ptr->motherRefVector()) {
        auto&& mPtr(edm::refToPtr(mRef));

        auto&& pItr(map.fwdMap.find(mPtr));
        if (pItr != map.fwdMap.end()) {
          outChild.parent = pItr->second;
          return true;
        }

        if (findMother(mPtr))
          return true;
      }
      return false;
    };
    
    findMother(inChild);
  }
}

DEFINE_TREEFILLER(GenParticlesFiller);
