#!/usr/bin/python2.7
# -*- coding:utf-8 -*-
import urllib
import urllib2
import re
import time
import os

from lxml import etree

def GetPage(url):
	user_agent = 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)'
	head = { 'User-Agent' : user_agent }
	try:
		request = urllib2.Request(url,headers = head)
		response = urllib2.urlopen(request)
		content = response.read()
	except urllib2.URLError, e:
		if hasattr(e,"code"):
			print e.code
		if hasattr(e,"reason"):
			print e.reason
		return None
	return content

def GetGroup(html):
	sel = etree.HTML(html)
	
	text = sel.xpath('//div/ul/li[@class="left-list_li"]/a/text()')
	source = sel.xpath('//div/ul/li[@class="left-list_li"]/a/@href')
	
	text1 = sel.xpath('//div/dl/dd/a[@target="_blank"]/text()')
	source1 = sel.xpath('//div/dl/dd/a[@target="_blank"]/@href')
	dic = dict(zip(text,source))
	dic1 = dict(zip(text1,source1))
	dic2 = dict(dic.items() + dic1.items())
#	print dic2
	return dic2

def BuildPath(dic):
	pass

def GetPicture(dic):
	path = os.getcwd()
	if not os.path.isdir('picture'):
		os.mkdir('picture') 
	os.chdir('picture')
	print os.getcwd()
	num = 1
	for i in dic.keys():
		if not os.path.isdir(i):
			os.mkdir(i)
			os.chdir(i)
			print os.getcwd()
			j = dic.get(i)	
			print j
			if len(j) == 0:
				continue
			html = GetPage(j)
			if html is None:
				continue
			sel = etree.HTML(html)	
		
			pic = sel.xpath('//div[@class="content-pic"]/a/img/@src')
			tot = sel.xpath('//div[@class="content-page"]/span[1]/text()')
			name = str(num) + '.jpg'
			print pic
			fd = open(name,'w')
			mm = "".join(pic)
			if len(mm) == 0:
				continue
			tmp = GetPage(mm)
			if len(tmp) == 0:
				continue
			fd.write(tmp)
			fd.close()
			base_html = j[0:-5] + '_'
			print base_html
			
			number = "".join(tot).encode('gbk')
			total_str = filter(str.isdigit,number)
			total = int(total_str)
			print total
			for i in range(total - 1):
				new_html = base_html + str(i+2) + '.html'
				print new_html
				if len(new_html) == 0:
					continue
				page_n = GetPage(new_html)
				if page_n is None:
					continue
				sel_n = etree.HTML(page_n)
				pic = sel_n.xpath('//div[@class="content-pic"]/a/img/@src')	
				if len(pic) == 0:
					continue
				num += 1
				name = str(num) + '.jpg'
				fd = open(name,'w')
				mm = "".join(pic)
				if len(mm) == 0:
					continue	
				tmp = GetPage(mm)
				if tmp is None:
					fd.close()
					continue
				fd.write(tmp)
				fd.close()

			os.chdir(path)
			os.chdir('picture')
	os.chdir(path)

if __name__ == "__main__":
	url = 'http://www.mm131.com'
	html = GetPage(url)
	dic = GetGroup(html)
	GetPicture(dic)

	fd = open('./mm131','w')
	fd.write(html)
	fd.close()


