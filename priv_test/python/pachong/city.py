#!/usr/bin/python2.7
#encoding:UTF-8
import urllib
import urllib2
import urllib, urllib2, sys


host = 'http://guolin.tech/api/china'
path = '/13'
method = 'GET'
appcode = ''
querys = ''
bodys = {}
url = host + path + '?' + querys

request = urllib2.Request(url)
response = urllib2.urlopen(request)
content = response.read()
if (content):
	print(content)














