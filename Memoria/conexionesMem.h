#include "funcionesMemoria.h"
#include <arpa/inet.h>//pasarlo a un .h
#include <sys/socket.h>
#include <commons/config.h>

void conexiones();

void messageHandler(char*,int);
