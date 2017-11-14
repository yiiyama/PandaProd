import FWCore.ParameterSet.Config as cms

from PandaProd.Producer.utils.addattr import AddAttr

def chsSequence(process, pfSource, skipIfExists = False):
    """
    Return a Sequence with PF CHS.
    """

    sequence = cms.Sequence()

    addattr = AddAttr(process, sequence)

    # Charged hadron subtraction
    addattr('pfCHS',
        cms.EDFilter("CandPtrSelector",
            src = cms.InputTag(pfSource),
            cut = cms.string("fromPV")
        ),
        skipIfExists = skipIfExists
    )

    return sequence
