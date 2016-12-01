#include "../interface/WeightsFiller.h"

WeightsFiller::WeightsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name)
{
  if (!_cfg.getUntrackedParameter<bool>("isRealData"))
    getToken_(genInfoToken_, _cfg, _coll, "genEventInfo");
}

void
WeightsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const& _setup, ObjectMapStore&)
{
  if (_inEvent.isRealData())
    _outEvent.weight = 1.;
  else {
    auto& genInfo(getProduct_(_inEvent, genInfoToken_, "genEventInfo"));
    _outEvent.weight = genInfo.weight();
  }
}

DEFINE_TREEFILLER(WeightsFiller);
