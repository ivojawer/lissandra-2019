#include "funcionesLFS.h"

extern char* puntoDeMontaje;

t_list *lista_describe;

int tipo_describe(char *comando) {
	char **tokens_comando = string_split(comando, " ");
	if (tokens_comando[1] == NULL)
		return 0;
	return 1;
}

struct describe *crear_descripcion(char *name, char *consistencia,
		int particiones, int tiempo_compactacion) {

	struct describe *new = malloc(sizeof(struct describe));
	new->compaction_time = tiempo_compactacion;
	new->consistency = strdup(consistencia);
	new->name = strdup(name);
	new->partitions = particiones;
	return new;
}

void descripcion_destroy(struct describe *self) {
	free(self->consistency);
	free(self->name);
	free(self);
}

void liberar_descripcion(t_list *lista_describe) {
	void liberar_elementos(void *elemento) {
		return (descripcion_destroy((struct describe *) elemento));
	}

	list_iterate(lista_describe, liberar_elementos);
}

void cargar_datos_tabla(char *tabla) {
	struct describe *descripcion = crear_descripcion(tabla, obtener_consistencia_metadata(tabla), obtener_particiones_metadata(tabla),obtener_tiempo_compactacion_metadata(tabla));
	list_add(lista_describe, descripcion);
}

void mostrar_campos_describe(void *element) {
	struct describe * recv = malloc(sizeof(struct describe));
	recv = (struct describe *) element;
	printf(
			"Tabla: %s\nConsistencia: %s\nParticiones: %d\nCompaction Time: %d\n\n",
			recv->name, recv->consistency, recv->partitions,
			recv->compaction_time);
}

void mostrar_descripciones_metadata(t_list *lista_describe) {
	list_iterate(lista_describe, mostrar_campos_describe);
}

void agregar_tablas_a_describir() {
	struct dirent *sd;
	char* tablas = string_new();
	string_append(&tablas, puntoDeMontaje);
	string_append(&tablas, "Tablas/");
	DIR* dir = opendir(tablas);

	while ((sd = readdir(dir)) != NULL) {
		if ((strcmp((sd->d_name), ".") != 0)
				&& (strcmp((sd->d_name), "..") != 0)) {
			cargar_datos_tabla(sd->d_name);
		}
	}
	closedir(dir);
	free(tablas);
}

void describe_full(char *comando) {
	lista_describe = list_create();
	agregar_tablas_a_describir();
	mostrar_descripciones_metadata(lista_describe);
	liberar_descripcion(lista_describe);

}

void describe_particular(char *comando) {
	char *tabla = get_tabla(comando);
	if (existe_tabla(tabla)) {
		printf("Consistencia: %s\nParticiones: %d\nCompaction time: %d\n",
				obtener_consistencia_metadata(tabla),
				obtener_particiones_metadata(tabla),
				obtener_tiempo_compactacion_metadata(tabla));
	}
}

void rutina_describe(char* comando) {

	printf("Rutina DESCRIBE\n");
	int tipo = tipo_describe(comando);
	switch (tipo) {
	case 0:
		describe_full(comando);
		break;
	case 1:
		describe_particular(comando);
		break;
	default:
		printf("Error en comando DESCRIBE.\n");
	}

}
