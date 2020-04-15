/*Shell realizada por Alejandro Meza
 *FECHA: 28/10/2019
 *Este programa simula el funcionamiento de nuestra terminal
 */
 
#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "fragmenta.h"
#include <errno.h>
#include <string.h>

#define clear() printf("\n")

char** fraseFragmentada;
pid_t pid;

//prototipo de funciones irrelevantes para funcionamiento general
struct myDataType{
    int i;
    char ch;
}inputValue();

void run();

int check(char sym[9],char ch,int count);

struct myDataType inputValue(char sym[9],int count);

void Display(char sym[9]);

void play();

struct myDataType inputValue(char sym[9],int count);

void Display(char sym[9]);
//Fin de prototipo de funciones irrelevantes

//USO: imprime 'myShell:>' en cada iteracion
void mypromt(){
	printf("myShell:>");
}

//USO: manejador recibe señal de finalizar proceso.
void manejador1 (int sig){
	printf("\033[1;34m");
	printf("\n\n\n\n******************""************************");
	printf("\n\n\t**** END OF MY SHELL ****");
	printf("\n\n\n******************""************************");
	printf("\033[1;34m");
	printf("\n");
	borrarg(fraseFragmentada);
	kill(pid, SIGKILL);  
	exit(0);
}

//USO: despliega una 'pequenia' ayuda para el usuario
void help(){
	puts("\n***Bienvenido a la ayuda de la Shell***"
	      "\nLista de algunos comandos utiles soportados por la shell:"
	      "\n>cat utility to concatenate files to standart outputs"
	      "\n>chmod utility to change file acces permissions"
	      "\n>chrpg utilty to change file group ownership"
	      "\n>chown utility to change file owner and group"
	      "\n>dir utility to list the contents of a directory"
	      "\n>echo utility to display a line of text"
	      "\n>gzip The GNU compression utility"
	      "\n>gunzip The GNU uncompression utility"
	      "\n>kill utility to send signals to processes"
	      "\n>ls utility to list directoty contents"
	      "\n>mkdir utilty to make directories"
	      "\n>mv utility to move/rename files"
	      "\n>play you can play tic tac toe if you want :)"
	      "\n>ps utility to report process status"
	      "\n>pwd utility to print the name of the current directory"
	      "\n>rm utility to remove file or directories"
	      "\n>tar the tar archiving utility(optional)");
	       clear();
}

//USO: Da la bienvenida a la shell al usuario
void start_shell(){
	clear();
	printf("\033[1;31m");
	printf("\n\n\n\n******************""************************");
	printf("\n\n\n\t****My SHELL****");
	printf("\n\n\t-USALA BAJO TU RESPONSABILIDAD-");
	printf("\n\n\n\n******************""************************");
	char *name = getenv("USER");
	printf("\n\n\n\tUSUARIO ES: @%s", name);
	printf("\033[0m");
	printf("\n");
	sleep(1);
	clear();
}

