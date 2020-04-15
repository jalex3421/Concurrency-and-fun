/* AUTOR: Alejandro Meza Tudela
/ FECHA: 21/10/2019
* Implementacion del modulo fragmenta
*/

#include <stdio.h>
#include "fragmenta.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>


char **fragmenta(const char *cadena){
	char *copy; //array which copy original array
	copy = (char*)malloc(strlen(cadena));
	//first destiny, then source 
	strcpy(copy, cadena);
	char *token;
	//start to count tokens-->strtok 
	int counter = 0; 
	token = strtok((char*)cadena, " ");
	if(token == NULL){
		perror("%s\n");
		exit(-1);
	}
	//counter = 1;
	while(token != NULL){
		//token = strtok(NULL, " "); 
		if(token != NULL){
			counter++;
		}
		token = strtok(NULL, " "); 
	}
	//we have made our first traversal
	char **arg;
	arg = (char**)malloc((counter*sizeof(char*)) +1);
	counter = 0;
	token = strtok(copy, " ");
	if( token == NULL){
		perror("%s\n");
		exit(-1);
	}
	while(token!=NULL){
		//token = strtok(NULL, " ");
		if(token !=	 NULL){
				arg[counter] = (char*)malloc(sizeof(char)*strlen(token));
				strcpy(arg[counter], token);
				counter++; 
		}
		token = strtok(NULL, " ");
	}
	arg[counter] = NULL; //ok?
	return arg;
}

void borrarg(char **arg){
	int i = 0; 
	while(arg[i] != NULL){
		free(arg[i]);
		i++;
	}
	free(arg); 
}
	
/*
int main(){

    char cadena[150];
    char** fraseFragmentada;
    int i;
    
    strcpy(cadena,"Esto es solo un ejemplo guay\0"); //Suponemos que el usuario introduce esta frase
    
    printf("El usuario ha introducido la frase:\n%s\n",cadena);
    printf("Si le aplico la fragmentación, puedo recorrer el array mostrando cada token:\n");
    fraseFragmentada = fragmenta(cadena); // Fragmento mi cadena
    i = 0; // Comienzo leyendo el elemento 0 de mi array
    while(fraseFragmentada[i] != NULL){ // Recorro mi array fragmentado hasta que encuentre un NULL
        printf("Token nº %d: %s (Posición de memoria %p)\n", i, fraseFragmentada[i], fraseFragmentada[i]);
        i++;
    }
    printf("Encontrado fin de array, NULL\n");
    return 0;
}*/
