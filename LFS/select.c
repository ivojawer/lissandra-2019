#include "funcionesLFS.h"

extern t_list* memtable;
extern char* puntoDeMontaje;


t_list *particion_encontrada;
t_list *registros_encontrados;
t_list *tabla_encontrada;

extern int control;
extern char array_aux[128];
extern int flag_key_value;

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

		char* root = string_duplicate(puntoDeMontaje);
		string_append(&root,"Tablas/");

		dir = opendir(root);

		if(dir==NULL){
			printf("No se puedo abrir el directorio de Tablas\n");
		}else{
			while( (sd = readdir(dir)) != NULL){
				if(!strcmp((sd->d_name), tabla_name)){
					encontrado = 1;
				}
			}
		closedir(dir);
		free(root);
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
	string_append(&archivoParticion,"part");
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
		 printf("El bloque %s no se pudo abrir.\n", bloque_nr);
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
}



 int size_of_bloque(t_bloque *block)
 {
	 t_bloque *bloque = block;
	 char* root = string_new();
	 string_append(&root,puntoDeMontaje);
	 string_append(&root,"Bloques/");
	 string_append(&root,"bloque");
	 string_append(&root,bloque->name);
	 string_append(&root,".bin");

	 FILE *bf;

	 bf = fopen(root, "r+b");
	 fseek(bf, 0L, SEEK_END);
	 int file_size = ftell(bf);
	 fclose(bf);
	 free(root);
	 return file_size;
 }

 void cargar_timestamp_value(t_list *bloques_buscar, t_list *timestamp_valor, uint16_t key)
 {
	 int flag_last_bloque = 0;
	 int size_bloque;
	 int cant_bloques = list_size(bloques_buscar);
	 t_bloque *block = malloc(sizeof(t_bloque));

	 for(int i = 0; i < cant_bloques; i++){
		 t_bloque *block = list_get(bloques_buscar, i);
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

	if (access(pathAbsoluto, R_OK) == -1){
		printf("NO EXISTE TABLA\n");
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
		 int i = 0, cont = 0;
		 char temp_helper[4];
		 char bloque[8] = "";
		 char size[8] = "";
		 int flag_bloque = 0;

		 while(fread(&temp_helper[0], 1, 1, f) == 1){
			 if(temp_helper[0] >= 48 && temp_helper[0] <= 57){
				 flag_bloque = 1;
				 bloque[i] = temp_helper[0];
				 i++;
			 }else{
				 if(flag_bloque == 1){
					 strcpy(&bloque[i], "\0");
					 flag_bloque = 0;
					 i = 0;
					 cont++;
					 if(cont == 1){
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
	string_append(&root,puntoDeMontaje);
	string_append(&root,"Tablas/");
	string_append(&root,tabla);

	if(type_flag == 1){ //Buscar temporales
		 DIR *dir;
		 dir = opendir(root);
		 struct dirent *entrada;
		 char **entrada_aux;
		 while ((entrada = readdir(dir)) != NULL){
			 entrada_aux = string_split(entrada->d_name, ".");
			 if(&(&entrada_aux)[1] != NULL){
				 if (strcmp(entrada_aux[1],"tmp")==0){
					 char* rootAux = string_duplicate(root);
					 string_append(&rootAux,"/");
					 string_append(&rootAux, entrada->d_name);
					 cargar_bloques(rootAux, bloques_buscar);
					 free(rootAux);
				 }
			 }
		 }
		closedir(dir);
	 }else{
		 string_append(&root, "/part");
		 string_append(&root, string_itoa(particion_buscar));
	 	 string_append(&root,".bin");
	 	cargar_bloques(root, bloques_buscar);
	 }
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
		printf("No se encontro la key %d de la tabla %s en la memtable.\n", key, tabla);
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

