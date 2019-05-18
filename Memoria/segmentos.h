#include "funcionesMemoria.h"

#ifndef SEGMENTOS_H_
#define SEGMENTOS_H_


typedef struct{
	char* nombreDeTabla;
	t_list* tablaDePaginas;
}segmento;



void agregarDato(segmento* miSegmento,int key, int timestamp,char* value,int flagModificado,void* comienzoMarco);



#endif /* SEGMENTOS_H_ */
