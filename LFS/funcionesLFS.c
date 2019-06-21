#include "funcionesLFS.h"

extern t_log* logger;

extern t_list* memtable;

extern int cantidadBloques;
extern int tamanioBloques;

extern char* puntoDeMontaje;
extern int retardo; //en milisegundos
extern int tamanioValue;
extern int tiempoDump; //en milisegundos

extern FILE *fp_dump;

extern char array_aux[128];

//////////////////////////////////////////////


int size_particion = 0;           //// de estas no estoy 100% segura si hay alguna obsoleta
t_list *particion_encontrada;
t_list *registros_encontrados;
t_list *tabla_encontrada;
t_list *lista_describe;
int wait_particiones = 1;
char *consistency;


void mandarAEjecutarRequest(request* requestAEjecutar) {

	char *parametros = string_duplicate(requestAEjecutar->parametros); //Esto es para que se pueda hacer un free() en consola.c sin que rompa
	switch (requestAEjecutar->requestEnInt) {
	case SELECT:
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) rutina_select, parametros);

			pthread_detach(h_select);

			//enviarString("Me llego un select, ocupate macho", socketLFSAMEM);

			break;
		}

	case INSERT:
		{
			pthread_t h_insert;

			pthread_create(&h_insert, NULL, (void *) rutina_insert, parametros);

			pthread_detach(h_insert);

			break;
		}

	case CREATE:
		{
			pthread_t h_create;

			pthread_create(&h_create, NULL, (void *) rutina_create, parametros);

			pthread_detach(h_create);

			break;
		}

	case DESCRIBE:
		{

			pthread_t h_describe;

			pthread_create(&h_describe, NULL, (void *) rutina_describe, parametros);

			pthread_detach(h_describe);

			break;
		}

	case DROP:
		{

			pthread_t h_drop;

			pthread_create(&h_drop, NULL, (void *) rutina_drop, parametros);

			pthread_detach(h_drop);

			break;

		}

	}

	liberarRequest(requestAEjecutar);
}


void iniciar_variables(){

	//asigno variables globales del LFS.config
	t_config* config = config_create("../../CONFIG/LFS.config");
	puntoDeMontaje = string_duplicate(config_get_string_value(config,"PUNTO_MONTAJE"));
	retardo = config_get_int_value(config,"RETARDO");
	tamanioValue = config_get_int_value(config,"TAMAÑO_VALUE");
	tiempoDump = config_get_int_value(config,"TIEMPO_DUMP");

	//asigno variables globales del Metadata.bin
	char* dirMetadata = string_duplicate(puntoDeMontaje);
	string_append(&dirMetadata,"Metadata/Metadata.bin");

	t_config* metadataLFS = config_create(dirMetadata);
	cantidadBloques = config_get_int_value(metadataLFS,"BLOCKS");
	tamanioBloques = config_get_int_value(metadataLFS,"BLOCK_SIZE");

	memtable = list_create();
	tabla_encontrada = list_create();
	registros_encontrados = list_create();
	particion_encontrada = list_create();
	fp_dump = NULL;
	memset(array_aux, 0X0, sizeof(array_aux));
	sem_init(&dump_semaphore, 0, 1);

	//agrego bitarray de cargar_configuracion_FS()
	crear_bitarray(cantidadBloques);

	config_destroy(config);
	config_destroy(metadataLFS);
	free(dirMetadata);
}


char* get_tabla(char* comando)
{
	char **tokens_comando = string_split(comando, " ");
	char *tabla = tokens_comando[1];
	return tabla;
}

int get_key(char* comando)
{
	char **tokens_comando = string_split(comando, " ");
	char *key = tokens_comando[2];
	return atoi(key);
}

char *get_value(char* comando)
{
	int primera_ocurrencia = str_first_index_of('"', comando);
	int segunda_ocurrencia = str_last_index_of('"', comando);
	return string_substring(comando, primera_ocurrencia + 1, segunda_ocurrencia - primera_ocurrencia - 1);
}

//retorna el timestamp escrito en la request o el del epoch unix si la request no trae uno
double get_timestamp(char* comando) {
	if (comando[string_length(comando) - 1] == '"'){ //si no incluyo un timestamp en la request
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
	}else{
		int indice_inicial = str_last_index_of('"', comando) + 1;
		char* str_timestamp = string_substring(comando, indice_inicial,  string_length(comando) - 1 - str_last_index_of('"', comando));
		double double_timestamp = atol(str_timestamp);
		return double_timestamp;
	}
}


char *get_consistencia(char *comando)
{
	char** tokens_comando = string_split(comando, " ");
	char *consistencia = tokens_comando[2];
	return consistencia;
}


