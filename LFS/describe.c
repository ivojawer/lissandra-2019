#include "funcionesLFS.h"

extern int socket_memoria;
extern char* puntoDeMontaje;
t_list *lista_metadata_enviar;
t_list *lista_describe;


void liberar_metadata(metadataTablaLFS *self){
	free(self->nombre);
	free(self);
}

int stoint_consistencia(char *consistencia_string)
{
	int consistencia;
	if(strcmp(consistencia_string, "SC") == 0){
		consistencia = 0;
	}else if(strcmp(consistencia_string, "SHC") == 0){
		consistencia = 1;
	}else if(strcmp(consistencia_string, "EC") == 0){
		consistencia = 2;
	}else{
		printf("Consistencia No reconocida\n");
	}
	return consistencia;
}

void liberar_metadata_a_enviar(t_list *lista_enviar)
{
	void liberar_lista_metadata(void *elemento){
		return liberar_metadata((metadataTablaLFS *)elemento);
	}

	list_destroy_and_destroy_elements(lista_enviar, liberar_lista_metadata);
}


int tipo_describe(char *comando) {
	if (strcmp(comando, " ") == 0)
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
	struct describe *descripcion = crear_descripcion(tabla,
			obtener_consistencia_metadata(tabla),
			obtener_particiones_metadata(tabla),
			obtener_tiempo_compactacion_metadata(tabla));
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

void agregar_tablas_a_describir()
{
	struct dirent *sd;
	char* tablas = string_new();
	string_append(&tablas, puntoDeMontaje);
	string_append(&tablas, "Tablas/");
	DIR* dir = opendir(tablas);

	while ((sd = readdir(dir)) != NULL) {
		if ((strcmp((sd->d_name), ".") != 0) &&
			(strcmp((sd->d_name), "..") != 0)){
			cargar_datos_tabla(sd->d_name);
		}
	}
	closedir(dir);
	free(tablas);
}


void transformar_y_agregar(struct describe *metadata, t_list *lista_metadata_enviar)
{
	metadataTablaLFS *struct_metadata = malloc(sizeof(metadataTablaLFS));
	struct_metadata->compactTime = metadata->compaction_time;
	struct_metadata->consistencia = stoint_consistencia(metadata->consistency);
	struct_metadata->nombre = strdup(metadata->name);
	struct_metadata->particiones = metadata->partitions;
	list_add(lista_metadata_enviar, struct_metadata);
}

void convert_to_metadataTablaLFS(t_list *lista_recv)
{
	lista_metadata_enviar = list_create();
	void agregar_a_otra_lista(void *elemento){
		return transformar_y_agregar((struct describe *)elemento, lista_metadata_enviar);
	}

	list_iterate(lista_recv, agregar_a_otra_lista);
}

void describe_full()
{
	lista_describe = list_create();
	agregar_tablas_a_describir();
	mostrar_descripciones_metadata(lista_describe);
	convert_to_metadataTablaLFS(lista_describe);
	enviarMetadatasConHeader(socket_memoria, lista_metadata_enviar, METADATAS);
	liberar_descripcion(lista_describe);
	liberar_metadata_a_enviar(lista_metadata_enviar);
}


void describe_particular(char *comando)
{
	char *tabla = get_tabla(comando);
	if (existe_tabla(tabla)) {
		metadataTablaLFS *struct_metadata = malloc(sizeof(metadataTablaLFS));
		struct_metadata->nombre = strdup(tabla);
		struct_metadata->compactTime = obtener_tiempo_compactacion_metadata(tabla);
		struct_metadata->particiones = obtener_particiones_metadata(tabla);
		char *consistency = strdup(obtener_consistencia_metadata(tabla));
		struct_metadata->consistencia = stoint_consistencia(consistency);
		printf("Nombre: %s\nConsistencia: %s\nParticiones: %d\nCompaction time: %d\n",
				struct_metadata->nombre, consistency, struct_metadata->particiones,
				struct_metadata->compactTime);
		t_list *lista_metadata = list_create();
		list_add(lista_metadata, struct_metadata);
		enviarMetadatasConHeader(socket_memoria, lista_metadata, METADATAS);
		free(consistency);

		liberar_metadata_a_enviar(lista_metadata);

	}else{
		printf("La tabla no se encuentra en el sistema\n");
		enviarIntConHeader(socket_memoria, ERROR, RESPUESTA);
	}
}


void rutina_describe(char* comando) {
	printf("Rutina DESCRIBE\n");
	int tipo = tipo_describe(comando);
	switch (tipo) {
	case 0:
		describe_full();
		break;
	case 1:
		describe_particular(comando);
		break;
	default:
		printf("Error en comando DESCRIBE.\n");
	}
}
