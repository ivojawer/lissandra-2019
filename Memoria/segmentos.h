/*
 * segmentos.h
 *
 *  Created on: May 16, 2019
 *      Author: ivan
 */

#ifndef SEGMENTOS_H_
#define SEGMENTOS_H_

#include "funcionesMemoria.h";

typedef struct{
	char* nombreDeTabla;
	t_list* tablaDePaginas;
}segmento;



void agregarDato(segmento*,int key, int timestamp,char* value,int flagModificado);



#endif /* SEGMENTOS_H_ */
