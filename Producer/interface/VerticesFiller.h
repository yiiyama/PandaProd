#ifndef PandaProd_Producer_VerticesFiller_h
#define PandaProd_Producer_VerticesFiller_h

#include "FillerBase.h"

#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

class VerticesFiller : public FillerBase {
 public:
  VerticesFiller(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&);
  ~VerticesFiller() {}

  void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) override;

 private:
  typedef std::vector<PileupSummaryInfo> PUSummaryCollection;

  edm::EDGetTokenT<reco::VertexCollection> verticesToken_;
  edm::EDGetTokenT<PUSummaryCollection> puSummariesToken_;
};

#endif
