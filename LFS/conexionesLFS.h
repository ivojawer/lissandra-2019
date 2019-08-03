#ifndef CONEXIONESLFS_H_
#define CONEXIONESLFS_H_

#include "funcionesLFS.h"

void aceptar_conexiones();
int crearConexion(int);
void messageHandler(char*);
void crearPuntoDeMontaje();
struct socket_info{
	int socket_memoria;
};

#endif /* CONEXIONESLFS_H_ */
