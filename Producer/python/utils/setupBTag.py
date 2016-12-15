import FWCore.ParameterSet.Config as cms
import RecoBTag.Configuration.RecoBTag_cff as btag
import RecoBTag.CTagging.RecoCTagging_cff as ctag
import RecoVertex.AdaptiveVertexFinder.inclusiveVertexing_cff as vertexing
from RecoBTag.SecondaryVertex.trackSelection_cff import trackSelectionBlock

from PandaProd.Producer.utils.addattr import AddAttr

vertexingConfig = {} # {suffix: (candidates, primaryVertex)}

# Can call for each candidates & PV collection pair to construct secondary vertices from.
# Seems like CMS only uses particleFlow & offlinePrimaryVertices (or their MINIAOD equivalents) pair regardless of what types of jets they are b-tagging.
# So in the end we only call this function once.
def initBTag(process, suffix, candidates = 'particleFlow', primaryVertex = 'offlinePrimaryVertices'):
    if suffix in vertexingConfig:
        if (candidates, primaryVertex) != vertexingConfig[suffix]:
            raise RuntimeError('Duplicate vertexing configuration name')

        return cms.Sequence()

    if len(vertexingConfig) == 0:
        # ESProducers and vertexing sequences
        process.load('RecoBTag.ImpactParameter.impactParameter_EventSetup_cff')
        process.load('RecoBTag.CTagging.cTagging_EventSetup_cff')
        process.load('RecoBTag.SecondaryVertex.secondaryVertex_EventSetup_cff')
        process.load('RecoBTag.Combined.combinedMVA_EventSetup_cff')
        process.load('RecoBTag.SoftLepton.softLepton_EventSetup_cff')

    vertexingConfig[suffix] = (candidates, primaryVertex)

    sequence = cms.Sequence()

    addattr = AddAttr(process, sequence, suffix)

    vertexFinder = addattr('inclusiveCandidateVertexFinder',
        vertexing.inclusiveCandidateVertexFinder.clone(
            primaryVertices = primaryVertex,
            tracks = candidates
        )
    )

    vertexMerger = addattr('candidateVertexMerger',
        vertexing.candidateVertexMerger.clone(
            secondaryVertices = vertexFinder
        )
    )

    vertexArbitrator = addattr('candidateVertexArbitrator',
        vertexing.candidateVertexArbitrator.clone(
            primaryVertices = primaryVertex,
            tracks = candidates,
            secondaryVertices = vertexMerger
        )
    )

    secondaryVertices = addattr('inclusiveCandidateSecondaryVertices',
        vertexing.inclusiveCandidateSecondaryVertices.clone(
            secondaryVertices = vertexArbitrator
        )
    )

    vertexFinderCvsL = addattr('inclusiveCandidateVertexFinderCvsL',
        vertexing.inclusiveCandidateVertexFinderCvsL.clone(
            primaryVertices = primaryVertex,
            tracks = candidates
        )
    )

    vertexMergerCvsL = addattr('candidateVertexMergerCvsL',
        vertexing.candidateVertexMergerCvsL.clone(
            secondaryVertices = vertexFinderCvsL
        )
    )

    vertexArbitratorCvsL = addattr('candidateVertexArbitratorCvsL',
        vertexing.candidateVertexArbitratorCvsL.clone(
            primaryVertices = primaryVertex,
            tracks = candidates,
            secondaryVertices = vertexMergerCvsL
        )
    )

    secondaryVerticesCvsL = addattr('inclusiveCandidateSecondaryVerticesCvsL',
        vertexing.inclusiveCandidateSecondaryVerticesCvsL.clone(
            secondaryVertices = 'candidateVertexArbitratorCvsL' + suffix
        )
    )

    return sequence


# used in both setupBTag and setupDoubleBTag
ipTagInfosNameBase = 'pfImpactParameterTagInfos'
ivfTagInfosNameBase = 'pfInclusiveSecondaryVertexFinderTagInfos'

def makeIpTagInfos(jetCollectionName, vsuffix, deltaR = 0.4):
    return btag.pfImpactParameterTagInfos.clone(
        jets = jetCollectionName,
        candidates = vertexingConfig[vsuffix][0],
        primaryVertex = vertexingConfig[vsuffix][1],
        maxDeltaR = deltaR
    )

