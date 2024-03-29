#include "funcionesLFS.h"

//extern int socket_memoria;
extern void liberar_tabla(void *elemento);
extern bool comparar_nombre(char *tabla, void *tabla_mt);
extern char* puntoDeMontaje;
extern t_list *memtable;
extern t_log* logger;
extern bool coincide_tabla(void *elemento, char *tabla);

void iterar_busqueda_de_bloques(void (foo)(char *, int, int, t_list *),
		char *name, int part, int flag, t_list *lista, int cant) {
	int i;
	for (i = 0; i < cant; i++) {
		foo(name, i, flag, lista);
	}
}

void eliminar_contenido_de_tabla(char *tabla) {
	DIR *dir;
	struct dirent *sd;
	char* root = string_new();
	string_append(&root, puntoDeMontaje);
	string_append(&root, "Tablas/");
	string_append(&root, tabla);
	dir = opendir(root);

	string_append(&root, "/");
	while ((sd = readdir(dir)) != NULL) {
		if ((strcmp((sd->d_name), ".") != 0)
				&& (strcmp((sd->d_name), "..") != 0)) {

			char* aux = string_new();
			string_append(&aux, root);
			string_append(&aux, sd->d_name);
			remove(aux);
			free(aux);
		}
	}
	free(root);
	closedir(dir);
}

void eliminar_tabla_fisicamente(char *tabla) {
	char *root = string_new();
	string_append(&root, puntoDeMontaje);
	string_append(&root, "Tablas/");
	string_append(&root, tabla);
	rmdir(root);
	free(root);
}

void liberar_bloques(t_list *bloques_buscar) {
	void desmarcar_bloque(void *elemento) {
		return (desmarcar_bloque_bitmap((t_bloque *) elemento));
	}
	list_iterate(bloques_buscar, desmarcar_bloque);
}

void eliminar_tabla(char *tabla) {
	t_list *bloques_buscar = list_create();
	int particiones = obtener_particiones_metadata(tabla);
	iterar_busqueda_de_bloques(buscar_bloques_particion, tabla, 1, 0,
			bloques_buscar, particiones); //part.bin
	buscar_bloques_particion(tabla, 1, 1, bloques_buscar); //.tmp
	eliminar_contenido_de_tabla(tabla);
	eliminar_tabla_fisicamente(tabla);
	liberar_bloques(bloques_buscar);
}

void exit_tabla_compact(char *tabla) {
	bool coincide_valor(void *elemento) {
		return coincide_tabla(elemento, tabla);
	}

//	struct flag_y_tabla *tabla_buscada = malloc(sizeof(struct flag_y_tabla)); -malloc sacado
	struct flag_y_tabla *tabla_buscada = list_find(lista_tabla_compact, coincide_valor);
	tabla_buscada->exit_flag = 1;
	list_remove_by_condition(lista_tabla_compact, coincide_valor);
	list_add(lista_tabla_compact, tabla_buscada);
}

void rutina_drop(void* parametros) {

	struct parametros *info = (struct parametros*) parametros;
	char *comando = strdup(info->comando);
	int socket_cliente = info->socket_cliente;

	free(info);

	char *tabla = get_tabla(comando);

	free(comando);

	char* dropEnString = string_new();
	string_append(&dropEnString,"DROP ");
	string_append(&dropEnString,tabla);

	char* textoALoggear = string_new();
	string_append(&textoALoggear,"INICIA ");
	string_append(&textoALoggear,dropEnString);
	loggearCyanClaro(logger,textoALoggear);
	free(textoALoggear);

//	modificar_op_control(tabla, 3); //para no cruzarse con niguno
	sem_wait(&dump_semaphore);
	sem_wait(&compactar_semaphore);
	if (existe_tabla(tabla)) {
		eliminar_tabla(tabla);
		exit_tabla_compact(tabla);

		t_list *tabla_encontrada = list_create();
		tabla_encontrada = filtrar_tabla_memtable(tabla);
		if (!lista_vacia(tabla_encontrada)) {

			void destruir_tabla(void *elemento) {
				return liberar_tabla(elemento);
			}

			bool coincide_nombre(void *tabla_mt) {
				return comparar_nombre(tabla, tabla_mt);
			}

			list_remove_and_destroy_by_condition(memtable, coincide_nombre,
					destruir_tabla);

			bool coincide_nombre_metadata(void *elemento) {
				return buscar_metadata_tabla((struct describe *)elemento, tabla);
			}

			void destruir_elemento_metadata(void *elemento) {
				return descripcion_destroy((struct describe *)elemento);
			}

			list_remove_and_destroy_by_condition(lista_metadatas, coincide_nombre_metadata,
												 destruir_elemento_metadata);

		}
		sem_post(&dump_semaphore);
		sem_post(&compactar_semaphore);
		if(socket_cliente != -1)
		{
			enviarIntConHeader(socket_cliente, TODO_BIEN, RESPUESTA);
		}
		textoALoggear = string_new();
		string_append(&textoALoggear,dropEnString);
		string_append(&textoALoggear,": Se elimino la tabla");
		loggearVerdeClaro(logger,textoALoggear);
		free(textoALoggear);
	} else {
		sem_post(&dump_semaphore);
		sem_post(&compactar_semaphore);
		log_error(logger,"%s: La tabla no existe");
		if(socket_cliente != -1)
		{
			enviarIntConHeader(socket_cliente, TABLA_NO_EXISTE, RESPUESTA);
		}

	}
	free(dropEnString);
//	modificar_op_control(strdup(tabla), 4);
}
