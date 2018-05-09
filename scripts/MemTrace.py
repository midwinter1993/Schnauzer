#!/usr/bin/env python
# -*- coding: utf-8 -*-


from SrcInfo import SrcInfoDB


class MemAllocEvent(object):
    def __init__(self, addr, sz, callstack):
        super(MemAllocEvent, self).__init__()
        self.addr_ = int(addr, 16)
        self.sz_ = int(sz)
        self.callstack_ = callstack.strip().split()

    def contains(self, addr):
        return self.addr_ <= addr < self.addr_ + self.sz_

    def __str__(self):
        ret = ['%d %d:' % (self.addr_, self.sz_)]
        db = SrcInfoDB()
        indent = '  '
        for idx, call in enumerate(self.callstack_):
            ret.append(indent * (idx+1) + str(db.lookup(call)))
        db.close()
        return '\n'.join(ret)


class MemTrace(object):
    def __init__(self, fpath):
        super(MemTrace, self).__init__()
        self.fpath_ = fpath
        self.mem_alloc_events_ = []
        self.loadFile(fpath)

    def loadFile(self, fpath):
        with open(fpath, 'r') as fd:
            for line in fd:
                alloc_info, callstack  = line.strip().split(':')
                addr, sz = alloc_info.split()
                self.mem_alloc_events_.append(MemAllocEvent(addr, sz, callstack))

    def showAllTrace(self):
        for mem_evt in self.mem_alloc_events_:
            print str(mem_evt)

    def checkAddr(self, addr):
        for mem_evt in self.mem_alloc_events_:
            if mem_evt.contains(addr):
                print str(mem_evt)
