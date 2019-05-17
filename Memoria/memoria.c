#include "requests.h"
#include "funcionesMemoria.h"


t_config*config;
t_log* logger;

int main() {

	logger = log_create("memoria.log", "memoria", 1, 0);
	config =
			config_create(
					"/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/memoria.config");

	pthread_t h_consola;
	pthread_t h_conexiones;

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);


	int tamanioMemoria = 2048; //todo config

	void* comienzoMemoria = malloc(tamanioMemoria);

	t_list* tablaSegmentos = crearTablaSegmentos();

	nuevaTabla(tablaSegmentos,"TABLA1");

	segmento* miSegmento = tablaSegmentos->head->data;
	log_info(logger,"Nombre de mi nueva tabla es: %s",miSegmento->nombreDeTabla);


	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	free(comienzoMemoria);
	return 1;
}
