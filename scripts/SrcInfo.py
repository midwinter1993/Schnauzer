#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
import sqlite3


class SrcInfo(object):
    def __init__(self, line_info, op_info):
        self.line_info_ = line_info
        self.op_info_ = op_info

    def __str__(self):
        return '%s [%s]' % (self.line_info_, self.op_info_)


class SrcInfoDB(object):
    def __init__(self):
        super(SrcInfoDB, self).__init__()
        self.sqlite_file_ = os.environ['SRCINFO_DB']
        self.conn_ = sqlite3.connect(self.sqlite_file_)
        self.cursor_ = self.conn_.cursor()

    def lookup(self, inst_id):
        table_name = 'instructions'
        id_column = 'inst_id'
        info_column = 1
        op_column = 2

        sql = 'SELECT * FROM {0} WHERE inst_id="{1}"'.format(table_name,
                                                             inst_id)
        self.cursor_.execute(sql)
        all_rows = self.cursor_.fetchall()

        if len(all_rows) != 1:
            print inst_id, len(all_rows), self.sqlite_file_
            assert False
        return SrcInfo(all_rows[0][info_column], all_rows[0][op_column])

    def close(self):
        self.conn_.close()


