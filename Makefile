all : jinfo

jinfo: jinfo.c
	gcc -Wall -g3 -o jinfo jinfo.c

jinfo-static: jinfo.c
	gcc -Wall -o jinfo jinfo.c 
clean:
	rm -rf jinfo
