#!/usr/bin/python2.7
import sys
import sqlite3
import have_param
import create_table


def get_param():
	if len(sys.argv) < 2:
		print("xxx.py argv")
		sys.exit() 
	
	conn = sqlite3.connect(create_table.table_file)
	cur = conn.cursor()
	for i in have_param.read_table(create_table.param_file):
		sqm = "select * from %s where %s.name='%s'" % (i,i,sys.argv[1])
		cur.execute(sqm)
		
		result = cur.fetchone()
		if result is not None:
			print(result)

	conn.close()


if __name__ == "__main__":
	get_param()





