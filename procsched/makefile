fragmenta: fragmenta.c
	gcc -Wall -c fragmenta.c
	
procsched: procsched.c fragmenta.o
	gcc -Wall -o procsched procsched.c fragmenta.o

all: fragmenta procsched
