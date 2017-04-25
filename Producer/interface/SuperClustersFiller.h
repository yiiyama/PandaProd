#ifndef PandaProd_Producer_SuperClustersFiller_h
#define PandaProd_Producer_SuperClustersFiller_h

#include "FillerBase.h"
#include "SCMIPHaloTagger.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

class SuperClustersFiller : public FillerBase {
 public:
  SuperClustersFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~SuperClustersFiller() {}

  void branchNames(panda::utils::BranchList& eventBranches, panda::utils::BranchList&) const override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;

 protected:
  typedef edm::View<reco::SuperCluster> SuperClusterView;
  typedef edm::View<pat::PackedCandidate> PackedCandidateView;

  NamedToken<SuperClusterView> superClustersToken_;
  NamedToken<EcalRecHitCollection> ebHitsToken_;
  NamedToken<EcalRecHitCollection> eeHitsToken_;
  NamedToken<PackedCandidateView> pfCandidatesToken_;

  SCMIPHaloTagger mipTagger_;
};

#endif
