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



typedef struct {
	int timestamp;
	int key;
	char value;
}marco;

typedef struct{
	marco* dato;
	int flagModificado;
}pagina;


t_list* crearTablaSegmentos();
t_list* crearTablaPaginas();
void nuevaTabla(t_list*,char*);
pagina* nuevoDato(t_list* tablaPaginas,int flagModificado,int key, int timestamp, char* value,void*);
void consola();
void mandarAEjecutarRequest(request* requestAEjecutar);
void mandarCreateALFS(char*,char*,int,char*);
void Select(char* parametros,t_list*);
void insert(char* parametros,void*);
void create(char* parametros);
void describe(char* parametro);
void drop(char* parametro);

#endif /* FUNCIONESMEMORIA_H_ */
