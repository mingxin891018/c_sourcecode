#!/usr/bin/python

def read_table(file_name):
	table = []
	line = []
	f = open(file_name,'r');
	for line in f.readlines():
		if line[0] == '#':
			continue
		if line[0] == '[' and line[len(line)-2] == ']':
			table.append(line[1:-2])
	f.close()
	return table


def read_param(file_name):
	line = []
	param = {}

	f = open(file_name,'r')
	for line in f.readlines() :
		if line[0] == '#':
			continue
		if line[0] == '[' and line[len(line)-2] == ']':
			table = line[1:-2]
			param[table] = {}
		for i in line :
			if i == '=' :
				name = line[0:line.index('=')]
				value = line[line.index('=')+1:-1]
				param[table].setdefault(name,value)
	
	print('read param file finish!!!')
	return param



if __name__ == "__main__" :
	print(read_table('./swdefaultparam.txt'))
	print(read_param('./swdefaultparam.txt'))



