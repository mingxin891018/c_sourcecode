#!/usr/bin/python
import os
print os.path.dirname(__file__)
print os.getcwd()
#print os.name()
#print os.listdir('/tmp')
"""
try: 
	os.remove('/home/zhaomingxin/tmp/python/111')
	os.rmdir('/home/zhaomingxin/tmp/python/111')
	os.mkdir('/home/zhaomingxin/tmp/python/111')
	os.mkdirs('/home/zhaomingxin/tmp/python/111/1/2/3/4')
except OSError:
	print('-------------')
"""
print os.path.isfile('/home/zhaomingxin/tmp/python/111')
print os.path.isdir('/home/zhaomingxin/tmp/python')
print os.path.split('/home/zhaomingxin/tmp/python/os.py')
os.system('ls')
print os.stat('/home/zhaomingxin/tmp/python/os.py')


