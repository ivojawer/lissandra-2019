#include "funcionesKernel.h"

t_config*config;
t_log* logger;
t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
t_list* listaTablas;
t_list* tiemposInsert;
t_list* tiemposSelect;
t_list* memorias;
int idInicial;
int quantum;
int multiprocesamiento;
int proximaMemoriaEC;
sem_t sem_multiprocesamiento;
sem_t sem_cambioId;
sem_t sem_disponibleColaREADY;
sem_t sem_operacionesTotales;
sem_t sem_tiemposInsert;
sem_t sem_tiemposSelect;
sem_t sem_gossiping;
sem_t sem_actualizacionMetadatas;

int main() {

	logger = log_create("kernel.log", "kernel", 1, 0);
	config = config_create(
			"/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/kernel.config");

	quantum = config_get_int_value(config, "QUANTUM");
	multiprocesamiento = config_get_int_value(config, "MULTIPROCESAMIENTO");

	colaNEW = list_create();
	colaREADY = list_create();
	listaEXEC = list_create();
	listaEXIT = list_create();
	listaTablas = list_create();
	tiemposInsert = list_create();
	tiemposSelect = list_create();
	memorias = list_create();

	idInicial = 1000;
	proximaMemoriaEC = -1;

	pthread_t h_consola;
	pthread_t h_planificador;
	pthread_t h_primeraConexion;

	sem_init(&sem_multiprocesamiento, 0, multiprocesamiento);
	sem_init(&sem_cambioId, 0, 1);
	sem_init(&sem_disponibleColaREADY, 0, 0);
	sem_init(&sem_operacionesTotales, 0, 1);
	sem_init(&sem_tiemposInsert, 0, 1);
	sem_init(&sem_tiemposSelect, 0, 1);
	sem_init(&sem_gossiping, 0, 1);
	sem_init(&sem_actualizacionMetadatas, 0, 1);

	char* ipMemoriaPrincipal = config_get_string_value(config, "IP_MEMORIA");
	int puertoMemoriaPrincipal = config_get_int_value(config, "PUERTO_MEMORIA");

	seed* seedPrincipal = malloc(sizeof(seed));

	seedPrincipal->ip = ipMemoriaPrincipal;
	seedPrincipal->puerto = puertoMemoriaPrincipal;

	pthread_create(&h_primeraConexion, NULL, (void *) conectarseAUnaMemoria,
			seedPrincipal);
	pthread_detach(h_primeraConexion);

	pthread_create(&h_planificador, NULL, (void *) planificadorREADYAEXEC,
			NULL);
	pthread_detach(h_planificador);

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_join(h_consola, NULL);

	//Matar lo que se pueda

	config_destroy(config);
	log_destroy(logger);

	sem_destroy(&sem_multiprocesamiento);
	sem_destroy(&sem_cambioId);
	sem_destroy(&sem_disponibleColaREADY);
	sem_destroy(&sem_operacionesTotales);
	sem_destroy(&sem_tiemposInsert);
	sem_destroy(&sem_tiemposSelect);

	matarListas();

	return 0; //Gracias tux tkm
}
