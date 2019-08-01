#include "funcionesLFS.h"

extern t_log* logger;
extern t_log* dump_logger;

extern t_list* memtable;

extern int cantidadBloques;
extern int tamanioBloques;


extern char* puntoDeMontaje;
extern int retardo; //en milisegundos
extern int tamanioValue;
extern int tiempoDump; //en milisegundos
extern int full_space;
extern void crear_hilo_compactacion(char *tabla, int tiempo_compactacion);

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


void ejecutar_peticion()
{
//	while(1){
//		sem_wait(&requests_disponibles);
////		printf("Tengo %d peticiones para ejecutar\n", cola_requests->elements_count);
//		requestConSocket* request_a_ejecutar = list_get(cola_requests, 0); //Si por x motivo se acumulan varias, esto deberia cambiar
//		log_info(logger,"Se va a ejecutar %s",requestStructAString(request_a_ejecutar));
//		mandarAEjecutarRequest(request_a_ejecutar, socket_cliente);
//		list_remove(cola_requests,0); //Esto no se que tan bien esta pero en algun lado tengo que sacar la request
//	}
}



void mandarAEjecutarRequest(request* requestAEjecutar, int socket)
{
	struct parametros *parametros = malloc(sizeof(struct parametros));
	parametros->socket_cliente = socket;
	parametros->comando = string_duplicate(requestAEjecutar->parametros); //Esto es para que se pueda hacer un free() en consola.c sin que rompa
//	char *parametros = string_duplicate(requestAEjecutar->parametros); //Esto es para que se pueda hacer un free() en consola.c sin que rompa
	switch (requestAEjecutar->requestEnInt) {
	case SELECT:
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) rutina_select, parametros);

			pthread_detach(h_select);

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
	case CONFIG_RETARDO:
		{
			pthread_t h_configRetardo;

			pthread_create(&h_configRetardo,NULL, (void*) cambiarRetardo, parametros);

			pthread_detach(h_configRetardo);

			break;
		}

	case CONFIG_DUMP:
		{
			pthread_t h_configDump;

			pthread_create(&h_configDump,NULL, (void*) cambiarTiempoDump, parametros);

			pthread_detach(h_configDump);

			break;
		}

	}

	liberarRequest(requestAEjecutar);
}

void cambiarRetardo(void* parametros){

	struct parametros* info = (struct parametros*) parametros;
	char* retardo = info->comando;

	t_config* config = config_create("../../CONFIG/LFS.config");
	config_set_value(config,"RETARDO",retardo);
	config_save(config);
	config_destroy(config);
}

void cambiarTiempoDump(void* parametros){

	struct parametros* info = (struct parametros*) parametros;
	char* tiempoDump = info->comando;

	t_config* config = config_create("../../CONFIG/LFS.config");
	config_set_value(config,"TIEMPO_DUMP",tiempoDump);
	config_save(config);
	config_destroy(config);
}


void iniciar_variables(){

	tot = 0;
	cola_requests = list_create();
	lista_metadatas = list_create();
	struct inotify *st_inotify = malloc(sizeof(struct inotify));

	sem_init(&dump_semaphore, 1, 1);
	sem_init(&op_control_semaphore, 1, 1);
	sem_init(&compactar_semaphore, 1, 1);

	//asigno variables globales del LFS.config
	t_config* config = config_create("/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/LFS.config");
	puntoDeMontaje = string_duplicate(config_get_string_value(config,"PUNTO_MONTAJE"));
	retardo = config_get_int_value(config,"RETARDO");
	tamanioValue = config_get_int_value(config,"TAMAÑO_VALUE");
	tiempoDump = config_get_int_value(config,"TIEMPO_DUMP");
	sem_init(&refresh_config, 0, 1);
	st_inotify->config_root = strdup("/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/LFS.config");

	pthread_create(&h_inotify, NULL, (void *)control_inotify, st_inotify);

	controlExistenciaLFS();

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
	full_space = 0;
	memset(array_aux, 0X0, sizeof(array_aux));

//	op_control_list = list_create();
//	cargar_op_control_tablas();
	lista_tabla_compact = list_create();
	cargar_metadata_tablas();

	sem_init(&requests_disponibles,0,0);
	sem_init(&bloques_bitmap,1,1);

	//agrego bitarray de cargar_configuracion_FS()
	crear_bitarray(cantidadBloques);

	config_destroy(config);
	config_destroy(metadataLFS);
	free(dirMetadata);
}

