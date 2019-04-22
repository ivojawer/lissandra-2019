#include "requests.h"
#include "funcionesKernel.h"

t_config*config;
t_log* logger;
t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
int idInicial;
int quantum;

int main() {

	logger = log_create("kernel.log", "kernel", 0, 0);
	config = config_create(
			"/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/kernel.config");

	quantum = config_get_int_value(config,"QUANTUM");

	colaNEW = list_create();
	colaREADY = list_create();
	listaEXEC = list_create();
	listaEXIT = list_create();

	idInicial = 0;

	pthread_t h_consola;
	pthread_t h_conexiones;
	pthread_t h_planificador;

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);
	pthread_create(&h_planificador, NULL, (void *) planificador, NULL);

	pthread_detach(h_planificador);
	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	return 1;
}
