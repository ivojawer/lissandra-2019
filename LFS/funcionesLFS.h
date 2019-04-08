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
#include <unistd.h>

void consola();
void mandarAEjecutarRequest(int request, char* parametros);
void Select(char* parametros);
void insert(char* parametros);
void create(char* parametros);
void describe(char* parametro);
void drop(char* parametro);

#endif /* FUNCIONESFM9_H_ */
