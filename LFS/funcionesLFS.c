#include "funcionesLFS.h"

extern t_list* listaDeNombreDeTablas;  //// esta no se quien la puso, tecnicamente es del lfs viejo pero no se para que?
extern int socketLFSAMEM; // ?

extern t_log* logger;
extern t_list* memtable;
extern t_bitarray* bitarray;

extern int cantidadBloques;
extern int tamanioBloques;

extern char* puntoDeMontaje;
extern int retardo; //en milisegundos
extern int tamanioValue;
extern int tiempoDump; //en milisegundos


//////////////////////////////////////////////


int size_particion=0;           //// de estas no estoy 100% segura si hay alguna obsoleta
t_list *particion_encontrada;
t_list *registros_encontrados;
t_list *tabla_encontrada;
t_list *lista_describe;
int wait_particiones = 1;
char *consistency;


void mandarAEjecutarRequest(request* requestAEjecutar) {


	char* parametros = string_duplicate(requestAEjecutar->parametros); //Esto es para que se pueda hacer un free() en consola.c sin que rompa
	switch (requestAEjecutar->requestEnInt) {
	case SELECT:
		;
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) rutina_select, parametros);

			pthread_detach(h_select);

			//enviarString("Me llego un select, ocupate macho", socketLFSAMEM);

			break;
		}

	case INSERT:
		;
		{
			pthread_t h_insert;

			pthread_create(&h_insert, NULL, (void *) rutina_insert, parametros);

			pthread_detach(h_insert);

			break;
		}

	case CREATE:
		;

		{
			pthread_t h_create;

			pthread_create(&h_create, NULL, (void *) rutina_create, parametros);

			pthread_detach(h_create);

			break;
		}

	case DESCRIBE:
		;
		{

			pthread_t h_describe;

			pthread_create(&h_describe, NULL, (void *) rutina_describe, parametros);

			pthread_detach(h_describe);

			break;
		}

	case DROP:
		;
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
	t_config* config = config_create("/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/CONFIG/LFS.config");
	puntoDeMontaje = config_get_string_value(config,"PUNTO_MONTAJE");
	retardo = config_get_string_value(config,"RETARDO");
	tamanioValue = config_get_string_value(config,"TAMAÑO_VALUE");
	tiempoDump = config_get_string_value(config,"TIEMPO_DUMP");

	//asigno variables globales del Metadata.bin
	t_config* metadataLFS = config_create("Metadata/Metadata.bin");
	cantidadBloques = config_get_int_value(metadataLFS,"BLOCKS");
	tamanioBloques = config_get_int_value(metadataLFS,"BLOCK_SIZE");

	//chequear si de estas hay alguna que no va
	memtable = list_create();
	tabla_encontrada = list_create();
	registros_encontrados = list_create();
	particion_encontrada = list_create();
	fp_dump = NULL;
	memset(array_aux, 0X0, sizeof(array_aux));

	//agrego bitarray de cargar_configuracion_FS()
	crear_bitarray(cantidadBloques);
}


char* get_tabla(char* comando) {
	char **tokens_comando = string_split(comando, " ");
	char *tabla = tokens_comando[1];
	return tabla;
}

int get_key(char* comando) {
	char **tokens_comando = string_split(comando, " ");
	char *key = tokens_comando[2];
	return atoi(key);
}

char* get_value(char* comando) {
	int primera_ocurrencia = str_first_index_of('"', comando);
	int segunda_ocurrencia = str_last_index_of('"', comando);

	return string_substring(comando, primera_ocurrencia + 1, segunda_ocurrencia - primera_ocurrencia - 1);
}

