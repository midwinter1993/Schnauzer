#!/usr/bin/env python
# -*- coding: utf-8 -*-


class CommandTable(object):
    def __init__(self):
        super(CommandTable, self).__init__()

        self.command_table_ = {}

    def collect(self, f):
        self.command_table_[f.__name__] = f
        return f

    def run(self, arguments):
        cmd, _ = filter(lambda x: x[1] is True, arguments.iteritems())[0]
        self.command_table_[cmd](arguments)

