#ifndef FUNCIONESLFS_H_
#define FUNCIONESLFS_H_

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <pthread.h>
#include "requests.h"
#include <unistd.h>
#include <commons/collections/list.h>
#include <sys/stat.h>
#include <sys/types.h>



typedef struct {
	char* nombreTabla;
	t_list* datosAInsertar;
} t_tablaEnMemTable;

typedef struct {
	int timestamp;
	int key;
	char* value;
} registro;

void consola();
void conexiones();
void mandarAEjecutarRequest(request* requestAEjecutar);
void Select(char* parametros);
void insert(char* parametros);
void create(char* parametros);
void describe(char* parametro);
void drop(char* parametro);
int tablaYaExiste(char* nombreTabla);
void crearTablaEnMemTable(char* nombreDeTabla);
int tablaExisteEnMemTable(char* nombreDeLaTabla);
t_tablaEnMemTable* getTablaPorNombre(t_list* memoriaTemp, char* nombreDeTabla);
//t_tablaEnMemTable* ultimaTabla(t_list* memTemp);
//t_tablaEnMemTable* ultimoDato(t_list* memTemp);
#endif /* FUNCIONESLFS_H_ */
