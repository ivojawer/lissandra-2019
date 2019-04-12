#include "funcionesLFS.h"

t_config*config;
t_log* logger;
t_list* listaDeNombreDeTablas;
t_list* memTable;

int main() {

	logger = log_create("LFS.log", "LFS", 1, 0);
	config = config_create("/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/LFS.config");
	listaDeNombreDeTablas = list_create();
	memTable = list_create();

	pthread_t h_consola;
	pthread_t h_conexiones;


	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);

	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	return 1;
}
