#!/usr/bin/env python
# -*- coding: utf-8 -*-


import copy
import re
import os
from collections import defaultdict

from SrcInfo import SrcInfoDB
from utils import Color
from utils import rm_file
from utils import list_dir
from utils import shell_exec
from utils import ROOT_DIR
from utils import yaml_load
from utils import yaml_dump
from utils import current_time


class RaceDetector(object):
    ''' Just a shell command wrapper'''

    def __init__(self):
        super(RaceDetector, self).__init__()

    def detect(self, detect_dir, sched):
        self.moveTraceFile(detect_dir)
        #  self.removeEmptyFile(detect_dir)
        self.detectTotalRace(detect_dir, sched)

    def moveTraceFile(self, detect_dir):
        for f in os.listdir('/dev/shm'):
            if f.endswith('.trace') or f.endswith('.mem-trace'):
                src_fname = os.path.join('/dev/shm', f.replace('&', '\\&'))
                dst_fname = f.replace('&', '/')
                shell_exec('mv %s %s' % (src_fname, dst_fname))

    def removeEmptyFile(self, detect_dir):
        for f in list_dir(detect_dir):
            # Iterate files while deleting
            if not os.path.exists(f):
                continue
            if os.path.getsize(f) == 0:
                print 'rm', f
                rm_file(f)
                # To remove corresponding trace file
                if f.endswith('.race'):
                    print 'rm', f.replace('.race', '.trace')
                    rm_file(f.replace('.race', '.trace'))

    def detectTotalRace(self, detect_dir, sched):
        for trace_file in list_dir(detect_dir, ext='.trace'):
            if sched is not None and sched not in trace_file:
                continue
            race_file = trace_file.replace('.trace', '.race')
            if not os.path.exists(race_file):
                print 'TO detect:', trace_file
                bin = os.path.join(ROOT_DIR, 'bin/detect')
                shell_exec('%s %s' % (bin, trace_file))


class RuntimeRace(object):
    @classmethod
    def parse(cls, line):
        # tid:timestamp:inst_id tid:timestamp:inst_id addr
        info = line.strip().split()
        first_tid, first_ts, first_inst_id = info[0].strip().split(':')
        second_tid, second_ts, second_inst_id = info[1].strip().split(':')
        addr = info[2]

        return RuntimeRace(first_tid, first_inst_id, first_ts,
                           second_tid, second_inst_id, second_ts,
                           addr)

    def __init__(self, first_tid, first_inst_id, first_ts,
                       second_tid, second_inst_id, second_ts, addr):
        self.first_tid_ = first_tid
        self.first_ts_ = first_ts
        self.first_inst_id_ = first_inst_id
        self.second_tid_ = second_tid
        self.second_ts_ = second_ts
        self.second_inst_id_ = second_inst_id
        self.addr_ = addr

    def __str__(self):
        return '%s @%s [%s]  X  %s @%s [%s]  Addr: %s' % (self.first_tid_,
                                                          self.first_ts_,
                                                          self.first_inst_id_,
                                                          self.second_tid_,
                                                          self.second_ts_,
                                                          self.second_inst_id_,
                                                          self.addr_)


class InstRace(object):
    @classmethod
    def parse(cls, line):
        # inst_id:w/r inst_id:w/r
        first_info, second_info = line.strip().split()
        first_inst_id, first_op = first_info.split(':')
        second_inst_id, second_op = second_info.split(':')

        return InstRace(first_inst_id, first_op, second_inst_id, second_op)

    def __init__(self, first_inst_id, first_op, second_inst_id, second_op):
        super(InstRace, self).__init__()
        if first_inst_id > second_inst_id:
            first_inst_id, second_inst_id = second_inst_id, first_inst_id
            first_op, second_op = second_op, first_op

        self.first_inst_id_ = first_inst_id
        self.first_op_ = first_op
        self.second_inst_id_ = second_inst_id
        self.second_op_ = second_op

    def __hash__(self):
        return hash((self.first_inst_id_, self.second_inst_id_))

    def __eq__(self, other):
        return self.first_inst_id_ == other.first_inst_id_ and \
               self.second_inst_id_ == other.second_inst_id_

    def __ne__(self, other):
        return not self.__eq__(other)

    def __str__(self):
        return '%s:%s %s:%s' % (self.first_inst_id_, self.first_op_,
                                self.second_inst_id_, self.second_op_)


