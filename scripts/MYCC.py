#!/usr/bin/env python
# -*- coding: utf-8 -*-

import subprocess
import sys
import os

THIS_DIR = os.path.abspath(os.path.dirname(__file__))
PROJ_DIR = os.path.join(THIS_DIR, os.path.pardir)
MY_LIB_DIR = os.path.join(PROJ_DIR, 'bin')
EXT_LIB_DIR = os.path.join(PROJ_DIR, 'extlibs')


CFLAGS = '-Xclang -load -Xclang {0}/instrument.so -g'.format(MY_LIB_DIR)
LDFLAGS = ('-L{0} -Wl,-rpath,{0} -lruntime '
           '-L{1} -Wl,-rpath,{1} -lsqlite3 ').format(MY_LIB_DIR, EXT_LIB_DIR)


def execShell(cmd_list, prompt=None):
    if type(cmd_list) is str:
        cmd_list = cmd_list.split()
    elif type(cmd_list) is not list:
        print 'Command list error'
        sys.exit(1)

    if prompt:
        print prompt,

    # Not shell=True
    return subprocess.check_call(cmd_list)


def hookCompilingSimple(cmd):
    CC = cmd[0]
    options = cmd[1:]

    #
    #  Our instrument options (e.g. -Xclang -load for linker) may cause warnings.
    #  When -Werror used, compilation will fail.
    #
    if '-Werror' in options:
        options.remove('-Werror')

    #
    # To collect complete source code info, we disable optimization
    #
    optimizations = ('-O1', '-O2', '-O3')
    for o in optimizations:
        if o in options:
            options.remove(o)

    new_cmd = [CC] + CFLAGS.split() + options + LDFLAGS.split()
    print '>>> ', ' '.join(new_cmd)
    execShell(new_cmd)


def main(wrap_cc, cc):
    if not sys.argv[0].endswith(wrap_cc):
        print "ERROR ", wrap_cc, sys.argv
        sys.exit(1)

    cmd = sys.argv[:]
    cmd[0] = cc

    #
    # For ./configure, we should not hook the compilation.
    #
    is_config = os.environ.get('CONFIGURE')
    if is_config:
        execShell(cmd)
    else:
        hookCompilingSimple(cmd)

if __name__ == "__main__":
    CC = 'clang'
    WRAP_CC = 'MYCC'
    main(WRAP_CC, CC)
