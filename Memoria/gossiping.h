
#ifndef GOSSIPING_H_
#define GOSSIPING_H_

#include "funcionesMemoria.h"

typedef struct{
	char* ip;
	int puerto;
}seed;

void gossiping();









void agregarSeed(char* ip, int port);
void cargarSeedsIniciales();
void agregarIPsYPORTsALista(char**,char**);
void mostrarSeeds();
void mostrarSeed();

#endif /* GOSSIPING_H_ */
