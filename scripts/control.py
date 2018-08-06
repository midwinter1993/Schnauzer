#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Usage:
    control.py sys <cmd> <config> [--prologue <prologue>]
                                  [--epilogue <epilogue>]
                                  [--run-time <run-time>]
                                  [--trigger <trigger-cmd>]
                                  [--trigger-time <trigger-time>]

Options:
    -h --help     Show this screen.
    -p <prologue>, --prologue <prologue>
    -e <epilogue>, --epilogue <epilogue>
    -t <run-time>, --run-time <run-time>
    -x <trigger-cmd>, --trigger <trigger-cmd>
    -z <trigger-time>, --trigger-time <trigger-time>
"""

import json
import logging
import math
import multiprocessing
import os
import pprint
import random
import signal
import subprocess
import sys
import time
from itertools import combinations

import yaml
from docopt import docopt

from utils import current_time
from utils import Color
from utils import PROJ_DIR


def nCr(n, r):
    f = math.factorial
    return f(n) / f(r) / f(n-r)


def nPr(n, r):
    f = math.factorial
    return f(n) / f(n-r)

log_file = os.path.join(PROJ_DIR, 'bak/logs', current_time() + '.log')
subprocess.check_call(['mkdir', '-pv', os.path.join(PROJ_DIR, 'bak/logs')])
logging.basicConfig(filename=log_file,
                    level=logging.DEBUG,
                    format='%(message)s')

def log_info(*args):
    #  print args
    logger = logging.getLogger()
    logging.info(' '.join(map(str, args)))
    logger.handlers[0].flush()


RANDOM_MONEY = 0
INF_MONEY = 10000000


def yaml_load(fpath):
    with open(fpath, 'r') as fp:
        return yaml.load(fp.read())


def yaml_dump(fpath, y):
    with open(fpath, 'w') as fp:
        fp.write(yaml.dump(y, width=1000))


class SigSender(multiprocessing.Process):
    def __init__(self, target_pid, sleep_time=4, sig=signal.SIGINT):
        multiprocessing.Process.__init__(self)
        self.target_pid_ = target_pid
        self.sleep_time_ = sleep_time
        self.sig_ = sig

    def run(self):
        time.sleep(self.sleep_time_)
        print 'To Kill:',
        try:
            os.kill(self.target_pid_, self.sig_)
        except OSError:
            print 'FAILURE'
        else:
            print 'SUCCESS'


class Runner(multiprocessing.Process):
    def __init__(self, cmd, delay=0, redirect=False):
        multiprocessing.Process.__init__(self)
        self.cmd_ = cmd
        self.delay_ = delay
        self.redirect_ = redirect

    def run(self):
        try:
            if self.delay_:
                time.sleep(self.delay_)

            if self.redirect_:
                FNULL = open(os.devnull, 'w')
                process = subprocess.Popen(self.cmd_,
                                           shell=True,
                                           preexec_fn=os.setsid,
                                           stdout=FNULL,
                                           stderr=subprocess.STDOUT)
            else:
                process = subprocess.Popen(self.cmd_,
                                           shell=True,
                                           preexec_fn=os.setsid)

            if process.pid == -1:
                print 'run', cmd, 'error'
                return process.returncode
            process.wait()
            return process.returncode
        except KeyboardInterrupt:
            # SIGTERM for mysql
            #  os.killpg(os.getpgid(process.pid), signal.SIGTERM)
            os.killpg(os.getpgid(process.pid), signal.SIGINT)
            process.wait()


class Ctrl(object):
    def __init__(self, arg):
        super(Ctrl, self).__init__()
        self.arg_ = arg
        self.nr_run_ = 0

        self.prologue_cmd_ = arg.get('--prologue')
        self.epilogue_cmd_ = arg.get('--epilogue')
        self.cmd_ = arg.get('<cmd>')
        self.trigger_cmd_ = arg.get('--trigger')
        self.run_time_ = self.parseTime('--run-time')
        self.trigger_time_ = self.parseTime('--trigger-time')

        assert self.run_time_ >= self.trigger_time_
        self.checkConfig()

    def parseTime(self, name, default_time=3):
        t = self.arg_.get(name)
        if t is None:
            return default_time
        return int(t)

    def checkConfig(self):
        cfg_path = self.arg_.get('<config>')
        if cfg_path is None:
            print 'LN_CONFIG not found'
            sys.exit(0)
        else:
            print '<config> is', cfg_path
            log_info('<config> is', cfg_path)

        if cfg_path != os.environ.get('LN_CONFIG'):
            print 'LN_CONFIG not match'
            print 'os.environ["LN_CONFIG"] is', os.environ.get('LN_CONFIG')
            sys.exit(0)

        self.cfg_path_ = cfg_path
        self.cfg_ = yaml_load(cfg_path)
        log_info(yaml.dump(self.cfg_, width=1000))

    def nrRun(self):
        return self.nr_run_

    def prologue(self):
        if self.prologue_cmd_ is not None:
            process = subprocess.Popen(self.prologue_cmd_, shell=True)
            process.wait()

    def epilogue(self):
        time.sleep(5)
        if self.epilogue_cmd_ is not None:
            print 'Epilogue', self.epilogue_cmd_
            process = subprocess.Popen(self.epilogue_cmd_, shell=True)
            process.wait()

    def runTest(self):
        self.nr_run_ += 1

        print '**************', self.nr_run_, '********************'
        self.prologue()

        runner = Runner(self.cmd_, redirect=False)
        runner.start()

        if self.trigger_cmd_:
            trigger = Runner(self.trigger_cmd_,
                             delay=self.trigger_time_,
                             redirect=False)
            trigger.start()

        sig_sender = SigSender(runner.pid, self.run_time_, signal.SIGINT)
        sig_sender.start()

        if self.trigger_cmd_:
            trigger.join()
        sig_sender.join()
        runner.join()

        self.epilogue()
        print '**********************************'


class SysCtrl(Ctrl):
    def __init__(self, arg):
        super(SysCtrl, self).__init__(arg)

    def start(self):
        if self.cfg_['SCS']['RANDOM_MONEY']:
            print 'RANDOM_MONEY set True for systematic test'
            sys.exit(0)
        self.sysThread(self.cfg_)

    def sysThread(self, cfg):
        num_thread = cfg['INFO']['NUM_THREAD']
        for threads in combinations(cfg['INFO']['CTRL_THREAD'], 2):
            log_info(threads)
            self.sysMoney(cfg, threads)

    def generateMoney(self):
        scale = 1
        X = lambda i: i
        Y = lambda i: i ** 2
        Z = lambda i: 4 ** i

        for n in (1, 8):
            for m in xrange(1, 8):
                yield (2 ** n, 2 ** m,)

    def sysMoney(self, cfg, threads):

        length = max(len(cfg['SCS']['INIT_MONEY']), max(threads))

        for moneys in self.generateMoney():
            cfg['SCS']['INIT_MONEY'] = [random.randint(2, 33) for _ in xrange(length)]

            for t, m in zip(threads, moneys):
                cfg['SCS']['INIT_MONEY'][t] = m

            print cfg['SCS']['INIT_MONEY']
            log_info(self.cfg_['SCS']['INIT_MONEY'])

            # dump config
            yaml_dump(self.cfg_path_, cfg)
            # run
            summary = self.runTest()


if __name__ == '__main__':

    arguments = docopt(__doc__, version='0.0')

    controller = SysCtrl(arguments)

    controller.start()
