#include "../interface/VerticesFiller.h"

#include "DataFormats/VertexReco/interface/Vertex.h"

VerticesFiller::VerticesFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name)
{
  getToken_(verticesToken_, _cfg, _coll, "vertices");
  if (!_cfg.getUntrackedParameter<bool>("isRealData"))
    getToken_(puSummariesToken_, _cfg, _coll, "puSummaries");
}

void
VerticesFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup, ObjectMapStore&)
{
  auto& inVertices(getProduct_(_inEvent, verticesToken_, "vertices"));

  _outEvent.npv = 0;
  for (auto& vtx : inVertices) {
    if (vtx.ndof() < 4)
      continue;
    if (std::abs(vtx.z()) > 24.)
      continue;
    if (vtx.position().rho() > 2.)
      continue;

    ++_outEvent.npv;
  }

  if (!_inEvent.isRealData()) {
    auto& puSummaries(getProduct_(_inEvent, puSummariesToken_, "puSummaries"));
    for (auto& pu : puSummaries) {
      if (pu.getBunchCrossing() == 0) {
        _outEvent.npvTrue = pu.getTrueNumInteractions();
        break;
      }
    }
  }
}

DEFINE_TREEFILLER(VerticesFiller);
