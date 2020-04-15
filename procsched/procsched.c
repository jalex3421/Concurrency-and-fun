/*Planificador realizado por Alejandro Meza
 *FECHA DE INICIO DE PROYECTO: 11/11/2019
 *Simula colas de dos niveles apropiativos.
 *Nivel 1: FIFO
 *Nivel 2: Round Robin de 5 segundos
 *Prioridad mayor los proceso de menor nivel
 *Se admiten entradas tambien por fichero. 
 *FIN: se muestran cambios de contexto y procesos finalizados
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "fragmenta.h" //Se reutiliza y se corrige fragmenta de la practica anterior
#define clear() printf("\n")

//struct necesaria al trabajarse con colas de mensajes
//Mensaje compuesto de lo siguiente: 
//1.nivel (1 o 2)
//2.id -->lo mismo que pid. Lo identifica
//3.cadena: array donde se guarda el mensaje
struct msgbuf{
	long level; //se determina el nivel de prioridad
	long id; //es lo mismo que el pid
	char cadena[256]; //sting copy
};

char** fraseFragmentada;
int pid;
int idCola,r;  
int procesos_concluidos = 0;
int cambios_contexto = 0;
int pidant = -1; //para denotar que el proceso no es nuevo en la cola
int dead_child; //variable para indicar que un hijo esta muerto 
struct msgbuf msgbuffer;

//USO: accion que se invoca al finalizar el programa
void end_p(int sig){
	clear();
	printf("===========================================================\n");
	printf("\t\tEl programa ha finalizado\n");
	printf("\t\tNumero de procesos concluidos: %d\n", procesos_concluidos); 
	printf("\t\tNumero de cambios de contexto: %d\n",cambios_contexto);
	printf("============================================================\n");
	//se desencolan todos los procesos residuales: por eso el 0. 
	while(msgrcv(idCola, &(msgbuffer), sizeof(struct msgbuf)-sizeof(long), 0, IPC_NOWAIT) != -1){
		kill(msgbuffer.id, SIGTERM);
	} //mato a procesos residuales
	r = msgctl(idCola, IPC_RMID,NULL); //se borra la cola de forma permanente. 
	if(r == -1){
		perror("ERROR: %s \n");
	}
	kill(getppid(), SIGTERM); //se manda al padre ha terminar de manera amable
	exit(0); 
}

//USO: se determina que un proceso ha acabado.
//     Cuenta los procesos concluidos.
void our_dead_child(int sig){
	procesos_concluidos++;
	dead_child = 1;
	printf("El proceso finalizo\n");
}

//CON ESTA ACCION PAUSO UN P2 SI VIENE UN P1.
void prioridad(int sig){
	if(msgbuffer.level== 2){
		printf("interrumpido proceso de tipo 2\n");
		kill(msgbuffer.id, SIGSTOP); //SIGSTOP: detiene proceso. No ignorable. 
		if(dead_child == 0){ //proceso no muerto aÃºn:quedan cuantos 
			//CUARTO PARAMETRO: llamada a funcion queda bloqueada hasta que se pueda enviar el mensaje. 
			msgsnd(idCola, &msgbuffer,sizeof(struct msgbuf)-sizeof(long),0);
		}
		else{
			printf("Proceso ha sido finalizado\n"); 
		}
	}
	else{ //en caso de que sea prioridad 1, si llega otro de prioridad 1, no hay reaccion
			printf("no haga nada\n");
		}
}

//Modifica el valor de dead_child. 0 indica que sigue con vida.
void mod_dead_child(int dead_child){
	dead_child = 0; 
}

void start_scheduler(){
	clear();
	printf("\033[1;32m");
	printf("\n\n\n\n******************""************************");
	printf("\n\n\n\t****MY SCHEDULER****");
	printf("\n\n\n\n******************""************************");
	char *name = getenv("USER");
	printf("\n\n\n\tUSUARIO ES: @%s", name);
	clear();
	printf("\033[0m");
	clear();
	sleep(1);
}


int main(int argc,char *argv[]){
	char linea[256];
	int fd; 
	int n;
	int status;
	start_scheduler();
	idCola = msgget(IPC_PRIVATE, 0777|IPC_CREAT); //se crea la cola de mensajes
	if(idCola == -1){
		perror("%s\n"); 
		r = msgctl(idCola, IPC_RMID, NULL);
	}
	pid = fork();
	if(pid == -1){
		printf("Proceso no creado de manera eficiente\n"); 
	}
	//HIJO: planificador
	else if(pid ==0){ 
		printf("{ID: %ld} HIJO PLANIFICADOR\n",(long)getpid());
		clear();
		//SIGACTION(accion que hace cambiar comportamiento, accion asociada a signum, antigua reaccion)
		struct sigaction action; 
		action.sa_handler = &our_dead_child;//dead_child =1.Notifica que proceso acabo. 
		action.sa_flags=SA_NOCLDSTOP; //se avisa si hijo ha muerto, no se si se ha detenido. 
		sigaction(SIGCHLD,&action,NULL); //en caso de hijo muerto, activa su funcion 
		signal(SIGINT, &end_p); //reacciona a CTRL C
		while(1){
			dead_child = 0;
			/////////////////////FIFO////////////////////////////
			if(msgrcv(idCola,&(msgbuffer),sizeof(struct msgbuf)-sizeof(long),1,IPC_NOWAIT)>0){
				//desencola msgbuffer
				printf("MENSAJE DE TIPO %ld HA SIDO RECIBIDO\n",msgbuffer.level); 
				fraseFragmentada = fragmenta(msgbuffer.cadena);
				//hijos del planificador realizan un execvp
				msgbuffer.id = fork(); 
				if(msgbuffer.id == -1){
					perror("%s\n");
				}
				//Nieto ejecuta el proceso en cuestion
					else if ((msgbuffer.id)==0){
						printf("{ID: %ld} NIETO DEL PLANIFICADOR\n",(long)getpid());
						int e = execvp(fraseFragmentada[0], fraseFragmentada);
						if(e == -1){
							printf("Se ha producido un error en la fragemtacion\n"); 
							borrarg(fraseFragmentada); 
						}
					}
					else{ //proceso padre dentro del planificador
						signal(SIGUSR1,&prioridad); //Atento a la llegada de proceso con prioridad 1
						printf("{ID: %ld} HIJO DEL PLANIFICADOR\n",(long)getpid());
						printf("pidant pasa de ser %d a %ld\n", pidant,msgbuffer.id);
						pidant = msgbuffer.id; 
					    waitpid(msgbuffer.id, &status, 0); 
					}
				
		}
		//////////////////////FIN FIFO/////////////////////////////////////////
								
		///////////////////ROUND ROBIN 5 /////////////////////////////////////////
		else if(msgrcv(idCola, &(msgbuffer),sizeof(struct msgbuf)-sizeof(long),2,IPC_NOWAIT)>0){
			printf("Estoy leyendo de prioridad 2\n");
			//MOD: necesaria, pues si un proceso muere variable estaria en 1. 
			//     Procesos restantes se considerarian tambien muertos 
			mod_dead_child(dead_child);
			printf("MENSAJE DE TIPO %ld ha sido recibido\n", msgbuffer.level); 
			//si pid es nuevo o id es 0 (por defecto determinada para un nuevo proceso)
			//se cuentan cambios de contexto  
			if((msgbuffer.id != pidant) || (msgbuffer.id == 0)){
				cambios_contexto+=1;
				printf("Se produce un nuevo cambio de contexto\n");
			}
			//PROCESO ES NUEVO
			if(msgbuffer.id == 0){
				printf("p2 va a fragmnetar\n"); //testeo
				fraseFragmentada = fragmenta(msgbuffer.cadena);
				printf("p2 ya ha fragmentado\n"); //testeo
				msgbuffer.id = fork();
	            
				if(msgbuffer.id == -1){
					perror("%s\n"); 
				}
				else if(msgbuffer.id == 0){
					printf("{ID: %ld} NIETO DEL PLANIFICADOR\n",(long)getpid());
					int er = execvp(fraseFragmentada[0], fraseFragmentada);
					if(er == -1){
						printf("Se ha producido un error en la fragemntacion\n"); 
						borrarg(fraseFragmentada); 
					}
				}
				//padre
				else{
					//atento si llega uno de proceso 1
					signal(SIGUSR1,&prioridad);
					printf("{ID: %ld} HIJO DEL PLANIFICADOR\n",(long)getpid());
					printf("pidant pasa de ser %d a %ld\n", pidant,msgbuffer.id);
					pidant = msgbuffer.id;
					sleep(5); 
					kill(msgbuffer.id, SIGSTOP); //se acaban los cuantos de 5 uds
					if(dead_child == 0){ //proceso no muerto aÃºn:quedan cuantos
						printf("ID:%ld Proceso se vuelve a encolar\n",msgbuffer.id); 
						msgsnd(idCola, &msgbuffer,sizeof(struct msgbuf)-sizeof(long),0);
					}
					else{
						printf("{ID:%ld} Proceso ha sido finalizado\n",(long)getpid()); 
					}
					//waitpid(pid, &status, 0);//modificacion
				}	
			}
			//PROCESO NO ES NUEVO
			else{  
				kill(msgbuffer.id, SIGCONT);
				//MOD
                signal(SIGUSR1,&prioridad);
				sleep(5); //MOD
				kill(msgbuffer.id, SIGSTOP); //se acaban los cuantos de 5 uds
				if(dead_child == 0){
					printf("ID:%ld Proceso se vuelve a encolar\n",msgbuffer.id); 
					msgsnd(idCola, &msgbuffer,sizeof(struct msgbuf)-sizeof(long),0);
				}
				else{
					printf("{ID:%ld} Proceso ha sido finalizado\n",(long)getpid()); 
				}
			}
		}   
	}
	}
	//////////////////////FIN ROUND ROBIN///////////////////////////////////////
	//PADRE ENCOLADOR: cola ya esta creada al principio del programa. 
	else if(pid!=0 && pid!=-1){ 
		clear();
		printf("{ID: %ld} PADRE ENCOLADOR", (long)getpid()); //printf del pid actual. 
		clear();
		if(argc == 2){ //si esta el fichero
			printf("confingfile.txt has been found\n");
			fd = open(argv[1],O_RDONLY);
			if(fd == -1){
				perror("ERROR: %s\n");
			}
			//STDIN posee mismo numero de file descriptor que fd. 
			//Luego entrada: lo que lea de fichero determinado 
			n= dup2(fd, STDIN_FILENO);
			if(n<0){
					perror("%s\n");
					}
			close(fd);
		}
		else{
			printf("confingfile.txt has  not been found\n");
		}
		// se guarda el nivel del mensaje. Se guarda en linea. 
		//MENSAJE: nivel nombreprograma arg1 arg2 arg3
		while(scanf("%ld %[^\n]",&(msgbuffer.level),linea) != EOF){ //
			getchar(); //para coger salto de linea 
			msgbuffer.id = 0; //para luego ver si un proceso habia entrado con anterioridad
			strcpy(msgbuffer.cadena, linea); // cadena posee lo mismo que linea
			printf("Se encola mensaje con prioridad %ld\n",msgbuffer.level); 
			//sleep(1);
			printf("%s\n", msgbuffer.cadena); //testeo
			msgsnd(idCola, &(msgbuffer),sizeof(struct msgbuf)-sizeof(long), 0);
			sleep(3);
			//por razones que desconozco, con este sleep(3) se pueden ver los resultados tambien por fichero
			//supongo que es algun error de sincronizacion
			if(msgbuffer.level == 1){ //si es FIFO
				printf("SIGUSR1 has been sent\n");
				kill(pid,SIGUSR1);
			}
		}
		printf("EOF\n"); 
		waitpid(pid, &status, 0);
		//borro la cola en caso de un funcionamiento ideal 
		r = msgctl(idCola, IPC_RMID,NULL);
		if(r == -1){
		perror("ERROR: %s \n");
		} 	
		}			
	return 0; 
} 
