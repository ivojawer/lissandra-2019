#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#define MAXBUFFER 100

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
 #include <semaphore.h>

typedef struct {
	int idScript;
	int lineasLeidas;
	char* direccionScript;
} script;

typedef struct {
	int request;
	char* parametros;
} parametros_hiloScript;

typedef struct {
	char* nombreTabla;
	int consistencia;
} tablaEnLista;


void consola();
void conexiones();
void planificadorREADYAEXEC();
void planificadorEXEC(int IdScript);

void crearScript(parametros_hiloScript* parametros);
int removerScriptDeLista(int id, t_list* lista);
int existeArchivo(char* direccion);
int encontrarScriptEnLista(int id, t_list* lista);
int ejecutarRequest(char* request);
char* leerLinea(char* direccion,int lineaALeer);
void metrics();

#endif /* FUNCIONESKERNEL_H_ */
