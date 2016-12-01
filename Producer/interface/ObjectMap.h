#ifndef PandaProd_Producer_ObjectMap_h
#define PandaProd_Producer_ObjectMap_h

#include "DataFormats/Common/interface/Ptr.h"
#include "PandaTree/Framework/interface/Object.h"

#include <map>
#include <string>
#include <utility>
#include <typeinfo>

class ObjectMapBase {
 public:
  typedef std::pair<size_t, size_t> TypeIDPair;
  virtual ~ObjectMapBase() {}
  virtual void clear() {};
  virtual TypeIDPair getId() const { return TypeIDPair(); };
};

template<class EDM, class PANDA>
class ObjectMap : public ObjectMapBase {
  typedef edm::Ptr<EDM> EDMPtr;

 public:
  std::map<EDMPtr, PANDA const*> fwdMap;
  std::map<PANDA const*, EDMPtr> bwdMap;

  void clear() override { fwdMap.clear(); bwdMap.clear(); }
  TypeIDPair getId() const override { return TypeIDPair(typeid(EDM).hash_code(), typeid(PANDA).hash_code()); }

  void add(EDMPtr const& edmRef, PANDA const& pandaObj) { fwdMap.emplace(edmRef, &pandaObj); bwdMap.emplace(&pandaObj, edmRef); }
};

class ObjectMapStore {
 public:
  ~ObjectMapStore() { for (auto& idmap : store_) delete idmap.second; }
  template<class EDM, class PANDA> ObjectMap<EDM, PANDA>& get(std::string const&);
  template<class EDM, class PANDA> ObjectMap<EDM, PANDA> const& get(std::string const&) const;
  void clear() { for (auto& idmap : store_) idmap.second->clear(); }

 private:
  std::map<std::string, ObjectMapBase*> store_;
};

template<class EDM, class PANDA>
ObjectMap<EDM, PANDA>&
ObjectMapStore::get(std::string const& _name)
{
  auto sItr(store_.find(_name));
  if (sItr == store_.end()) {
    sItr = store_.emplace(_name, new ObjectMap<EDM, PANDA>).first;
  }
  else {
    if (sItr->second->getId() != ObjectMapBase::TypeIDPair(typeid(EDM).hash_code(), typeid(PANDA).hash_code()))
      throw std::runtime_error("Map type wrong");
  }

  return *static_cast<ObjectMap<EDM, PANDA>*>(sItr->second);
}

template<class EDM, class PANDA>
ObjectMap<EDM, PANDA> const&
ObjectMapStore::get(std::string const& _name) const
{
  auto* map(store_.at(_name));
  if (map->getId() != ObjectMapBase::TypeIDPair(typeid(EDM).hash_code(), typeid(PANDA).hash_code()))
    throw std::runtime_error("Map type wrong");

  return static_cast<ObjectMap<EDM, PANDA> const&>(*map);
}

#endif
