#include "../interface/PFCandsFiller.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/TrackReco/interface/TrackBase.h"

#include "PandaProd/Auxiliary/interface/PackedValuesExposer.h"

PFCandsFiller::PFCandsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(candidatesToken_, _cfg, _coll, "common", "pfCandidates");
  getToken_(puppiMapToken_, _cfg, _coll, "puppiMap", false);
  getToken_(puppiInputToken_, _cfg, _coll, "puppiInput", false);
  getToken_(puppiNoLepMapToken_, _cfg, _coll, "puppiNoLepMap", false);
  getToken_(puppiNoLepInputToken_, _cfg, _coll, "puppiNoLepInput", false);
  getToken_(verticesToken_, _cfg, _coll, "common", "vertices");
}

void
PFCandsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("pfCandidates");
  _eventBranches.emplace_back("tracks");
}

void
PFCandsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  edm::Handle<reco::CandidateView> candsHandle;
  auto& inCands(getProduct_(_inEvent, candidatesToken_, &candsHandle));
  auto& inVertices(getProduct_(_inEvent, verticesToken_));

  std::map<reco::Candidate const*, reco::CandidatePtr> puppiPtrMap;
  std::map<reco::Candidate const*, reco::CandidatePtr> puppiNoLepPtrMap;

  if (!puppiMapToken_.second.isUninitialized() && !puppiNoLepMapToken_.second.isUninitialized()) {
    // connect inCands and the puppi candidates by references to the base collection
    // PuppiProducer produces a ValueMap<CandidatePtr> (ref to input -> puppi candidate)
    // If the input to PuppiProducer is itself a ref collection (e.g. PtrVector), we need
    // to map the refs down to the orignal collection.
    // In more practical terms:
    //   edm::Ref<View>(viewHandle, iview) maps to a puppi candidate via puppiMap
    //   View::refAt(iview).key() is the index of the PF candidate in the original collection

    std::map<reco::CandidatePtr, reco::Candidate const*> inCandsMap;
    for (unsigned iC(0); iC != inCands.size(); ++iC) {
      auto ptrToPF(inCands.ptrAt(iC)); // returns a pointer to the original collection (as opposed to Ref<CandidateView> ref(candsHandle, iC));
      inCandsMap[ptrToPF] = &inCands.at(iC);
    }

    auto& puppiMap(getProduct_(_inEvent, puppiMapToken_));
    edm::Handle<reco::CandidateView> puppiInputHandle;
    auto& puppiInput(getProduct_(_inEvent, puppiInputToken_, &puppiInputHandle));
    for (unsigned iC(0); iC != puppiInput.size(); ++iC) {
      edm::Ref<reco::CandidateView> inputRef(puppiInputHandle, iC);
      auto& puppiPtr(puppiMap[inputRef]);

      auto ptrToPF(puppiInput.ptrAt(iC));
      auto inCandsItr(inCandsMap.find(ptrToPF));
      if (inCandsItr == inCandsMap.end()) {
        // You are here because of a misconfiguration or because the input to puppi had some layer(s) of PF candidate cloning.
        // It may be possible to trace back to the original PF collection through calls to sourceCandidatePtr()
        // but for now we don't need to implement it.
        throw std::runtime_error("Cannot find candidate matching a puppi input");
      }

      puppiPtrMap[inCandsItr->second] = puppiPtr;
    }

    auto& puppiNoLepMap(getProduct_(_inEvent, puppiNoLepMapToken_));
    edm::Handle<reco::CandidateView> puppiNoLepInputHandle;
    auto& puppiNoLepInput(getProduct_(_inEvent, puppiNoLepInputToken_, &puppiNoLepInputHandle));
    for (unsigned iC(0); iC != puppiNoLepInput.size(); ++iC) {
      edm::Ref<reco::CandidateView> inputRef(puppiNoLepInputHandle, iC);
      auto& puppiNoLepPtr(puppiNoLepMap[inputRef]);

      auto ptrToPF(puppiNoLepInput.ptrAt(iC));
      auto inCandsItr(inCandsMap.find(ptrToPF));
      if (inCandsItr == inCandsMap.end()) {
        // See above
        throw std::runtime_error("Cannot find candidate matching a puppiNoLep input");
      }

      puppiNoLepPtrMap[inCandsItr->second] = puppiNoLepPtr;
    }
  }

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
      auto& puppiCand(puppiPtrMap[&inCand]);
      auto& puppiNoLepCand(puppiNoLepPtrMap[&inCand]);

      double puppiW(puppiCand.isNonnull() ? puppiCand->pt() / inCand.pt() : 0.);
      double puppiNoLepW(puppiNoLepCand.isNonnull() ? puppiNoLepCand->pt() / inCand.pt() : 0.);

      // if (puppiCand.isNull())
      //   std::cerr << "puppi not found for key " << iP << std::endl;
      // if (puppiNoLepCand.isNull())
      //   std::cerr << "puppiNoLep not found for key " << iP << std::endl;
      
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

    outCand.hCalFrac = inPacked->hcalFraction();

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
    auto& ptr(ptrList[idx]);
    objectMap.add(ptr, outCand);
    auto& puppiPtr(puppiPtrMap[ptr.get()]);
    if (puppiPtr.isNonnull())
      puppiMap.add(puppiPtr, outCand);

    // add track information for charged hadrons
    // track order matters; track ref from PFCand are set during Event::getEntry relying on the order
    switch (outCand.ptype) {
    case panda::PFCand::hp:
    case panda::PFCand::hm:
    case panda::PFCand::ep:
    case panda::PFCand::em:
    case panda::PFCand::mup:
    case panda::PFCand::mum:
      {
        auto& track(_outEvent.tracks.create_back());
        PackedPatCandidateExposer exposer(static_cast<pat::PackedCandidate const&>(*ptr));

        auto* bestTrack(ptr->bestTrack());
        if (bestTrack) {
          track.setPtError(bestTrack->ptError());
          // Only highPurity is filled in miniAOD, see https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookTrackAnalysis
          track.highPurity = bestTrack->quality(reco::TrackBase::highPurity);
        }
        track.packedDxy = exposer.packedDxy();
        track.packedDz = exposer.packedDz();
        track.packedDPhi = exposer.packedDPhi();
      }
      break;
    default:
      break;
    }
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
