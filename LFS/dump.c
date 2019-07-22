#include "dump.h"

extern t_list* memtable;
extern t_log *dump_logger;

extern int cantidadBloques;
extern int tamanioBloques;

extern int retardo; //en milisegundos
extern int tiempoDump; //en milisegundos

extern char* puntoDeMontaje;

struct bloques_tmp *crear_bloques_tmp(char *tabla) {
	struct bloques_tmp *new = malloc(sizeof(struct bloques_tmp));
	new->tabla = strdup(tabla);
	new->bloques = list_create();
	new->size_total = 0;
	return new;
}

void destroy_nr_bloque(void *elemento) {
	struct bloque *bloque = elemento;
	free(bloque);
}

void bloques_tmp_destroy(void *elemento) {
	struct bloques_tmp *self = (struct bloques_tmp *) elemento;
	free(self->tabla);
	void liberar_nr_bloque(void *elemento) {
		return destroy_nr_bloque(elemento);
	}
	list_clean_and_destroy_elements(self->bloques, liberar_nr_bloque);
	free(self);
}

struct bloque *crear_nr_bloque(int nr_bloque) {
	struct bloque *new = malloc(sizeof(struct bloque));
	new->nr_block = nr_bloque;
	return new;
}

void agregar_bloque_lista_tmp(t_list *bloques, int bloque_dump) {
	struct bloque *nr_bloque = malloc(sizeof(struct bloque));
	nr_bloque = crear_nr_bloque(bloque_dump);
	list_add(bloques, nr_bloque);
}

void grabar_registro(char *root, char *registro_completo, int length_registro,
					 int space_full, int index, int table_change,
					 struct bloques_tmp *bloques_tmp, int flag_close_file)
{
	if (flag_close_file == 1 && fp_dump != NULL) {
		fclose(fp_dump);
		fp_dump = NULL;
	} else {
		espacio_libre = tamanioBloques;

		if (table_change == 0) {
			fp_dump = fopen(root, "r+b");
			size = 0;
			suma_size = 0;
			j = 0;
		}

		if (space_full == 2) {
			space_full = 0;
		}

		if (space_full == 1) { //Completo lo que falta de un registro
			size = 0;
			suma_size = 0;
			while (index < length_registro) {
				fputc(registro_completo[index], fp_dump);
				size = sizeof(registro_completo[index]);
				suma_size += size;
				index++;
				bloques_tmp->size_total += size;
			}
			space_full = 2;
		}

		if (space_full == 0){ //Escribo registro
			j = 0;
			while ((j < length_registro) && (suma_size < espacio_libre)) {
				fputc(registro_completo[j], fp_dump);
				size = sizeof(registro_completo[j]);
				suma_size += size;
				j++;
				bloques_tmp->size_total += size;
			}
			if ((j < length_registro) && ((suma_size == espacio_libre)
			    || (suma_size > espacio_libre))){ //Si se queda sin espacio
				bloque_dump = elegir_bloque_libre(cantidadBloques); //Si devuelve -1, eliminar lo del otro registro y abortar.

				 if(bloque_dump == -1){
					 int i;
					 char empty[1] = "";
					 strcpy(empty, " ");
					 bloques_tmp->size_total -= j; //MENTIRA, pero solo me quedo con el size de datos
					 fp_dump->_offset = suma_size-j; //vuelvo al inicio de la ultima linea
					 fseek(fp_dump, 0l, SEEK_CUR);
					 for(i = 0; i < j; i++){
						 fputc(empty[0], fp_dump);
					 }
					 fclose(fp_dump);
					 err_flag = 1;
					 return;
				 }

				fclose(fp_dump);
				agregar_bloque_lista_tmp(bloques_tmp->bloques, bloque_dump);
				char* otroRoot = string_duplicate(puntoDeMontaje);
				string_append(&otroRoot, "Bloques/bloque");
				string_append(&otroRoot, string_itoa(bloque_dump));
				string_append(&otroRoot, ".bin");
				grabar_registro(otroRoot, registro_completo, length_registro, 1,
						j, 0, bloques_tmp, 0);
			}//si se queda sin espacio
		}
	}// si se mantiene en fp_dump
}

