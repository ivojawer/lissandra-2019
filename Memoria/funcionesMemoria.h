#ifndef FUNCIONESMEMORIA_H_
#define FUNCIONESMEMORIA_H_


#define RAIZCONFIG "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/"

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
#include <math.h>
#include "gossiping.h"
#include <semaphore.h>


typedef struct {
	unsigned long long timestamp;
	uint16_t key;
	char value;
}marco;

typedef struct {
	int id;
	sem_t semaforoDeHilo;
	void* punteroARespuesta;
}hiloDeEjecucion;

typedef struct{
	bool vacio;
}disponibilidad;

typedef struct{
	int nroMarco;
	int ultimoUso;
	int flagModificado;
}pagina;

typedef struct{
	request* laRequest;
	int idKernel;
}requestConID;

typedef struct{
	int nombre;
	int elSocket;
	seed* laSeed;
}memoriaGossip;

void* comienzoMemoria;
disponibilidad* marcos;
int cantMarcos;
int tamanioMarco;

void ejecutarRequests();
segmento* encuentroTablaPorNombre(char* nombreTabla);
marco* getMarcoFromPagina(pagina*);
marco* getMarcoFromIndex(int);
bool filtroNombreTabla( char*,segmento*);
t_list* crearTablaSegmentos();
t_list* crearTablaPaginas();
segmento* nuevaTabla(t_list*,char*);
pagina* nuevoDato(t_list* tablaPaginas, int flagModificado, uint16_t key,
		unsigned long long  timestamp, char* value);
pagina* getPagina(uint16_t key, char* nombreTabla);
void consola();
void mandarAEjecutarRequest(request* requestAEjecutar);
void mandarRequestALFS(int,char*);
void Select(char* parametros);
void insert(char* parametros);
void create(char* parametros);
void describe(char* parametro);
void drop(char* parametro);
void journal();
void journalAutomatico();
void refreshConfig();
unsigned long long tiempoActual();
void reconexionLFS();

#endif /* FUNCIONESMEMORIA_H_ */
