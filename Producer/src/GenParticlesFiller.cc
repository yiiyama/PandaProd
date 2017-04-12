#include "../interface/GenParticlesFiller.h"

#include "DataFormats/Common/interface/RefToPtr.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"

#include "PandaProd/Auxiliary/interface/PackedValuesExposer.h"
#include "PandaTree/Utils/interface/PNode.h"

typedef edm::Ptr<reco::GenParticle> GenPtr;

typedef ObjectMap<reco::GenParticle, panda::GenParticle> GenParticleMap;

unsigned counter(0);

struct PNodeWithGenPtr : public PNode {
  GenPtr genPtr{};

  PNodeWithGenPtr(GenPtr const& _ptr, std::map<GenPtr, PNodeWithGenPtr*>& _nodeMap, PNode* _mother = 0) {
    auto& inCand(*_ptr);
    pdgId = inCand.pdgId();
    status = inCand.status();
    statusBits = inCand.statusFlags().flags_;
    mass = inCand.mass();
    pt = inCand.pt();
    eta = inCand.eta();
    phi = inCand.phi();
    mother = _mother;

    ownDaughters = false;

    genPtr = _ptr;

    _nodeMap[genPtr] = this;

    for (auto& dref : inCand.daughterRefVector()) {
      GenPtr dptr(edm::refToPtr(dref));

      auto nItr(_nodeMap.find(dptr));

      if (nItr != _nodeMap.end()) {
        // this node is already constructed

        PNode* dnode(nItr->second);
        PNode* dmother(dnode->mother);

        if (dmother) {
          // and it's someone's daughter

          if (dmother == this) // mine!?
            continue;

          bool takeCustody(false);
          if (!isHadronic() && dmother->isHadronic())
            takeCustody = true;
          else if (isHadronic() && !dmother->isHadronic())
            takeCustody = false;
          else
            takeCustody = reco::deltaR2(eta, phi, dnode->eta, dnode->phi) < reco::deltaR2(dmother->eta, dmother->phi, dnode->eta, dnode->phi);

          if (takeCustody) {
            dnode->mother = this;
            daughters.push_back(dnode);
            std::vector<PNode*>::iterator dItr(std::find(dmother->daughters.begin(), dmother->daughters.end(), dnode));
            if (dItr != dmother->daughters.end()) // can happen that this daughter is still being pushed into mother's daughter list
              dmother->daughters.erase(dItr);
          }
        }
        else {
          dnode->mother = this;
          daughters.push_back(dnode);
        }
      }
      else
        daughters.push_back(new PNodeWithGenPtr(dptr, _nodeMap, this));
    }
  }

  void fillPanda(panda::GenParticleCollection& _outParticles, GenParticleMap& _map, int parentIdx = -1) const {
    auto& outParticle(_outParticles.create_back());
    int myidx(_outParticles.size() - 1);

    fillP4(outParticle, *genPtr);

    outParticle.pdgid = pdgId;
    outParticle.finalState = (status == 1);
    outParticle.statusFlags = statusBits.to_ulong();
    outParticle.parent.idx() = parentIdx;

    _map.add(genPtr, outParticle);

    for (auto* d : daughters)
      static_cast<PNodeWithGenPtr*>(d)->fillPanda(_outParticles, _map, myidx);
  }
};

GenParticlesFiller::GenParticlesFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  getToken_(genParticlesToken_, _cfg, _coll, "common", "genParticles");
}

void
GenParticlesFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("genParticles");
}

void
GenParticlesFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  auto& inParticles(getProduct_(_inEvent, genParticlesToken_));

  auto& outParticles(_outEvent.genParticles);

  auto& objectMap(objectMap_->get<reco::GenParticle, panda::GenParticle>());

  std::map<GenPtr, PNodeWithGenPtr*> nodeMap;

  for (unsigned iP(0); iP != inParticles.size(); ++iP) {
    auto& inCand(inParticles.at(iP));
    if (inCand.motherRefVector().size() == 0) {
      auto* rootNode(new PNodeWithGenPtr(inParticles.ptrAt(iP), nodeMap));
      rootNode->pruneDaughters();
      rootNode->fillPanda(outParticles, objectMap);
    }
  }

  // ownDaughter is false; need to clean up pnodes
  for (auto& node : nodeMap)
    delete node.second;
}

DEFINE_TREEFILLER(GenParticlesFiller);
