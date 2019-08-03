#include "funcionesMemoria.h"

t_log* logger;
t_log* loggerJournal;
t_list* tablaSegmentos;
t_list* hilosEnEjecucion;
t_list* colaDeRequests;
t_list* tablaGossiping;
sem_t requestsDisponibles;
sem_t sem_gossiping;
sem_t agregadoRequests;
//sem_t sem_cargarSeeds;
sem_t sem_journal;
sem_t sem_refreshConfig;
sem_t sem_LFSconectandose;
sem_t sem_recepcionLFS;
sem_t conexionMemoria;
int nombreMemoria;
int socketKernel;
int sleepJournal;
int sleepGossiping;
int retardoAccesoLFS;
int retardoAccesoMemoria;
int idScriptKernel;
int caracMaxDeValue;
int puertoServidor;
char* tablaSelect;
char* dirConfig;

int main() {

	printf("Insertar nombre config:\n");
	t_config* config;
	while(1){
		char* nombreArchivoConfig = string_new();
		string_append(&nombreArchivoConfig, readline("Nombre="));
		dirConfig = string_duplicate(RAIZCONFIG);
		string_append(&dirConfig, nombreArchivoConfig);
		free(nombreArchivoConfig);
		config = config_create(dirConfig);
		if(config != NULL)
			break;
		else
			printf("Nombre config invalido, vuelva a ingresar:\n");
	}

	hilosEnEjecucion = list_create();
	colaDeRequests = list_create();
	sem_init(&requestsDisponibles, 0, 0);
	sem_init(&sem_gossiping, 0, 1);
	sem_init(&agregadoRequests,0,1);
//	sem_init(&sem_cargarSeeds,0,1);
	sem_init(&sem_journal, 0, 1);
	sem_init(&sem_refreshConfig, 0, 1);
	sem_init(&sem_LFSconectandose, 0, 1);
	sem_init(&sem_recepcionLFS, 0, 0);
	sem_init(&conexionMemoria, 0, 1);

	int tamanioMemoria = config_get_int_value(config, "TAM_MEM");

	nombreMemoria = config_get_int_value(config, "MEMORY_NUMBER");
	sleepJournal = config_get_int_value(config, "RETARDO_JOURNAL");
	sleepGossiping = config_get_int_value(config, "RETARDO_GOSSIPING");
	retardoAccesoLFS = config_get_int_value(config, "RETARDO_FS");
	retardoAccesoMemoria = config_get_int_value(config, "RETARDO_MEM");
	puertoServidor = config_get_int_value(config, "PUERTO_ESCUCHA");
	idScriptKernel = -1;

	char* nombreLogger = string_new(); //Por si acaso no hacerle free a este string
	string_append(&nombreLogger,"memoria");
	string_append(&nombreLogger,string_itoa(nombreMemoria));
	string_append(&nombreLogger,".log");

	logger = log_create(nombreLogger, "memoria", 1, 0); //3er parametro en 1 para mostrarlos en consola. Sino en 0
	loggerJournal = log_create("journal.log","memoria",0,0);

	config_destroy(config);

	caracMaxDeValue = primeraConexionLFS();

	log_info(logger, "Caracteres value:%d", caracMaxDeValue);
	if (caracMaxDeValue != -1) {
		loggearAzulClaro(logger, "Se conecto el LFS.");
		pthread_t h_respuestaLFS;
		pthread_create(&h_respuestaLFS, NULL, (void *) manejarRespuestaLFS,
		NULL);
		pthread_detach(h_respuestaLFS);

	}

	socketKernel = -1; //Para luego comprobar si se conecto o no

//	if (caracMaxDeValue == -1)
//	{
//		return -1;
//	}

	//reservo toda la memoria
	comienzoMemoria = malloc(tamanioMemoria);
	log_info(logger, "Cree mi memoria tamanio: %d.", tamanioMemoria);
	//printf("comienzo memoria:%p\n", comienzoMemoria);
	//divido en marcos

	tamanioMarco = caracMaxDeValue * sizeof(char) + sizeof(uint16_t)
			+ sizeof(unsigned long long); //value + key + timestamp
	log_info(logger, "el tamanio de mi marco es: %d", tamanioMarco);
	cantMarcos = tamanioMemoria / tamanioMarco;
	log_info(logger, "cantidad de marcos: %d", cantMarcos);
	marcos = malloc(sizeof(disponibilidad) * cantMarcos);
	for (int i = 0; i < cantMarcos; i++) {
		marcos[i].vacio = true;
	}
//	creo tabla de segmentos

	tablaSegmentos = crearTablaSegmentos();

	cargarSeedsIniciales();

	tablaGossiping = list_create();

	pthread_t h_consola;
	pthread_t h_ejecucionRequests;
	pthread_t h_conexiones;
	pthread_t h_refreshGossiping;
	pthread_t h_refreshConfig;
	pthread_t h_journalAutomatico;

	pthread_create(&h_conexiones, NULL, (void *) aceptarConexiones, NULL);
	pthread_create(&h_ejecucionRequests, NULL, (void *) ejecutarRequests, NULL);
	pthread_create(&h_refreshConfig, NULL, (void *) refreshConfig, NULL);
	pthread_create(&h_journalAutomatico, NULL, (void *) journalAutomatico,
	NULL);
	pthread_create(&h_refreshGossiping, NULL, (void *) hacerGossipingAutomatico,
			NULL);
	pthread_create(&h_consola, NULL, (void *) consola, NULL);

	pthread_detach(h_conexiones);
	pthread_detach(h_ejecucionRequests);
	pthread_detach(h_refreshConfig);
	pthread_detach(h_journalAutomatico);
	pthread_detach(h_refreshGossiping);
	pthread_join(h_consola, NULL);

	//Se cerro la consola

	if (socketKernel != -1) {
		enviarInt(socketKernel, -1); //Si el kernel alguna vez recibe -1, mata la memoria.
		close(socketKernel);
	}

	sem_wait(&sem_gossiping);

	for (int i = 0; i < list_size(tablaGossiping); i++) {
		memoriaGossip* unaMemoria = list_get(tablaGossiping, i);

		if (unaMemoria->nombre != nombreMemoria) {
			enviarInt(unaMemoria->elSocket, -1);
		}
	}

	log_destroy(logger);
	free(comienzoMemoria);
	return 1;
}