class RaceFile(object):

    def __init__(self, fpath=None):
        super(RaceFile, self).__init__()
        self.inst_races_ = []
        self.normal_inst_races_ = []
        self.fpath_ = fpath

    def showSrcInfo(self, target_inst_races):
        db = SrcInfoDB()
        fd = open(self.fpath_.replace('.race', '.race-summary'), 'w')

        print '========= %70s =========\n' % Color.green(self.fpath_)
        fd.write('========= %70s =========\n' % Color.green(self.fpath_))

        run_races = self.loadRuntimeRace(target_inst_races)

        for inst_race in target_inst_races:
            src_info_1 = db.lookup(inst_race.first_inst_id_)
            src_info_2 = db.lookup(inst_race.second_inst_id_)

            race_str = '%s %s  X  %s %s' % (inst_race.first_inst_id_,
                                            src_info_1,
                                            inst_race.second_inst_id_,
                                            src_info_2)
            seperator_str = '=' * 90

            print '\n'.join([seperator_str, race_str, seperator_str])
            if len(run_races[inst_race]) < 10:
                for run_race in run_races[inst_race]:
                    print run_race
            else:
                print len(run_races[inst_race]),
                print 'File:', self.fpath_.replace('.race', '.race-summary')
            print Color.pink('-' * 90), '\n'

            fd.writelines([seperator_str, race_str, seperator_str])
            for run_race in run_races[inst_race]:
                fd.write(str(run_race))
                fd.write('\n')
            fd.write(Color.pink('-' * 90) + '\n')

        print Color.blue(seperator_str),'\n'
        db.close()
        fd.close()

    def loadRuntimeRace(self, target_inst_races):
        run_races = defaultdict(lambda :[])
        with open(self.fpath_, 'r') as fd:
            while True:
                try:
                    line = next(fd)
                except StopIteration:
                    return run_races
                inst_race = InstRace.parse(line)

                line = next(fd)
                num = int(re.findall(r'\d+', line)[0])

                for i in xrange(num):
                    line = next(fd)
                    if inst_race in target_inst_races:
                        run_races[inst_race].append(RuntimeRace.parse(line))
        return run_races

    def add(self, inst_race):
        if inst_race not in self.inst_races_:
            self.inst_races_.append(inst_race)

    def addNormal(self, inst_race):
        if inst_race not in self.normal_inst_races_:
            self.normal_inst_races_.append(inst_race)

    def isEmpty(self):
        return len(self.inst_races_) == 0

    def filePath(self):
        return self.fpath_

    def nrNormalInstRace(self):
        return len(self.normal_inst_races_)

    def __len__(self):
        return len(self.inst_races_)

    def __le__(self, other):
        return self.fpath_ < other.fpath_

    def __eq__(self, other):
        return self.fpath_ == other.fpath_

    def __ne__(self, other):
        return not self.fpath_ == other.fpath_


class RaceFileManager(object):
    def __init__(self):
        super(RaceFileManager, self).__init__()
        self.tot_inst_race_ = defaultdict(int)
        self.tot_race_file_ = []
        self.acc_nr_tot_inst_race_ = []
        self.normal_race_ = set()

    @staticmethod
    def fnameTimestamp(fname):
        # s = [benchmark_name date time sched.ext]
        s = fname.split('_')
        return '%s_%s' % (s[1], s[2])

    def loadNorlmalRace(self, detect_dir):
        # .../detect => .../summary
        summary_dir = detect_dir.replace('detect', 'summary')
        for f in list_dir(summary_dir, ext='NORMAL.tot-race'):
            with open(f, 'r') as fd:
                for line in fd:
                    self.normal_race_.add(InstRace.parse(line))

    def isNormalRace(self, r):
        return r in self.normal_race_

    def loadTotalRaceFile(self, detect_dir, sched):
        #  if sched != 'NORMAL':
            #  self.loadNorlmalRace(detect_dir)

        for f in list_dir(detect_dir, ext='.race', fkey=self.fnameTimestamp):
            if sched is not None and sched not in f:
                continue
            self.loadRaceFile(f)

    def loadRaceFile(self, fpath):
        # File format
        # inst_id:w/r inst_id:w/r
        # --- {num of runtime race} ---
        # tid:timestamp:inst_id tid:timestamp:inst_id addr
        rf = RaceFile(fpath)
        self.tot_race_file_.append(rf)
        with open(fpath, 'r') as fd:
            while True:
                try:
                    line = next(fd)
                except StopIteration:
                    break

                inst_race = InstRace.parse(line)
                if not self.isNormalRace(inst_race):
                    rf.add(inst_race)
                    self.tot_inst_race_[inst_race] += 1
                else:
                    rf.addNormal(inst_race)

                line = next(fd)
                num = int(re.findall(r'\d+', line)[0])
                for i in xrange(num):
                    line = next(fd)

        #  if rf.isEmpty():
            #  print rf.filePath()
        self.acc_nr_tot_inst_race_.append(len(self.tot_inst_race_))

    def nrAccDistinctRaceEachFile(self):
        return self.acc_nr_tot_inst_race_

    def nrDistinctRaceEachFile(self):
        rf_size = []
        for rf in self.tot_race_file_:
            rf_size.append(len(rf))
        return rf_size

    def nrNormalRaceEachFile(self):
        sz = []
        for rf in self.tot_race_file_:
            sz.append(rf.nrNormalInstRace())
        return sz

    def nrTotalRaceEachFile(self):
        a = self.nrDistinctRaceEachFile()
        b = self.nrNormalRaceEachFile()
        c = []
        for x, y in zip(a, b):
            c.append(x + y)
        return c

    def nrRaceFile(self):
        return len(self.tot_race_file_)

    def saveInfo(self, fpath):
        t = current_time()

        with open(fpath+'_%s.tot-race' % t, 'w') as fd:
            for inst_race in self.tot_inst_race_.keys():
                fd.write(str(inst_race))
                fd.write('\n')

        summary = {}
        summary['nr-acc-distinct-race'] = self.nrAccDistinctRaceEachFile()
        summary['nr-distinct-race'] = self.nrDistinctRaceEachFile()
        summary['nr-normal-race'] = self.nrNormalRaceEachFile()
        summary['nr-total-race'] = self.nrTotalRaceEachFile()

        yaml_dump(fpath+'_%s.tot-summary' % t, summary)

    def filterCommonRace(self, threshold):
        n = 0
        for inst_race in self.tot_inst_race_.keys():
            if self.tot_inst_race_[inst_race] >= threshold:
                del self.tot_inst_race_[inst_race]
                n += 1
        return n

    def showSummary(self):
        for rf in sorted(self.tot_race_file_, key=lambda x: x.fpath_):
            self.showRaceFileSummary(rf)

    def showRaceFileSummary(self, rf):
        target_inst_races = []
        for inst_race in rf.inst_races_:
            if inst_race in self.tot_inst_race_:
                target_inst_races.append(inst_race)
        rf.showSrcInfo(target_inst_races)
