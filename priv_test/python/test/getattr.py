#!/usr/bin/python
class B(object):
	version = 2.0
	def __init__(self,x):
		self.x = x
		self.y = 200
	
	def say(self,msg):
		print 'this is say #%s#' % msg

def test(mine):
	print 'this is C\'s test %s' % mine

b = B(100)
print b.x
print b.y
m = b.say('zhaomingxin')
m
test('################');




