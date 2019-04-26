#!/usr/bin/python
class Test(object):
	def __init__(self,x):
		self.x = x;
	def func(self,message):
		print message
			 
	def test(msg):
		print msg



object1=Test(20)
x = object1.func('zhaomingxin')
x
y = object1.test
y

t = Test.func
t(object1,"zhaomingxin 1")






