#ifndef REQUESTS_H_
#define REQUESTS_H_

#define SELECT 0
#define INSERT 1
#define CREATE 2
#define DESCRIBE 3
#define DROP 4
#define JOURNAL 5
#define ADD 6
#define RUN 7
#define METRICS 8

#define SC 0
#define SHC 1
#define EC 2

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct{
	int requestEnInt;
	char* parametros;
}request;


int esUnaRequestValida(char* request, char* parametro);
int queRequestEs(char* palabra);
int esUnParametroValido(int request, char* parametro);
int queConsistenciaEs(char* string);
int devolverSoloRequest(char* request);
void liberarArrayDeStrings(char** array);
char* devolverSoloParametros(char* request);
void empaquetarYEnviarRequest(char* request, int aQuien);
char* recibirRequest(int deQuien, t_log* logger);


#include "requests.h"

#endif /* REQUESTS_H_ */
