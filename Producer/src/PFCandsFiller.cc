#include "../interface/PFCandsFiller.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

#include "PandaProd/Auxiliary/interface/PackedValuesExposer.h"

PFCandsFiller::PFCandsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(candidatesToken_, _cfg, _coll, "common", "pfCandidates");
<<<<<<< HEAD
  getToken_(puppiToken_, _cfg, _coll, "puppi", false);
  getToken_(puppiNoLepToken_, _cfg, _coll, "puppiNoLep", false);
=======
  getToken_(puppiMapToken_, _cfg, _coll, "puppiMap", false);
  getToken_(puppiNoLepMapToken_, _cfg, _coll, "puppiNoLepMap", false);
  getToken_(verticesToken_, _cfg, _coll, "common", "vertices");
>>>>>>> branch-003-devel
}

void
PFCandsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("pfCandidates");
}

void
PFCandsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
<<<<<<< HEAD
  edm::Handle<reco::CandidateView> candsHandle;
  auto& inCands(getProduct_(_inEvent, candidatesToken_, &candsHandle));

  // i-th element of the vector is a puppi candidate whose sourceCandidatePtr->key is i
  std::vector<reco::CandidatePtr> puppiPtrMap;
  std::vector<reco::CandidatePtr> puppiNoLepPtrMap;

  if (!puppiToken_.second.isUninitialized() && !puppiNoLepToken_.second.isUninitialized()) {
    auto& inPuppi(getProduct_(_inEvent, puppiToken_));
    unsigned iC(0);
    for (auto& cand : inPuppi) {
      auto&& sourcePtr(cand.sourceCandidatePtr(0));
      if (sourcePtr.id() == candsHandle.id()) {
        if (sourcePtr.key() >= puppiPtrMap.size())
          puppiPtrMap.resize(sourcePtr.key() + 1);

        puppiPtrMap[sourcePtr.key()] = inPuppi.ptrAt(iC);
      }
      else
        std::cerr << "source of puppi is not PF candidates: " << sourcePtr.id() << " " << candsHandle.id() << std::endl;
      ++iC;
    }

    auto& inPuppiNoLep(getProduct_(_inEvent, puppiNoLepToken_));
    iC = 0;
    for (auto& cand : inPuppiNoLep) {
      auto&& sourcePtr(cand.sourceCandidatePtr(0));
      if (sourcePtr.id() == candsHandle.id()) {
        if (sourcePtr.key() >= puppiNoLepPtrMap.size())
          puppiNoLepPtrMap.resize(sourcePtr.key() + 1);

        puppiNoLepPtrMap[sourcePtr.key()] = inPuppiNoLep.ptrAt(iC);
      }
      else
        std::cerr << "source of puppiNoLep is not PF candidates: " << sourcePtr.id() << " " << candsHandle.id() << std::endl;

      ++iC;
    }
  }
=======
  auto& inCands(getProduct_(_inEvent, candidatesToken_));
  FloatMap const* inPuppiMap(0);
  if (!puppiMapToken_.second.isUninitialized())
    inPuppiMap = &getProduct_(_inEvent, puppiMapToken_);
  FloatMap const* inPuppiNoLepMap(0);
  if (!puppiNoLepMapToken_.second.isUninitialized())
    inPuppiNoLepMap = &getProduct_(_inEvent, puppiNoLepMapToken_);
  auto& inVertices(getProduct_(_inEvent, verticesToken_));
