#include "dump.h"

extern t_list* memtable;
extern t_log *dump_logger;
extern t_log *logger;

extern sem_t sem_memtable;

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
		struct bloques_tmp *bloques_tmp, int flag_close_file) {

	if (flag_close_file == 1 && fp_dump != NULL) {
		fclose(fp_dump);
		fp_dump = NULL;
	} else {
		espacio_libre = tamanioBloques;

		if (table_change == 0) {
			fp_dump = fopen(root, "ab");
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

		if (space_full == 0) { //Escribo registro
			j = 0;
			while ((j < length_registro) && (suma_size < espacio_libre)) {
				fputc(registro_completo[j], fp_dump);
				size = sizeof(registro_completo[j]);
				suma_size += size;
				j++;
				bloques_tmp->size_total += size;
			}
			if ((j < length_registro)
					&& ((suma_size == espacio_libre)
							|| (suma_size > espacio_libre))) { //Si se queda sin espacio
				bloque_dump = elegir_bloque_libre(cantidadBloques); //Si devuelve -1, eliminar lo del otro registro y abortar.

				if (bloque_dump == -1) {
					int i;
					char empty[1] = "";
					strcpy(empty, " ");
					bloques_tmp->size_total -= j; //MENTIRA, pero solo me quedo con el size de datos
					fp_dump->_offset = suma_size - j; //vuelvo al inicio de la ultima linea
					fseek(fp_dump, 0l, SEEK_CUR);
					for (i = 0; i < j; i++) {
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
			}
		}
	}
}

void guardar_registros_en_bloques(t_registro *registro_recv, int table_change,
		struct bloques_tmp *bloques_tmp_tabla) {
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
		if (bloque_dump != -1) {
			agregar_bloque_lista_tmp(bloques_tmp_tabla->bloques, bloque_dump); //Para crear el archivo temporal
			sprintf(root, "%sBloques/bloque%d.bin", puntoDeMontaje,
					bloque_dump);
		} else {
			err_flag = 1;
			return;
		}
	}
	grabar_registro(root, registro_completo, strlen(registro_completo), 0, 0,
			table_change, bloques_tmp_tabla, 0);
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
	next_tmp = contar_archivos_con_extension(root, "tmp"); // + 1;

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
	void liberar_elementos_particion(void *elemento) {
		if (elemento != NULL)
			return registro_destroy((t_registro *) elemento);
	}
	list_iterate(particion->lista_registros, liberar_elementos_particion);
}

void liberar_tabla(void *elemento) {
	t_tabla *tabla = elemento;
	if (tabla != NULL) {
		void liberar_elementos_tabla(void *elemento) {
			if (elemento != NULL)
				return liberar_elementos_particiones(elemento);
		}

		if (list_size(tabla->lista_particiones) > 0) {
			list_iterate(tabla->lista_particiones, liberar_elementos_tabla);

			void liberar_particiones(void *elemento) {
				if (elemento != NULL)
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

void dump2() {
	pthread_mutex_lock(&dump_semaphore); //influye en todas las tablas
	err_flag = 0;

	if (!lista_vacia(memtable)) {
		int i, j, k;
		int table_change;
		int siga_siga = 0;
		lista_bloques_tmp = list_create();
		log_info(dump_logger, "Se va a hacer el dump");
		sem_wait(&sem_memtable);
		for (i = 0; i < list_size(memtable); i++) {
			space_full = 0;
			fp_dump = NULL;
			table_change = 0;
			t_tabla *tabla = malloc(sizeof(t_tabla));
			tabla = list_get(memtable, i);
			if (existe_tabla(tabla->name_tabla)) {
				struct bloques_tmp *bloques_tmp_tabla = malloc(
						sizeof(struct bloques_tmp));
				bloques_tmp_tabla = crear_bloques_tmp(tabla->name_tabla); //Bloques donde se guardan registros
				for (j = 0; j < list_size(tabla->lista_particiones); j++) {
					t_particion *particion = malloc(sizeof(t_particion));
					particion = list_get(tabla->lista_particiones, j);
					for (k = 0; k < list_size(particion->lista_registros);
							k++) {
						t_registro *registro = malloc(sizeof(t_registro));
						registro = list_get(particion->lista_registros, k);
						if (strlen(registro->value) > 0) {
							if (table_change == 0 && k == 1)
								table_change = 1;
							guardar_registros_en_bloques(registro, table_change,
									bloques_tmp_tabla); //Control de error aca
							grabar_registro("NULL", "NULL", 0, 0, 0, 0,
									bloques_tmp_tabla, 1); //Close fp_dump
							if (err_flag == 1) {
								full_space++;
								log_info(dump_logger,
										"Dump finalizado. No se pudo guardar un registro por falta de espacio.");
								if (full_space == 1)
									printf(
											"Dump finalizado. No se pudo guardar un registro por falta de espacio\n");
								pthread_mutex_unlock(&dump_semaphore);
								sem_post(&sem_memtable);
								return;
							}
							full_space = 0;
							siga_siga = 1;
						} else {
							free(bloques_tmp_tabla);
							free(lista_bloques_tmp);
							siga_siga = 0;
						}
						void liberar_elementos_particion(void *elemento) {
							return registro_destroy((t_registro *) elemento);
						}
						list_remove_and_destroy_element(
								particion->lista_registros, k,
								liberar_elementos_particion);
					}
				}
//				grabar_registro("NULL", "NULL", 0, 0, 0, 0, bloques_tmp_tabla,
//						1); //Close fp_dump
				if (siga_siga == 1) {
					list_add(lista_bloques_tmp, bloques_tmp_tabla);
				}
			}
		}
		if (siga_siga == 1) {
			guardar_bloques_metadata(lista_bloques_tmp);
			liberar_lista_bloques(lista_bloques_tmp);
		}
		sem_post(&sem_memtable);
		pthread_mutex_unlock(&dump_semaphore);
	} else {
		pthread_mutex_unlock(&dump_semaphore);
	}
}

//---------

char* direccionDeUnaTabla(char* nombreTabla) {

	char *rootTabla = string_new();
	string_append(&rootTabla, puntoDeMontaje);
	string_append(&rootTabla, "Tablas/");
	string_append(&rootTabla, nombreTabla);

	return rootTabla;
}

char* direccionDeUnBloque(int numeroBloque) {

	char *rootBloque = string_new();
	string_append(&rootBloque, puntoDeMontaje);
	string_append(&rootBloque, "Bloques/bloque");
	string_append(&rootBloque, string_itoa(numeroBloque));
	string_append(&rootBloque, ".bin");
	return rootBloque;

}

void crearLosPunterosYAsociarBloquesTemporalesVacios(t_tabla* tablaADumpear) {

	char *rootTabla = direccionDeUnaTabla(tablaADumpear->name_tabla);
	string_append(&rootTabla, "/");

	char* size = string_new();
	string_append(&size, "SIZE=");
	string_append(&size, "0");
	string_append(&size, "\n");

	for (int i = 0; i < list_size(tablaADumpear->lista_particiones); i++) {

		char* root = string_new();

		string_append(&root, rootTabla);
		string_append(&root, tablaADumpear->name_tabla);
		string_append(&root, "0");
		string_append(&root, ".tmp");

		int free_block = elegir_bloque_libre(cantidadBloques);
		if (free_block == -1) {
			printf("No hay bloques libres\n");
		} else {
			FILE *fp;
			fp = fopen(root, "w+b");
			fputs(size, fp);
			fprintf(fp, "%s", "BLOCKS=[");

			fputs(string_itoa(free_block), fp);
			fprintf(fp, "%s", "]");

			fclose(fp);
		}
		free(root);
	}
	free(size);
	free(rootTabla);
}

t_list* bloquesAsociadosAParticionDeTabla(char* nombreTabla,
		int numeroParticion, char* terminacion) { //A terminacion se refiere a ".bin" ".tmp" ".tmpc"
	char* rootTabla = direccionDeUnaTabla(nombreTabla);
	string_append(&rootTabla, "/");
	string_append(&rootTabla, nombreTabla);
	string_append(&rootTabla, string_itoa(numeroParticion));
	string_append(&rootTabla, terminacion);
	t_config* informacionDeLaParticion = config_create(rootTabla);
	char** bloquesAsociadosEnString = config_get_array_value(
			informacionDeLaParticion, "BLOCKS");

	t_list* bloquesAsociados = list_create();

	for (int i = 0; bloquesAsociadosEnString[i] != NULL; i++) {
		int numeroBloque = atoi(bloquesAsociadosEnString[i]);
		list_add(bloquesAsociados, &numeroBloque);
	}

	config_destroy(informacionDeLaParticion); //Supongo que el config_destroy destruye todas las referencias

	return bloquesAsociados;

}

int espacioOcupadoEnBloque(int numeroBloque) {
	char* direccionBloque = direccionDeUnBloque(numeroBloque);
	FILE* archivoBloque = fopen(direccionBloque, "r");
	if (archivoBloque == NULL) {
		return -1;
	}

	fseek(archivoBloque, 0L, SEEK_END);

	int espacioOcupado = ftell(archivoBloque);

	fclose(archivoBloque);

	free(direccionBloque);

	return espacioOcupado;
}

//char* registroAString(registro* unRegistro) {
//	char* registroEnString = string_new();
//	string_append(&registroEnString, string_itoa(unRegistro->timestamp));
//	string_append(&registroEnString, ";");
//	string_append(&registroEnString, string_itoa(unRegistro->key));
//	string_append(&registroEnString, ";");
//	string_append(&registroEnString, unRegistro->value);
//
//	return registroEnString;
//
//}

char* registroAString(t_registro* registro) {

	char* reg = string_new();

	char* unaKey = string_itoa(registro->key); //no se si funciona con uint16
	char* unTimestamp = string_itoa(registro->timestamp);
	char* unValue = string_duplicate(registro->value); //es necesario? ah

	string_append(&reg, unTimestamp);
	string_append(&reg, ";");
	string_append(&reg, unaKey);
	string_append(&reg, ";");
	string_append(&reg, unValue);
	string_append(&reg, "\n");
	return reg;
}

char* tratarDeEscribirRegistroEnBloque(char* registroEnString, int numeroBloque) { //Se devuelve lo que falta escribir

	int espacioDisponible = tamanioBloques
			- espacioOcupadoEnBloque(numeroBloque);

	if (tamanioBloques <= 0) {
		return registroEnString;
	}

	char* direccionBloque = direccionDeUnBloque(numeroBloque);

	FILE* archivoBloque = fopen(direccionBloque, "a");

	int caracteresInsertados = 0;

	for (; espacioDisponible > 0; espacioDisponible--) {
		fputc(registroEnString[caracteresInsertados], archivoBloque);

		if (registroEnString[caracteresInsertados + 1] == '\0') //Si el ultimo caracter escrito es el "ultimo escribible", el proximo es '\0'
				{
			fclose(archivoBloque);
			return NULL;
		}
		caracteresInsertados++;
	}
	fclose(archivoBloque);

	char* registroAcotado = registroEnString + caracteresInsertados;

	return registroAcotado;

}

int asociarBloqueLibreAParticionDeTabla(char* nombreTabla, int numeroParticion,
		char* terminacion) {

	int free_block = elegir_bloque_libre(1);

	if (free_block == -1) {
		return -1;
	}

	char* rootTabla = direccionDeUnaTabla(nombreTabla);

	string_append(&rootTabla, "/");
	string_append(&rootTabla, nombreTabla);
	string_append(&rootTabla, string_itoa(numeroParticion));
	string_append(&rootTabla, terminacion);

	t_config* informacionDeLaParticion = config_create(rootTabla);
	char* size = string_duplicate(
			config_get_string_value(informacionDeLaParticion, "SIZE"));
	char* blocks = string_duplicate(
			config_get_string_value(informacionDeLaParticion, "BLOCKS"));

	char* blocksInvertido = string_reverse(blocks);
	char* blocksInvertidoSinCorchete = string_substring_from(blocksInvertido,
			1);

	blocks = string_reverse(blocksInvertidoSinCorchete);
	string_append(&blocks, ",");
	string_append(&blocks, string_itoa(free_block));
	string_append(&blocks, "]");

	free(blocksInvertidoSinCorchete);
	free(blocksInvertido);

	config_destroy(informacionDeLaParticion);

	char* stringSize = string_new();
	string_append(&stringSize, "SIZE=");
	string_append(&stringSize, size);

	char* stringBlocks = string_new();
	string_append(&stringBlocks, "BLOCKS=");
	string_append(&stringBlocks, blocks);

	FILE* archivoParticion = fopen(rootTabla, "w");
	fputs(stringSize, archivoParticion);
	fputs(stringBlocks, archivoParticion);
	fclose(archivoParticion);

	return free_block;

}

int hayEspacioSuficienteParaRegistro(t_registro* registroQueSeQuiereMeter,
		char* nombreTabla, int numeroParticion, char* terminacion) {

	char* rootTabla = direccionDeUnaTabla(nombreTabla);

	int tamanioRegistro = strlen(registroAString(registroQueSeQuiereMeter)); //NO SE CONSIDERA EL \0 NI EL \n . POR QUE ? NO HAY TIMEPO  PARA PENSAR

	int espacioDisponibleEnBloquesAsociados = 0;

	string_append(&rootTabla, "/");
	string_append(&rootTabla, nombreTabla);
	string_append(&rootTabla, string_itoa(numeroParticion));
	string_append(&rootTabla, terminacion);

	t_list* bloquesDeLaTabla = bloquesAsociadosAParticionDeTabla(nombreTabla,
			numeroParticion, terminacion);

	if (bloquesDeLaTabla != NULL) {
		for (int i = 0; i < list_size(bloquesDeLaTabla); i++) {
			int* unBloque = list_remove(bloquesDeLaTabla, 0);
			int espacioDisponibleEnBloque = tamanioBloques
					- espacioOcupadoEnBloque(*unBloque);
			espacioDisponibleEnBloquesAsociados += espacioDisponibleEnBloque;
		}
	}

	if (espacioDisponibleEnBloquesAsociados >= tamanioRegistro) {
		return 1;
	}

	int bloquesNecesarios = (tamanioRegistro
			- espacioDisponibleEnBloquesAsociados) / tamanioBloques;

	if (tamanioRegistro % tamanioBloques != 0) //Se necesita un poco mas que el cociente de la division
			{
		bloquesNecesarios++;
	}

	return controlar_bloques_disponibles(bloquesNecesarios);

}

void actualizarSizeDeParticionTemporal(char* nombreTabla, int numeroParticion,
		char* terminacion) {
	char* rootTabla = direccionDeUnaTabla(nombreTabla);

	string_append(&rootTabla, "/");
	string_append(&rootTabla, nombreTabla);
	string_append(&rootTabla, string_itoa(numeroParticion));
	string_append(&rootTabla, terminacion);

	t_config* informacionDeLaParticion = config_create(rootTabla);
	char** blocksArray = config_get_array_value(informacionDeLaParticion,
			"BLOCKS");

	int nuevoSize = 0;

	for (int i = 0; blocksArray[i] != NULL; i++) {
		int unBloque = atoi(blocksArray[i]);
		nuevoSize += espacioOcupadoEnBloque(unBloque);
	}

	char* blocks = string_duplicate(
			config_get_string_value(informacionDeLaParticion, "BLOCKS"));

	config_destroy(informacionDeLaParticion);

	char* stringSize = string_new();
	string_append(&stringSize, "SIZE=");
	string_append(&stringSize, string_itoa(nuevoSize));
	string_append(&stringSize, "\n");

	char* stringBlocks = string_new();
	string_append(&stringBlocks, "BLOCKS=");
	string_append(&stringBlocks, blocks);

	FILE* archivoParticion = fopen(rootTabla, "w");
	fputs(stringSize, archivoParticion);
	fputs(stringBlocks, archivoParticion);
	fclose(archivoParticion);

}

int escribirRegistroEnBloqueTemporal(t_registro* unRegistro, char* nombreTabla,
		int numeroParticion) {

	if (!hayEspacioSuficienteParaRegistro(unRegistro, nombreTabla,
			numeroParticion, ".tmp")) {
		return -1;
	}

	t_list* bloquesYaAsociadosALaParticion = bloquesAsociadosAParticionDeTabla(
			nombreTabla, numeroParticion, ".tmp");

	char* registroAEscribir = registroAString(unRegistro);

	while (list_size(bloquesYaAsociadosALaParticion) != 0) {
		int* unBloque = list_remove(bloquesYaAsociadosALaParticion, 0);

		registroAEscribir = tratarDeEscribirRegistroEnBloque(registroAEscribir,
				*unBloque);

		actualizarSizeDeParticionTemporal(nombreTabla, numeroParticion, ".tmp");

		if (registroAEscribir == NULL) {
			return 1;
		}

	}

	while (1) { //Se verifico que hay tamaño suficiente para meter el registro, eventualmente tiene que terminar de meterlo

		int nuevoBloque = asociarBloqueLibreAParticionDeTabla(nombreTabla,
				numeroParticion, ".tmp");

		registroAEscribir = tratarDeEscribirRegistroEnBloque(registroAEscribir,
				nuevoBloque);



		if (registroAEscribir == NULL) {
			return 1;
		}

	}

	actualizarSizeDeParticionTemporal(nombreTabla, numeroParticion, ".tmp");

}

void escribirRegistrosDeMemtableEnBloquesTemporales(t_list* registrosAEscribir,
		char* nombreTabla, int numeroParticion) {

	while (list_size(registrosAEscribir) != 0) {
		t_registro* unRegistro = list_remove(registrosAEscribir, 0);
		escribirRegistroEnBloqueTemporal(unRegistro, nombreTabla,
				numeroParticion);
	}

}

void dump() {
	int tablaEstaEnElFileSystem(t_tabla* unaTabla) {
		return existe_tabla(unaTabla->name_tabla);
	}

	pthread_mutex_lock(&dump_semaphore);
	sem_wait(&sem_memtable);

	log_info(logger, "Se va a hacer el dump");

	if (memtable == NULL || list_size(memtable) == 0) {
		sem_post(&sem_memtable);
		pthread_mutex_unlock(&dump_semaphore);
		log_info(logger, "Se termino el dump");
		return;
	}

	while (list_size(memtable) != 0) {

		t_tabla* tablaADumpear = list_remove(memtable, 0);

		if (tablaEstaEnElFileSystem(tablaADumpear)) {
			crearLosPunterosYAsociarBloquesTemporalesVacios(tablaADumpear);

			if (tablaADumpear->lista_particiones != NULL) {
				for (int i = 0; i < list_size(tablaADumpear->lista_particiones);
						i++) {
					t_particion* unaParticion = list_remove(
							tablaADumpear->lista_particiones, 0);

					escribirRegistrosDeMemtableEnBloquesTemporales(
							unaParticion->lista_registros,
							tablaADumpear->name_tabla, 0); //El 0 es porque resulta que tiene que ser en un solo archivo

				}
			}

		}
	}
	log_info(logger, "Se termino el dump");
	sem_post(&sem_memtable);
	pthread_mutex_unlock(&dump_semaphore);

}

void ejecutar_dump() {
//	struct timespec tim, tim_2;
//	tim.tv_sec = tiempoDump*0.001;
//	tim.tv_nsec = 0;

	int sleepDump;

	while (1) {
//		nanosleep(&tim, &tim_2);
//		sem_wait(&refresh_config); //Con los semaforos no lo reconoce
		sleepDump = tiempoDump / 1000;
		sleep(sleepDump);

		dump();
//		sem_post(&refresh_config);
	}
}
