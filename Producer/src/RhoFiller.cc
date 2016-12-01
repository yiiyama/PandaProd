#include "../interface/RhoFiller.h"

RhoFiller::RhoFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name)
{
  getToken_(rhoToken_, _cfg, _coll, "rho");
}

void
RhoFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup, ObjectMapStore&)
{
  _outEvent.rho = getProduct_(_inEvent, rhoToken_, "rho");
}

DEFINE_TREEFILLER(RhoFiller);
