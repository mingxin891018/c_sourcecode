#!/usr/bin/python2.7
# coding=UTF-8
import re
 
# 将正则表达式编译成Pattern对象
pattern = re.compile(r'hello')
  
# 使用Pattern匹配文本，获得匹配结果，无法匹配时将返回None
match = pattern.match('hello world!')

if match:
    # 使用Match获得分组信息
	print match.group()
			    
### 输出 ###
# hello


text = "JGood is a handsome boy, he is cool, clever, and so on..."
m = re.match(r"\w*oo\w*", text)
if m:
	print m.group()
else:
	print 'not match'  

m = re.match(r'hello', 'hello world!')
print m.group()















