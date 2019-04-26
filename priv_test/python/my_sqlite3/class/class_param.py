#!/usr/bin/python
import sqlite3
import os
import sys


class ParamInFile:
	def __init__(self,ParamPath):
		self.table = []
		self.param = {}
		line = []

		f = open(ParamPath,'r')
		for line in f.readlines():
			if line[0] == '#':
				continue
			if line[0] == '[' and line[len(line) - 2] == ']':
				self.table.append(line[1:-2])
		f.close()
#		print(self.table)

		f = open(ParamPath,'r')
		for line in f.readlines():
		  	if line[0] == '#':
		  		continue
		  	if line[0] == '[' and line[len(line) - 2] == ']':
	  			tmp = line[1:-2]
				self.param[tmp] = {}
			for i in line:
				if i == '=':
					name = line[0:line.index('=')]
					value = line[line.index('=')+1:-1]
					self.param[tmp].setdefault(name,value)
		f.close()
#		print(self.param)

	def Give_table(self):
		return self.table

	def Give_Param(self):
		return self.param

class ParamToSqlite:
	def __init__(self,SqliteName,TableName,ParamName):
		if os.path.isfile(SqliteName):
			os.remove(SqliteName)
			print('table is already delete!!!')
		
		self.sqlitename = SqliteName
		self.table = TableName
		self.paramtable = ParamName
		
		conn = sqlite3.connect(self.sqlitename)
		cur = conn.cursor()
		for i in TableName:
			sqm = "create table " + i + "(name text primary key not NULL,value text)"
			cur.execute(sqm)
			print('create ' + i +' table success!!!')
			conn.commit()
		
		for i in TableName:
			for j in ParamName[i].iteritems():
				ins = "insert into " + i + " values (?,?)"
				cur.execute(ins,j)
				conn.commit()

		conn.close()


	def GetParam(self,Name):
		conn = sqlite3.connect(self.sqlitename)
		cur = conn.cursor()
		for i in self.table:
			sqm = "select * from %s where %s.name='%s'" % (i,i,Name)
			cur.execute(sqm)
			result =  cur.fetchone()
		
			if result is not None:
				self.ret = []
				self.ret.append(i)
				for j in result:
					self.ret.append(j)
				break
			else:
				self.ret = 'Not this param'
		conn.close()
		
		return self.ret

	def SetParam(self,Name,Value):
		ret = self.GetParam(Name)
#		print(ret)
		
		conn = sqlite3.connect(self.sqlitename)
		cur = conn.cursor()
		sqm = "UPDATE %s SET value ='%s' WHERE name ='%s'" %(ret[0],Value,ret[1])
#		print(sqm)
		cur.execute(sqm)
		conn.commit()
		conn.close()
		
		

if __name__ == '__main__':
	test = ParamInFile('./swdefaultparam.txt')
	sqtest = ParamToSqlite('./.temp.db',test.Give_table(),test.Give_Param())
	
	print(sqtest.GetParam('auto_time'))
	sqtest.SetParam('auto_time','shuifangduimian')
	print(sqtest.GetParam('auto_time'))






