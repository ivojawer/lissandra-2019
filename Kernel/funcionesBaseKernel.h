#ifndef FUNCIONESBASEKERNEL_H_
#define FUNCIONESBASEKERNEL_H_

#define MAXBUFFER 300

#define RAIZSCRIPTS "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Kernel/scripts/"
#define DIRCONFIG "/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/kernel.config"

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
#include <semaphore.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/config.h>

typedef struct {
	int idScript;
	int lineasLeidas;
	char* direccionScript;
	int esPorConsola;
	sem_t semaforoDelScript;
	void* resultadoDeEnvio;
} script;

typedef struct {
	int request;
	char* parametros;
} parametros_hiloScript;

typedef struct {
	time_t duracion;
	time_t tiempoInicio;
} tiempoDeOperacion;

typedef struct {
	char* ip;
	int puerto;
	int* consistencias;
	int nombre;
	int socket;
	int estaViva;
	t_list* scriptsEsperando;
	sem_t sem_cambioScriptsEsperando;
} memoriaEnLista;

void journal();
char* leerLinea(char* direccion, int lineaALeer);
int removerScriptDeLista(int id, t_list* lista);
script* encontrarScriptEnLista(int id, t_list* lista);
int criterioDeTabla(char* nombreTabla);
char* devolverTablaDeRequest(request* request);
int esDescribeGlobal(request* request);
int existeArchivo(char* direccion);
void limpiarBuffer(char* buffer);
int charsDeBuffer(char* buffer);
char* scriptConRaiz(char* script);
int crearArchivoParaRequest(script* script, request* requestAArchivo);
int moverScript(int scriptID, t_list* listaOrigen, t_list* listaDestino);
void mostrarListaScripts(t_list* lista);
void limpiarListasTiempos();
int promedioDeTiemposDeOperaciones(t_list* tiempos);
int esMasNuevoQue30Segundos(tiempoDeOperacion tiempoOperacion);
void insertarTiempo(time_t tiempoInicial, time_t tiempoFinal, int request);
t_list* filterCasero_esMasNuevoQue30Segundos(t_list* tiempos);
void matarListas();
int encontrarPosicionDeMemoria(int memoriaAEncontrar);
int memoriaECSiguiente(int memoriaInicialEC);
void enviarRequestAMemoria(request* requestAEnviar, int memoria);
int recibirRespuestaDeMemoria(int memoria);
int determinarAQueMemoriaEnviar(request* unaRequest);
int unaMemoriaCualquiera();
int memoriaHash(int key);
void matarMemoria(int nombreMemoria);
int seedYaExiste(seed* unaSeed);
void actualizarMetadatas(t_list* metadatas);
void agregarUnaMetadata(metadataTablaLFS* unaMetadata);
int manejarRespuestaDeMemoria(script* elScript, request* laRequest, int memoria);
int laMemoriaTieneConsistencias(memoriaEnLista* unaMemoria);
void sacarScriptDeEspera(int nombreScript, memoriaEnLista* laMemoria);
int existeTabla(char* nombreTabla);
void enviarPeticionesDeGossip();

#endif /* FUNCIONESBASEKERNEL_H_ */
