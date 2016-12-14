#include "../interface/PFCandsFiller.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

PFCandsFiller::PFCandsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(candidatesToken_, _cfg, _coll, "common", "pfCandidates");
  getToken_(puppiMapToken_, _cfg, _coll, "puppiMap", false);
  getToken_(puppiNoLepMapToken_, _cfg, _coll, "puppiNoLepMap", false);
}

void
PFCandsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  auto& inCands(getProduct_(_inEvent, candidatesToken_));
  FloatMap const* inPuppiMap(0);
  if (!puppiMapToken_.second.isUninitialized())
    inPuppiMap = &getProduct_(_inEvent, puppiMapToken_);
  FloatMap const* inPuppiNoLepMap(0);
  if (!puppiNoLepMapToken_.second.isUninitialized())
    inPuppiNoLepMap = &getProduct_(_inEvent, puppiNoLepMapToken_);

  auto& outCands(_outEvent.pfCandidates);

  std::vector<edm::Ptr<reco::Candidate>> ptrList;

  unsigned iP(-1);
  for (auto& inCand : inCands) {
    ++iP;

    auto&& ref(inCands.refAt(iP));

    auto* inPacked(dynamic_cast<pat::PackedCandidate const*>(&inCand));

    auto& outCand(outCands.create_back());

    fillP4(outCand, inCand);

    outCand.q = inCand.charge();
    outCand.pftype = inCand.pdgId();

    if (inPuppiMap) {
      // if puppi collection is given, use its weight
      outCand.puppiW = (*inPuppiMap)[ref];
    }
    else if (inPacked) {
      // just fill what's stored in packed candidate
      outCand.puppiW = inPacked->puppiWeight();
    }

    if (inPuppiNoLepMap)
      outCand.puppiWNoLep = (*inPuppiNoLepMap)[ref];
    else if (inPacked)
      outCand.puppiWNoLep = inPacked->puppiWeightNoLep();

    ptrList.push_back(inCands.ptrAt(iP));
  }

  // sort the output electrons
  auto originalIndices(outCands.sort(panda::ptGreater));

  // make reco <-> panda mapping
  auto& objectMap(objectMap_->get<reco::Candidate, panda::PFCand>());
  
  for (unsigned iP(0); iP != outCands.size(); ++iP) {
    auto& outCand(outCands[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outCand);
  }
}

DEFINE_TREEFILLER(PFCandsFiller);
