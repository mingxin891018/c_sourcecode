#!/usr/bin/python2.7
#encoding:UTF-8
import urllib
import urllib2


user_agent = 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)'
head = { 'User-Agent' : user_agent }
request = urllib2.Request("http://10.10.10.186:33200/EPG/jsp/getchannellist.jsp?usertoken=null&User=63594&pwd=&NTID=00:07:63:5b:d9:e4&ip=10.10.5.204&Version=EC2108EB&lang=1&checkwords=881d314c&timestamp=20181030170100",headers = head)
response = urllib2.urlopen(request)
buf = response.read()
#print buf

fd = open('./111.txt','w')
fd.write(buf)
fd.close()


