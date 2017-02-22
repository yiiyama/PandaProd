#include "../interface/WeightsFiller.h"

#include "FWCore/Framework/interface/Run.h"

#include "PandaTree/Framework/interface/IOUtils.h"

#include "TDOMParser.h"
#include "TXMLNode.h"
#include "TXMLAttr.h"

WeightsFiller::WeightsFiller(std::string const& _name, edm::ParameterSet const& _cfg, edm::ConsumesCollector& _coll) :
  FillerBase(_name, _cfg)
{
  if (!isRealData_) {
    getToken_(genInfoToken_, _cfg, _coll, "common", "genEventInfo");
    getToken_(lheEventToken_, _cfg, _coll, "common", "lheEvent");
    getTokenRun_(lheRunToken_, _cfg, _coll, "common", "lheRun");
    saveSignalWeights_ = getParameter_<bool>(_cfg, "signalWeights");
  }
}

void
WeightsFiller::branchNames(panda::utils::BranchList& _eventBranches, panda::utils::BranchList&) const
{
  _eventBranches.emplace_back("weight");
  if (!isRealData_) {
    _eventBranches.emplace_back("genReweight");
    // Turned off temporarily
    _eventBranches.push_back("!genReweight.genParam");
  }
}

void
WeightsFiller::addOutput(TFile& _outputFile)
{
  if (isRealData_)
    hSumW_ = new TH1D("hSumW", "SumW", 1, 0., 1.);
  else
    hSumW_ = new TH1D("hSumW", "SumW", 8, 0., 8.);

  hSumW_->SetDirectory(&_outputFile);

  hSumW_->GetXaxis()->SetBinLabel(1, "Nominal");
  if (!isRealData_) {
    char const* labels[] = {
      "r1f2",
      "r1f5",
      "r2f1",
      "r2f2",
      "r5f1",
      "r5f5",
      "pdf"
    };

    for (unsigned iL(0); iL != sizeof(labels) / sizeof(char const*); ++iL)
      hSumW_->GetXaxis()->SetBinLabel(iL + 2, labels[iL]);

    if (saveSignalWeights_) {
        // groupTree_ = new TTree("weightGroups", "weightGroups");
        // gcombine_ = new TString;
        // gtype_ = new TString;
        // groupTree_->Branch("combine", "TString", &gcombine_);
        // groupTree_->Branch("type", "TString", &gtype_);

        weightTree_ = new TTree("weights", "weights");
        wid_ = new TString;
        weightTree_->Branch("id", "TString", &wid_); // currently only have the ability to save ID
        // wtitle_ = new TString;
        // weightTree_->Branch("title", "TString", &wtitle_);
        // weightTree_->Branch("group", &gid_, "group/i");
    }
  }

  eventTree_ = static_cast<TTree*>(_outputFile.Get("events"));
}

void
WeightsFiller::fill(panda::Event& _outEvent, edm::Event const& _inEvent, edm::EventSetup const&)
{
  if (isRealData_) {
    _outEvent.weight = 1.;
    return;
  }

  auto& genInfo(getProduct_(_inEvent, genInfoToken_));
  _outEvent.weight = genInfo.weight();

  auto* lheEvent(getProductSafe_(_inEvent, lheEventToken_));
  if (lheEvent) {
    double weights[7]{};
    bool firstEvent = (nSignalWeights_<0);
    
    getLHEWeights_(*lheEvent, weights, _outEvent.genReweight.genParam);
    
    if (firstEvent && eventTree_!=0)
      eventTree_->Branch("genReweight.genParam",_outEvent.genReweight.genParam,
                      TString::Format("genReweight.genParam[%i]",nSignalWeights_));

    _outEvent.genReweight.r1f2DW = weights[0] / lheEvent->originalXWGTUP() - 1.;
    _outEvent.genReweight.r1f5DW = weights[1] / lheEvent->originalXWGTUP() - 1.;
    _outEvent.genReweight.r2f1DW = weights[2] / lheEvent->originalXWGTUP() - 1.;
    _outEvent.genReweight.r2f2DW = weights[3] / lheEvent->originalXWGTUP() - 1.;
    _outEvent.genReweight.r5f1DW = weights[4] / lheEvent->originalXWGTUP() - 1.;
    _outEvent.genReweight.r5f5DW = weights[5] / lheEvent->originalXWGTUP() - 1.;
    _outEvent.genReweight.pdfDW = weights[6] / lheEvent->originalXWGTUP() - 1.;
  }
}

void
WeightsFiller::fillAll(edm::Event const& _inEvent, edm::EventSetup const&)
{
  if (isRealData_) {
    hSumW_->Fill(0.5);
    return;
  }

  auto& genInfo(getProduct_(_inEvent, genInfoToken_));
  hSumW_->Fill(0.5, genInfo.weight());

  auto* lheEvent(getProductSafe_(_inEvent, lheEventToken_));
  if (lheEvent) {
    double weights[7];
    getLHEWeights_(*lheEvent, weights);

    // PDF variation will always be greater than nominal by construction
    for (unsigned iW(0); iW != 7; ++iW)
      hSumW_->Fill(iW + 1.5, weights[iW]);
  }
}

