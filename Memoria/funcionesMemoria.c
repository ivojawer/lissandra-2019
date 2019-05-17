#include "funcionesMemoria.h"


extern t_log* logger;






//TABLA DE SEGMENTOS

t_list* crearTablaSegmentos(){
	return list_create();
}



segmento* ultimoSegmento(t_list* tablaSegmentos){
	return list_get(tablaSegmentos,tablaSegmentos->elements_count - 1);
}

void nuevaTabla(t_list* tablaSegmentos,char* nombreTabla){ //todo: destroy de esto
	segmento* nuevoSegmento = malloc(sizeof(segmento));
	nuevoSegmento->nombreDeTabla = malloc(strlen(nombreTabla)*sizeof(char));
	strcpy(nuevoSegmento->nombreDeTabla,nombreTabla);
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
	return list_get(tablaPaginas,tablaPaginas->elements_count - 1);
}


void* asignoPaginaEnMarco(int key, int timestamp,char* value){
	return value; //todo esta funcion deberia meterle a un marco los datos que llegan
}

pagina* nuevoDato(t_list* tablaPaginas,int flagModificado,int key, int timestamp, char* value){
	pagina* nuevaPagina = malloc(sizeof(pagina));

	nuevaPagina->flagModificado= flagModificado;
	nuevaPagina->dato = asignoPaginaEnMarco(key,timestamp,value);


	list_add(tablaPaginas,nuevaPagina);

	pagina* paginaAgregada = ultimaPagina(tablaPaginas);//para testear

	return nuevaPagina;
}






//------------esto esta mal pero quiero acordarme como lo hice-----------------
//void crearSegmento(void* comienzoMemoria, int nroSegmento,char* nombreTabla){
//	void* posMemoria = comienzoMemoria + sizeof(segmento)*nroSegmento;
//	segmento nuevoSegmento;
//	nuevoSegmento.nombreDeTabla=nombreTabla;
//	nuevoSegmento.tablaDePaginas = list_create();
//	log_info(logger,"lo voy a copiar en memoria es: %s\n", nuevoSegmento.nombreDeTabla);
//	memcpy(posMemoria,&nuevoSegmento,sizeof(segmento));
//}








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

void Select(char* parametros,t_list* memoria) {

	char** parametrosEnVector = string_n_split(parametros, 2, " ");

	char* tabla = parametrosEnVector[0];
	int key = atoi(parametrosEnVector[1]);

	if(existeKeyEnMemoria(memoria,tabla,key)){
		segmento* segmentoEncontrado = segmento_find(memoria,tabla);
		pagina* paginaEncontrada = pagina_find(segmentoEncontrado->tablaDePaginas,key);
		imprimirValue(paginaEncontrada->dato);
		free(segmentoEncontrado);
		free(paginaEncontrada);
	}
	else{
		char* value = mandarSelectALSF(parametros);
		imprimirValue(value);
		cachearEnMemoria(tabla,key,value,memoria);
		free(value);
	}

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

	if(existeSegmento(memoria,tabla)){
		segmento* segmentoEncontrado = segmento_find(memoria,tabla);
		if(existePagina(segmentoEncontrado->tablaDePaginas,key)){
			actualizarValue(memoria,tabla,key,value);
		}
		else{
			cargarNuevaPagina(memoria,tabla,key,value);
		}
		free(segmentoEncontrado);
	}
	else{
		cargarNuevoSegmento(memoria,tabla,key,value);
	}
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
