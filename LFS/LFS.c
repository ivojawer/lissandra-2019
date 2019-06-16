#include "funcionesLFS.h"

t_log* logger;
t_list* memtable;
t_bitarray* bitarray;

t_config*config;
t_config*metadataLFS;

//variables del config Metadata/metadata.bin
int cantidadBloques;
int tamanioBloques;

//variables del config CONFIG/LFS.config
char* puntoDeMontaje;
int retardo; //en milisegundos
int tamanioValue;
int tiempoDump; //en milisegundos
FILE *fp_dump;
int control = 0;
int flag_key_value = 0;
char array_aux[128] = "";


int main() {


	logger = log_create("LFS.log", "LFS", 1, 0);
	//logger = log_create("LFS.log", "LFS", false, LOG_LEVEL_INFO);

	iniciar_variables(); //// creo que estan bien las que deje - chequear

	pthread_t h_consola;
	pthread_t h_conexiones;


	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);


	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	return 1;
}
