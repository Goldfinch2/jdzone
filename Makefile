all : jdzone

jinfo: jinfo.c
	gcc -Wall -g3 -o jdzone jdzone.c

jinfo-static: jdzone.c
	gcc -Wall -o jdzone jdzone.c 
clean:
	rm -rf jdzone
