#!/usr/bin/python2.7
# -*- coding:utf-8 -*-
import urllib
import urllib2
import re
import time

from lxml import etree

def page_pa(url):
	page = 2
	user_agent = 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)'
	head = { 'User-Agent' : user_agent }
	try:
		request = urllib2.Request(url,headers = head)
		response = urllib2.urlopen(request)
		content = response.read()#.decode('utf-8')
	#	print content
		file_tmp = './temp'
		fd = open(file_tmp,'w')
		fd.write(url)
		fd.write(content)
		fd.close()
	except urllib2.URLError, e:
		if hasattr(e,"code"):
			print e.code
		if hasattr(e,"reason"):
			print e.reason
	
	#pattern = re.compile('<div*?',re.S)
	#items = re.findall(pattern,content)
	#print items
	sel = etree.HTML(content)
	fd_data = open('./qiushi.txt','a+')
	if url == 'https://www.qiushibaike.com/text':
		next_link = sel.xpath('//a[@class="contentHerf"]/@href')
		text = sel.xpath('//a/div[1]/span[1]/text()')
		for k in text:
#			print k
			fd_data.write(k.encode('utf-8').replace("\n\n\n\n", "\n"))
		return next_link[0]
	else:
		next_link = sel.xpath('//input[@id="articleNextLink"]/@value')
		text = sel.xpath('//div[@class="content"]/text()')
		for i in  text:
#			print i
			fd_data.write(i.encode('utf-8'))
	fd_data.close()
	tmp = ''
	for j in  next_link:
		tmp += j 
	return j
if __name__ == "__main__":
	count = 10
	url_main = 'https://www.qiushibaike.com'
	url_start = '/text'
	
	while(count):
		next_page = url_main + url_start
		print('this_page = %s') % next_page
		tmp = page_pa(next_page)
		print('next_action = %s') % tmp
		url_start = tmp
		
#		time.sleep(1)
		count -= 1