int main(int argc, char *argv[]){
    char linea[256];
    int status = 0;
    int n,r;
    start_shell(); 
    do{ //se plantea un do while para ejecutar en bucle todo lo que se debe
		mypromt(); //printf myshell:>
		scanf("%[^\n]", linea); //guardas hasta el salto de linea en array de caracteres
		getchar(); //coges salto de linea 
		fraseFragmentada= fragmenta(linea); //fraseFragmentada cont linea fragmentada. Char **fraseFragmentada
		if(strcmp(fraseFragmentada[0], "exit") == 0){ //found "exit" in command line
				exit(0);
		 }
		pid = fork(); //procedemos a hacer los hijos. pid_t variable global
		if(pid == -1){ //si pid ==-1 -->error
			perror("%s\n");
		}
		else if(pid == 0){ //si es el hijo
			if(strcmp(fraseFragmentada[0],"help") == 0){ //found "help" in command line
				help();
		    }//si despues exit/ctrc -->fallo?
		    else{
				int i = 0,fd; //fd--> file descriptor
				while(fraseFragmentada[i] != NULL){ //meintras existan tokens..
					if(strcmp(fraseFragmentada[0], "play") == 0){
						play();
					}
					//CASO 1: encontrar ">" 
					//Redirigir salida a lo que exista despues de >
					else if(strcmp(fraseFragmentada[i], ">") == 0){
						int ret;
						//i+1 porque es lo que se encuentra despues de >
						//en caso de no existir, se crea en el acto
						fd = open(fraseFragmentada[i+1],O_WRONLY|O_TRUNC|O_CREAT, 0666);
						if(fd<0){
							perror("OPEN_ERROR: %s \n");
						}
						//fd contiene el numero identificador del archivo de interes
						//dup2--> hacemos que el sdtout tenga mismo numero que fd.
						//luego salida estandar sera ahora donde esta el fd.
						//dup2(old fd, new fd)--> new fd es la copia de old fd
						ret =dup2(fd,STDOUT_FILENO); //redirigimos el stdout
						if(ret<0){
							perror("%s\n");
						}
						close(fd); //por convencion 
						fraseFragmentada[i] = NULL; //terminamos ejecucion 
					}
					//CASO2: encontrar "<"
					//Redirigir la entrada a lo que exista despues de "<"
					else if(strcmp(fraseFragmentada[i], "<") == 0){
						int ret2;
						//si no existe fichero, se crea
						fd = open(fraseFragmentada[i+1], O_CREAT|O_RDONLY, 0400); //0400->permiso solo de lectura
						//EXAMPLE: cat<salida.txt -->coge por fichero en vez de por teclado
						if(fd<0){
							perror("OPEN_ERROR: %s \n");
						}
						//stdin contendra mismo numero que fd
						//luego se habra redirigido al fichero de interes 
						ret2 = dup2(fd,STDIN_FILENO); //redirigimos el stdin
						//dup2(a,b) El b sera una copia exacta de a
						if(ret2<0){
							perror("%s\n");
						}
						close(fd); //por convencion
						fraseFragmentada[i] = NULL; //termino la ejecucion
					}
					//CASO 3: encontrar ">>"
					//Redirigir la salida a lo que se encuentra tras >>. Se ADICIONA. 
					else if(strcmp(fraseFragmentada[i], ">>") == 0){
						int ret3;
						//se da por supuesto que existe
						fd = open(fraseFragmentada[i+1], O_APPEND|O_WRONLY|O_RDONLY, 0700); 
						if(fd<0){
							perror("OPEN_ERROR: %s \n");
						}
						//STDOUT tiene mismo numero fd que fd
						ret3 = dup2(fd,STDOUT_FILENO); //redirigimos el stdout
						//en nuestro caso concreto, añadimos al final de un fichero existente
						if(ret3<0){
							perror("%s\n");
						}
						close(fd); //convencion 
						fraseFragmentada[i] = NULL; //termino la ejecucion 
					}
					//Pipes!!
					/*Por problemas que desconozco, el programa da error
					 * si se introduce more kk.txt | grep hola
					 * en algunos ordenes de ejecucion aleatorios
					 * Si se ejecuta de acuerdo a las especificaciones,no
					 * ha deberia existir ningun problema
					 * VER FRAGMENTA DE PLANIFICADOR
					 */
					 //CASO 4: "|" -->more kk.txt | grep hola 
					 //more: more command is used to view the text files in the command prompt
					 //grep: is used to match coincidences and give output
					 //Proceso 1: ejecuta comando more. Salida se escribe en pipe.
					 //Proceso 2: ejecuta comando grep. Lee de pipe para tener una entrada decente. 
					else if (strcmp(fraseFragmentada[i],"|") == 0){
						char **second; //same type as fraseFragmentada
						int miTuberia[2];
						second = &fraseFragmentada[i+1];
						pipe(miTuberia); //tuberia contiene 2 file descriptors
						fraseFragmentada[i] = NULL;
						pid = fork();
						switch(pid){
							case 0 : //caso hijo
							         //ejecuta comando more del ejemplo
							         close(miTuberia[0]);
							         //stdout tiene mismo file descriptor que mituberia[1] 
							         dup2(miTuberia[1], STDOUT_FILENO);
							         close(miTuberia[1]);//convencion 
							         r = execvp(fraseFragmentada[0], fraseFragmentada);
							         if(r<1){
										 perror("%s\n");
										 borrarg(fraseFragmentada);
									 }
									 //se ejecuta fraseFragmentada[0]. 
									break;
							//SI DA ERROR		
							case -1 : 
							          perror("%s\n"); 
							          break;
							default: //caso padre-->ejecuta com. derecha
							        //en el ejemplo es grep 
							         close(miTuberia[1]);
							         //input se toma de canal de lectura de tuberia
							         dup2(miTuberia[0], STDIN_FILENO);
							         close(miTuberia[0]); //convencion 
							         free(fraseFragmentada); //liberamos la fraseFragmentada anterior 
							         fraseFragmentada = second;
							         second[i] = 0; //es lo mismo que NULL 
							         i = 0;
							         r = execvp(second[i], fraseFragmentada);
							         if(r<1){
										 perror("%s\n");
										 borrarg(second);
									 }
							         break;
				       }
				    }
					i++;
				}
                //Si se ha finalizado alguna ejecucion (salvo pipes)
                //se viene aqui para terminar de ejecutar los comandos 
				n=execvp(fraseFragmentada[0], fraseFragmentada); 
				if(n == -1){
					//si ha habido error, se liberan los recursos 
					printf("Comando no soportado realmente por la shell\n");
					borrarg(fraseFragmentada);
				}
			} 
		}
		else if(pid	!= 0 && pid !=-1){//si es padre
			signal(SIGINT, manejador1); //nos preparamos a la llegada de SIGINT
			//si llega, activamos manejador
			wait(&status); //esperamos a qque termine el hijo de ser ejecutado 
		}
	}while(1);
	return 0;    
	pause(); //espera signal 
} 
//FUNCIONES DEDICADAS AL TRES EN RAYA-->si se pone play en la shell se 
//se puede jugar

