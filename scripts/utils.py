#!/usr/bin/env python
# -*- coding: utf-8 -*-


import subprocess
import datetime
import os
import yaml


ROOT_DIR = os.path.dirname(os.path.realpath(__file__)).replace('scripts', '')


def yaml_load(fpath):
    with open(fpath, 'r') as fp:
        return yaml.load(fp.read())


def yaml_dump(fpath, y):
    with open(fpath, 'w') as fp:
        fp.write(yaml.dump(y, width=1000))


class Color:
    PINK = '\033[95m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

    @staticmethod
    def green(*args):
        print Color.GREEN + ' '.join(map(str, args)) + Color.ENDC

    @staticmethod
    def blue(*args):
        print Color.BLUE + ' '.join(map(str, args)) + Color.ENDC

    @staticmethod
    def pink(*args):
        print Color.PINK + ' '.join(map(str, args)) + Color.ENDC


def rm_file(filename):
    try:
        if os.path.exists(filename):
            os.remove(filename)
    except OSError as e:  # this would be "except OSError, e:" before Python 2.6
        if e.errno != errno.ENOENT:  # errno.ENOENT = no such file or directory
            raise  # re-raise exception if a different error occurred


def list_dir(dir_path, ext=None, fkey=None):
    files = os.listdir(dir_path)
    if ext:
        files = filter(lambda f: f.endswith(ext), files)

    mtime = lambda f: os.stat(os.path.join(dir_path, f)).st_mtime
    files = sorted(files, key=fkey if fkey else mtime)

    for f in files:
        yield os.path.join(dir_path, f)


def shell_exec(command, to_print=True):
    if to_print:
        print command

    try:
        process = subprocess.Popen(command, shell=True)
        process.wait()
        return process.returncode
    except KeyboardInterrupt:
        print '~~~~ TO KILL ~~~~'
        os.kill(process.pid, signal.SIGINT)

def current_time():
    return datetime.datetime.now().strftime('%Y-%m-%d_%H:%M:%S')

def extend_list(lst, l):
    if len(lst) < l:
        lst.extend([0 for _ in xrange(l-len(lst))])
