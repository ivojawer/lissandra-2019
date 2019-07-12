#include "segmentos.h"


void agregarDato(segmento* miSegmento,uint16_t key, unsigned long long timestamp,char* value,int flagModificado){
	nuevoDato(miSegmento->tablaDePaginas,flagModificado,key,timestamp,value); //TODO: Marca de que key y timestamp estaban en int, los cambie a como deberian estar
}
