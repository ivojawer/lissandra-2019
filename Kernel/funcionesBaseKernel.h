#ifndef FUNCIONESBASEKERNEL_H_
#define FUNCIONESBASEKERNEL_H_

#define MAXBUFFER 100

#define RAIZSCRIPTS "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Kernel/scripts/"

typedef struct {
	int idScript;
	int lineasLeidas;
	char* direccionScript;
	int esPorConsola;
} script;

typedef struct {
	int request;
	char* parametros;
} parametros_hiloScript;

typedef struct {
	char* nombreTabla;
	int consistencia;
} tablaEnLista;


#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <pthread.h>
#include "requests.h"
#include <unistd.h>
#include <commons/collections/list.h>
 #include <semaphore.h>

char* leerLinea(char* direccion,int lineaALeer);
int removerScriptDeLista(int id, t_list* lista);
int encontrarScriptEnLista(int id, t_list* lista);
int criterioDeTabla(char* nombreTabla);
char* devolverTablaDeRequest(char* request);
int esDescribeGlobal(char* request);
int existeArchivo(char* direccion);
void limpiarBuffer(char* buffer);
int charsDeBuffer(char* buffer);
char* scriptConRaiz(char* script);
int crearArchivoParaRequest(script* script, char* request);
int moverScript(int scriptID, t_list* listaOrigen, t_list* listaDestino);
void mostrarListaScripts(t_list* lista);

#endif /* FUNCIONESBASEKERNEL_H_ */
