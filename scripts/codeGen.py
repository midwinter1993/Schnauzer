#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import string
import jinja2
import codecs

PROJ_ROOT_DIR = os.path.curdir  #os.path.join('..', os.path.dirname(os.path.abspath(__file__)))
TEMPLATE_DIR = os.path.join(PROJ_ROOT_DIR, 'runtime/core')

INSTR_TEMPLATE = '''\
    virtual void {before_func}(instID_t inst_id, {params}) {{ }}
    virtual void {after_func}(instID_t inst_id, {params}) {{ }}
'''

WRAP_FUNC_TEMPLATE = ''' {{
    inc_global_tick();
    DO_INSTRUMENT(exe->{before_func}(inst_id, {args}));
    {ret_type} ret = {func}({args});
    DO_INSTRUMENT(exe->{after_func}(inst_id, {args}));
    return ret;
}}
'''


def camel_to_snake(snake_str):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def snake_to_camel(name):
    components = name.split('_')
    return components[0] + "".join(x.title() for x in components[1:])


class Func(object):
    def __init__(self, ret_type, func_name, arg_list):
        super(Func, self).__init__()
        self.ret_type_ = ret_type
        self.func_name_ = func_name
        self.params_list_ = arg_list

    @staticmethod
    def parseFunc(line):
        match = re.search(r'(.+) (.+)\((.*)\)', line)
        ret_type = match.group(1)
        func_name = match.group(2)
        params = match.group(3)
        arg_list = Func.parseParams(params)

        #  print ret_type
        #  print func_name
        #  print arg_list
        return Func(ret_type, func_name, arg_list)

    @staticmethod
    def parseParams(params):
        arg_list = []
        for arg_pair_str in params.split(','):
            arg_pair_str = arg_pair_str.strip()
            idx = len(arg_pair_str) - 1
            while idx >= 0 and (arg_pair_str[idx].isalnum() or
                                arg_pair_str[idx] == '_'):
                idx -= 1
            assert idx > 0

            arg_type = arg_pair_str[:idx + 1].strip()
            arg_name = arg_pair_str[idx + 1:].strip()
            arg_list.append((arg_type, arg_name))

        return arg_list

    def __str__(self):
        return '{ret} {name}({params})'.format(
            ret=self.ret_type_,
            name=self.func_name_,
            params=self.makeParamsList())

    def makeParamsList(self):
        params_str = ','.join(
            map(lambda x: '{ty} {name}'.format(ty=x[0], name=x[1]),
                self.params_list_))
        return params_str

    def makeArgsList(self):
        args_str = ', '.join(map(lambda x: x[1], self.params_list_))
        return args_str

    def makeWrapFunc(self):
        return '''{func_type} {func_body}'''.format(
            func_type=self.makeWrapFuncType(),
            func_body=self.makeWrapFuncBody())

    def makeWrapFuncType(self):
        params_str = ', '.join(
            map(lambda x: '{ty} {name}'.format(ty=x[0], name=x[1]),
                self.params_list_))
        return '{ret_type} {name}_wrapper(uint64_t inst_id, {params})'.format(
            ret_type=self.ret_type_, name=self.func_name_, params=params_str)

    def makeWrapFuncBody(self):
        before_func, after_func = self.makeInstrFuncNames()
        return WRAP_FUNC_TEMPLATE.format(
            ret_type=self.ret_type_,
            before_func=before_func,
            args=self.makeArgsList(),
            func=self.func_name_,
            after_func=after_func)

    def makeInstrFuncNames(self):
        before_func = snake_to_camel(
            'before_{func}'.format(func=self.func_name_))
        after_func = snake_to_camel(
            'after_{func}'.format(func=self.func_name_))
        return before_func, after_func

    def makeInstrFunc(self):
        before_func, after_func = self.makeInstrFuncNames()
        return INSTR_TEMPLATE.format(
            before_func=before_func,
            after_func=after_func,
            params=self.makeParamsList())


def loadFile(fpath):
    with codecs.open(fpath, 'r', 'utf-8') as fp:
        return fp.read()


def dump_executor(instr_funcs_str):
    template = jinja2.Template(
        loadFile(os.path.join(TEMPLATE_DIR, 'Executor.template')))
    #  print(template.render(instr_funcs=instr_funcs_str))
    with open(os.path.join(TEMPLATE_DIR, 'Executor.h'), 'w') as fp:
        fp.write(template.render(instr_funcs=instr_funcs_str))


def dump_interface(wrap_funcs_str):
    template = jinja2.Template(
        loadFile(os.path.join(TEMPLATE_DIR, 'Interface.template')))
    #  print(template.render(wrap_funcs=wrap_funcs_str))
    with open(os.path.join(TEMPLATE_DIR, 'Interface.cpp'), 'w') as fp:
        fp.write(template.render(wrap_funcs=wrap_funcs_str))


if __name__ == "__main__":
    wrap_funcs = []
    instr_funcs = []
    with open('./wrap_func.def', 'r') as fp:
        for line in fp:
            f = Func.parseFunc(line)
            wrap_funcs.append(f.makeWrapFunc())
            instr_funcs.append(f.makeInstrFunc())
    dump_interface('\n'.join(wrap_funcs))
    dump_executor('\n'.join(instr_funcs))
