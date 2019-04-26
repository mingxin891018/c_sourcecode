#!/usr/bin/python2.7
import sys
import sqlite3
import have_param
import create_table

def set_param:
	if len(sys.argv()) < 2:
		print("./set_param.py xxxx")
		sys.exit()
	conn = sqlite3.connect(create_table.table_file)
	cur = conn.cursor()
	
	result = conn.fetchone()
	sqm = ""



	conn.commit()
	conn.close()