>>>>>>> branch-003-devel

  auto& outCands(_outEvent.pfCandidates);

  std::vector<reco::CandidatePtr> ptrList;

  unsigned iP(-1);
  for (auto& inCand : inCands) {
    ++iP;

    auto* inPacked(dynamic_cast<pat::PackedCandidate const*>(&inCand));

    auto& outCand(outCands.create_back());

    if (inPacked) {
      // directly fill the packed values to minimize the precision loss
      PackedPatCandidateExposer exposer(*inPacked);
      outCand.packedPt = exposer.packedPt();
      outCand.packedEta = exposer.packedEta();
      outCand.packedPhi = exposer.packedPhi();
      outCand.packedM = exposer.packedM();
      if (puppiPtrMap.empty())
        outCand.packedPuppiW = exposer.packedPuppiweight();
      if (puppiNoLepPtrMap.empty())
        outCand.packedPuppiWNoLepDiff = exposer.packedPuppiweightNoLepDiff();

      auto vtxRef(inPacked->vertexRef());
      if (vtxRef.isNonnull())
        outCand.vertex.idx() = vtxRef.key();
      else // in reality this seems to never happen
        outCand.vertex.idx() = -1;
    }
    else {
      fillP4(outCand, inCand);
      outCand.vertex.idx() = -1;
    }

    // if puppi collection is given, use its weight
    if (!puppiPtrMap.empty() && !puppiNoLepPtrMap.empty()) {
      auto& puppiCand(puppiPtrMap[iP]);
      auto& puppiNoLepCand(puppiNoLepPtrMap[iP]);

      double puppiW(puppiCand.isNonnull() ? puppiCand->pt() / inCand.pt() : 0.);
      double puppiNoLepW(puppiNoLepCand.isNonnull() ? puppiNoLepCand->pt() / inCand.pt() : 0.);

      if (puppiCand.isNull())
        std::cerr << "puppi not found for key " << iP << std::endl;
      if (puppiNoLep.isNull())
        std::cerr << "puppiNoLep not found for key " << iP << std::endl;
      
      outCand.setPuppiW(puppiW, puppiNoLepW);
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

  auto ByVertexAndPt([](panda::Element const& e1, panda::Element const& e2)->Bool_t {
      auto& p1(static_cast<panda::PFCand const&>(e1));
      auto& p2(static_cast<panda::PFCand const&>(e2));
      if (p1.vertex.idx() == p2.vertex.idx())
        return p1.pt() > p2.pt();
      else
        return unsigned(p1.vertex.idx()) < unsigned(p2.vertex.idx());
    });

  // sort the output electrons
  auto originalIndices(outCands.sort(ByVertexAndPt));

  // make reco <-> panda mapping
  auto& objectMap(objectMap_->get<reco::Candidate, panda::PFCand>());
  auto& puppiMap(objectMap_->get<reco::Candidate, panda::PFCand>("puppi"));
  
  for (unsigned iP(0); iP != outCands.size(); ++iP) {
    auto& outCand(outCands[iP]);
    unsigned idx(originalIndices[iP]);
    objectMap.add(ptrList[idx], outCand);
    auto& puppiPtr(puppiPtrMap[idx]);
    if (puppiPtr.isNonnull())
      puppiMap.add(puppiPtr, outCand);
  }

  outCandidates_ = &outCands;

  orderedVertices_.resize(inVertices.size());
  for (unsigned iV(0); iV != inVertices.size(); ++iV)
    orderedVertices_[iV] = inVertices.ptrAt(iV);
}

void
PFCandsFiller::setRefs(ObjectMapStore const& _objectMaps)
{
  auto& vtxMap(_objectMaps.at("vertices").get<reco::Vertex, panda::RecoVertex>().fwdMap);

  unsigned nVtx(orderedVertices_.size());

  unsigned iVtx(0);
  unsigned iPF(0);
  for (auto& cand : *outCandidates_) {
    unsigned idx(cand.vertex.idx());

    while (idx != iVtx && iVtx != nVtx) { // first candidate of the next vertex
      vtxMap.at(orderedVertices_[iVtx])->pfRangeMax = iPF;
      ++iVtx;
    }

    if (iVtx == nVtx)
      break;

    ++iPF;
  }

  while (iVtx != nVtx) {
    vtxMap.at(orderedVertices_[iVtx])->pfRangeMax = iPF;
    ++iVtx;
  }
}

DEFINE_TREEFILLER(PFCandsFiller);