int get_particiones(char *comando)
{
	char** tokens_comando = string_split(comando, " ");
	return atoi(tokens_comando[3]);
}


int get_tiempo_compactacion(char *comando)
{
	char** tokens_comando = string_split(comando, " ");
	return atoi(tokens_comando[4]);
}



int obtener_particiones_metadata(char* tabla)
{

	char* archivoMetadata = string_new();
	char* unaTabla = string_duplicate(tabla);
	string_append(&archivoMetadata,puntoDeMontaje);
	string_append(&archivoMetadata,"Tablas/");
	string_append(&archivoMetadata,unaTabla);
	string_append(&archivoMetadata,"/metadata.config");

	t_config* config = config_create(archivoMetadata);
	int nr_particiones_metadata = config_get_int_value(config, "PARTITIONS");
	config_destroy(config);
	free(archivoMetadata);
	free(unaTabla);
	return nr_particiones_metadata;
}


char *obtener_consistencia_metadata(char* tabla)
{
	char* archivoMetadata = string_new();
	char* unaTabla = string_duplicate(tabla);
	string_append(&archivoMetadata,puntoDeMontaje);
	string_append(&archivoMetadata,"Tablas/");
	string_append(&archivoMetadata,unaTabla);
	string_append(&archivoMetadata,"/metadata.config");

	t_config* config = config_create(archivoMetadata);
	consistency = malloc(strlen(config_get_string_value(config, "CONSISTENCY")));
	strcpy(consistency, config_get_string_value(config, "CONSISTENCY"));
	config_destroy(config);
	free(archivoMetadata);
	free(unaTabla);
	return consistency;
}


int obtener_tiempo_compactacion_metadata(char* tabla)
{
	char* archivoMetadata = string_new();
	char* unaTabla = string_duplicate(tabla);
	string_append(&archivoMetadata,puntoDeMontaje);
	string_append(&archivoMetadata,"Tablas/");
	string_append(&archivoMetadata,unaTabla);
	string_append(&archivoMetadata,"/metadata.config");

	t_config* config = config_create(archivoMetadata);
	int tiempo_compactacion = config_get_int_value(config, "COMPACTION_TIME");
	config_destroy(config);
	free(archivoMetadata);
	free(unaTabla);
	return tiempo_compactacion;
}



void crear_bloques_FS(int nr_blocks)
{
	for(int i = 0; i < nr_blocks; i++){

		char* root = string_duplicate(puntoDeMontaje);
		string_append(&root,"Bloques/bloque");
		string_append(&root,string_itoa(i));
		string_append(&root,".bin");
		FILE *fp;
		fp = fopen(root, "w+b");
		fclose(fp);
		free(root);
	}
}


/*
 * Supongo que Bitmap.bin EXISTE
 * Si esta vacio el Bitmap, le carga la cantidad de bloques que dice el arhivo Metada
 * y los setea a todos en 0. Despues crea a los Bloques (supongo que no existen)
 * Si el Bitmap tiene datos supongo que ya existe los Bloques y algunos estan lleno y otros no
 */

void setear_bitarray(int nr_blocks)
{
	int i = 0;
	char c;
	char *root = string_duplicate(puntoDeMontaje);
	string_append(&root,"/Metadata/Bitmap.bin");

	FILE *fp;
	fp = fopen(root, "r+b");
	if(fp == NULL)
	{
		log_error(logger,"Hubo un error abriendo el bitmap.");
		return;
	}

	fseek(fp, 0L, SEEK_END);
	int file_size = ftell(fp);

	if (file_size == 0){	//Bitmap vacio
		fseek(fp, 0L, SEEK_SET);
		for(i = 0; i < nr_blocks; i++){
			bitarray_clean_bit(bitarray, i);
			fprintf(fp, "%d",  bitarray_test_bit(bitarray, i));
		}
		crear_bloques_FS(nr_blocks);
	}else{
		printf("Bitmap ya seteado\n");
		fseek(fp, 0L, SEEK_SET);
		while(!feof(fp)){
			c = fgetc(fp);
			if(!feof(fp)){
				if(atoi(&c) == 1){
					bitarray_set_bit(bitarray, i);
				}else{
					bitarray_clean_bit(bitarray, i);
				}
				i++;
			}
		}
	}
	fclose(fp);
	free(root);
}

void crear_bitarray(int nr_blocks){
	char *data;
	data = malloc(nr_blocks);
	memset(data, '0', nr_blocks);

	bitarray = bitarray_create_with_mode(data, nr_blocks, LSB_FIRST);
	setear_bitarray(nr_blocks);
}
