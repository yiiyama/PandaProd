#ifndef PandaProd_Auxiliary_PackedValuesExposer_h
#define PandaProd_Auxiliary_PackedValuesExposer_h

#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"

//! Expose packed values which have no public accessor but are protected members
class PackedPatCandidateExposer : public pat::PackedCandidate {
 public:
  PackedPatCandidateExposer(pat::PackedCandidate const& cand) : pat::PackedCandidate(cand) {}
  uint16_t packedPt() const { return packedPt_; }
  uint16_t packedEta() const { return packedEta_; }
  uint16_t packedPhi() const { return packedPhi_; }
  uint16_t packedM() const { return packedM_; }
};

//! Expose packed values which have no public accessor but are protected members
class PackedGenParticleExposer : public pat::PackedGenParticle {
 public:
  PackedGenParticleExposer(pat::PackedGenParticle const& part) : pat::PackedGenParticle(part) {}
  uint16_t packedPt() const { return packedPt_; }
  uint16_t packedY() const { return packedY_; }
  uint16_t packedPhi() const { return packedPhi_; }
  uint16_t packedM() const { return packedM_; }
};

#endif
