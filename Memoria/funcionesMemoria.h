#ifndef FUNCIONESMEMORIA_H_
#define FUNCIONESMEMORIA_H_

#define DIRCONFIG "memoria.config"

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
#include <time.h>
#include <stdbool.h>
#include "gossiping.h"


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
disponibilidad* marcos;
int cantMarcos;
int tamanioMarco;

segmento* encuentroTablaPorNombre(char* nombreTabla);
bool filtroNombreTabla( char*,segmento*);
t_list* crearTablaSegmentos();
t_list* crearTablaPaginas();
segmento* nuevaTabla(t_list*,char*);
pagina* nuevoDato(t_list* tablaPaginas,int flagModificado,int key, int timestamp, char* value);
pagina* getPagina(int key, char* nombreTabla);
void consola();
void mandarAEjecutarRequest(request* requestAEjecutar);
void mandarALFS(int,char*);
void Select(char* parametros);
void insert(char* parametros);
void create(char* parametros);
void describe(char* parametro);
void drop(char* parametro);
void journal();

#endif /* FUNCIONESMEMORIA_H_ */
