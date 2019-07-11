#ifndef FUNCIONESMEMORIA_H_
#define FUNCIONESMEMORIA_H_


#define DIRCONFIG "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/memoria.config"

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
#include "journalAutomatico.h"
#include <stdbool.h>
#include <math.h>
#include "gossiping.h"
#include <semaphore.h>


typedef struct {
	int timestamp; //TODO: Cambiar
	int key;
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
pagina* nuevoDato(t_list* tablaPaginas, int flagModificado, int key,
		int timestamp, char* value);
pagina* getPagina(int key, char* nombreTabla);
void consola();
void mandarAEjecutarRequest(request* requestAEjecutar);
void mandarRequestALFS(int,char*);
char* Select(char* parametros);
int insert(char* parametros);
int create(char* parametros);
t_list* describe(char* parametro);
int drop(char* parametro);
void journal();
int enviarYRecibirSeeds(memoriaGossip* memoriaDestino);

#endif /* FUNCIONESMEMORIA_H_ */
