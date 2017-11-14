import FWCore.ParameterSet.Config as cms

class AddAttr(object):
    def __init__(self, process, sequence, postfix = ''):
        self.process = process
        self.sequence = sequence
        self.postfix = postfix
        self.last = None

    def __call__(self, name, attr, skipIfExists = False):
        fullname = name + self.postfix
        tag = cms.InputTag(fullname)

        if hasattr(self.process, fullname):
            if skipIfExists:
                return tag
            else:
                raise RuntimeError('process already has an attribute named ' + fullname)

        setattr(self.process, fullname, attr)
        self.sequence += attr

        self.last = attr

        return tag
