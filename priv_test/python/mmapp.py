#!/usr/bin/python2.7
import os
import sys
import hashlib

path1 = '/home/zhaomingxin/tmp/python/test'
path2 = '/home/zhaomingxin/tmp/python'

def Give_Md5(Path):
	file1_list = os.listdir(Path)
	md5_num1 = {}
	for i in file1_list :
		tmp = os.path.join(Path,i)
		if os.path.isfile(tmp):
			fd = open(tmp,"r")
			fcont = fd.read()
			fd.close()
			fmd5 = hashlib.md5(fcont)
			md5_num = fmd5.hexdigest()
			md5_num1[i] = md5_num
	#print(md5_num1)
	return md5_num1

if __name__ == "__main__":
	dict1 = Give_Md5(path1)
	dict2 = Give_Md5(path2)
#	print(dict1)
#	print(dict2)
	
	for i in dict1.keys():
		if dict1.get(i) == dict2.get(i):
			del dict1[i]
			del dict2[i]
	print(path1)
	for j in dict1.iteritems():
		print(j)
	print(path2)
	for k in dict2.iteritems():
		print(k)


