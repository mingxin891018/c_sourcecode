#!/usr/bin/python
########################################################################
class Person(object):
	def __init__(self, first_name, last_name):
		"""Constructor"""
		self.first_name = first_name
		self.last_name = last_name

	@property
	def full_name(self):
		return "%s %s" % (self.first_name, self.last_name)


p=Person("zhao","mingxin")
p.full_name


