#!/usr/bin/python
import class_param

test = class_param.ParamInFile('./swdefaultparam.txt')
sqtest = class_param.ParamToSqlite('./.temp.db',test.Give_table(),test.Give_Param())