void guardar_registros_en_bloques(t_registro *registro_recv, int table_change,
								 struct bloques_tmp *bloques_tmp_tabla)
{
	/*
	 * table_change = 0 NUEVA TABLA/CAMBIAR TABLA
	 * table_change = 1 NO CAMBIAR TABLA
	 */

	t_registro *registro = registro_recv;

	char* registro_completo = string_new();
	string_append(&registro_completo, string_itoa(registro->timestamp));
	string_append(&registro_completo, ";");
	string_append(&registro_completo, string_itoa(registro->key));
	string_append(&registro_completo, ";");
	string_append(&registro_completo, registro->value);
	string_append(&registro_completo, "\n");

	if (table_change == 0) {
		memset(&root[0], 0x0, sizeof(root));
		bloque_dump = elegir_bloque_libre(cantidadBloques);
		if(bloque_dump != -1){
			agregar_bloque_lista_tmp(bloques_tmp_tabla->bloques, bloque_dump); //Para crear el archivo temporal
			sprintf(root, "%sBloques/bloque%d.bin", puntoDeMontaje, bloque_dump);
		}else{
			err_flag = 1;
			return;
		}
	}
	grabar_registro(root, registro_completo, strlen(registro_completo), 0,
					0, table_change, bloques_tmp_tabla, 0);
}