//retorna el timestamp escrito en la request o el del epoch unix si la request no trae uno
double get_timestamp(char* comando) {
	if (comando[string_length(comando) - 1] == '"') { //si no incluyo un timestamp en la request
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
	} else {
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


int lista_vacia(t_list *lista)
{
	if(list_size(lista)==0){
		return 1;
	}else{
		return 0;
	}
}



int obtener_particiones_metadata(char* tabla)
{

	char* archivoMetadata = string_new();
	char* unaTabla = string_duplicate(tabla);
	string_append(&archivoMetadata,puntoDeMontaje);
	string_append(&archivoMetadata,"Tablas/");
	string_append(&archivoMetadata,unaTabla);
	string_append(&archivoMetadata,"/metadata"); ///// no se si las tenes guardadas como /metadata.config

	t_config* config = config_create(archivoMetadata);
	int nr_particiones_metadata = config_get_int_value(config, "PARTITIONS");
	config_destroy(config);
	free(archivoMetadata); //creo
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
	string_append(&archivoMetadata,"/metadata"); ///// no se si las tenes guardadas como /metadata.config

	t_config* config = config_create(archivoMetadata);
	consistency = malloc(strlen(config_get_string_value(config, "CONSISTENCY")));
	strcpy(consistency, config_get_string_value(config, "CONSISTENCY"));
	config_destroy(config);
	free(archivoMetadata); //creo
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
	string_append(&archivoMetadata,"/metadata"); ///// no se si las tenes guardadas como /metadata.config

	t_config* config = config_create(archivoMetadata);
	int tiempo_compactacion = config_get_int_value(config, "COMPACTION_TIME");
	config_destroy(config);
	free(archivoMetadata); //creo
	free(unaTabla);
	return tiempo_compactacion;
}

//////↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓DESARROLLO SELECT↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//////

t_bloque *crear_bloque_buscar(char *bloque)
{
	   t_bloque *new = malloc(sizeof(t_bloque));
	   new->name=strdup(bloque);
	   return new;
}

static void bloque_destroy(t_bloque *self) {
//    free(self->name);
    free(self);
}

static void agregar_bloque_busqueda(t_list *lista_agregar, t_bloque *bloque_buscado)
{
	list_add(lista_agregar, bloque_buscado);

}


t_par_valor_timestamp *crear_valor_timestamp_buscar(unsigned long timestamp, char *valor)
{
	t_par_valor_timestamp *new = malloc(sizeof(t_par_valor_timestamp));
	new->timestamp = timestamp;
	new->valor = strdup(valor);
	return new;
}

static void valor_timestamp_destroy(t_par_valor_timestamp *self) {
    free(self->valor);
    free(self);
}


static void agregar_valor_timestamp(t_list *timestamp_valor, t_par_valor_timestamp *par_valor_timestamp)
{
	list_add(timestamp_valor, par_valor_timestamp);
}



void liberar_bloques_buscar(t_list *lista_bloques)
{
	void liberar_elementos(void *elemento){
		return(bloque_destroy((t_bloque *)elemento));
	}

	list_iterate(lista_bloques, liberar_elementos);
}

void liberar_timestamp_valor(t_list *lista_timestamp_valor)
{
	void liberar_elementos(void *elemento){
		return(valor_timestamp_destroy((t_par_valor_timestamp *)elemento));
	}

	list_iterate(lista_timestamp_valor, liberar_elementos);
}




int buscar_tabla_FS(char *tabla_name)        ///// esta funcion no se usa y CREO que hace lo mismo que existe_tabla ... la sacamos?
{
	DIR *dir;
		struct dirent *sd;
		int encontrado=0;
		dir = opendir("../Tablas");

		if(dir==NULL){
			printf("No se puedo abrir el directorio de Tablas\n");
		}else{
			while( (sd = readdir(dir)) != NULL){
				if(!strcmp((sd->d_name), tabla_name)){
					encontrado = 1;
				}
			}
		closedir(dir);
		}
		return encontrado;


}


int obtener_size_particion(char *tabla, int particion_buscar)
{
	char* archivoParticion = string_new();
	char* unaTabla = string_duplicate(tabla);
	string_append(&archivoParticion,puntoDeMontaje);
	string_append(&archivoParticion,"Tablas/");
	string_append(&archivoParticion,unaTabla);
	string_append(&archivoParticion,"/");
	char* numeroParticion = string_itoa(particion_buscar);
	string_append(&archivoParticion,numeroParticion);
	string_append(&archivoParticion,".bin");
	//esto quedo feo con append, si queres lo volvemos al viejo pero con el punto de montaje

	t_config* config = config_create(archivoParticion);
	if(config == NULL){
		printf("La particion no existe\n");
		config_destroy(config);
		free(archivoParticion);
		free(numeroParticion);
		free(unaTabla);
		return -1;
	}else{
		int size_particion = config_get_int_value(config, "Size");
		config_destroy(config);
		free(archivoParticion);
		free(numeroParticion);
		free(unaTabla);
		return size_particion;
	}
	config_destroy(config);
}

int nr_particion_key(uint16_t key, int nr_particiones_metadata)
{
	return key%nr_particiones_metadata;
}


void comparar_key_y_agregar_valor (uint16_t key_recv, uint16_t key, char *valor, unsigned long timestamp, t_list *timestamp_valor)
{
	 if(key_recv == key){
		 t_par_valor_timestamp *valor_time = crear_valor_timestamp_buscar(timestamp, valor);
		 agregar_valor_timestamp(timestamp_valor, valor_time);
	}
}


int contar_comas(char *temp)
{
	int comas = 0, i = 0;
	while(i < strlen(temp)){
		if(temp[i] == 59)
			comas++;
		i++;
	}
	return comas;
}


 void buscar_key_bloques(char* bloque_nr, uint16_t key, t_list *timestamp_valor, int flag_last_bloque, int size_bloque)
 {
	 char* nroBloque = string_duplicate(bloque_nr);
	 char* root = string_new();
	 string_append(&root,puntoDeMontaje);
	 string_append(&root,"Bloques/");
	 string_append(&root,nroBloque);
	 string_append(&root,".bin");

	 int bloque_size = 0;
	 FILE *f;
	 f = fopen(root, "rb");
	 if(f==NULL){
		 printf("El archivo de Bloque %s no se ha podido abrir correctamente\n", bloque_nr);
	 }else{
		 char temp[32]= "";
		 char **tokens_registro;
		 char *string_aux_2;

		 while(!feof(f) && bloque_size < size_bloque){
			 fgets(temp, 32, f);
			 bloque_size += strlen(temp)*sizeof(char);
			 int nr_comas = contar_comas(temp);
			 tokens_registro = string_split(temp, ";");
			 if(flag_key_value != 0){
				if(control == 1 && nr_comas == 2){
					comparar_key_y_agregar_valor(atoi(tokens_registro[1]), key, strdup(tokens_registro[2]), atoi(tokens_registro[0]), timestamp_valor);
					flag_key_value = 0;
					control = 0;
					memset(array_aux, 0X0, sizeof(array_aux));
				}else{
					string_aux_2 = malloc(strlen(temp)*sizeof(char));
					strcpy(string_aux_2, temp);
				 	strcat(array_aux, string_aux_2);
//				 	printf("MOD: %s\n", array_aux);
				 	tokens_registro = string_split(array_aux, ";");
				 	comparar_key_y_agregar_valor(atoi(tokens_registro[1]), key, strdup(tokens_registro[2]), atoi(tokens_registro[0]), timestamp_valor);
				 	memset(array_aux, 0x0, sizeof(array_aux));
				 	free(string_aux_2);
				 	flag_key_value = 0;
				 	control = 0;
				 	}
				 }else{ //Si la flag == 0
					 if((nr_comas == 2) && ((flag_last_bloque == 1) && (bloque_size == size_bloque))){
						 comparar_key_y_agregar_valor(atoi(tokens_registro[1]), key, tokens_registro[2], atoi(tokens_registro[0]), timestamp_valor);

					 }else if(nr_comas == 2 && bloque_size < size_bloque){ //En el medio
						 comparar_key_y_agregar_valor(atoi(tokens_registro[1]), key, tokens_registro[2], atoi(tokens_registro[0]), timestamp_valor);
					 }else{
						if(nr_comas == 2)
							 control = 1;
						flag_key_value = 1;
						strcpy(array_aux, temp);
					 }
				 }
		 }
	 fclose(f);
	}
	 free(root);
	 free(nroBloque);
} //fin buscar_key_bloques



 int size_of_bloque(t_bloque *block)
 {
	 t_bloque *bloque = block;
	 char* root = string_new();
	 string_append(&root,puntoDeMontaje);
	 string_append(&root,"Bloques/");
	 string_append(&root,bloque->name);
	 string_append(&root,".bin");

	 FILE *bf;

	 bf = fopen(root, "r+b");
	 fseek(bf, 0L, SEEK_END);
	 int file_size = ftell(bf);
	 fclose(bf);
	 free(root);
	 free(bloque);
	 return file_size;
 }

 void cargar_timestamp_value(t_list *bloques_buscar, t_list *timestamp_valor, uint16_t key)
 {
	 int i;
	 int flag_last_bloque = 0;
	 int size_bloque;
	 int cant_bloques = list_size(bloques_buscar);
	 t_bloque *block = malloc(sizeof(t_bloque));
	 printf("Largo lista: %d\n", list_size(bloques_buscar));
	 for(i=0; i<cant_bloques;i++){
		 block = list_get(bloques_buscar, i);
		 if(i == cant_bloques-1)
			 flag_last_bloque = 1;
		 size_bloque = size_of_bloque(block);
		 buscar_key_bloques(block->name, key, timestamp_valor, flag_last_bloque, size_bloque);
	 }
	 free(block);
 }



int existe_tabla(char *tabla)
{
	char* nombreTablaGuardado = string_duplicate(tabla);
	string_to_upper(nombreTablaGuardado);

	char* pathAbsoluto = string_new();
	string_append(&pathAbsoluto,puntoDeMontaje);
	string_append(&pathAbsoluto,"Tablas/");
	string_append(&pathAbsoluto,nombreTablaGuardado);

	if (access(pathAbsoluto, F_OK) == -1){
	   free(pathAbsoluto);
	   free(nombreTablaGuardado);
	   return 0;
	}

	free(pathAbsoluto);
	free(nombreTablaGuardado);
	return 1; // devuelve 1 si existe la tabla, sino devuelve 0.
}


int cargar_bloques(char *root, t_list *bloques_buscar)
{
	FILE *f;
	f = fopen(root, "rb");
	if(f==NULL){
			printf("El archivo particion no se ha podido abrir correctamente\n");
			return 0;
	}else{
		 int i=0, cont=0;
		 char temp_helper[4];
		 char bloque[8]="";
		 char size[8]="";
		 int flag_bloque=0;

		 while(fread(&temp_helper[0], 1, 1, f)==1){
			 if(temp_helper[0]>=48 && temp_helper[0]<=57){
				 flag_bloque=1;
				 bloque[i]= temp_helper[0];
				 i++;
			 }else{
				 if(flag_bloque==1){
					 strcpy(&bloque[i], "\0");
					 flag_bloque=0;
					 i=0;
					 cont++;
					 if(cont==1){
						 strcpy(size, bloque);
					 }else{
						 t_bloque *bloque_creado = crear_bloque_buscar(bloque);
						 agregar_bloque_busqueda(bloques_buscar, bloque_creado);
					 }
				 }
			 }
		}
		fclose(f);
		return 1;
	}
}


void buscar_bloques_particion(char *tabla, int particion_buscar, int type_flag, t_list *bloques_buscar)
{
	char* root = string_new();
	char* unaTabla = string_duplicate(tabla);
	string_append(&root,puntoDeMontaje);
	string_append(&root,"Tablas/");
	string_append(&root,unaTabla);

	if(type_flag == 1){ //Buscar temporales
		 DIR *dir;
		 dir = opendir(root);
		 struct dirent *entrada;
		 char **entrada_aux;
		 while ((entrada = readdir(dir)) != NULL){
			 entrada_aux = string_split(entrada->d_name, ".");
			 if(&(&entrada_aux)[1] != NULL){
				 if (strcmp(entrada_aux[1],"tmp")==0){
					 strcat(root, "/");
					 strcat(root, entrada->d_name);
					 cargar_bloques(root, bloques_buscar);
					 memset(&root, '0', sizeof(char)*64);
					 strcpy(root, "../Tablas/");
					 strcat(root, tabla);
				 }
			 }
		 }
		closedir(dir);
	 }else{
		 strcat(root, "/part");
	 	 char snum[4];
	 	 sprintf(snum, "%i", particion_buscar);
	 	 strcat(root, snum);
	 	 strcat(root,".bin");
	 	cargar_bloques(root, bloques_buscar);
	 }
	free(unaTabla);
	free(root);
 }//fin buscar_bloques_particion



bool comparar_nombre(char *tabla, void *tabla_mt)
{
	return (!strcmp(((t_tabla *)tabla_mt)->name_tabla, tabla));
}

bool comparar_particion(int particion_buscar, void *particion)
{
	return (particion_buscar == ((t_particion *)particion)->num);
}

bool comparar_registro(uint16_t key, void *registro)
{
	return (key == ((t_registro *)registro)->key);
}


t_list *filtrar_tabla_memtable(char *tabla)
{
	printf("largo memtable: %d\n",list_size(memtable));

	bool coincide_nombre(void *tabla_mt){
		return comparar_nombre(tabla, tabla_mt);
	}

	tabla_encontrada = list_filter(memtable, coincide_nombre); //Supongo que solo extrae 1 tabla
	if(lista_vacia(tabla_encontrada))
		printf("La tabla <%s> no se encuentra en la memtable\n", tabla);

	return tabla_encontrada;
}


t_list *filtrar_particion_tabla(t_list *tabla_encontrada, int particion_buscar)
{
	bool coincide_particion(void *particion){
		return comparar_particion(particion_buscar, particion);
	}

	if(lista_vacia(tabla_encontrada)){
		return tabla_encontrada;
	}else{

		t_tabla *tabla_recbida = malloc(sizeof(t_tabla));
		tabla_recbida = (t_tabla*)list_get(tabla_encontrada, 0);
		particion_encontrada = list_filter(tabla_recbida->lista_particiones, coincide_particion);
		//free(tabla_recbida);
		return particion_encontrada;
	}
}


t_list *filtrar_registros_particion(t_list *particion_encontrada, uint16_t key)
{
	bool coincide_key(void *registro){
		return comparar_registro(key, registro);
	}

	if(lista_vacia(particion_encontrada))
		return particion_encontrada;

	registros_encontrados = list_filter(((t_particion *)list_get(particion_encontrada, 0))->lista_registros, coincide_key);
	return registros_encontrados;

}




t_par_valor_timestamp *filtrar_timestamp_mayor(t_list *timestamp_valor, int list_size)
{
	int i;
	t_par_valor_timestamp *value_aux, *value;
	value_aux = (t_par_valor_timestamp *)list_get(timestamp_valor, 0);
	for(i=1; i<list_size; i++){
		value = (t_par_valor_timestamp *)list_get(timestamp_valor, i);
		if(value->timestamp > value_aux->timestamp){
			value_aux = value;
		}
	}
	return value_aux;
}


 void buscar_en_todos_lados(char *tabla, uint16_t key, int particion_buscar)
 {
	t_list *bloques_buscar= list_create();
	t_list *timestamp_valor = list_create();
	buscar_bloques_particion(tabla, particion_buscar, 0, bloques_buscar); //Busca particion.bin
	buscar_bloques_particion(tabla, particion_buscar, 1, bloques_buscar); //Busca .tmp
	cargar_timestamp_value(bloques_buscar, timestamp_valor, key); //Lee en los bloques y carga los pares en lista "timestamp_valor"

	t_list *registros_encontrados = filtrar_registros_particion(filtrar_particion_tabla(filtrar_tabla_memtable(tabla), particion_buscar), key); /*Busca en la Memtable.
																							Devuelve lista de registros que cumplen con la key*/
	int cant_registros = list_size(registros_encontrados);
	int i;
	if(cant_registros!=0){
		for(i=0; i<cant_registros; i++){
			t_registro *registro_extraido = malloc(sizeof(t_registro));
			registro_extraido = (t_registro *)list_get(registros_encontrados,i);
			char *valor = strdup(registro_extraido->value);
			agregar_valor_timestamp(timestamp_valor, crear_valor_timestamp_buscar(registro_extraido->timestamp, valor));
		}
	}else{
		printf("No se encontraron registros con la key %d en la tabla %s en la memtable\n", key, tabla);
	}
	printf("Largo lista *timestamp_valor* %d\n", list_size(timestamp_valor));
	if(lista_vacia(timestamp_valor)){
		printf("*timestamp_valor* VACIO\n");
	}else{
		t_par_valor_timestamp *timestamp_value_max = filtrar_timestamp_mayor(timestamp_valor, list_size(timestamp_valor));
		printf("El valor de la key ingresada es: %s\n", timestamp_value_max->valor);
	}
	liberar_bloques_buscar(bloques_buscar);
	liberar_timestamp_valor(timestamp_valor);
}


void rutina_select(char* comando)
{

	printf("operacion: select\n");

	char *tabla=get_tabla(comando);
	printf("tabla: %s\n", get_tabla(comando));

	uint16_t key = get_key(comando);
	printf("key: %d\n", get_key(comando));

	if(existe_tabla(tabla)){
		int nr_particiones_metadata = obtener_particiones_metadata(tabla);
		int particion_buscar = nr_particion_key(key, nr_particiones_metadata);
		printf("La particion a buscar es: %d\n", particion_buscar);

		buscar_en_todos_lados(tabla, key, particion_buscar);
	}else{
		printf("No se ha podido realizar la operacion\n");
		}
}


//////↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑DESARROLLO SELECT↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑//////


//////↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓DESARROLLO INSERT↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//////

t_registro *crear_registro(unsigned long timestamp, uint16_t key, char *value)
{
	t_registro *new = malloc(sizeof(t_registro));
	new->key = key;
	new->timestamp = timestamp;
	new->value = strdup(value);

	return new;
}

void registro_destroy(t_registro *self)
{
	free(self->value);
	free(self);
}


t_particion *crear_particion_memtable(int size, int particion_buscar)
{
	t_particion *new = malloc(sizeof(t_particion));
	new->num = particion_buscar;
	new->size = size;
	new->lista_registros = list_create();

	return new;
}

void particion_destroy(t_particion *self)
{
	free(self->lista_registros);
	free(self);
}

t_particion *agregar_registro_en_particion_nueva(t_particion *nueva_particion, t_registro *registro_nuevo)
{
	list_add(nueva_particion->lista_registros, registro_nuevo);
	return nueva_particion;
}

t_tabla *crear_tabla_memtable(char *tabla)
{
	t_tabla *new = malloc(sizeof(t_tabla));
	new->name_tabla = strdup(tabla);
	new->lista_particiones = list_create();
	return new;
}

void tabla_destroy(t_tabla *self)
{
	free(self->name_tabla);
	free(self->lista_particiones);
	free(self);
}


t_tabla *agregar_particion_en_tabla_nueva(t_tabla *nueva_tabla, t_particion *nueva_particion)
{
	list_add(nueva_tabla->lista_particiones, nueva_particion);
	return nueva_tabla;
}

void agregar_tabla_memtable(t_list* memtable, t_tabla *nueva_tabla)
{
	list_add(memtable, nueva_tabla);
}


void agregar_particion_en_tabla_existente(char* tabla, t_particion *nueva_particion)
{
	int i;
	t_tabla *tabla_extraida;
	for(i=0; i<list_size(memtable); i++){
		tabla_extraida = (t_tabla *)list_get(memtable, i);
		if (!strcmp(tabla_extraida->name_tabla, tabla)){
			list_add(tabla_extraida->lista_particiones, nueva_particion);
		}
	}
}


void agregar_registro_en_particion_existente(char *tabla, int particion_buscar, t_registro *registro_nuevo)
{
	int i, j;
	t_tabla *tabla_extraida = malloc(sizeof(t_tabla));
	t_particion *particion_extraida = malloc(sizeof(t_particion));

	for(i=0; i<list_size(memtable); i++){
		tabla_extraida = (t_tabla*)list_get(memtable, i);
		if (!strcmp(tabla_extraida->name_tabla, tabla)){
			for(j=0; j<list_size(tabla_extraida->lista_particiones); j++){
				particion_extraida = (t_particion*)list_get(tabla_extraida->lista_particiones, j);
				if(particion_extraida->num == particion_buscar){
					list_add(particion_extraida->lista_registros, registro_nuevo);
				}
			}
		}
	}
	//free(particion_extraida);
	//free(tabla_extraida);
}




//void liberar_tabla_encontrada(t_list *tabla_encontrada)
//{
//	void liberar_tabla(void *elemento){
//		tabla_destroy((t_tabla *)elemento);
//	}
//
//	list_iterate(tabla_encontrada, liberar_tabla);
//}

void rutina_insert(char* comando)
{
	printf("operacion: insert\n");

	char *tabla=get_tabla(comando);
	printf("tabla: %s\n", tabla);

	uint16_t key = get_key(comando);
	printf("key: %d\n", key);

	char* value = get_value(comando);
	if(strlen(value)>tamanioValue){
		printf("El valor ingresado sobrepasa el tamaño maximo permitido.\n");
	}else{
		printf("value: %s\n", value);

		unsigned long timestamp = get_timestamp(comando);
		printf("timestamp: %lu\n", timestamp);

		if(existe_tabla(tabla)){
			int nr_particiones_metadata = obtener_particiones_metadata(tabla);
			int particion_buscar = nr_particion_key(key, nr_particiones_metadata);
			int size = obtener_size_particion(tabla, particion_buscar);
			if(size>0){
				printf("La particion donde se agregara el registro es: %d\n", particion_buscar);

				t_list *tabla_encontrada = list_create();
				tabla_encontrada = filtrar_tabla_memtable(tabla);
				t_list *lista_particion_encontrada = filtrar_particion_tabla(tabla_encontrada, particion_buscar);
				t_registro *registro_nuevo = malloc(sizeof(t_registro));
				registro_nuevo = crear_registro(timestamp, key, value);

				if(lista_vacia(tabla_encontrada)){
					t_particion *nueva_particion = malloc(sizeof(t_particion));
					nueva_particion = crear_particion_memtable(size, particion_buscar);
					agregar_registro_en_particion_nueva(nueva_particion, registro_nuevo);
					t_tabla *nueva_tabla = malloc(sizeof(t_tabla));
					nueva_tabla = crear_tabla_memtable(tabla);
					agregar_particion_en_tabla_nueva(nueva_tabla, nueva_particion);
					agregar_tabla_memtable(memtable, nueva_tabla);
				}else if(lista_vacia(lista_particion_encontrada)){ //tabla en memtable pero la particion esta vacia
					t_particion *nueva_particion = crear_particion_memtable(size, particion_buscar);
					agregar_registro_en_particion_nueva(nueva_particion, registro_nuevo);
					agregar_particion_en_tabla_existente(tabla, nueva_particion);
				}else{
					agregar_registro_en_particion_existente(tabla, particion_buscar, registro_nuevo);
				}
				printf("Registro agregado a la particion\n");
			}

		}else{
			printf("La tabla solicitada no se encuentra en el sistema\n");
		}
	}
//	liberar_tabla_encontrada(tabla_encontrada);
}



//////↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑DESARROLLO INSERT↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑//////


//////↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓DESARROLLO CREATE↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//////

void agregar_salto_de_linea(char *string)
{
	strcat(string, "\n");
}

void guardar_bitarray(t_bitarray *bitarray, long int index)
{
	char* root = string_new();
	string_append(&root,puntoDeMontaje);
	string_append(&root,"Metadata/Bitmap.bin");


	FILE *fp = fopen(root, "r+b");
	fp->_offset = index;
	fseek(fp, 0l, SEEK_CUR);
	fprintf(fp, "%d",  bitarray_test_bit(bitarray, index));
	fclose(fp);
	free(root);
}


int elegir_bloque_libre(int nr_bloques)
{
	int free_block, i, flag_free_block = 0;
	for(i = 0; i < nr_bloques; i++){
		if (flag_free_block == 0){
			if(bitarray_test_bit(bitarray, i) == 0){
				flag_free_block = 1;
				free_block = i;
				bitarray_set_bit(bitarray, i);
				guardar_bitarray(bitarray, i);
				return free_block;
			}
		}
	}
	return -1;
}

void crear_particiones(char *dir, int particiones)
{
	char *root_aux = malloc(strlen(dir)+strlen("/part"));
	memset(root_aux, '0', strlen(dir)+strlen("/part"));
	char *block_aux = malloc(sizeof(12));
	char *part_aux = malloc(sizeof(32));
	strcpy(root_aux, dir);
	strcat(root_aux, "/part");
	char *root = malloc(strlen(root_aux)+sizeof(part_aux)+strlen(".bin"));
	char *size_text = malloc(strlen("Size=")+sizeof(int)*4);
	char *size_aux = malloc(32);
	strcpy(size_text, "Size=");
	sprintf(size_aux, "%d", tamanioBloques);
	strcat(size_text, size_aux);
	strcat(size_text, "\n");

	int i;
	for(i = 0; i < particiones; i++){
		sprintf(part_aux, "%d", i);
		strcpy(root, root_aux);
		strcat(root, part_aux);
		strcat(root, ".bin");

		FILE *fp;
		fp = fopen(root, "w+b");
		int free_block = elegir_bloque_libre(cantidadBloques);
		if(free_block == -1){
			printf("No hay bloques libres\n");
		}else{
			fputs(size_text, fp);
			fprintf(fp, "%s", "Block=[");
			sprintf(block_aux, "%d", free_block);
			fputs(block_aux, fp);
			fprintf(fp, "%s", "]");
			memset(root, '0', strlen(root));
			memset(part_aux, '0', strlen(part_aux));
			fclose(fp);
		}
	}
	free(size_text);
	free(root_aux);
	free(root);
	free(block_aux);
	free(part_aux);
}


int crear_tabla_FS(char *tabla, int particiones, char *consistencia, int compact_time)
{
	//creo que esta es la razon por la que rompe tdo, preguntar que se puede copiar de la mia para no hardcodear path

	int flag_creacion;
	char *tabla_dir = malloc(strlen("../Tablas")+strlen(tabla)+24);
	char *dir_metadata = malloc(strlen("../Tablas")+strlen(tabla)+strlen("/metadata.config")+24);
	char *consistencia_tabla = malloc(strlen("CONSISTENCY=")+strlen(consistencia)+4);
	char *part_aux = malloc(24);
	char *compact_time_aux = malloc(24);
	sprintf(part_aux,"%d", particiones);
	sprintf(compact_time_aux, "%d", compact_time);
	char *particiones_tabla = malloc(strlen("PARTITIONS=")+strlen(compact_time_aux)+4);
	char *compactacion_tabla = malloc(strlen("COMPACTION_TIME=")+strlen(compact_time_aux)+4);


	strcpy(tabla_dir, "../Tablas/");
	strcat(tabla_dir, tabla);

	strcpy(dir_metadata, tabla_dir);
	strcat(dir_metadata, "/metadata.config");

	strcpy(consistencia_tabla, "CONSISTENCY=");
	strcat(consistencia_tabla, consistencia);
	agregar_salto_de_linea(consistencia_tabla);

	strcpy(particiones_tabla, "PARTITIONS=");
	strcat(particiones_tabla, part_aux);
	agregar_salto_de_linea(particiones_tabla);

	strcpy(compactacion_tabla, "COMPACTION_TIME=");
	strcat(compactacion_tabla, compact_time_aux);

	flag_creacion = mkdir(tabla_dir,0777);

	crear_particiones(tabla_dir, particiones);

	FILE *fp;
	fp = fopen(dir_metadata, "w+");
	if(fp == NULL){
		printf("Fallo Creacion archivo metadata\n");
	}else{
		fputs(consistencia_tabla, fp);
		fputs(particiones_tabla, fp);
		fputs(compactacion_tabla, fp);
	}
	free(tabla_dir);
	free(consistencia_tabla);
	fclose(fp);
	free(dir_metadata);
	return flag_creacion;
}


void rutina_create(char* comando)
{
	//CREATE [NOMBRE_TABLA] [TIPO_CONSISTENCIA] [NUMERO_PARTICIONES] [COMPACTION_TIME]
	printf("operacion: create\n");

	char *tabla= strdup(get_tabla(comando));
	printf("tabla: %s\n", tabla);

	char *consistencia = get_consistencia(comando);
	printf("tipo de consistencia: %s\n",consistencia);

	int particiones = get_particiones(comando);
	printf("numero de particiones: %d\n", particiones);

	int compactacion = get_tiempo_compactacion(comando);
	printf("tiempo de compactacion: %d\n",compactacion);

	if(existe_tabla(tabla)){
		printf("\nLa tabla ya existe.\n");
		log_info(logger, "Se intento crear una tabla ya existente [%s]", tabla);
	}else{
		printf("Se creara la tabla\n");
		if(crear_tabla_FS(tabla, particiones, consistencia, compactacion)==0)
			printf("Se creo la tabla de forma correcta\n");

	}
}


//////↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑DESARROLLO CREATE↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑//////


//////↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓DESARROLLO DESCRIBE↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//////

int tipo_describe(char *comando)
{
	char **tokens_comando = string_split(comando, " ");
	if(tokens_comando[1] == NULL)
		return 0;
	return 1;
}


struct describe *crear_descripcion(char *name, char *consistencia, int particiones, int tiempo_compactacion)
{
	struct describe *new = malloc(sizeof(struct describe));
	new->compaction_time = tiempo_compactacion;
	new->consistency = strdup(consistencia);
	new->name = strdup(name);
	new->partitions = particiones;
	return new;
}

void descripcion_destroy(struct describe *self)
{
	free(self->consistency);
	free(self->name);
	free(self);
}

void liberar_descripcion(t_list *lista_describe)
{
	void liberar_elementos(void *elemento){
		return(descripcion_destroy((struct describe *)elemento));
	}

	list_iterate(lista_describe, liberar_elementos);
}


void cargar_datos_tabla(char *tabla)
{
	struct describe *descripcion = crear_descripcion(tabla, obtener_consistencia_metadata(tabla), obtener_particiones_metadata(tabla),
													obtener_tiempo_compactacion_metadata(tabla));
	list_add(lista_describe, descripcion);
}

void mostrar_campos_describe(void *element)
{
	struct describe * recv = malloc(sizeof(struct describe));
	recv = (struct describe *)element;
	printf("Table Name: %s\nConsistency: %s\nPartitions: %d\nCompaction_Time: %d\n\n", recv->name, recv->consistency,
																	recv->partitions, recv->compaction_time);
}

void mostrar_descripciones_metadata(t_list *lista_describe)
{
	list_iterate(lista_describe, mostrar_campos_describe);
}


void agregar_tablas_a_describir()
{
	struct dirent *sd;
	char* tablas = string_new();
	string_append(&tablas, puntoDeMontaje);
	string_append(&tablas, "Tablas");
	DIR* dir = opendir(tablas);

	while((sd = readdir(dir)) != NULL){
		if((strcmp((sd->d_name),".") != 0) && (strcmp((sd->d_name),"..") != 0)){
			cargar_datos_tabla(sd->d_name);
		}
	}
	closedir(dir);
	free(tablas);
}

void describe_full(char *comando)
{
	lista_describe = list_create();
	agregar_tablas_a_describir();
	mostrar_descripciones_metadata(lista_describe);
	liberar_descripcion(lista_describe);

}

void describe_particular(char *comando)
{
	char *tabla = get_tabla(comando);
	if(existe_tabla(tabla)){
		printf("Consistency: %s\nPartitions: %d\nCompaction_Time: %d\n", obtener_consistencia_metadata(tabla),
								obtener_particiones_metadata(tabla), obtener_tiempo_compactacion_metadata(tabla));
	}
}

void rutina_describe(char* comando)
{
	printf("Rutina describe\n");
	int tipo = tipo_describe(comando);
	switch (tipo){
	case 0:
		describe_full(comando);
		break;
	case 1:
		describe_particular(comando);
		break;
	default:
		printf("Error en Comando Describe.\n");
	}

}

//////↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑DESARROLLO DESCRIBE↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑//////


//////↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓DESARROLLO DUMP↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//////
struct bloques_tmp *crear_bloques_tmp(char *tabla)
{
	struct bloques_tmp *new = malloc(sizeof(struct bloques_tmp));
	new->tabla = strdup(tabla);
	new->bloques = list_create();
	new->size_total = 0;
	return new;
}

void destroy_nr_bloque(void *elemento)
{
	struct bloque *bloque = elemento;
	free(bloque);
}

void bloques_tmp_destroy(void *elemento)
{
	struct bloques_tmp *self = (struct bloques_tmp *)elemento;
	free(self->tabla);
	void liberar_nr_bloque(void *elemento){
		return destroy_nr_bloque(elemento);
	}
	list_clean_and_destroy_elements(self->bloques, liberar_nr_bloque);
	free(self);
}

struct bloque *crear_nr_bloque(int nr_bloque)
{
	struct bloque *new = malloc(sizeof(struct bloque));
	new->nr_block = nr_bloque;
	return new;
}


void agregar_bloque_lista_tmp(t_list *bloques, int bloque_dump)
{
	struct bloque *nr_bloque = malloc(sizeof(struct bloque));
	nr_bloque = crear_nr_bloque(bloque_dump);
	list_add(bloques, nr_bloque);
}


void grabar_registro(char *root, char *registro_completo, int length_registro, int space_full, int index, int table_change,
					 struct bloques_tmp *bloques_tmp, int flag_close_file)
{
	if(flag_close_file == 1 && fp_dump != NULL){
		fclose(fp_dump);
		fp_dump = NULL;
	}else{
		espacio_libre = tamanioBloques;

		if(table_change == 0){
			fp_dump = fopen(root, "ab");
			size = 0;
			suma_size = 0;
			j = 0;
		}

		if(space_full == 2){
			space_full = 0;
		}

		if(space_full == 1){ //Completo lo que falta de un registro
			size = 0;
			suma_size = 0;
			while(index < length_registro){
				fputc(registro_completo[index], fp_dump);
				size = sizeof(registro_completo[index]);
				suma_size += size;
				index++;
				bloques_tmp->size_total += size;
			}
			space_full = 2;
		}

		if(space_full == 0){ //Escribo registro
			j = 0;
			while((j < length_registro) && (suma_size < espacio_libre)){
				fputc(registro_completo[j], fp_dump);
				size = sizeof(registro_completo[j]);
				suma_size += size;
				j++;
				bloques_tmp->size_total += size;
			}
			if((j < length_registro) && ((suma_size == espacio_libre) || (suma_size > espacio_libre))){ //Si se queda sin espacio
				fclose(fp_dump);
				bloque_dump = elegir_bloque_libre(cantidadBloques);
				agregar_bloque_lista_tmp(bloques_tmp->bloques, bloque_dump);
				char root[128] = "";
				sprintf(root, "../Bloques/bloque%d.bin", bloque_dump);
				grabar_registro(root, registro_completo, length_registro, 1, j, 0, bloques_tmp, 0);
			}
		}
	}
}



void guardar_registros_en_bloques(t_registro *registro_recv, int table_change, struct bloques_tmp *bloques_tmp_tabla)
{
	/*
	 * table_change = 0 NUEVA TABLA/CAMBIAR TABLA
	 * table_change = 1 NO CAMBIAR TABLA
	 */
	t_registro *registro = malloc(sizeof(t_registro));
	registro = registro_recv;

	char root[128] = "";
	char timestamp_char[32];
	char key_char[16];
	sprintf(timestamp_char, "%lu", registro->timestamp); /*Si no, usar snprintf() */
	sprintf(key_char, "%d", registro->key);
	int length_registro = strlen(timestamp_char)+ strlen(key_char)+strlen(registro->value)+3;
	char registro_completo[length_registro];
	strcpy(registro_completo, timestamp_char);
	strcat(registro_completo, ";");
	strcat(registro_completo, key_char);
	strcat(registro_completo, ";");
	strcat(registro_completo, registro->value);
	strcat(registro_completo, "\n");

	if(table_change == 0){
		bloque_dump = elegir_bloque_libre(cantidadBloques);
		agregar_bloque_lista_tmp(bloques_tmp_tabla->bloques, bloque_dump); //Para crear el archivo temporal
		sprintf(root, "../Bloques/bloque%d.bin", bloque_dump);
	}
	grabar_registro(root, registro_completo, length_registro, 0, 0, table_change, bloques_tmp_tabla, 0);
}


int contar_temporales(char *root)
{
	int cont = 0;
	DIR * dir;
	dir = opendir(root);
	struct dirent *entrada;
	char **entrada_aux;
	while ((entrada = readdir(dir)) != NULL){
		entrada_aux = string_split(entrada->d_name, ".");
		if(entrada_aux[1] != NULL){
			if (strcmp(entrada_aux[1],"tmp")==0){
				cont++;
			}
		}
	}
	closedir(dir);
	return cont;
}

int obtener_size(char *size_array) //Se puede usar en COMPACTACION. Recibe la primer linea leída
{
	char **token = string_split(size_array, "=");
	int size = 0;
	if(token[1] == NULL)
		return size;
	size = atoi(token[1]);
	return size;
}


int archivo_vacio(FILE *fp) //Devuelve el tamaño. 0 si esta vacio
{
	fseek(fp, 0L, SEEK_END);
	int file_size = 0;
	file_size = ftell(fp);
	return file_size;
}

void agregar_bloque_particion(void *elemento)
{
	struct bloques_tmp *blocks_info = (struct bloques_tmp *)elemento;
	char root[128];
	int next_tmp;
	int i;
	int cant_bloques = list_size(blocks_info->bloques);
	sprintf(root, "../Tablas/%s", blocks_info->tabla);
	next_tmp = contar_temporales(root) + 1;
	sprintf(root, "../Tablas/%s/%s%d.tmp", blocks_info->tabla, blocks_info->tabla, next_tmp);
	FILE *fp;
	fp = fopen(root, "a");

	fprintf(fp, "Size=%d\n", blocks_info->size_total);
	fprintf(fp, "Blocks=[");
	for(i = 0; i < cant_bloques; i++){
		struct bloque *bloque = malloc(sizeof(struct bloque));
		bloque = list_get(blocks_info->bloques, i);
		fprintf(fp, "%d", bloque->nr_block);
		if(i <= cant_bloques-2)
			fputs(",", fp);
	}
	fputs("]", fp);
	fclose(fp);
}


void liberar_elementos_particiones(void *elemento)
{
	t_particion *particion = elemento;
	void liberar_elementos_particion(void *elemento){
		return registro_destroy((t_registro *)elemento);
	}
	list_iterate(particion->lista_registros, liberar_elementos_particion);
}


void liberar_tabla(void *elemento)
{
	t_tabla *tabla = elemento;
	void liberar_elementos_tabla(void *elemento){
		return liberar_elementos_particiones(elemento);
	}
	list_iterate(tabla->lista_particiones, liberar_elementos_tabla);

	void liberar_particiones(void *elemento){
		return particion_destroy((t_particion *)elemento);
	}
	list_iterate(tabla->lista_particiones, liberar_particiones);

	tabla_destroy(tabla);
}


void liberar_memtable()
{
	void liberar_elementos(void *elemento){
		return liberar_tabla(elemento);
	}
	list_iterate(memtable, liberar_elementos);
}

void guardar_bloques_metadata(t_list *lista_bloques_tmp)
{
	void guardar_bloque_temp(void *elemento){
		return agregar_bloque_particion(elemento);
	}
	list_iterate(lista_bloques_tmp, guardar_bloque_temp);
}


void liberar_lista_bloques(t_list *lista_bloques_tmp)
{
	void liberar_bloques_tmp(void *elemento){
		return bloques_tmp_destroy(elemento);
	}
	list_clean_and_destroy_elements(lista_bloques_tmp, liberar_bloques_tmp);
}

void dump()
{
	int i,j,k;
	int table_change;
	lista_bloques_tmp = list_create();
	for(i = 0; i < list_size(memtable); i++){
		space_full = 0;
		fp_dump = NULL;
		table_change = 0;
		t_tabla *tabla  = malloc(sizeof(t_tabla));
		tabla = list_get(memtable,i);
//		printf("TABLE NAME: %s\n", tabla->name_tabla);
		if(existe_tabla(tabla->name_tabla)){
			struct bloques_tmp *bloques_tmp_tabla = malloc(sizeof(struct bloques_tmp));
			bloques_tmp_tabla = crear_bloques_tmp(tabla->name_tabla);
			for(j = 0; j < list_size(tabla->lista_particiones); j++){
				t_particion *particion = malloc(sizeof(t_particion));
				particion = list_get(tabla->lista_particiones, j);
				for(k = 0; k < list_size(particion->lista_registros); k++){
					t_registro *registro = malloc(sizeof(t_registro));
					registro = list_get(particion->lista_registros, k);
					if(table_change == 0 && k == 1)
						table_change = 1;
					guardar_registros_en_bloques(registro, table_change, bloques_tmp_tabla);
				}
			}
			grabar_registro("NULL", "NULL", 0, 0, 0, 0, bloques_tmp_tabla, 1);//Close fp_dump
			list_add(lista_bloques_tmp, bloques_tmp_tabla);
			}
		}
	liberar_memtable();
	memtable = list_create();
	guardar_bloques_metadata(lista_bloques_tmp);
	liberar_lista_bloques(lista_bloques_tmp);
}


void ejecutar_dump(){

	struct itimerval initial;
	struct timeval tiempo_inicial;

	tiempo_inicial.tv_usec = (tiempoDump)*1000;
	tiempo_inicial.tv_sec = 0;

	memset(&(initial.it_interval), 0x0, sizeof(initial.it_value));
	memset(&(initial.it_value), 0x0, sizeof(initial.it_value));

	initial.it_interval = tiempo_inicial;
	initial.it_value = tiempo_inicial;

	signal(SIGALRM, &dump);

	if (setitimer(ITIMER_REAL, &initial, NULL) == -1) {
		perror("error calling setitimer()");
	}

	while(1){
		pause();
		sleep(retardo);
	}
}

//////↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑DESARROLLO DUMP↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑//////



//////↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓DESARROLLO DROP↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓//////

void iterar_busqueda_de_bloques(void (foo)(char *, int, int, t_list *), char *name, int part, int flag, t_list *lista, int cant)
{
	int i;
	for(i = 0; i < cant; i++){
		foo(name, i, flag, lista);
	}
}

void eliminar_contenido_de_tabla(char *tabla)
{
	DIR *dir;
	struct dirent *sd;
	char *root = malloc(strlen("../Tablas/")+34);
	strcpy(root, "../Tablas/");
	strcat(root, tabla);
	dir = opendir(root);

	strcat(root, "/");
	char *aux = malloc(strlen(root)+34);
	while((sd = readdir(dir)) != NULL){
		if((strcmp((sd->d_name),".") != 0) && (strcmp((sd->d_name),"..") != 0)){
			strcpy(aux, root);
			strcat(aux, sd->d_name);
			remove(aux);
			memset(aux, '0', strlen(aux));
		}
	}
	free(root);
	free(aux);
	closedir(dir);
}

void eliminar_tabla_fisicamente(char *tabla)
{
	char *root = malloc(strlen("../Tablas/")+34);
	strcpy(root, "../Tablas/");
	strcat(root, tabla);
	rmdir(root);
	free(root);
}


void desmarcar_bloque_bitmap(t_bloque *elemento)
{
	char block_root[128];
	int block = atoi(elemento->name);
	bitarray_clean_bit(bitarray, block);
	guardar_bitarray(bitarray, block);
	sprintf(block_root, "../Bloques/bloque%d.bin", block);
	FILE *fp;
	fp = fopen(block_root, "w+");
	fclose(fp);

}

void liberar_bloques(t_list *bloques_buscar)
{
	void desmarcar_bloque(void *elemento){
		return(desmarcar_bloque_bitmap((t_bloque *)elemento));
	}
	list_iterate(bloques_buscar, desmarcar_bloque);
}

void eliminar_tabla(char *tabla)
{
	t_list *bloques_buscar= list_create();
	int particiones = obtener_particiones_metadata(tabla);
	iterar_busqueda_de_bloques(buscar_bloques_particion, tabla, 1, 0, bloques_buscar, particiones); //part.bin
	buscar_bloques_particion(tabla, 1, 1, bloques_buscar); //.tmp
	eliminar_contenido_de_tabla(tabla);
	eliminar_tabla_fisicamente(tabla);
	liberar_bloques(bloques_buscar);
}

void rutina_drop(char* comando)
{
	printf("Rutina Drop\n");
	char *tabla = get_tabla(comando);
	if(existe_tabla(tabla)){
		eliminar_tabla(tabla);
	}
}

//////↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑DESARROLLO DROP↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑//////


void crear_bloques_FS(int nr_blocks)
{
	int i;
	char aux[24]="";
	int size = strlen("../Bloques/bloque")+strlen(".bin")+sizeof(aux);
	char *root = malloc(size);


	for(i = 0; i < nr_blocks; i++){
		sprintf(aux, "%d", i);
		strcpy(root, "../Bloques/bloque");
		strcat(root, aux);
		strcat(root, ".bin");
		FILE *fp;
		fp = fopen(root, "w+b");
		memset(root, '0', strlen(root));
		fclose(fp);
	}
}


/*
 * Supongo que Bitmap.bin EXISTE
 * Si esta vacio el Bitmap, le carga la cantidad de bloques que dice el arhivo Metada
 * y los setea a todos en 0. Despues crea a los Bloques (supongo que no existen)
 * Si el Bitmap tiene datos supongo que ya existe los Bloques y algunos estan lleno y otros no
 */

void crear_bitarray(int nr_blocks){
	char data[nr_blocks];
	memset(&data, '0', sizeof(data));

	bitarray = bitarray_create_with_mode(data, sizeof(data), LSB_FIRST);
	setear_bitarray(bitarray, nr_blocks);
}

void setear_bitarray(t_bitarray *bitarray, int nr_blocks)
{
	long i=0;
	char c;
	char *root = malloc(strlen("../Metadata/Bitmap.bin"));
	strcpy(root, "../Metadata/Bitmap.bin");
	FILE *fp;
	fp = fopen(root, "r+b");
	if(fp == NULL)
		printf("No se pudo abrir/crear el archivo de bitmap\n");

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
	free(root);
	fclose(fp);
}




//retorna el indice de la primera ocurrencia del char o -1 si no aparece nunca
int str_first_index_of(char c, char* cadena) {
	int i=0;
	while(cadena[i] != c && cadena[i] != '\0') {
		i++;
	}

	return cadena[i] == '\0'? -1 : i;
}

//retorna el indice de la ultima ocurrencia del char o -1 si no aparece nunca
int str_last_index_of(char c, char* cadena) {
	int ultima_ocurrencia = -1;

	for(int i=0; i < string_length(cadena); i++) {
		if (cadena[i] == c) {
			ultima_ocurrencia = i;
		}
	}

	return ultima_ocurrencia;
}

