#ifndef PandaProd_Producer_SecondaryVerticesFiller_h
#define PandaProd_Producer_SecondaryVerticesFiller_h

#include "FillerBase.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidate.h"

class SecondaryVerticesFiller : public FillerBase {
 public:
  SecondaryVerticesFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~SecondaryVerticesFiller() {}

  void branchNames(panda::utils::BranchList&, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;
  void setRefs(ObjectMapStore const&) override;

 protected:

  typedef edm::View<reco::VertexCompositePtrCandidate> SecondaryVertexView;
  NamedToken<SecondaryVertexView> secondaryVerticesToken_;

};

#endif
