
#ifndef GOSSIPING_H_
#define GOSSIPING_H_

#include "funcionesMemoria.h"


void gossiping();









void agregarSeed(char* ip, int port);
void cargarSeedsIniciales();
void agregarIPsYPORTsALista(char**,char**);
void mostrarSeeds();
void mostrarSeed();

#endif /* GOSSIPING_H_ */
