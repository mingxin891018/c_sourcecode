#!/usr/bin/python2.7
#encoding:UTF-8
import urllib
import urllib2
import urllib, urllib2, sys


host = 'http://jisutianqi.market.alicloudapi.com'
path = '/weather/query'
method = 'GET'
appcode = 'a3df3be49510466f84928a2aed39e9ed'
querys = 'city=%E5%AE%89%E9%A1%BA&citycode=citycode&cityid=cityid&ip=ip&location=location'
bodys = {}
url = host + path + '?' + querys

request = urllib2.Request(url)
request.add_header('Authorization', 'APPCODE ' + appcode)
response = urllib2.urlopen(request)
content = response.read()
if (content):
	print(content)














