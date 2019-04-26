#!/usr/bin/python
s = [13, 4, 1, 6, 2, 9, 7, 0, 8, 5]
for i in range(0, len(s) - 1):
	for j in range(i + 1, 0, -1):
		if s[j] < s[j - 1]:
			s[j], s[j - 1] = s[j - 1], s[j]
for m in range(0, len(s)):
	print(s[m])




