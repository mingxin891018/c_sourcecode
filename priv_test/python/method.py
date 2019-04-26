#!/usr/bin/python
#!/usr/bin/python
# -*- coding: utf-8 -*-  

#eclipse pydev, python 3.3  
#by C.L.Wang  

class A(object):  

	_g = 1  

	def foo(self,x):  
		print('executing foo(%s,%s)'%(self,x))  

	@classmethod  
	def class_foo(cls,x):  
		print('executing class_foo(%s,%s)'%(cls,x))  
	
	@staticmethod  
	def static_foo(x):  
		print('executing static_foo(%s)'%x)     

a = A()  
a.foo(1)  

a.class_foo(1)  
A.class_foo(1)  

a.static_foo(1)  
A.static_foo('hi')  


print(a.foo)  
print(a.class_foo)  
print(a.static_foo)  









