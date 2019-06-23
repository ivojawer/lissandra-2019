#include "funcionesMemoria.h"
#include <arpa/inet.h>//pasarlo a un .h
#include <sys/socket.h>
#include <commons/config.h>

void messageHandler(char*,int);
void aceptarConexiones();
int primeraConexionLFS();
void conexionLFS();
void comunicacionConKernel();
void comunicacionConLFS();
void manejoErrorKernel();
void manejoErrorLFS();
void enviarRespuestaAlKernel(int id, int respuesta);
void conectarseAOtraMemoria(seed* laSeed);
int posicionMemoriaEnLista(int nombreMemoria);
int memoriaYaEstaConectada(int nombreMemoria);
void enviarRegistroComoInsert(registro* registroAEnviar);
