#ifndef PandaProd_Producer_VerticesFiller_h
#define PandaProd_Producer_VerticesFiller_h

#include "FillerBase.h"

#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

#include "TH1D.h"

class VerticesFiller : public FillerBase {
 public:
  VerticesFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~VerticesFiller() {}

  void branchNames(panda::utils::BranchList&, panda::utils::BranchList&) const override;
  void addOutput(TFile&) override;
  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&) override;
  void fillAll(edm::Event const&, edm::EventSetup const&) override;

 protected:
  typedef std::vector<PileupSummaryInfo> PUSummaryCollection;

  NamedToken<reco::VertexCollection> verticesToken_;
  NamedToken<PUSummaryCollection> puSummariesToken_;

  TH1D* hNPVReco_{0};
  TH1D* hNPVTrue_{0};

  //! fillAll and fill will collect identical information -> cache it in fillAll
  unsigned short npvCache_{0};
  unsigned short npvTrueCache_{0};
};

#endif
