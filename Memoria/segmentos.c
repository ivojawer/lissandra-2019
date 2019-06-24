#include "segmentos.h"


void agregarDato(segmento* miSegmento,int key, int timestamp,char* value,int flagModificado){
	nuevoDato(miSegmento->tablaDePaginas,flagModificado,key,timestamp,value); //TODO: Cambiar tipos key y timestamp
}
