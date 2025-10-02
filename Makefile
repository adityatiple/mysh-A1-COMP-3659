mysh: mysh.o mystring.o myheap.o myfunctions.o
	gcc mysh.o mystring.o myheap.o myfunctions.o -o mysh

mysh.o: mysh.c mystring.h jobs.h
	gcc -c mysh.c

myfunctions.o: myfunctions.c myfunctions.h mystring.h myheap.h
	gcc -c myfunctions.c

mystring.o: mystring.c mystring.h
	gcc -c mystring.c

myheap.o: myheap.c myheap.h
	gcc -c myheap.c

clean:
	/usr/bin/rm -f *.o mysh

all: clean mysh
