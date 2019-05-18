#include "funcionesMemoria.h"


extern t_log* logger;

extern t_list* tablaSegmentos;




//TABLA DE SEGMENTOS

t_list* crearTablaSegmentos(){
	return list_create();
}



segmento* ultimoSegmento(t_list* tablaSegmentos){
	return list_get(tablaSegmentos,tablaSegmentos->elements_count - 1);
}

void nuevaTabla(t_list* tablaSegmentos,char* nombreTabla){ //todo: destroy de esto
	segmento* nuevoSegmento = malloc(sizeof(segmento));
//	nuevoSegmento->nombreDeTabla = malloc(strlen(nombreTabla)*sizeof(char));
//	strcpy(nuevoSegmento->nombreDeTabla,nombreTabla);

	nuevoSegmento->nombreDeTabla = string_duplicate(nombreTabla);

	nuevoSegmento->tablaDePaginas = crearTablaPaginas();


	log_info(logger,"tabla por agregar: %s",nuevoSegmento->nombreDeTabla);

	list_add(tablaSegmentos,nuevoSegmento);

	log_info(logger,"tabla agregada: %s",(ultimoSegmento(tablaSegmentos))->nombreDeTabla);
}





//TABLA DE PAGINAS

t_list* crearTablaPaginas(){
	return list_create();
}

pagina* ultimaPagina(t_list* tablaPaginas){
	pagina* lastPagina = list_get(tablaPaginas,tablaPaginas->elements_count - 1);
	return lastPagina ;
}


marco* asignoPaginaEnMarco(int key, int timestamp,char* value, void* comienzoMarco){
	//timestamp->key->value

	void* marcoParaKey=mempcpy(comienzoMarco,&timestamp,sizeof(int));

	void* marcoParaValue = mempcpy(marcoParaKey,&key,sizeof(int));

	memcpy(marcoParaValue,value,20);//maximo carac string
	log_info(logger,"Marco para value: %d",marcoParaValue);
	log_info(logger,"Value: %s", marcoParaValue);

	return comienzoMarco; //no tiene mucho sentido recibir y devolver comienzoMarco pero todavia no me decidi como hacer esto
}

pagina* nuevoDato(t_list* tablaPaginas,int flagModificado,int key, int timestamp, char* value,void* comienzoMemoria){
	pagina* nuevaPagina = malloc(sizeof(pagina));
	nuevaPagina->flagModificado= flagModificado;
	nuevaPagina->dato = asignoPaginaEnMarco(key,timestamp,value,comienzoMemoria);

	log_info(logger,"value pagina a agregar: %s", &nuevaPagina->dato->value);



	list_add(tablaPaginas,nuevaPagina);

	pagina* paginaAgregada = ultimaPagina(tablaPaginas);//para testear

	log_info(logger,"value de mi pagina agregada: %s",&paginaAgregada->dato->value);

	return nuevaPagina;
}






//Esto es copy del LFS (cambie lo que hacia explotar la memoria porque no lo usabamos en nada)

void mandarAEjecutarRequest(request* requestAEjecutar) {

	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	char* parametros = string_duplicate(requestAEjecutar->parametros); //Esto es para que se pueda hacer un free() en consola.c sin que rompa

	switch (requestAEjecutar->requestEnInt) {
	case SELECT:
		;
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) Select, parametros);

			pthread_detach(h_select);

			break;
		}

	case INSERT:
		;
		{
			pthread_t h_insert;

			pthread_create(&h_insert, NULL, (void *) insert, parametros);

			pthread_detach(h_insert);

			break;
		}

	case CREATE:
		;

		{
			pthread_t h_create;

			pthread_create(&h_create, NULL, (void *) create, parametros);

			pthread_detach(h_create);

			break;
		}

	case DESCRIBE:
		;
		{

			pthread_t h_describe;

			pthread_create(&h_describe, NULL, (void *) describe, parametros);

			pthread_detach(h_describe);

			break;
		}

	case DROP:
		;
		{

			pthread_t h_drop;

			pthread_create(&h_drop, NULL, (void *) drop, parametros);

			pthread_detach(h_drop);

			break;

		}

	}

	liberarRequest(requestAEjecutar);
}

bool filtroNombreTabla(char* nombreTabla,segmento* segmentoAComparar){
	return strcmp(nombreTabla,segmentoAComparar->nombreDeTabla)-1;
}

segmento* encuentroTablaPorNombre(char* nombreTabla, t_list* tablaDeSegmentos){
	t_link_element* element = tablaDeSegmentos->head;
	int position = 0;
	while (element != NULL && !filtroNombreTabla(nombreTabla,element->data)) {
		element = element->next;
		position++;
	}
	return element->data;
}

pagina* encuentroDatoPorKey(segmento* tabla, int key){
	bool filtroKey(pagina* pag){
		return pag->dato->key == key;
	}
	list_find(tabla->tablaDePaginas,(void*)filtroKey);

}

pagina* getPagina(t_list* tablaDeSegmentos, int key, char* nombreTabla){
	segmento* tabla = encuentroTablaPorNombre(nombreTabla, tablaDeSegmentos);
	log_info(logger, "Encontre una tabla con el nombre: %s", tabla->nombreDeTabla);
	pagina* dato = encuentroDatoPorKey(tabla,key);
	log_info(logger, "Encontre un dato con el value: %s", &dato->dato->value);
	return dato;
}

void Select(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 2, " ");
	char* tabla =  parametrosEnVector[0];
	string_to_upper(tabla);
	int key = atoi(parametrosEnVector[1]);



	log_info(logger,"Select de tabla: %s - key: %d",tabla,key);


	pagina* paginaPedida=getPagina(tablaSegmentos,key,tabla);

	printf("Registro pedido: %s\n",&paginaPedida->dato->value);

	free(parametrosEnVector[1]);
	free(parametrosEnVector[0]);
	free(parametrosEnVector);
	free(parametros);

}

void insert(char* parametros, void* memoria) {

	char** parametrosEnVector = string_n_split(parametros, 3, " ");

	char* tabla = parametrosEnVector[0];
	int key = atoi(parametrosEnVector[1]);
	char* value = parametrosEnVector[2]; //TODO: Sacarle las comillas

//	if(existeSegmento(memoria,tabla)){
//		segmento* segmentoEncontrado = segmento_find(memoria,tabla);
//		if(existePagina(segmentoEncontrado->tablaDePaginas,key)){
//			actualizarValue(memoria,tabla,key,value);
//		}
//		else{
//			cargarNuevaPagina(memoria,tabla,key,value);
//		}
//		free(segmentoEncontrado);
//	}
//	else{
//		cargarNuevoSegmento(memoria,tabla,key,value);
//	}
}


void create(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 4, " ");

	char* tabla = string_duplicate(parametrosEnVector[0]);
	char* consistencia = parametrosEnVector[1];
	int particiones = atoi(parametrosEnVector[2]);
	char* tiempoCompactacion = parametrosEnVector[3];

	mandarCreateALFS(tabla,consistencia,particiones,tiempoCompactacion);

	liberarArrayDeStrings(parametrosEnVector);
	free(parametros);

}

void mandarCreateALFS(char* tabla ,char* consistencia,int particiones  ,char* tiempoCompactacion){
	//todo
}

void describe(char* parametro) {

	if (strcmp(parametro, " ")) //Si hay un parametro
			{

		return;
	}

}

void drop(char* parametro) {

}