void controlExistenciaLFS(){

	char* directorioBloques = string_duplicate(puntoDeMontaje);
	string_append(&directorioBloques,"/Bloques");
	DIR* directorio = opendir(directorioBloques);
	if(directorio == NULL){
		mkdir(directorioBloques,0700);
		log_info(logger,"[Control existencia]: Creacion del directorio Bloques");
	}else{
		closedir(directorio);
		log_info(logger,"[Control existencia]: Se encuentra el directorio Bloques");
	}
	free(directorioBloques);



	char* directorioTablas = string_duplicate(puntoDeMontaje);
	string_append(&directorioTablas,"/Tablas");
	directorio = opendir(directorioTablas);
	if(directorio == NULL){
		mkdir(directorioTablas,0700);
		log_info(logger,"[Control existencia]: Creacion del directorio Tablas");
	}else{
		closedir(directorio);
		log_info(logger,"[Control existencia]: Se encuentra el directorio Tablas");
	}
	free(directorioTablas);



	char* directorioMetadata = string_duplicate(puntoDeMontaje);
	string_append(&directorioMetadata,"/Metadata");
	directorio = opendir(directorioMetadata);
	if(directorio == NULL){
		mkdir(directorioMetadata,0700);
		log_info(logger,"[Control existencia]: Creacion del directorio Metadata");


		char* pathBitmap = string_duplicate(directorioMetadata);
		string_append(&pathBitmap,"/Bitmap.bin");
		FILE* archivoBitmap = fopen(pathBitmap,"w");
		log_info(logger,"[Control existencia]: Creacion del archivo Bitmap.bin");
		fclose(archivoBitmap);
		free(pathBitmap);


		char* pathMetadata = string_duplicate(directorioMetadata);
		string_append(&pathMetadata,"/Metadata.bin");
		FILE* archivoMetadata = fopen(pathMetadata,"w");
		log_info(logger,"[Control existencia]: Creacion del archivo Metadata.bin");

		printf("Insertar cantidad de bloques:\n");
		char* cantidadBloques = string_new();
		string_append(&cantidadBloques,"BLOCKS=");
		string_append(&cantidadBloques,readline("BLOCKS="));
		string_append(&cantidadBloques,"\n");
		fputs(cantidadBloques,archivoMetadata);
		free(cantidadBloques);

		printf("Insertar tamanio de bloques:\n");
		char* tamanioBloques = string_new();
		string_append(&tamanioBloques,"BLOCK_SIZE=");
		string_append(&tamanioBloques,readline("BLOCK_SIZE="));
		string_append(&tamanioBloques,"\n");
		fputs(tamanioBloques,archivoMetadata);
		free(tamanioBloques);

		printf("Insertar magic number:\n");
		char* magicNumber = string_new();
		string_append(&magicNumber,"MAGIC_NUMBER=");
		string_append(&magicNumber,readline("MAGIC_NUMBER="));
		string_append(&magicNumber,"\n");
		fputs(magicNumber,archivoMetadata);
		free(magicNumber);

		fclose(archivoMetadata);
		free(pathMetadata);

	}else{
		closedir(directorio);
		log_info(logger,"[Control existencia]: Se encuentra el directorio Metadata");
	}
	free(directorioMetadata);

}



char* get_tabla(char* comando)
{
	char **tokens_comando = string_split(comando, " ");
	char *tabla = tokens_comando[0];
	return tabla;
}

int get_key(char* comando)
{
	char **tokens_comando = string_split(comando, " ");
	char *key = tokens_comando[1];
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
	char *consistencia = tokens_comando[1];
	return consistencia;
}


int get_particiones(char *comando)
{
	char** tokens_comando = string_split(comando, " ");
	return atoi(tokens_comando[2]);
}


int get_tiempo_compactacion(char *comando)
{
	char** tokens_comando = string_split(comando, " ");
	return atoi(tokens_comando[3]);
}

