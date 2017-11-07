#include "../interface/SecondaryVerticesFiller.h"

SecondaryVerticesFiller::SecondaryVerticesFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{

  // These are different from VerticesFiller
  getToken_(secondaryVerticesToken_, _cfg, _coll, "source");

}

void
SecondaryVerticesFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList& _runBranches) const
{
  _eventBranches.emplace_back(getName());
}

void
SecondaryVerticesFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{

  auto& inSVs(getProduct_(_inEvent, secondaryVerticesToken_));
  auto& outSVs(_outEvent.secondaryVertices);

  auto& objMap(objectMap_->get<reco::VertexCompositePtrCandidate, panda::SecondaryVertex>());

  unsigned iVtx(0);
  for (auto& inSV : inSVs) {

    auto& outSV(outSVs.create_back());
    fillP4(outSV, inSV);

    outSV.x = inSV.vx();
    outSV.y = inSV.vy();
    outSV.z = inSV.vz();

    outSV.ndof = inSV.vertexNdof();
    outSV.chi2 = inSV.vertexNormalizedChi2();
    outSV.ntrk = inSV.numberOfDaughters();

    auto ptr(inSVs.ptrAt(iVtx++));
    objMap.add(ptr, outSV);

  }

}

DEFINE_TREEFILLER(SecondaryVerticesFiller);
