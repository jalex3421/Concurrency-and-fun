fragmenta: fragmenta.c
	gcc -Wall -c fragmenta.c
	
minishell: minishell.c fragmenta.o
	gcc -Wall -o minishell minishell.c fragmenta.o

all: fragmenta minishell