int buscar_particiones_metadata_en_disco(char *tabla)
{
	char* archivoMetadata = string_new();
	char* unaTabla = string_duplicate(tabla);
	string_append(&archivoMetadata,puntoDeMontaje);
	string_append(&archivoMetadata,"Tablas/");
	string_append(&archivoMetadata,unaTabla);
	string_append(&archivoMetadata,"/metadata.config");

	t_config* config = NULL;
	config = config_create(archivoMetadata);
	if (config == NULL){
		printf("ERROR. No se pudo obtener metadata de %s\n",archivoMetadata);
		log_info(dump_logger, "ERROR de tabla %s", tabla);
		return -1;
	}
	int nr_particiones_metadata = config_get_int_value(config, "PARTITIONS");
	config_destroy(config);
	free(archivoMetadata);
	free(unaTabla);
	return nr_particiones_metadata;
}

struct describe *crear_metadata_tabla(char *tabla, char *consistencia, int particiones, int compactacion)
{
	struct describe *new_metadata = malloc(sizeof(struct describe));
	new_metadata->name = strdup(tabla);
	new_metadata->consistency = strdup(consistencia);
	new_metadata->partitions = particiones;
	new_metadata->compaction_time = compactacion;
	return new_metadata;
}


void agregar_metadata_tabla(char *tabla, char *consistencia, int particiones, int compactacion)
{
	struct describe *new_metadata = crear_metadata_tabla(tabla, consistencia, particiones, compactacion);
	list_add(lista_metadatas, new_metadata);
}


bool buscar_metadata_tabla(struct describe *elemento, char *tabla)
{
	return(strcmp(elemento->name, tabla) == 0);
}

int buscar_metadata_en_lista(char *tabla)
{
	bool buscar_elemento(void *elemento) {
		return buscar_metadata_tabla((struct describe *)elemento, tabla);
	}
	struct describe *metadata_buscada = NULL;
	metadata_buscada = list_find(lista_metadatas, buscar_elemento);
	if (metadata_buscada != NULL)
		return metadata_buscada->partitions;
	else
		return -1;
}


int obtener_particiones_metadata(char* tabla)
{
		int particion_tabla;

		particion_tabla = buscar_metadata_en_lista(tabla);

		if(particion_tabla > 0)
			return particion_tabla;
		else{
			particion_tabla = buscar_particiones_metadata_en_disco(tabla);
			return particion_tabla;
		}
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

	consistency = string_duplicate(config_get_string_value(config, "CONSISTENCY"));

//	consistency = malloc(strlen(config_get_string_value(config, "CONSISTENCY"))); -malloc sacado, estas 2 lineas comentadas fueron reemplazadas por el string_duplicate de arriba
//	strcpy(consistency, config_get_string_value(config, "CONSISTENCY"));

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


void destroy_op_control(struct op_control *self)
{
	free(self->tabla);
	free(self);
}

void crear_control_op(char *tabla)
{
	struct op_control *s_op_control = malloc(sizeof(struct op_control));
	s_op_control->tabla = strdup(tabla);
	s_op_control->otros_flag = 0;
	s_op_control->drop_flag = 0;
	s_op_control->otros_blocked = 0;
	s_op_control->drop_flag = 0;
	sem_init(&(s_op_control->drop_sem), 1, 0);
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&(s_op_control->mutex), &attr);
	pthread_mutexattr_t attr_2;
	pthread_mutexattr_init(&attr_2);
	pthread_mutexattr_settype(&attr_2, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&(s_op_control->tabla_sem), &attr);
	list_add(op_control_list, s_op_control);
}


void cargar_metadata_tablas()
{
	struct dirent *sd;
	char* tablas = string_new();
	string_append(&tablas, puntoDeMontaje);
	string_append(&tablas, "Tablas/");
	DIR* dir = opendir(tablas);
	while ((sd = readdir(dir)) != NULL) {
		if ((strcmp((sd->d_name), ".") != 0) &&
			(strcmp((sd->d_name), "..") != 0)){
			agregar_metadata_tabla(sd->d_name, obtener_consistencia_metadata(sd->d_name),
					obtener_particiones_metadata(sd->d_name), obtener_tiempo_compactacion_metadata(sd->d_name));
		}
	}
	closedir(dir);
	free(tablas);
}



void cargar_op_control_tablas()
{
	struct dirent *sd;
	char* tablas = string_new();
	string_append(&tablas, puntoDeMontaje);
	string_append(&tablas, "Tablas/");
	DIR* dir = opendir(tablas);
	while ((sd = readdir(dir)) != NULL) {
		if ((strcmp((sd->d_name), ".") != 0) &&
			(strcmp((sd->d_name), "..") != 0)){
			crear_control_op(sd->d_name);
		}
	}
	closedir(dir);
	free(tablas);
}