int obtener_size(char *size_array) //Se puede usar en COMPACTACION. Recibe la primer linea leída
{
	char **token = string_split(size_array, "=");
	int size = 0;
	if (token[1] == NULL)
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

void agregar_bloque_particion(void *elemento) {
	struct bloques_tmp *blocks_info = (struct bloques_tmp *) elemento;
	int next_tmp;

	int cant_bloques = list_size(blocks_info->bloques);

	char* root = string_duplicate(puntoDeMontaje);
	string_append(&root, "Tablas/");
	string_append(&root, blocks_info->tabla);
	next_tmp = contar_archivos_con_extension(root,"tmp");// + 1;

	free(root);

	root = string_duplicate(puntoDeMontaje);

	string_append(&root, "Tablas/");
	string_append(&root, blocks_info->tabla);
	string_append(&root, "/");
	string_append(&root, blocks_info->tabla);
	string_append(&root, string_itoa(next_tmp));
	string_append(&root, ".tmp");

	FILE *fp;
	fp = fopen(root, "a");

	fprintf(fp, "SIZE=%d\n", blocks_info->size_total);
	fprintf(fp, "BLOCKS=[");
	for (int i = 0; i < cant_bloques; i++) {
		struct bloque *bloque = malloc(sizeof(struct bloque));
		bloque = list_get(blocks_info->bloques, i);
		fprintf(fp, "%d", bloque->nr_block);
		if (i <= cant_bloques - 2)
			fputs(",", fp);
	}
	fputs("]", fp);
	fclose(fp);
	free(root);
}

void liberar_elementos_particiones(void *elemento) {
	t_particion *particion = elemento;
	void liberar_elementos_particion(void *elemento){
		if(elemento != NULL)
		return registro_destroy((t_registro *) elemento);
	}
	list_iterate(particion->lista_registros, liberar_elementos_particion);
}

void liberar_tabla(void *elemento) {
	t_tabla *tabla = elemento;
	if(tabla != NULL){
	void liberar_elementos_tabla(void *elemento) {
		if(elemento != NULL)
			return liberar_elementos_particiones(elemento);
	}

	if(list_size(tabla->lista_particiones) > 0){
		list_iterate(tabla->lista_particiones, liberar_elementos_tabla);

		void liberar_particiones(void *elemento) {
			if(elemento != NULL)
				return particion_destroy((t_particion *) elemento);
		}
		list_iterate(tabla->lista_particiones, liberar_particiones);
	}
	}
}

void liberar_memtable() {
	void liberar_elementos(void *elemento) {
		return liberar_tabla(elemento);
	}
	list_iterate(memtable_aux, liberar_elementos);
}

void guardar_bloques_metadata(t_list *lista_bloques_tmp) {
	void guardar_bloque_temp(void *elemento) {
		return agregar_bloque_particion(elemento);
	}
	list_iterate(lista_bloques_tmp, guardar_bloque_temp);
}

void liberar_lista_bloques(t_list *lista_bloques_tmp) {
	void liberar_bloques_tmp(void *elemento) {
		return bloques_tmp_destroy(elemento);
	}
	list_clean_and_destroy_elements(lista_bloques_tmp, liberar_bloques_tmp);
}

void dump() {
	pthread_mutex_lock(&dump_semaphore); //influye en todas las tablas
	err_flag = 0;
	if(!lista_vacia(memtable)){
		int i, j, k;
		int table_change;
		int siga_siga = 0;
		lista_bloques_tmp = list_create();
		for (i = 0; i < list_size(memtable); i++){
			space_full = 0;
			fp_dump = NULL;
			table_change = 0;
			t_tabla *tabla = malloc(sizeof(t_tabla));
			tabla = list_get(memtable, i);
			if(existe_tabla(tabla->name_tabla)){
				struct bloques_tmp *bloques_tmp_tabla = malloc(sizeof(struct bloques_tmp));
				bloques_tmp_tabla = crear_bloques_tmp(tabla->name_tabla); //Bloques donde se guardan registros
				int length_tabla = list_size(tabla->lista_particiones);
				for(j = 0; j < length_tabla; j++){
					t_particion *particion = malloc(sizeof(t_particion));
					particion = list_get(tabla->lista_particiones, j);
					int length_particion = list_size(particion->lista_registros);
					for (k = 0; k < length_particion; k++) {
						t_registro *registro = malloc(sizeof(t_registro));
						registro = list_get(particion->lista_registros, k);
//						if(strlen(registro->value) > 0){
						if(registro->value != NULL && strlen(registro->value)>0){
							if (table_change == 0 && k == 1 && fp_dump != NULL)
								table_change = 1;
							guardar_registros_en_bloques(registro, table_change,
													 bloques_tmp_tabla);//Control de error aca
							if(err_flag == 1){
								full_space++;
								log_info(dump_logger, "Dump finalizado. No se pudo guardar un registro por falta de espacio.");
								if(full_space == 1)
									printf("Dump finalizado. No se pudo guardar un registro por falta de espacio\n");
								pthread_mutex_unlock(&dump_semaphore);
								return;
							}
							full_space = 0;
							siga_siga = 1;
						}else{
//							free(bloques_tmp_tabla); //Se deberian borrar pero tira Double Free. NO time
//							free(lista_bloques_tmp);
							siga_siga = 0;
						}
//						void liberar_elementos_particion(void *elemento) {
//							return registro_destroy((t_registro *) elemento);//cambiaste eso
//						}
//						list_remove_and_destroy_element(particion->lista_registros, k, liberar_elementos_particion);

					}
					void liberar_elementos_particion(void *elemento) {
						return liberar_registro_dump((t_registro *) elemento);
					}
					list_iterate(particion->lista_registros, liberar_elementos_particion);
				}
				grabar_registro("NULL", "NULL", 0, 0, 0, 0, bloques_tmp_tabla, 1); //Close fp_dump
				if(siga_siga == 1){
					list_add(lista_bloques_tmp, bloques_tmp_tabla);
				}
			}
		}
		if(siga_siga == 1){
			guardar_bloques_metadata(lista_bloques_tmp);
			liberar_lista_bloques(lista_bloques_tmp);
		}
	}
	pthread_mutex_unlock(&dump_semaphore);
}

void ejecutar_dump()
{
//	struct timespec tim, tim_2;
//	tim.tv_sec = tiempoDump*0.001;
//	tim.tv_nsec = 0;

	int sleepDump;

	while(1) {
//		nanosleep(&tim, &tim_2);
//		sem_wait(&refresh_config); //Con los semaforos no lo reconoce
		sleepDump = tiempoDump/1000;
		sleep(sleepDump);
		dump();
//		sem_post(&refresh_config);
	}
}