void play(){
    char reStart;
    again:
	run();
	printf("\nSi deseas jugar introduce 1: \nDe lo contrario lo que sea:");
	scanf("%s",&reStart);
	if(reStart == '1'){
            system("cls");
            goto again; //vuelve a la etiqueta	
        }
    else{
        exit(0);
	}
}

void run(){
    int count = 0;
    struct myDataType info; //info posee caracter y entero
    char symbol[9] = {'1','2','3','4','5','6','7','8','9'}; //valores iniciales de tablero
    Display(symbol);
    again:
    info = inputValue(symbol,count);
    symbol[info.i] = info.ch;
    system("cls");
    Display(symbol);
    if(check(symbol,info.ch,count)==1);
    else{
        count++;
        goto again;
    }
}

int check(char sym[9],char ch,int count){
    int i;
    for(i = 0;i<=6; i+=3)//it's for row
        if(sym[i] == ch && sym[i+1]==ch&&sym[i+2]==ch){
            printf("El ganador es : %c",ch);return 1;
        }
    for(i = 0;i<3; i++)//it's for column
        if(sym[i]==ch && sym[i+3]==ch&&sym[i+6]==ch){
            printf("El ganador es : %c",ch);return 1;
        }
    if(sym[0]==ch && sym[4]==ch&&sym[8]==ch){
            printf("El ganador es : %c",ch);return 1;
        }
    else if(sym[2]==ch && sym[4]==ch && sym[6]==ch){
            printf("El ganador es : %c",ch);return 1;
        }
    else if(count==8){
        printf("Increible, pero se ha producido un empate");
        return 1;
    }else return 0;
}

struct myDataType inputValue(char sym[9],int count){
    char value;
    int i;
    struct myDataType info;
    inputAgain:
    if(count%2 == 0){ //turnos pares juega obviamente el jugador  1
        printf("\nJuega J1 (X):");
    }else{
        printf("\nJuega J2 (O):");
    }
    scanf("%s",&value);
    for(i=0;i<9;i++){

        if(value == sym[i]){
            info.i = i;
            if(count%2 == 0)
                info.ch = 'X';
            else
                info.ch = 'O';
            break;
        }else{
            info.i = -1;
            info.ch = ' ';
        }
    }
    if(info.i == -1){
        printf("\nInput is not Valid");
        goto inputAgain;
    }
    return info;
}

void Display(char sym[9]){
	printf("\t\t\t\tTRES	EN	RAYA");
	printf("\nSimbolo del jugador 1: X");
	printf("\nSimbolo del jugador2: O");
	printf("\n\t\t\t       |       |       ");
	printf("\n\t\t\t   %c   |   %c   |   %c   ",sym[0],sym[1],sym[2]);
	printf("\n\t\t\t-------|-------|-------");
	printf("\n\t\t\t   %c   |   %c   |   %c   ",sym[3],sym[4],sym[5]);
	printf("\n\t\t\t-------|-------|-------");
	printf("\n\t\t\t   %c   |   %c   |   %c   ",sym[6],sym[7],sym[8]);
	printf("\n\t\t\t       |       |       ");
	printf("\n");
}