void compactacion_tablas_existentes()
{
		struct dirent *sd;
		char* tablas = string_new();
		string_append(&tablas, puntoDeMontaje);
		string_append(&tablas, "Tablas");
		DIR* dir = opendir(tablas);
		while ((sd = readdir(dir)) != NULL) {
			if ((strcmp((sd->d_name), ".") != 0) &&
				(strcmp((sd->d_name), "..") != 0)){
				crear_hilo_compactacion(sd->d_name,
						obtener_tiempo_compactacion_metadata(sd->d_name));
			}
		}
		closedir(dir);
		free(tablas);
}



bool comparar_nombre_op_control(void *elemento, char *tabla)
{
	return (!strcmp(((struct op_control *)elemento)->tabla, tabla));
}


void modificar_op_control(char *tabla, int mod_flag)
{

	bool coincide_nombre_op(void *elemento){
		return comparar_nombre_op_control(elemento, tabla);
	}

	sem_wait(&op_control_semaphore);

//	struct op_control *tabla_a_controlar = malloc(sizeof(struct op_control));
	struct op_control *tabla_a_controlar = NULL;
	tabla_a_controlar = list_remove_by_condition(op_control_list, coincide_nombre_op);

	if (tabla_a_controlar != NULL) {
		switch (mod_flag) {
		case 1:
//			pthread_mutex_lock(&(tabla_a_controlar->mutex));
			tabla_a_controlar->otros_flag++;
			if (tabla_a_controlar->drop_flag == 0 &&
				tabla_a_controlar->insert_flag == 0){
//				pthread_mutex_unlock(&(tabla_a_controlar->mutex));
				list_add(op_control_list, tabla_a_controlar);
				sem_post(&op_control_semaphore);
			}else{
				tabla_a_controlar->otros_blocked++;
//				pthread_mutex_unlock(&(tabla_a_controlar->mutex));
				list_add(op_control_list, tabla_a_controlar);
				sem_post(&op_control_semaphore);
				pthread_mutex_lock(&(tabla_a_controlar->tabla_sem));
			}
			break;
		case 2:
//			pthread_mutex_lock(&(tabla_a_controlar->mutex));
			tabla_a_controlar->otros_flag--;
			if(tabla_a_controlar->otros_blocked > 0 || tabla_a_controlar->insert_flag > 0){
				tabla_a_controlar->otros_blocked--;
				pthread_mutex_unlock(&(tabla_a_controlar->tabla_sem));
			}else if (tabla_a_controlar->otros_flag == 0 &&
				tabla_a_controlar->drop_flag > 0) { //Si no hay otra request ejecutando y hay un Drop esperando
				sem_post(&(tabla_a_controlar->drop_sem));
			}
//			pthread_mutex_unlock(&(tabla_a_controlar->mutex));
			list_add(op_control_list, tabla_a_controlar);
			sem_post(&op_control_semaphore);
			break;
		case 3: //viene un Drop
//			pthread_mutex_lock(&(tabla_a_controlar->mutex));
			tabla_a_controlar->drop_flag++;
			if (tabla_a_controlar->otros_flag > 0) {
//				pthread_mutex_unlock(&(tabla_a_controlar->mutex));
				sem_wait(&(tabla_a_controlar->drop_sem));
			} else {
//				pthread_mutex_unlock(&(tabla_a_controlar->mutex));
				pthread_mutex_lock(&(tabla_a_controlar->tabla_sem));
			}
			list_add(op_control_list, tabla_a_controlar);
			sem_post(&op_control_semaphore);
			break;
		case 4:
			tabla_a_controlar->drop_flag--;
			pthread_mutex_unlock(&(tabla_a_controlar->tabla_sem));//Tal vez conviene no levantarlo
			destroy_op_control(tabla_a_controlar);
			sem_post(&op_control_semaphore);
			break;
		case 5: //Compactacion (se comporta parecido a Drop)
//			pthread_mutex_lock(&(tabla_a_controlar->mutex));
			if (tabla_a_controlar->otros_flag > 0 ||
				tabla_a_controlar->drop_flag > 0){
				tabla_a_controlar->drop_flag++;
//			pthread_mutex_unlock(&(tabla_a_controlar->mutex));
			sem_wait(&(tabla_a_controlar->drop_sem));
			} else {
				tabla_a_controlar->drop_flag++;
//				pthread_mutex_unlock(&(tabla_a_controlar->mutex));
				pthread_mutex_lock(&(tabla_a_controlar->tabla_sem));
			}
			list_add(op_control_list, tabla_a_controlar);
			sem_post(&op_control_semaphore);
			break;
		case 6: //fin Compactacion
//			pthread_mutex_lock(&(tabla_a_controlar->mutex));
			tabla_a_controlar->drop_flag--;
			pthread_mutex_unlock(&(tabla_a_controlar->tabla_sem));
//			pthread_mutex_unlock(&(tabla_a_controlar->mutex));
			list_add(op_control_list, tabla_a_controlar);
			sem_post(&op_control_semaphore);
			break;
		case 7:
//			pthread_mutex_lock(&(tabla_a_controlar->mutex));
			tabla_a_controlar->insert_flag++;
//			pthread_mutex_unlock(&(tabla_a_controlar->mutex));
			pthread_mutex_lock(&(tabla_a_controlar->tabla_sem));
			list_add(op_control_list, tabla_a_controlar);
			sem_post(&op_control_semaphore);
			break;
		case 8:
//			pthread_mutex_lock(&(tabla_a_controlar->mutex));
			tabla_a_controlar->insert_flag--;
			pthread_mutex_unlock(&(tabla_a_controlar->tabla_sem));
			if (tabla_a_controlar->otros_flag == 0 &&
				tabla_a_controlar->drop_flag > 0) { //Si no hay otra request ejecutando y hay un Drop esperando
				sem_post(&(tabla_a_controlar->drop_sem));
			}
//			pthread_mutex_unlock(&(tabla_a_controlar->mutex));
			list_add(op_control_list, tabla_a_controlar);
			sem_post(&op_control_semaphore);
			break;
		default :
			printf("mod_flag No reconocido\n");
			break;
		}
	}else {
		sem_post(&op_control_semaphore);
		return;
	}
}

