#ifndef FUNCIONESLFS_H_
#define FUNCIONESLFS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <pthread.h>
#include "requests.h"
#include "sockets.h"
#include <unistd.h>
#include <commons/collections/list.h>
#include <sys/stat.h>
#include <sys/types.h>



typedef struct {
	char* nombreTabla;
	t_list* datosAInsertar;
} t_tablaEnMemTable;

void consola();
void conexiones();
void mandarAEjecutarRequest(int request, char* parametros);
void Select(char* parametros);
void insert(char* parametros);
void create(char* parametros);
void describe(char* parametro);
void drop(char* parametro);

#endif /* FUNCIONESLFS_H_ */
