#include "funcionesMemoria.h"

t_log* logger;
t_config* config;

t_list* tablaSegmentos;
t_list* hilosEnEjecucion;
t_list* colaDeRequests;
sem_t requestsDisponibles;
sem_t sem_gossiping;
sem_t sem_cargarSeeds;
sem_t sem_journal;
int nombreMemoria;

int main() {


	logger = log_create("memoria.log", "memoria", 1, 0); //3er parametro en 1 para mostrarlos en consola. Sino en 0


	config= config_create(DIRCONFIG); //DUDA: Si es un config por memoria esto va en la carpeta CONFIG tmb  lo hago una por proyecto como aca

	hilosEnEjecucion = list_create();
	colaDeRequests = list_create();
	sem_init(&requestsDisponibles,0,0);
	sem_init(&sem_gossiping,0,1);
	sem_init(&sem_cargarSeeds,0,1);
	sem_init(&sem_journal,0,1);

	int tamanioMemoria = config_get_int_value(config, "TAM_MEM");

	nombreMemoria = config_get_int_value(config, "MEMORY_NUMBER");

	config_destroy(config);

	int caracMaxDeValue = primeraConexionLFS();

//	if (caracMaxDeValue == -1)
//	{
//		return -1;
//	}



	//reservo toda la memoria

	log_info(logger, "Cree mi memoria tamanio: %d.", tamanioMemoria);

	comienzoMemoria = malloc(tamanioMemoria);
	//printf("comienzo memoria:%p\n", comienzoMemoria);
	//divido en marcos


	tamanioMarco = tamanioMarco * sizeof(char) + sizeof(uint16_t) + sizeof(unsigned long long); //value + key + timestamp TODO: Cambiar timestamp si el tipo se distinto
	log_info(logger,"el tamanio de mi marco es: %d", tamanioMarco);
	cantMarcos = tamanioMemoria/tamanioMarco; //tamanio marco siempre es mult de 2 asi que da entero
	log_info(logger,"cantidad de marcos: %d",cantMarcos);
	marcos=malloc(sizeof(disponibilidad)*cantMarcos);
	for(int i = 0; i<cantMarcos;i++){
		marcos[i].vacio=true;
	}
//	creo tabla de segmentos

	tablaSegmentos = crearTablaSegmentos();


	pthread_t h_consola;
	pthread_t h_ejecucionRequests;
	pthread_t h_conexiones;
	pthread_t h_refreshGossiping;

	pthread_create(&h_ejecucionRequests, NULL, (void *) ejecutarRequests, NULL);
	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) aceptarConexiones, NULL);
	pthread_create(&h_refreshGossiping, NULL, (void *) gossiping, NULL);

	pthread_detach(h_ejecucionRequests);
	pthread_detach(h_conexiones);
	pthread_detach(h_refreshGossiping);
	pthread_join(h_consola, NULL);

	log_destroy(logger);
	free(comienzoMemoria);
	return 1;
}