void
WeightsFiller::fillEndRun(panda::Run& _outRun, edm::Run const& _inRun, edm::EventSetup const&)
{
  // Set weight names
  // Update this function too if changing the set of weights to save

  // LHERunInfoProduct (and GenRunInfoProduct FWIW) have mergeProduct method which forbids them
  // from being fetched in beginRun

  return; // something is wrong with the LHE header in signal files. fill the tree a different way

  if (isRealData_ || !saveSignalWeights_)
    return;

  auto* lheRun(getProductSafe_(_inRun, lheRunToken_));
  if (!lheRun) {
    return;
  }

  std::map<TString, unsigned> weightGroups; // use TStrings to cover non-int IDs
  std::map<TString, TString> weightTitles;

  // Parse the LHE run header and find the initrwgt block
  for (auto&& hItr(lheRun->headers_begin()); hItr != lheRun->headers_end(); ++hItr) {
    auto& lheHdr(*hItr);

    if (lheHdr.tag() != "initrwgt")
      continue;

    // xml document needs a root node
    TString buffer("<initrwgt>");

    // append lines stripping whitespaces and newlines
    for (auto&& line : lheHdr.lines()) {
      TString text(line);
      text = text.Strip(TString::kBoth, ' ');
      text = text.Strip(TString::kBoth, '\n');
      buffer += text;
    }

    // clean up the buffer - at least remove trailing open bracket (yes that happens sometimes)
    int len(buffer.Length());
    for (; len != 0; --len) {
      if (buffer[len - 1] == '>')
        break;
    }
    buffer = buffer(0, len);

    buffer += "</initrwgt>";


    TDOMParser parser;
    parser.SetValidate(false); // we don't define XML namespace etc.
    parser.ParseBuffer(buffer.Data(), buffer.Length());
    auto& document(*parser.GetXMLDocument());
    auto& rootnode(*document.GetRootNode());

    unsigned iG(-1);

    // traverse through the XML document and collect all reweight factor names
    auto* wgNode(rootnode.GetChildren());
    do {
      // ignore non-weightgroup nodes
      if (std::strcmp(wgNode->GetNodeName(), "weightgroup") != 0)
        continue;

      ++iG;

      // assume the first block is for scale variation
      if (iG == 0)
        continue;

      // loop over weightgroup nodes

      auto* weightNode(wgNode->GetChildren());
      do {
        // loop over weight definition nodes

        if (std::strcmp(weightNode->GetNodeName(), "weight") != 0)
          continue;

        TList* attr(weightNode->GetAttributes());
        if (!attr)
          continue;

        auto* wid(static_cast<TXMLAttr*>(attr->FindObject("id")));
        if (!wid)
          continue;

        TString id(wid->GetValue());

        weightGroups[id] = iG - 1; // first block is skipped

        // weight definition is a content of the child (text) node
        auto* contentNode = weightNode->GetChildren();
        if (!contentNode || contentNode->GetNodeType() != TXMLNode::kXMLTextNode)
          continue;

        TString content(contentNode->GetContent());
        content.Strip(TString::kBoth);

        weightTitles[id] = content;
      }
      while ((weightNode = weightNode->GetNextNode()));

      // weightgroup attributes
      TList& attributes(*wgNode->GetAttributes());
          
      auto* combine(static_cast<TXMLAttr*>(attributes.FindObject("combine")));
      if (combine)
        *gcombine_ = combine->GetValue();
      else
        *gcombine_ = "";

      auto* type(static_cast<TXMLAttr*>(attributes.FindObject("type")));
      if (type)
        *gtype_ = type->GetValue();
      else
        *gtype_ = "";

      groupTree_->Fill();
    }
    while ((wgNode = wgNode->GetNextNode()));

    // we don't need to look at any other LHE headers
    // break;
  }

  for (auto& idtitle : weightTitles) {
    *wid_ = idtitle.first;
    *wtitle_ = idtitle.second;
    gid_ = weightGroups.at(*wid_);
    if (TString::Itoa(wid_->Atoi(),10)==*wid_) {
      weightTree_->Fill();
    }
  }
}

void
WeightsFiller::getLHEWeights_(LHEEventProduct const& _lheEvent, double _weights[7], float _params[])
{
  // Update this function if changing the set of weights to be save

  double sumd2(0.);
  unsigned genParamCounter(0);

  for (auto& wgt : _lheEvent.weights()) {
    unsigned id(0);
    try {
      id =std::stoi(wgt.id);
    }
    catch (std::invalid_argument& ex) {
      if (saveSignalWeights_ && _params!=0) {
        _params[genParamCounter++] = wgt.wgt/_lheEvent.originalXWGTUP(); 
        if (nSignalWeights_<0) {
          *wid_ = wgt.id.c_str();
          weightTree_->Fill();
        }
      }
      continue;
    }

    if ((id >= 1 && id <= 9) || (id >= 1001 && id <= 1009)) {
      switch (id % 1000) {
      case 2:
        _weights[0] = wgt.wgt;
        break;
      case 3:
        _weights[1] = wgt.wgt;
        break;
      case 4:
        _weights[2] = wgt.wgt;
        break;
      case 5:
        _weights[3] = wgt.wgt;
        break;
      case 7:
        _weights[4] = wgt.wgt;
        break;
      case 9:
        _weights[5] = wgt.wgt;
        break;
      default:
        break;
      }
    }
    else if ((id >= 11 && id <= 110) || (id >= 2001 && id < 2100)) {
      double d(wgt.wgt - _lheEvent.originalXWGTUP());
      sumd2 += d * d;
    }
  }

  _weights[6] = std::sqrt(sumd2 / 99.) + _lheEvent.originalXWGTUP();

  if (saveSignalWeights_ && nSignalWeights_<0 && _params!=0)
    nSignalWeights_ = genParamCounter;
}

DEFINE_TREEFILLER(WeightsFiller);
