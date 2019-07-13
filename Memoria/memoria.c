#include "funcionesMemoria.h"

t_log* logger;



int caracMaxDeValue;
t_list* tablaSegmentos;
t_list* hilosEnEjecucion;
t_list* colaDeRequests;
sem_t requestsDisponibles;
sem_t sem_gossiping;
//sem_t sem_cargarSeeds;
sem_t sem_journal;
sem_t sem_refreshConfig;
sem_t sem_LFSconectandose;
sem_t sem_recepcionLFS;
int nombreMemoria;
int socketKernel;
char* dirConfig;
int sleepJournal;
int sleepGossiping;
int retardoAccesoLFS;
int retardoAccesoMemoria;
int idScriptKernel;
char* tablaSelect;

int main() {



	printf("Insertar nombre config:\n");
	char* nombreArchivoConfig = string_new ();
	string_append(&nombreArchivoConfig, readline("Nombre="));
	dirConfig = string_duplicate(RAIZCONFIG);
	string_append(&dirConfig,nombreArchivoConfig);
	free(nombreArchivoConfig);

	//string_append(&nombreArchivoConfig,".config");

	logger = log_create("memoria.log", "memoria", 1, 0); //3er parametro en 1 para mostrarlos en consola. Sino en 0


	t_config* config = config_create(dirConfig);



	hilosEnEjecucion = list_create();
	colaDeRequests = list_create();
	sem_init(&requestsDisponibles,0,0);
	sem_init(&sem_gossiping,0,1);
//	sem_init(&sem_cargarSeeds,0,1);
	sem_init(&sem_journal,0,1);
	sem_init(&sem_refreshConfig,0,1);
	sem_init(&sem_LFSconectandose,0,1);
	sem_init(&sem_recepcionLFS,0,0);

	int tamanioMemoria = config_get_int_value(config, "TAM_MEM");

	nombreMemoria = config_get_int_value(config, "MEMORY_NUMBER");
	sleepJournal = config_get_int_value(config, "RETARDO_JOURNAL");
	sleepGossiping = config_get_int_value(config, "RETARDO_GOSSIPING");
	retardoAccesoLFS = config_get_int_value(config, "RETARDO_FS");
	retardoAccesoMemoria = config_get_int_value(config, "RETARDO_MEM");
	idScriptKernel = -1;

	config_destroy(config);

	caracMaxDeValue = primeraConexionLFS();

	if (caracMaxDeValue != -1)
	{
		log_info(logger,"Se conecto el LFS.");
		pthread_t h_respuestaLFS;
		pthread_create(&h_respuestaLFS, NULL, (void *) manejarRespuestaLFS, NULL);
		pthread_detach(h_respuestaLFS);

	}

	socketKernel = -1; //Para luego comprobar si se conecto o no

//	if (caracMaxDeValue == -1)
//	{
//		return -1;
//	}



	//reservo toda la memoria

	log_info(logger, "Cree mi memoria tamanio: %d.", tamanioMemoria);

	comienzoMemoria = malloc(tamanioMemoria);
	//printf("comienzo memoria:%p\n", comienzoMemoria);
	//divido en marcos


	tamanioMarco = tamanioMarco * sizeof(char) + sizeof(uint16_t) + sizeof(unsigned long long); //value + key + timestamp
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
	pthread_t h_refreshConfig;
	pthread_t h_journalAutomatico;

	pthread_create(&h_ejecucionRequests, NULL, (void *) ejecutarRequests, NULL);
	pthread_create(&h_conexiones, NULL, (void *) aceptarConexiones, NULL);
	pthread_create(&h_refreshGossiping, NULL, (void *) gossiping, NULL);
	pthread_create(&h_refreshConfig, NULL, (void *) refreshConfig, NULL);
	pthread_create(&h_journalAutomatico, NULL, (void *) journalAutomatico, NULL);
	pthread_create(&h_consola, NULL, (void *) consola, NULL);




	pthread_detach(h_ejecucionRequests);
	pthread_detach(h_conexiones);
	pthread_detach(h_refreshGossiping);
	pthread_detach(h_refreshConfig);
	pthread_detach(h_journalAutomatico);
	pthread_join(h_consola, NULL);

	//Se cerro la consola

	if(socketKernel != -1)
	{
		enviarInt(socketKernel,-1); //Si el kernel alguna vez recibe -1, mata la memoria.
		close(socketKernel);
	}

	log_destroy(logger);
	free(comienzoMemoria);
	return 1;
}
