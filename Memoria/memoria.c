#include "requests.h"
#include "funcionesMemoria.h"


t_config*config;
t_log* logger;

int main() {


	logger = log_create("memoria.log", "memoria", 1, 0);

	config =config_create("memoria.config"); //DUDA: Si es un config por memoria esto va en la carpeta CONFIG tmb  lo hago una por proyecto como aca

	pthread_t h_consola;
	pthread_t h_conexiones;

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);

	//reservo toda la memoria
	int tamanioMemoria =config_get_int_value(config,"TAM_MEM");
	log_info(logger,"cree mi memoria tamanio: %d",tamanioMemoria);
	void* comienzoMemoria = malloc(tamanioMemoria);

	//divido en marcos
	int caracMaxDeValue = config_get_int_value(config,"CANT_MAX_CARAC");
	int tamanioMarco = caracMaxDeValue * sizeof(char) + sizeof(int) + sizeof(int); //value + key + timestamp
	log_info(logger,"el tamanio de mi marco es: %d", tamanioMarco);
	int cantMarcos = tamanioMemoria/tamanioMarco; //tamanio marco siempre es mult de 2 asi que da redondo
	log_info(logger,"cantidad de marcos: %d",cantMarcos);




	//creo tabla de segmentos
	t_list* tablaSegmentos = crearTablaSegmentos();

	//pruebas
	nuevaTabla(tablaSegmentos,"TABLA1");

	segmento* miSegmento = tablaSegmentos->head->data;
	log_info(logger,"Nombre de mi nueva tabla es: %s",miSegmento->nombreDeTabla);


	agregarDato(miSegmento,123,0,"hola",1,comienzoMemoria);

	pagina* datoAgregado= miSegmento->tablaDePaginas->head->data;
	log_info(logger,"dato agregado: key=%d - timestamp=%d - value=%s" , datoAgregado->dato->key,datoAgregado->dato->timestamp,&datoAgregado->dato->value);

	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	free(comienzoMemoria);
	return 1;
}
