#include "funcionesMemoria.h"

#ifndef SEGMENTOS_H_
#define SEGMENTOS_H_


typedef struct{
	char* nombreDeTabla;
	t_list* tablaDePaginas;
}segmento;



void agregarDato(segmento* miSegmento,uint16_t key, unsigned long long timestamp,char* value,int flagModificado);



#endif /* SEGMENTOS_H_ */
