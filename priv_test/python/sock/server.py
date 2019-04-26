#!/usr/bin/python2.7
import socket

fd = socket.socket()
host = socket.gethostname()
fd.bind((host,1989))
fd.listen(10)
while True:
	dst,addr = fd.accept()
	print("Accept form",addr)
	dst.send("Connected....")
	dst.close()





