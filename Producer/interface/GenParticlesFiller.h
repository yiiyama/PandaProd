#ifndef PandaProd_Producer_GenParticlesFiller_h
#define PandaProd_Producer_GenParticlesFiller_h

#include "FillerBase.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

class GenParticlesFiller : public FillerBase {
 public:
  GenParticlesFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~GenParticlesFiller() {}

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  typedef edm::View<reco::GenParticle> GenParticleView;

  NamedToken<GenParticleView> genParticlesToken_;
};

#endif
