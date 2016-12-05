#include "../interface/FillerBase.h"

FillerBase::FillerBase(std::string const& _fillerName, edm::ParameterSet const& _cfg) :
  fillerName_(_fillerName),
  isRealData_(getGlobalParameter_<bool>(_cfg, "isRealData")),
  useTrigger_(getGlobalParameter_<bool>(_cfg, "useTrigger"))
{
}

void
fillP4(panda::PParticle& _out, reco::Candidate const& _in)
{
  _out.pt = _in.pt();
  _out.eta = _in.eta();
  _out.phi = _in.phi();
}

void
fillP4(panda::PParticleM& _out, reco::Candidate const& _in)
{
  _out.pt = _in.pt();
  _out.eta = _in.eta();
  _out.phi = _in.phi();
  _out.mass = _in.mass();
}

FillerBase*
FillerFactoryStore::makeFiller(std::string const& _className, std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _collector) const
{
  try {
    return fillerFactories_.at(_className)(_name, _cfg, _collector);
  }
  catch (std::out_of_range& ex) {
    return 0;
  }
}

/*static*/
FillerFactoryStore*
FillerFactoryStore::singleton()
{
  static FillerFactoryStore fillerFactoryStore;
  return &fillerFactoryStore;
}

//! sort comparison by pt
/*!
  This function takes ContainerElements as arguments to conform with ContainerBase::Comparison
  but assumes that both arguments are static_castable to PParticle.
*/
namespace panda {
  ContainerBase::Comparison ptGreater([](ContainerElement const& p1, ContainerElement const& p2)->bool {
    return static_cast<PParticle const&>(p1).pt > static_cast<PParticle const&>(p2).pt;
  });
}