int contar_archivos_con_extension(char *root, char* extension) {
	int cont = 0;
	DIR * dir;
	dir = opendir(root);

	struct dirent *entrada;
	char **entrada_aux;
	while ((entrada = readdir(dir)) != NULL) {
		entrada_aux = string_split(entrada->d_name, ".");
		if (entrada_aux[1] != NULL) {
			if (strcmp(entrada_aux[1], extension) == 0) {
				cont++;
			}
		}
	}
	closedir(dir);
	return cont;
}

int nr_particion_key(uint16_t key, int nr_particiones_metadata)
{
	return key%nr_particiones_metadata;
}


int controlar_bloques_disponibles(int cantArchivos)
{
	int i;
	int cont = 0;
	for (i = 0; i < cantidadBloques; i++){
		if (bitarray_test_bit(bitarray, i) == 0)
			cont++;
	}
	if(cont >= cantArchivos)
		return 1;
	return 0;
}

void desmarcar_bloque_bitmap(t_bloque *elemento) {

	int block = atoi(elemento->name);
	bitarray_clean_bit(bitarray, block);
	guardar_bitarray(block);

	char* block_root = string_new();
	string_append(&block_root,puntoDeMontaje);
	string_append(&block_root,"Bloques/bloque");
	string_append(&block_root,string_itoa(block));
	string_append(&block_root,".bin");
	FILE *fp;
	fp = fopen(block_root, "w+");
	fclose(fp);

	free(block_root);

}

t_bloque *crear_bloque_buscar(char *bloque)
{
	   t_bloque *new = malloc(sizeof(t_bloque));
	   new->name=strdup(bloque);
	   return new;
}

void bloque_destroy(t_bloque *self) {
    free(self->name);
    free(self);
}

void control_inotify(void *param)
{
	struct inotify *p = (struct inotify *)param;
	while(1){
		esperarModificacionDeArchivo(strdup(p->config_root));

//		sem_wait(&refresh_config);

		t_config* config = config_create(p->config_root);

		retardo = config_get_int_value(config,"RETARDO");
		tamanioValue = config_get_int_value(config,"TAMAÑO_VALUE");
		tiempoDump = config_get_int_value(config,"TIEMPO_DUMP");

		config_destroy(config);

//		sem_post(&refresh_config);
	}
}