def makeIvfTagInfos(ipTagInfosName, vsuffix, deltaR = 0.3):
    # default 0.3 is correct
    return btag.pfInclusiveSecondaryVertexFinderTagInfos.clone(
        extSVCollection = 'inclusiveCandidateSecondaryVertices' + vsuffix,
        extSVDeltaRToJet = deltaR,
        trackIPTagInfos = ipTagInfosName
    )


# Give the list of btag discriminators (see below for names) in tags to run only a part of the full menu.
# vsuffix is the suffix given to initBTag that defines the secondary vertexing sequence.
# The optional argument addedTagInfos can be used to retrieve back the TagInfo modules added in order to compute the specified btag discriminators.
def setupBTag(process, jetCollection, suffix, vsuffix, muons = 'muons', electrons = 'gedGsfElectrons', tags = [], addedTagInfos = []):
    """
    Configure the BTag sequence for the given jet collection.
    The suffix will be appended to the CMSSW module names to
    distinguish sequences for multiple collections.
    """

    # btag info
    ipTagInfosName = ipTagInfosNameBase + suffix
    svTagInfosName = 'pfSecondaryVertexTagInfos' + suffix
    ivfTagInfosName = ivfTagInfosNameBase + suffix
    smTagInfosName = 'softPFMuonsTagInfos' + suffix
    seTagInfosName = 'softPFElectronsTagInfos' + suffix
    # ctag info
    ivfcvslTagInfosName = 'pfInclusiveSecondaryVertexFinderCvsLTagInfos' + suffix

    # modules below will be later setattr'ed to process using the names above
    impactParameterTagInfos = makeIpTagInfos(jetCollection, vsuffix)
    secondaryVertexTagInfos = btag.pfSecondaryVertexTagInfos.clone(
        trackIPTagInfos = ipTagInfosName
    )
    inclusiveSecondaryVertexFinderTagInfos = makeIvfTagInfos(ipTagInfosName, vsuffix)
    softPFMuonsTagInfos = btag.softPFMuonsTagInfos.clone(
        jets = jetCollection,
        muons = muons,
        primaryVertex = vertexingConfig[vsuffix][1]
    )
    softPFElectronsTagInfos = btag.softPFElectronsTagInfos.clone(
        jets = jetCollection,
        electrons = electrons,
        primaryVertex = vertexingConfig[vsuffix][1]
    )
    inclusiveSecondaryVertexFinderCvsLTagInfos = btag.pfInclusiveSecondaryVertexFinderCvsLTagInfos.clone(
        extSVCollection = 'inclusiveCandidateSecondaryVerticesCvsL' + vsuffix,
        trackIPTagInfos = ipTagInfosName
    )

    # impact parameter b-tags
    trackCountingHighEffBJetTags = btag.pfTrackCountingHighEffBJetTags.clone(
        tagInfos = [cms.InputTag(ipTagInfosName)]
    )

    # jet probability b-tags
    jetProbabilityBJetTags = btag.pfJetProbabilityBJetTags.clone(
        tagInfos = [cms.InputTag(ipTagInfosName)]
    )
    jetBProbabilityBJetTags = btag.pfJetBProbabilityBJetTags.clone(
        tagInfos = [cms.InputTag(ipTagInfosName)]
    )

    # secondary vertex b-tags
    simpleSecondaryVertexHighEffBJetTags = btag.pfSimpleSecondaryVertexHighEffBJetTags.clone(
        tagInfos = [cms.InputTag(svTagInfosName)]
    )

    # inclusive secondary vertex b-tags
    simpleInclusiveSecondaryVertexHighEffBJetTags = btag.pfSimpleInclusiveSecondaryVertexHighEffBJetTags.clone(
        tagInfos = [cms.InputTag(ivfTagInfosName)]
    )

    # combined secondary vertex b-tags
    combinedSecondaryVertexV2BJetTags = btag.pfCombinedSecondaryVertexV2BJetTags.clone(
        tagInfos = [cms.InputTag(ipTagInfosName), cms.InputTag(svTagInfosName)]
    )

    # inclusive combined secondary vertex b-tags
    combinedInclusiveSecondaryVertexV2BJetTags = btag.pfCombinedInclusiveSecondaryVertexV2BJetTags.clone(
        tagInfos = [cms.InputTag(ipTagInfosName), cms.InputTag(ivfTagInfosName)]
    )

    # soft lepton b-tags
    softPFMuonBJetTags = btag.softPFMuonBJetTags.clone(
        tagInfos = [cms.InputTag(smTagInfosName)]
    )
    softPFElectronBJetTags = btag.softPFElectronBJetTags.clone(
        tagInfos = [cms.InputTag(seTagInfosName)]
    )

    # super-combined b-tags
    combinedMVAV2BJetTags = btag.pfCombinedMVAV2BJetTags.clone(
        tagInfos = [
            cms.InputTag(ipTagInfosName),
            cms.InputTag(svTagInfosName),
            cms.InputTag(ivfTagInfosName),
            cms.InputTag(smTagInfosName),
            cms.InputTag(seTagInfosName)
        ]
    )

    # ctags
    combinedCvsLJetTags = ctag.pfCombinedCvsLJetTags.clone(
        tagInfos = [
            cms.InputTag(ipTagInfosName),
            cms.InputTag(ivfcvslTagInfosName),
            cms.InputTag(smTagInfosName),
            cms.InputTag(seTagInfosName)
        ]
    )
    combinedCvsBJetTags = ctag.pfCombinedCvsBJetTags.clone(
        tagInfos = [
            cms.InputTag(ipTagInfosName),
            cms.InputTag(ivfcvslTagInfosName),
            cms.InputTag(smTagInfosName),
            cms.InputTag(seTagInfosName)
        ]
    )

    # create sequence. Use a recursive function to resolve the dependencies
    dependencies = {
        'pfTrackCountingHighEffBJetTags': (trackCountingHighEffBJetTags, [ipTagInfosName]),
        'pfJetProbabilityBJetTags': (jetProbabilityBJetTags, [ipTagInfosName]),
        'pfJetBProbabilityBJetTags': (jetBProbabilityBJetTags, [ipTagInfosName]),
        'pfSimpleSecondaryVertexHighEffBJetTags': (simpleSecondaryVertexHighEffBJetTags, [ipTagInfosName, svTagInfosName]),
        'pfCombinedSecondaryVertexV2BJetTags': (combinedSecondaryVertexV2BJetTags, [ipTagInfosName, svTagInfosName]),
        'pfSimpleInclusiveSecondaryVertexHighEffBJetTags': (simpleInclusiveSecondaryVertexHighEffBJetTags, [ipTagInfosName, ivfTagInfosName]),
        'pfCombinedInclusiveSecondaryVertexV2BJetTags': (combinedInclusiveSecondaryVertexV2BJetTags, [ipTagInfosName, ivfTagInfosName]),
        'softPFMuonBJetTags': (softPFMuonBJetTags, [smTagInfosName]),
        'softPFElectronBJetTags': (softPFElectronBJetTags, [seTagInfosName]),
        'pfCombinedMVAV2BJetTags': (combinedMVAV2BJetTags, [ipTagInfosName, svTagInfosName, ivfTagInfosName, smTagInfosName, seTagInfosName]),
        'pfCombinedCvsLJetTags': (combinedCvsLJetTags, [ipTagInfosName, ivfcvslTagInfosName, smTagInfosName, seTagInfosName]),
        'pfCombinedCvsBJetTags': (combinedCvsBJetTags, [ipTagInfosName, ivfcvslTagInfosName, smTagInfosName, seTagInfosName])
    }

    tagInfos = {
        ipTagInfosName: (impactParameterTagInfos, []),
        svTagInfosName: (secondaryVertexTagInfos, [ipTagInfosName]),
        ivfTagInfosName: (inclusiveSecondaryVertexFinderTagInfos, [ipTagInfosName]),
        ivfcvslTagInfosName: (inclusiveSecondaryVertexFinderCvsLTagInfos, [ipTagInfosName]),
        smTagInfosName: (softPFMuonsTagInfos, []),
        seTagInfosName: (softPFElectronsTagInfos, []),
    }

    def addTagInfo(name, sequence):
        tagInfo, dependency = tagInfos[name]

        for depName in dependency:
            addTagInfo(depName, sequence)

        try:
            sequence.index(tagInfo)
        except:            
            sequence += tagInfo
            setattr(process, name, tagInfo)
            addedTagInfos.append(name)

    sequence = cms.Sequence()

    if len(tags) == 0:
        tags = dependencies.keys()

    for tagName in tags:
        tag, dependency = dependencies[tagName]

        for tagInfoName in dependency:
            addTagInfo(tagInfoName, sequence)

        sequence += tag
        setattr(process, tagName + suffix, tag)

    return sequence


