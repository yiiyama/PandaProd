#ifndef PandaProd_Producer_Filler_h
#define PandaProd_Producer_Filler_h

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "PandaTree/Objects/interface/Event.h"
#include "PandaTree/Objects/interface/Run.h"
#include "ObjectMap.h"

#include "TFile.h"

#include "tbb/concurrent_unordered_map.h"

class FillerBase {
 public:
  FillerBase(std::string const& fillerName) : fillerName_(fillerName) {}
  virtual ~FillerBase() {}

  virtual void addOutput(TFile&) {}
  virtual void fill(panda::Event&, edm::Event const&, edm::EventSetup const&, ObjectMapStore&) = 0;
  virtual void setRefs(ObjectMapStore const&) {}
  virtual void fillRun(panda::Run&, edm::Run const&, edm::EventSetup const&) {}

  std::string const& getName() const { return fillerName_; }

 protected:
  typedef std::vector<std::string> VString;

  template<class Product, edm::BranchType B = edm::InEvent>
  void getToken_(edm::EDGetTokenT<Product>&, edm::ParameterSet const&, edm::ConsumesCollector&, std::string const& pname, bool mandatory = true);

  template<class Product>
  Product const& getProduct_(edm::Event const&, edm::EDGetTokenT<Product> const&, std::string name);

  std::string fillerName_;
};

template<class Product, edm::BranchType B/* = edm::Event*/>
void
FillerBase::getToken_(edm::EDGetTokenT<Product>& _token, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll, std::string const& _pname, bool _mandatory/* = true*/)
{
  std::string paramValue;
  
  if (_mandatory)
    paramValue = _cfg.getUntrackedParameter<std::string>(_pname);
  else
    paramValue = _cfg.getUntrackedParameter<std::string>(_pname, "");

  if (paramValue.empty()) {
    if (_mandatory)
      throw edm::Exception(edm::errors::Configuration, fillerName_ + "::getToken_()")
        << "Missing or empty parameter " << _pname;
    else
      _token = edm::EDGetTokenT<Product>();
  }
 
  edm::InputTag tag(paramValue);
  _token = _coll.consumes<Product, B>(tag);
}

template<class Product>
Product const&
FillerBase::getProduct_(edm::Event const& _event, edm::EDGetTokenT<Product> const& _token, std::string _name)
{
  edm::Handle<Product> handle;
  if (!_event.getByToken(_token, handle))
    throw cms::Exception("ProductNotFound") << _name;
  return *handle;
}

void fillP4(panda::PParticle&, reco::Candidate const&);
void fillP4(panda::PParticleM&, reco::Candidate const&);

//--------------------------------------------------------------------------------------------------
// FillerFactory: a trick to register individual fillers as "plugin modules"
//--------------------------------------------------------------------------------------------------

typedef std::function<FillerBase*(std::string const&, edm::ParameterSet const&, edm::ConsumesCollector&)> FillerFactory;
            //typedef FillerBase* (*FillerFactory)(std::string const& name, edm::ParameterSet const&, edm::ConsumesCollector&);

// A singleton class to store information of the filler plugins
class FillerFactoryStore {
 public:
  // A utility class whose instantiation triggers the registration of a filler plugin
  template<class Filler>
  struct Registration {
    Registration(char const* _name)
    {
      singleton()->registerFactory(_name,
      [](std::string const& name, edm::ParameterSet const& cfg, edm::ConsumesCollector& collector)->FillerBase*
      {
        return new Filler(name, cfg, collector);
      });
    }
  };

  // Register a FillerFactory (filler generator function) under a given name
  void registerFactory(std::string const& _name, FillerFactory _f) { fillerFactories_[_name] = _f; }

  // Retrieve the FillerFactory and instantiate the filler
  FillerBase* makeFiller(std::string const& className, std::string const& fillerName, edm::ParameterSet const&, edm::ConsumesCollector&) const;

  static FillerFactoryStore* singleton();

 private:
  tbb::concurrent_unordered_map<std::string, FillerFactory> fillerFactories_;
};

// Should be moved to a utilities library once format conversion is done
namespace panda {
  extern ContainerBase::Comparison ptGreater;
}

// A macro that instantiates FillerFactoryStore::Registration for the class FILLER
#define DEFINE_TREEFILLER(FILLER) \
  FillerFactoryStore::Registration<FILLER> panda##FILLER##Registration(#FILLER)

#endif
