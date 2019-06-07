#include "funcionesLFS.h"

t_config*config;
t_config*metadataLFS;
t_config*bitMapMetadata;
t_log* logger;
t_list* memTable;
int cantidadBloques;
char* puntoDeMontaje;

int main() {

	logger = log_create("LFS.log", "LFS", 1, 0);

	config = config_create("/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/LFS.config");

	memTable = list_create();

	metadataLFS = config_create("Metadata/Metadata.bin");

	cantidadBloques = config_get_int_value(metadataLFS,"BLOCKS");

	puntoDeMontaje = config_get_string_value(config,"PUNTO_MONTAJE");

	bitMapMetadata = config_create("Metadata/BitMap.bin");

//	t_bitarray* bitMap = generarBitMap();
//	bitarray_set_bit(bitMap,0);
//	guardarBitMapEnConfig(bitMap);
//	bitarray_destroy(bitMap);
//	t_bitarray* bitMap2 = generarBitMap();
//	testearBitMap(bitMap2);


//	insert("Tabla1 12 VAYNE 43");
//	t_tablaEnMemTable* tabla = ultimaTabla(memTable);
//	dato* holis =ultimoDato(tabla->datosAInsertar); //funciona pero me tira un warning que no entiendo
//	printf("holis es ts: \n   Key: \n y value: \n %i %i %s",holis->timestamp,holis->key,holis->value);


	pthread_t h_consola;
	pthread_t h_conexiones;


	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);

	//create("TABLA-cz SC 4 60000");

	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	return 1;
}
