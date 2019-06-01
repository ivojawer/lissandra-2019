#include "funcionesLFS.h"

t_config*config;
t_log* logger;
t_list* memTable;

int main() {

	logger = log_create("LFS.log", "LFS", 1, 0);
	config = config_create("/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/LFS.config");
	memTable = list_create();

//	insert("Tabla1 12 VAYNE 43");
//	t_tablaEnMemTable* tabla = ultimaTabla(memTable);
//	dato* holis =ultimoDato(tabla->datosAInsertar); //funciona pero me tira un warning que no entiendo
//	printf("holis es ts: \n   Key: \n y value: \n %i %i %s",holis->timestamp,holis->key,holis->value);


	pthread_t h_consola;
	pthread_t h_conexiones;




	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);

	//create("TABLA-A SC 4 60000");

	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	return 1;
}
