#!/usr/bin/python
import sqlite3
import os
import have_param

param_file = './swdefaultparam.txt'
table_file = './.table'

def create_sqlite():
	if os.path.isfile(table_file):
		os.remove(table_file)
		print('table is already delete!!!')
	
	conn = sqlite3.connect(table_file)
	cur = conn.cursor()
	for i in have_param.read_table(param_file):
		sqm = "create table " + i + "(name text primary key not NULL,value text)"
		cur.execute(sqm)
		print('create ' + i +' table success!!!')
		
	conn.commit()
	conn.close()

def insert_default_param():
	create_sqlite()
	if os.path.isfile(table_file):
		print('find sqlite file !!!')
		conn = sqlite3.connect(table_file)
		cur = conn.cursor()
		param_dict = have_param.read_param(param_file)
		for i in have_param.read_table(param_file):
			for j in param_dict[i].iteritems():
				print(j)
				ins = "insert into " + i + " values (?,?)"
				print(ins)
				cur.execute(ins,j)
				conn.commit()
		
		conn.close()
	print("insert is finish!!!")


if __name__ == '__main__':
	insert_default_param()


