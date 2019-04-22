#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <pthread.h>
#include "requests.h"
#include <unistd.h>
#include "sockets.h"
#include <commons/collections/list.h>

typedef struct {
	int idScript;
	int lineasLeidas;
	int quantumRestante;
	char* direccionScript;
} script;

typedef struct {
	int request;
	char* parametros;
} parametros_hiloScript;


void consola();
void conexiones();
void planificador();

void crearScript(parametros_hiloScript* parametros);
int removerScriptDeLista(int id, t_list* lista);


#endif /* FUNCIONESKERNEL_H_ */
