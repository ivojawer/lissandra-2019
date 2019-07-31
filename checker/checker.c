#include "funcionesChecker.h"

t_list* memtable;

t_config*config;
t_config*metadataLFS;

//variables del config CONFIG/LFS.config
t_log *logger;
t_log *resultadoCheck;
t_log *dump_logger;
t_log* erroresCheck;
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
	resultadoCheck = log_create("RESULTADO.log","checker",1,0);
	erroresCheck = log_create("soloErrores.log","checker",0,0);
	dump_logger = log_create("Dump.log", "LFS", 0, 0);
	compact_logger = log_create("Compactacion.log", "LFS", 0, 0);

	iniciar_variables();

	consola();

	return 1;
}
