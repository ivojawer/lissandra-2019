#include "funcionesKernel.h"

t_config*config;
t_log* logger;
t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
t_list* listaTablas;
int idInicial;
int quantum;
int multiprocesamiento;
sem_t sem_multiprocesamiento;
sem_t sem_cambioId;
sem_t sem_disponibleColaREADY;


int main() {

	logger = log_create("kernel.log", "kernel", 1, 0);
	config = config_create(
			"/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/kernel.config");

	quantum = config_get_int_value(config,"QUANTUM");
	multiprocesamiento = config_get_int_value(config,"MULTIPROCESAMIENTO");

	colaNEW = list_create();
	colaREADY = list_create();
	listaEXEC = list_create();
	listaEXIT = list_create();
	listaTablas = list_create();

	idInicial = 1000;

	pthread_t h_consola;
	pthread_t h_conexiones;
	pthread_t h_planificador;

	sem_init(&sem_multiprocesamiento, 0, multiprocesamiento);
	sem_init(&sem_cambioId, 0, 1);
	sem_init(&sem_disponibleColaREADY, 0, 0);

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);
	pthread_create(&h_planificador, NULL, (void *) planificadorREADYAEXEC, NULL);

	pthread_detach(h_planificador);
	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	matarTodo();

	config_destroy(config);
	log_destroy(logger);
	sem_destroy(&sem_multiprocesamiento);
	sem_destroy(&sem_cambioId);
	sem_destroy(&sem_disponibleColaREADY);

	return 0; //Gracias tux tkm
}
