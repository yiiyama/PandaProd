#include "../interface/PFCandsFiller.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

PFCandsFiller::PFCandsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(candidatesToken_, _cfg, _coll, "candidates");
}

void
PFCandsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  auto& inCands(getProduct_(_inEvent, candidatesToken_));

  auto& outCands(_outEvent.pfCandidates);

  std::vector<edm::Ptr<reco::Candidate>> ptrList;

  unsigned iP(-1);
  for (auto& inCand : inCands) {
    ++iP;

    auto& outCand(outCands.create_back());

    fillP4(outCand, inCand);

    outCand.q = inCand.charge();
    outCand.pftype = inCand.pdgId();
    if (dynamic_cast<pat::PackedCandidate const*>(&inCand)) {
      auto& patCand(static_cast<pat::PackedCandidate const&>(inCand));
      outCand.puppiW = patCand.puppiWeight();
      outCand.puppiWNoLep = patCand.puppiWeightNoLep();
    }

    ptrList.push_back(inCands.ptrAt(iP));
  }

  // sort the output electrons
  auto originalIndices(outCands.sort(panda::ptGreater));

  // make reco <-> panda mapping
  auto& objectMap(objectMap_->get<reco::Candidate, panda::PPFCand>());
  
  for (unsigned iP(0); iP != outCands.size(); ++iP) {
    auto& outCand(outCands[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outCand);
  }
}

DEFINE_TREEFILLER(PFCandsFiller);
