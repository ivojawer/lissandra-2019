#ifndef GOSSIPING_H_
#define GOSSIPING_H_

#include "funcionesMemoria.h"


void gossiping();

int seedNoEstaConectada(seed* unaSeed);
void agregarSeed(char* ip, int port);
void cargarSeedsIniciales();
void agregarIPsYPORTsALista(char**,char**);
void mostrarSeeds();
void mostrarSeed();
int esLaMismaSeed(seed* unaSeed, seed* otraSeed);
void tratarDeConectarseASeeds();
void liberarSeed(seed* seedALiberar);
void agregarNuevasSeeds(t_list* seeds);

#endif /* GOSSIPING_H_ */
