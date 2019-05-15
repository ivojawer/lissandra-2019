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
#include <unistd.h>
#include <commons/collections/list.h>

typedef struct{
	char* nombreDeTabla;
	t_list* tablaDePaginas;
}segmento;

struct {
	int timestamp;
	int key;
	char* value;
	int flagModificado;
}pagina;

void consola();
void conexiones();
void mandarAEjecutarRequest(request* requestAEjecutar);
void mandarCreateALFS(char*,char*,int,char*);
void Select(char* parametros);
void insert(char* parametros);
void create(char* parametros);
void describe(char* parametro);
void drop(char* parametro);

#endif /* FUNCIONESMEMORIA_H_ */