def setupDoubleBTag(process, jetCollection, suffix, vsuffix, algo, addedTagInfos = []):
    if algo == 'ak8':
        deltaR = 0.8
        maxSVDeltaRToJet = 0.7
        weightsFile = 'BoostedDoubleSV_AK8_BDT_v3.weights.xml.gz'
    elif algo == 'ca15':
        deltaR = 1.5
        maxSVDeltaRToJet = 1.3
        weightsFile = 'BoostedDoubleSV_CA15_BDT_v3.weights.xml.gz'
    else:
        raise RuntimeError('Unknown algo ' + algo)

    boostedDoubleSecondaryVertexComputer = cms.ESProducer("CandidateBoostedDoubleSecondaryVertexESProducer",
        trackSelectionBlock,
        beta = cms.double(1.0),
        useCondDB = cms.bool(False),
        weightFile = cms.FileInPath('RecoBTag/SecondaryVertex/data/' + weightsFile),
        useGBRForest = cms.bool(True),
        useAdaBoost = cms.bool(False),
        trackPairV0Filter = cms.PSet(k0sMassWindow = cms.double(0.03))
    )

    boostedDoubleSecondaryVertexComputer.maxSVDeltaRToJet = cms.double(maxSVDeltaRToJet)
    boostedDoubleSecondaryVertexComputer.trackSelection.jetDeltaRMax = cms.double(deltaR)
    boostedDoubleSecondaryVertexComputer.R0 = cms.double(deltaR)

    setattr(process, 'boostedDoubleSecondaryVertexComputer' + suffix, boostedDoubleSecondaryVertexComputer)

    ipTagInfosName = ipTagInfosNameBase + suffix
    try:
        impactParameterTagInfos = getattr(process, ipTagInfosName)
    except AttributeError:
        impactParameterTagInfos = makeIpTagInfos(jetCollection, vsuffix, deltaR = deltaR)
        impactParameterTagInfos.computeProbabilities = False
        impactParameterTagInfos.computeGhostTrack = False
        setattr(process, ipTagInfosName, impactParameterTagInfos)

    ivfTagInfosName = ivfTagInfosNameBase + suffix
    try:
        inclusiveSecondaryVertexFinderTagInfos = getattr(process, ivfTagInfosName)
    except AttributeError:
        # deltaR for 0.4 jet is 0.3, but for fat jets is same as the jet radius
        inclusiveSecondaryVertexFinderTagInfos = makeIvfTagInfos(ipTagInfosName, vsuffix, deltaR = deltaR)
        inclusiveSecondaryVertexFinderTagInfos.trackSelection.jetDeltaRMax = deltaR
        inclusiveSecondaryVertexFinderTagInfos.vertexCuts.maxDeltaRToJetAxis = deltaR
        setattr(process, ivfTagInfosName, inclusiveSecondaryVertexFinderTagInfos)

    boostedDoubleSecondaryVertexBJetTags = cms.EDProducer("JetTagProducer",
        jetTagComputer = cms.string('boostedDoubleSecondaryVertexComputer' + suffix),
        tagInfos = cms.VInputTag(cms.InputTag(ipTagInfosName), cms.InputTag(ivfTagInfosName))
    )

    setattr(process, 'pfBoostedDoubleSecondaryVertexBJetTags' + suffix, boostedDoubleSecondaryVertexBJetTags)

    sequence = cms.Sequence(
        impactParameterTagInfos +
        inclusiveSecondaryVertexFinderTagInfos +
        boostedDoubleSecondaryVertexBJetTags
    )

    return sequence
