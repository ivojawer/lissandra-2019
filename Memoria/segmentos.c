#include "segmentos.h";


void agregarDato(segmento* miSegmento,int key, int timestamp,char* value,int flagModificado,void* comienzoMarco){
	nuevoDato(miSegmento->tablaDePaginas,flagModificado,key,timestamp,value,comienzoMarco);
}
