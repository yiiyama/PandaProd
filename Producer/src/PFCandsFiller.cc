#include "../interface/PFCandsFiller.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

#include "PandaProd/Auxiliary/interface/PackedValuesExposer.h"

PFCandsFiller::PFCandsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(candidatesToken_, _cfg, _coll, "common", "pfCandidates");
  getToken_(puppiMapToken_, _cfg, _coll, "puppiMap", false);
  getToken_(puppiNoLepMapToken_, _cfg, _coll, "puppiNoLepMap", false);
}

void
PFCandsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("pfCandidates");
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
    auto&& ptr(inCands.ptrAt(iP));

    auto* inPacked(dynamic_cast<pat::PackedCandidate const*>(&inCand));

    auto& outCand(outCands.create_back());

    if (inPacked) {
      // directly fill the packed values to minimize the precision loss
      PackedPatCandidateExposer exposer(*inPacked);
      outCand.packedPt = exposer.packedPt();
      outCand.packedEta = exposer.packedEta();
      outCand.packedPhi = exposer.packedPhi();
      outCand.packedM = exposer.packedM();
      if (!inPuppiMap)
        outCand.packedPuppiW = exposer.packedPuppiweight();
      if (!inPuppiNoLepMap)
        outCand.packedPuppiWNoLepDiff = exposer.packedPuppiweightNoLepDiff();
    }
    else
      fillP4(outCand, inCand);

    // if puppi collection is given, use its weight
    if (inPuppiMap && inPuppiNoLepMap)
    {
      //outCand.setPuppiW((*inPuppiMap)[ref], (*inPuppiNoLepMap)[ref]);
      fprintf(stderr,"puppi no lep weight=%.3f\n",(*inPuppiNoLepMap)[ref]);
      fprintf(stderr,"puppi weight=%.3f\n",(*inPuppiMap)[ref]);
      outCand.setPuppiW((*inPuppiMap)[ref], (*inPuppiNoLepMap)[ref]);

    }

    outCand.ptype = panda::PFCand::X;
    unsigned ptype(0);
    while (ptype != panda::PFCand::nPTypes) {
      if (panda::PFCand::pdgId_[ptype] == inCand.pdgId()) {
        outCand.ptype = ptype;
        break;
      }
      ++ptype;
    }

    ptrList.push_back(inCands.ptrAt(iP));
  }

  // sort the output electrons
  auto originalIndices(outCands.sort(panda::Particle::PtGreater));

  // make reco <-> panda mapping
  auto& objectMap(objectMap_->get<reco::Candidate, panda::PFCand>());
  
  for (unsigned iP(0); iP != outCands.size(); ++iP) {
    auto& outCand(outCands[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outCand);
  }
}

DEFINE_TREEFILLER(PFCandsFiller);
