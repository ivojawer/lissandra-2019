#include "funcionesLFS.h"
#include <arpa/inet.h>//pasarlo a un .h
#include <sys/socket.h>
#include <commons/config.h>

void conexiones();
int crearConexion(int);
void messageHandler(char*,int);