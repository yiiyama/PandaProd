#include "../interface/VerticesFiller.h"

#include "DataFormats/VertexReco/interface/Vertex.h"

VerticesFiller::VerticesFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(verticesToken_, _cfg, _coll, "vertices");
  if (!isRealData_) {
    getToken_(puSummariesToken_, _cfg, _coll, "puSummaries");
  }
}

void
VerticesFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  if (isRealData_)
    _eventBranches.push_back("!npvTrue");
}

void
VerticesFiller::addOutput(TFile& _outputFile)
{
  TDirectory::TContext context(&_outputFile);
  hNPVReco_ = new TH1D("hNPVReco", "N_{PV}^{reco}", 100, 0., 100.);
  if (!isRealData_)
    hNPVTrue_ = new TH1D("hNPVTrue", "N_{PV}^{true}", 100, 0., 100.);
}

void
VerticesFiller::fill(panda::Event& _outEvent, edm::Event const&, edm::EventSetup const&)
{
  _outEvent.npv = npvCache_;
  if (!isRealData_)
    _outEvent.npvTrue = npvTrueCache_;
}

void
VerticesFiller::fillAll(edm::Event const& _inEvent, edm::EventSetup const&)
{
  auto& inVertices(getProduct_(_inEvent, verticesToken_));

  npvCache_ = 0;
  for (auto& vtx : inVertices) {
    if (vtx.ndof() < 4)
      continue;
    if (std::abs(vtx.z()) > 24.)
      continue;
    if (vtx.position().rho() > 2.)
      continue;

    ++npvCache_;
  }

  hNPVReco_->Fill(npvCache_);

  if (!isRealData_) {
    auto& puSummaries(getProduct_(_inEvent, puSummariesToken_));
    for (auto& pu : puSummaries) {
      if (pu.getBunchCrossing() == 0) {
        npvTrueCache_ = pu.getTrueNumInteractions();
        hNPVTrue_->Fill(npvTrueCache_);
        break;
      }
    }
  }
}

DEFINE_TREEFILLER(VerticesFiller);
