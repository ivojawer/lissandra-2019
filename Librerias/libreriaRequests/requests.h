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

//Cosos de comunicacion:

//Operaciones:
#define HANDSHAKE 0
#define GOSSIPING 1
#define REQUEST 2
#define REGISTRO 3
#define DATO 4
#define OP_JOURNAL 5
#define RESPUESTA 6
#define METADATAS 7

//Respuestas:
#define ERROR 0
#define TODO_BIEN 1

//Identificadores de modulo:
#define KERNEL 0
#define MEMORIA 1
#define LFS 2

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/collections/list.h>

typedef struct {
	int requestEnInt;
	char* parametros;
} request;

typedef struct {

	char* nombre;
	int consistencia;
	int particiones;
	int compactTime;

} metadataTablaLFS;

typedef struct {
	char* ip;
	int puerto;
} seed;

//Cosas de requests:
int esUnaRequestValida(char* requestEnString);
int queRequestEs(char* palabra);
int esUnParametroValido(int request, char* parametro);
int queConsistenciaEs(char* string);
void liberarArrayDeStrings(char** array);
request* crearStructRequest(char* requestEnString);
char* requestStructAString(request* request);
int esDescribeGlobal(request* request);
void liberarRequest(request* request);

//Comunicacion:
void enviarIntConHeader(int intAEnviar, int header, int aQuien);
int recibirInt(int deQuien, t_log* logger);

void enviarStringConHeaderEId(char* string, int aQuien, int header, int id);
void enviarStringConHeader(char* string, int aQuien, int header);
char* recibirString(int deQuien, t_log* logger);

void enviarRequestConHeaderEId(int aQuien, request* requestAEnviar, int header,
		int id);
void enviarRequestConHeader(int aQuien, request* requestAEnviar, int header);
request* recibirRequest(int deQuien, t_log* logger);

void enviarMetadatasConHeaderEId(t_list* metadatas, int aQuien, int header,
		int id);
void enviarMetadatasConHeader(t_list* metadatas, int aQuien, int header);
t_list* recibirMetadatas(int deQuien, t_log* logger);

void enviarSeedsConHeader(int aQuien, t_list* seeds, int header);
t_list* recibirSeeds(int deQuien, t_log* logger);

void enviarVariosIntsConHeader(t_list* intsAEnviar, int header, int aQuien);

void enviarInt(int intAEnviar, int aQuien);

//Sockets:
int crearServidor(int puerto);
int conectarseAServidor(char* ip, int puerto);

#include "requests.h"

#endif /* REQUESTS_H_ */
