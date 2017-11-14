import FWCore.ParameterSet.Config as cms

class AddAttr(object):
    def __init__(self, process, sequence, postfix = ''):
        self.process = process
        self.sequence = sequence
        self.postfix = postfix
        self.last = None

    def __call__(self, name, attr, skipIfExists = False):
        fullname = name + self.postfix

        if not hasattr(self.process, fullname):
            setattr(self.process, fullname, attr)
        else:
            if skipIfExists:
                pass
            else:
                raise RuntimeError('process already has an attribute named ' + fullname)

        self.sequence += attr

        self.last = attr

        return cms.InputTag(fullname)
