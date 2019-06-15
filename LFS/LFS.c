#include "funcionesLFS.h"

t_config*config;
t_config*metadataLFS;
t_config*bitMapMetadata;
t_log* logger;
t_list* memTable;
int cantidadBloques;
char* puntoDeMontaje;
t_bitarray* bitMap;

int main() {

	logger = log_create("LFS.log", "LFS", 1, 0);

	config = config_create("/home/ivan/eclipse-workspace/tp-2019-1c-U-TN-Tecno/CONFIG/LFS.config");

	memTable = list_create();

	metadataLFS = config_create("Metadata/Metadata.bin");

	cantidadBloques = config_get_int_value(metadataLFS,"BLOCKS");

	puntoDeMontaje = config_get_string_value(config,"PUNTO_MONTAJE");

	bitMapMetadata = config_create("Metadata/BitMap.bin");

	//Estas son 3 formas de sacar el bitArray, la posta es con el config pero eSo nO AnDA~~
	//char* bitArray = config_get_string_value(bitMapMetadata,"BITMAP");
	//char bitArray[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	char* bitArray = string_repeat(0,cantidadBloques);

	bitMap = bitarray_create(bitArray,string_length(bitArray));

	pthread_t h_consola;
	pthread_t h_conexiones;


	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);


	// ---------Prueba Create-----------

//	create("TABLA2 SC 4 60000");

	// ---------Prueba Insert-----------

//	insert("Tabla2 12 VAYNE 43");
//	t_tablaEnMemTable* tabla = ultimaTabla(memTable);
//	registro* holis =ultimoDato(tabla->datosAInsertar); //funciona pero me tira un warning que no entiendo
//	printf("holis es ts:%i \n   Key:%i \n y value:%s \n   ",holis->timestamp,holis->key,holis->value);

	// ---------Prueba Select-----------

//	Select("TABLA-X 5");

	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	return 1;
}
