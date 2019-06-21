#include "funcionesMemoria.h"

t_log* logger;
t_list* tablaSegmentos;
t_list* seeds;
int caracMaxDeValue;
t_config* config;
int main() {

	logger = log_create("memoria.log", "memoria", 1, 0); //3er parametro en 1 para mostrarlos en consola. Sino en 0

	config = config_create(DIRCONFIG); //DUDA: Si es un config por memoria esto va en la carpeta CONFIG tmb  lo hago una por proyecto como aca

	int tamanioMemoria = config_get_int_value(config, "TAM_MEM");
	caracMaxDeValue = 20;//viene del handshake

	config_destroy(config);

	//reservo toda la memoria

	log_info(logger, "Cree mi memoria tamanio: %d.", tamanioMemoria);
	comienzoMemoria = malloc(tamanioMemoria);
	//printf("comienzo memoria:%p\n", comienzoMemoria);
	//divido en marcos

	tamanioMarco = caracMaxDeValue * sizeof(char) + sizeof(int) + sizeof(unsigned long); //value + key + timestamp
	log_info(logger,"el tamanio de mi marco es: %d", tamanioMarco);
	cantMarcos = tamanioMemoria/tamanioMarco; //tamanio marco siempre es mult de 2 asi que da redondo
	log_info(logger,"cantidad de marcos: %d",cantMarcos);
	marcos=malloc(sizeof(disponibilidad)*cantMarcos);
	for(int i = 0; i<cantMarcos;i++){
		marcos[i].vacio=true;
	}
//	creo tabla de segmentos
	tablaSegmentos = crearTablaSegmentos();

//	cargarSeedsIniciales();
	ponerComillas("hola");
	conexionLFS();

	pthread_t h_consola;
	pthread_t h_conexionKernel;
	pthread_t h_conexionLFS;
	pthread_t h_refreshGossiping;

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexionKernel, NULL, (void *) primeraConexionKernel, NULL);
	pthread_create(&h_conexionLFS, NULL, (void *) conexionLFS, NULL);
	pthread_create(&h_refreshGossiping, NULL, (void *) gossiping, NULL);

	pthread_detach(h_conexionKernel);
	pthread_detach(h_conexionLFS);
	pthread_detach(h_refreshGossiping);

	pthread_join(h_consola, NULL);

	log_destroy(logger);
	free(comienzoMemoria);
	return 1;
}
