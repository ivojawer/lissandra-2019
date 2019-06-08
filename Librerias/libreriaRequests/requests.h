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

#define MEMORIA_ERROR -1
#define MEMORIA_BIEN 1

#define KERNEL 0
#define MEMORIA 1
#define LFS 2

#define MAXBUFFER 100

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/collections/list.h>
#include <dirent.h>

typedef struct{
	int requestEnInt;
	char* parametros;
}request;

typedef struct{

	char* nombre;
	int consistencia;
	int particiones;
	int compactTime;

}metadataTablaLFS;

typedef struct {
	int timestamp;
	int key;
	char* value;
} registro;

int esUnaRequestValida(char* requestEnString);
int queRequestEs(char* palabra);
int get_timestamp();
int esUnParametroValido(int request, char* parametro);
int queConsistenciaEs(char* string);
void liberarArrayDeStrings(char** array);
request* crearStructRequest(char* requestEnString);
char* requestStructAString(request* request);
int esDescribeGlobal (request* request);
void liberarRequest(request* request);
char* recibirString(int deQuien, t_log* logger);
void enviarString(char* string, int aQuien);
void enviarMetadatas(t_list* metadatas, int aQuien);
t_list* recibirMetadatas (int deQuien, t_log* logger);
void enviarRequest (int aQuien, request* requestAEnviar);
request* recibirRequest(int deQuien,t_log* logger);
int recibirInt(int deQuien, t_log* logger);
void enviarInt (int intAEnviar, int aQuien);
int crearConexion(int puerto,char* ip);
char* get_value(char* string);
char* leerLinea(char* direccion, int lineaALeer);
#include "requests.h"

#endif /* REQUESTS_H_ */
