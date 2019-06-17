#ifndef REQUESTS_H_
#define REQUESTS_H_

#define SC 0
#define SHC 1
#define EC 2

#define SELECT 100
#define INSERT 101
#define CREATE 102
#define DESCRIBE 103
#define DROP 104
#define JOURNAL 105
#define ADD 106
#define RUN 107
#define METRICS 108

//Cosos de comunicacion:

//Operaciones:
#define HANDSHAKE 200
#define GOSSIPING 201
#define REQUEST 202
#define REGISTRO 203
#define DATO 204
#define OP_JOURNAL 205
#define RESPUESTA 206
#define METADATAS 207

//Respuestas:
#define ERROR 300
#define TODO_BIEN 301
#define MEM_LLENA 302

//Identificadores de modulo:
#define KERNEL 400
#define MEMORIA 401
#define LFS 402

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/collections/list.h>
#include<readline/history.h>

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

typedef struct {
	int timestamp;
	int key;
	char* value;
} registro;

//Analisis de requests:
int esUnaRequestValida(char* requestEnString);
int queRequestEs(char* palabra);
int esUnParametroValido(int request, char* parametro);
int queConsistenciaEs(char* string);
int esDescribeGlobal(request* request);

//Cosas de requests:
request* crearStructRequest(char* requestEnString);
char* requestStructAString(request* request);
void liberarRequest(request* request);

//MISC:
int str_first_index_of(char c, char* cadena);
int str_last_index_of(char c, char* cadena);
int lista_vacia(t_list *lista);
void liberarArrayDeStrings(char** array);

//Comunicacion:
void enviarInt(int aQuien, int intAEnviar);
void enviarIntConHeader(int aQuien, int intAEnviar, int header);
void enviarVariosIntsConHeader(int aQuien, t_list* intsAEnviar, int header);
int recibirInt(int deQuien, t_log* logger);

void enviarStringConHeaderEId(int aQuien, char* string, int header, int id);
void enviarStringConHeader(int aQuien, char* string, int header);
char* recibirString(int deQuien, t_log* logger);

void enviarRequestConHeaderEId(int aQuien, request* requestAEnviar, int header,
		int id);
void enviarRequestConHeader(int aQuien, request* requestAEnviar, int header);
request* recibirRequest(int deQuien, t_log* logger);

void enviarMetadatasConHeaderEId(int aQuien, t_list* metadatas, int header,
		int id);
void enviarMetadatasConHeader(int aQuien, t_list* metadatas, int header);
t_list* recibirMetadatas(int deQuien, t_log* logger);

void enviarSeedsConHeader(int aQuien, t_list* seeds, int header);
t_list* recibirSeeds(int deQuien, t_log* logger);

void enviarRegistroConHeaderEId(int aQuien, registro* unRegistro, int header,
		int id);
registro* recibirRegistro(int deQuien, t_log* logger);

//Sockets:
int crearServidor(int puerto);
int conectarseAServidor(char* ip, int puerto);

#include "requests.h"

#endif /* REQUESTS_H_ */
