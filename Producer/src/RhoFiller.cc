#include "../interface/RhoFiller.h"

RhoFiller::RhoFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(rhoToken_, _cfg, _coll, "rho");
  getToken_(rhoCentralCaloToken_, _cfg, _coll, "rhoCentralCalo");
}

void
RhoFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup)
{
  _outEvent.rho = getProduct_(_inEvent, rhoToken_);
  _outEvent.rhoCentralCalo = getProduct_(_inEvent, rhoToken_);
}

DEFINE_TREEFILLER(RhoFiller);
