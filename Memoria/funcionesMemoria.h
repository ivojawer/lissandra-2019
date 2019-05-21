#ifndef FUNCIONESMEMORIA_H_
#define FUNCIONESMEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <pthread.h>
#include "requests.h"
#include <string.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include "segmentos.h"
#include "conexionesMem.h"
#include <stdbool.h>



typedef struct {
	int timestamp;
	int key;
	char value;
}marco;

typedef struct{
	bool vacio;
	bool recentlyUsed;//esto es un bool?
}disponibilidad;

typedef struct{
	marco* dato;
	int flagModificado;
}pagina;


void* comienzoMemoria;
disponibilidad marcos[];

segmento* encuentroTablaPorNombre(char* nombreTabla, t_list* tablaDeSegmentos);
bool filtroNombreTabla( char*,segmento*);
t_list* crearTablaSegmentos();
t_list* crearTablaPaginas();
segmento* nuevaTabla(t_list*,char*);
pagina* nuevoDato(t_list* tablaPaginas,int flagModificado,int key, int timestamp, char* value);
pagina* getPagina(t_list* tablaSegmentos,int key, char* nombreTabla);
void consola();
void mandarAEjecutarRequest(request* requestAEjecutar);
void mandarSelectALFS(char*,int);
void mandarCreateALFS(char*,char*,int,char*);
void Select(char* parametros);
void insert(char* parametros);
void create(char* parametros);
void describe(char* parametro);
void drop(char* parametro);

#endif /* FUNCIONESMEMORIA_H_ */
