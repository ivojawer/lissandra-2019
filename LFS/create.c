#include "funcionesLFS.h"

extern t_log *logger;
extern int cantidadBloques;
extern char* puntoDeMontaje;
extern void iniciar_compactacion(void *arg);

void agregar_salto_de_linea(char *string) {
	string_append(&string, "\n");
}

void guardar_bitarray(int index) {

	char* root = string_duplicate(puntoDeMontaje);

	string_append(&root, "Metadata/Bitmap.bin");

	FILE *fp = fopen(root, "r+b");
	fp->_offset = index;
	fseek(fp, 0l, SEEK_CUR);
	fprintf(fp, "%i", bitarray_test_bit(bitarray, index));
	fclose(fp);
	free(root);
}

int elegir_bloque_libre(int nr_bloques) {
	int free_block, i, flag_free_block = 0;

	sem_wait(&bloques_bitmap);
	for (i = 0; i < nr_bloques; i++) {
		if (flag_free_block == 0) {
			if (bitarray_test_bit(bitarray, i) == 0) {
				flag_free_block = 1;
				free_block = i;
				bitarray_set_bit(bitarray, i);
				guardar_bitarray(i);
				sem_post(&bloques_bitmap);
				return free_block;
			}
		}
	}
	sem_post(&bloques_bitmap);
	return -1;
}

void crear_particiones(char *dir, int particiones) {
	char *root_aux = string_new();
	string_append(&root_aux, dir);
	string_append(&root_aux, "/part");

	char* size_text = string_new();
	string_append(&size_text, "SIZE=");
	string_append(&size_text, "0");
	string_append(&size_text, "\n");

	for (int i = 0; i < particiones; i++) {

		char* root = string_new();

		string_append(&root, root_aux);
		string_append(&root, string_itoa(i));
		string_append(&root, ".bin");

		FILE *fp;
		fp = fopen(root, "w+b");
		int free_block = elegir_bloque_libre(cantidadBloques);
		if (free_block == -1) {
			printf("No hay bloques libres\n");
			fclose(fp);
		} else {
			fputs(size_text, fp);
			fprintf(fp, "%s", "BLOCKS=[");

			fputs(string_itoa(free_block), fp);
			fprintf(fp, "%s", "]");

			fclose(fp);
		}
		free(root);
	}
	free(size_text);
	free(root_aux);
}

int crear_tabla_FS(char *tabla, int particiones, char *consistencia,
		int compact_time) {
	char *tabla_dir = string_new();
	string_append(&tabla_dir, puntoDeMontaje);
	string_append(&tabla_dir, "Tablas/");
	string_append(&tabla_dir, tabla);

	char* dir_metadata = string_new();
	string_append(&dir_metadata, tabla_dir);
	string_append(&dir_metadata, "/metadata.config");

	char* consistencia_tabla = string_new();
	string_append(&consistencia_tabla, "CONSISTENCY=");
	string_append(&consistencia_tabla, consistencia);
	agregar_salto_de_linea(consistencia_tabla);

	char* particiones_tabla = string_new();
	string_append(&particiones_tabla, "PARTITIONS=");
	string_append(&particiones_tabla, string_itoa(particiones));
	agregar_salto_de_linea(particiones_tabla);

	char* compactacion_tabla = string_new();
	string_append(&compactacion_tabla, "COMPACTION_TIME=");
	string_append(&compactacion_tabla, string_itoa(compact_time));

	int flag_creacion = mkdir(tabla_dir, 0777);

	crear_particiones(tabla_dir, particiones);

	agregar_metadata_tabla(tabla, consistencia, particiones, compact_time);

	FILE *fp;
	fp = fopen(dir_metadata, "w+");
	if (fp == NULL) {
		printf("Fallo Creacion archivo metadata\n");
	} else {
		fputs(consistencia_tabla, fp);
		fputs(particiones_tabla, fp);
		fputs(compactacion_tabla, fp);
		fclose(fp);
	}
	free(tabla_dir);
	free(consistencia_tabla);
	free(dir_metadata);
	free(compactacion_tabla);
	free(particiones_tabla);

//	if (flag_creacion == 0)
//		crear_control_op(tabla);

	return flag_creacion;
}

void crear_hilo_compactacion(char *tabla, int tiempo_compactacion) {
	struct param_compactacion *p_comp = malloc(
			sizeof(struct param_compactacion));
	p_comp->tabla = strdup(tabla);
	p_comp->tiempo_compact = tiempo_compactacion;
	pthread_create(&h_compactacion, NULL, (void *) iniciar_compactacion,
			(void *) p_comp); //pasar parametros
}

void rutina_create(void* parametros) {
	struct parametros *info = (struct parametros*) parametros;
	char *comando = strdup(info->comando);
	int socket_cliente = info->socket_cliente;

	printf("Operacion: CREATE\n");

	char *tabla = strdup(get_tabla(comando));
	printf("Tabla: %s\n", tabla);

	char *consistencia = get_consistencia(comando);
	printf("Consistencia: %s\n", consistencia);

	int particiones = get_particiones(comando);
	printf("Particiones: %d\n", particiones);

	int compactacion = get_tiempo_compactacion(comando);
	printf("Tiempo de compactacion: %d\n", compactacion);

	if (controlar_bloques_disponibles(particiones) == 0) {
		if (socket_cliente != -1) {
			enviarIntConHeader(socket_cliente, ERROR, RESPUESTA);
		}

		printf("Se excede la cantidad de particiones disponibles\n");
		return;
	}

	if (existe_tabla(tabla)) {
		log_info(logger, "Se intento crear la tabla ya existente: %s.\n",
				tabla);
		if (socket_cliente != -1) {
			enviarIntConHeader(socket_cliente, ERROR, RESPUESTA);
		}
	} else if (crear_tabla_FS(tabla, particiones, consistencia, compactacion)
			== 0) {
		printf("Se creo la tabla [%s].\n", tabla);

		if (socket_cliente != -1) {
			enviarIntConHeader(socket_cliente, TODO_BIEN, RESPUESTA);
		}

		crear_hilo_compactacion(tabla, compactacion);

	} else {
		printf("Ocurrio un problema\nNo se pudo crear la Tabla\n");

		if (socket_cliente != -1) {
			enviarIntConHeader(socket_cliente, ERROR, RESPUESTA);
		}

	}
}
