import sys
from pathlib import Path, PurePath
import os.path
import shutil

def main ():
	#Create a path object and list all the headers in it
	p = Path (sys.argv[1])
	for h in list (p.glob ('**/public/*.h')):
		parts = list (h.parts)
		#Omit the public part
		index = parts.index ('public')
		if index != -1:
			del parts[index]
		#Resolve path
		dst = Path ('headers')
		for p in parts:
			if not dst.exists ():
				os.makedirs (dst)
			dst /= p
		print (dst)
		#Copy
		shutil.copy (h, dst)

if __name__ == "__main__":
	main ()