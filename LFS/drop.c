#include "funcionesLFS.h"

extern char* puntoDeMontaje;

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
	string_append(&root,puntoDeMontaje);
	string_append(&root,"Tablas/");
	string_append(&root, tabla);
	rmdir(root);
	free(root);
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

void rutina_drop(char* comando) {
	printf("Rutina DROP\n");
	char *tabla = get_tabla(comando);
	if (existe_tabla(tabla)) {
		eliminar_tabla(tabla);
	}
}
