/*
 * segmentos.h
 *
 *  Created on: May 16, 2019
 *      Author: ivan
 */

#ifndef SEGMENTOS_H_
#define SEGMENTOS_H_

#include "funcionesMemoria.h"

typedef struct{
	char* nombreDeTabla;
	t_list* tablaDePaginas;
}segmento;



void agregarDato(segmento* miSegmento,int key, int timestamp,char* value,int flagModificado,void* comienzoMarco);



#endif /* SEGMENTOS_H_ */
