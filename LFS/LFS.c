#include "funcionesLFS.h"

t_list* memtable;

t_config*config;
t_config*metadataLFS;
sem_t sem_memtable;

//variables del config CONFIG/LFS.config
t_log *logger;
t_log *dump_logger;
t_log *compact_logger;
char *puntoDeMontaje;
int retardo; //en milisegundos
int tamanioValue;
int tiempoDump; //en milisegundos
FILE *fp_dump;
int control = 0;
int flag_key_value = 0;
char array_aux[128] = "";

//variables del config Metadata/metadata.bin
int cantidadBloques;
int tamanioBloques;

int main() {

	logger = log_create("LFS.log", "LFS", 1, 0);
	dump_logger = log_create("Dump.log", "LFS", 0, 0);
	compact_logger = log_create("Compactacion.log", "LFS", 0, 0);

	sem_init(&sem_memtable, 0, 1);

	iniciar_variables();

	pthread_t h_consola;
	pthread_t h_conexiones;
	pthread_t h_dump;
	pthread_t h_peticiones;

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_dump, NULL, (void *) ejecutar_dump, NULL);
	pthread_create(&h_conexiones, NULL, (void *) aceptar_conexiones, NULL);
	pthread_create(&h_peticiones, NULL, (void *) ejecutar_peticion, NULL);
	compactacion_tablas_existentes();

	pthread_detach(h_conexiones);
	pthread_detach(h_peticiones);
	pthread_detach(h_dump);
	pthread_detach(h_inotify);
	pthread_join(h_consola, NULL);

	return 1;
}
