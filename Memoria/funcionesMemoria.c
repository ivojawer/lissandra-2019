#include "funcionesMemoria.h"

extern t_log* logger;
extern t_config* config;
extern t_list* tablaSegmentos;
extern int socketLFS;
extern sem_t requestsDisponibles;
extern t_list* colaDeRequests;
extern int socketKernel;

void ejecutarRequests() {

	while (1) {

		sem_wait(&requestsDisponibles);

		requestConID* requestEID = list_get(colaDeRequests, 0);

		request* requestAEjecutar = requestEID->laRequest;

		switch (requestAEjecutar->requestEnInt) {
		case SELECT: {

			char* datoObtenido = Select(requestAEjecutar->parametros);

			if (requestEID->idKernel) {

				if (datoObtenido == NULL) {

					enviarRespuestaAlKernel(requestEID->idKernel, ERROR);

				} else {
					enviarStringConHeaderEId(socketKernel, datoObtenido, DATO,
							requestEID->idKernel);
				}

			}
			break;
		}

		case INSERT: {

			int resultado = insert(requestAEjecutar->parametros);

			if (requestEID->idKernel) {
				enviarRespuestaAlKernel(requestEID->idKernel, resultado);
			}

			break;
		}

		case DROP: {

			int resultado = drop(requestAEjecutar->parametros);
			if (requestEID->idKernel) {
				enviarRespuestaAlKernel(requestEID->idKernel, resultado);
			}
			break;
		}
		case CREATE: {
			int resultado = create(requestAEjecutar->parametros);
			if (requestEID->idKernel) {
				enviarRespuestaAlKernel(requestEID->idKernel, resultado);
			}

			break;
		}
		case DESCRIBE: {

			t_list* resultado = describe(requestAEjecutar->parametros);

			metadataTablaLFS* metadataPrueba = list_get(resultado,0);

			if (requestEID->idKernel) {

				if(metadataPrueba->consistencia == -1){
					enviarRespuestaAlKernel(requestEID->idKernel, ERROR);
				}
				else
				{
					enviarMetadatasConHeaderEId(socketKernel,resultado,METADATAS,requestEID->idKernel);
				}
			}

			break;
		}
		case JOURNAL: {
			journal();
			break;
		}

		}
	}

}

//TABLA DE SEGMENTOS

t_list* crearTablaSegmentos() {
	return list_create();
}

segmento* ultimoSegmento(t_list* tablaSegmentos) {
	return list_get(tablaSegmentos, tablaSegmentos->elements_count - 1);
}

segmento* nuevaTabla(t_list* tablaSegmentos, char* nombreTabla) { //todo: destroy de esto
	segmento* nuevoSegmento = malloc(sizeof(segmento));

	nuevoSegmento->nombreDeTabla = string_duplicate(nombreTabla);

	nuevoSegmento->tablaDePaginas = crearTablaPaginas();

//	log_info(logger,"tabla por agregar: %s",nuevoSegmento->nombreDeTabla);

	list_add(tablaSegmentos, nuevoSegmento);

//	log_info(logger,"tabla agregada: %s",ultimoSegmento(tablaSegmentos)->nombreDeTabla);
	return nuevoSegmento;
}

//TABLA DE PAGINAS

t_list* crearTablaPaginas() {
	return list_create();
}


pagina* ultimaPagina(t_list* tablaPaginas){ //para testear
	pagina* lastPagina = list_get(tablaPaginas,tablaPaginas->elements_count - 1);
	return lastPagina ;
}

void asignoPaginaEnMarco(int key, int timestamp, char* value,
		void* comienzoMarco) {
	//timestamp->key->value el orden importa

//	printf("value:%s\n",value);
//	printf("key:%d\n",key);
//	printf("timestamp:%d\n",timestamp);

	void* marcoParaKey = mempcpy(comienzoMarco, &timestamp, sizeof(int));
	void* marcoParaValue = mempcpy(marcoParaKey, &key, sizeof(int));
	memcpy(marcoParaValue, value, caracMaxDeValue); //maximo carac string
//	log_info(logger,"Marco donde asigne: %p",comienzoMarco);
}

int numeroMarcoDondeAlocar() {
	for (int i = 0; i < cantMarcos; i++) {
		if (marcos[i].vacio) {
			log_info(logger, "numero marco libre encontrado:%d", i);
			marcos[i].vacio = false;
			return i;
		}
	}
	return -1; //falta aplicar algoritmo LRU si no encontro ninguna libre
}


pagina* nuevoDato(t_list* tablaPaginas, int flagModificado, int key,
		int timestamp, char* value) {

	pagina* nuevaPagina = malloc(sizeof(pagina));

	nuevaPagina->flagModificado = flagModificado;

	nuevaPagina->nroMarco = numeroMarcoDondeAlocar();
//	printf("marco pagina a asigar en marco:%p\n", nuevaPagina->dato);

	asignoPaginaEnMarco(key, timestamp, value, getMarcoFromPagina(nuevaPagina));
//	nuevaPagina->dato =comienzoMemoria;
//	asignoPaginaEnMarco(key,timestamp,value,comienzoMemoria);

//	printf("value pagina a agregar: %s", &nuevaPagina->dato->value);

	list_add(tablaPaginas, nuevaPagina);

	pagina* paginaAgregada = ultimaPagina(tablaPaginas); //para testear
	log_info(logger, "value de mi pagina agregada: %s",
			&getMarcoFromPagina(paginaAgregada)->value);

	return nuevaPagina;
}

segmento* encuentroTablaPorNombre(char* nombreTabla) {
	bool comparoNombreTabla(segmento* segmentoAComparar) {
		return strcmp(nombreTabla, segmentoAComparar->nombreDeTabla) == 0;
	}

	return list_find(tablaSegmentos, (void*) comparoNombreTabla);
}

pagina* encuentroDatoPorKey(segmento* tabla, int key) {
	bool comparoKey(pagina* pag) {
//		printf("key a comparar:%d - key encontrada:%d\n", key, pag->dato->key);
		return getMarcoFromPagina(pag)->key == key;
	}
	return list_find(tabla->tablaDePaginas, (void*) comparoKey);

}

pagina* getPagina(int key, char* nombreTabla) { //retorna un NULL si no existe la tabla o la pagina

	segmento* tabla = encuentroTablaPorNombre(nombreTabla);
	printf("tabla pedida:%p\n", tabla);
	if (tabla != NULL) {
		log_info(logger, "Encontre una tabla con el nombre: %s",
				tabla->nombreDeTabla);
		pagina* dato = encuentroDatoPorKey(tabla, key);
		log_info(logger, "Encontre un dato con el value: %s",
				&getMarcoFromPagina(dato)->value);

		return dato;
	} else
		return NULL;
}

char* Select(char* parametros) {
	char** parametrosEnVector = string_n_split(parametros, 2, " ");
	char* tabla = parametrosEnVector[0];
	string_to_upper(tabla);
	int key = atoi(parametrosEnVector[1]);

	liberarArrayDeStrings(parametrosEnVector);

	log_info(logger, "Select de tabla: %s - key: %d", tabla, key);

	pagina* paginaPedida = getPagina(key, tabla);

	if (paginaPedida != NULL) {
		printf("Registro pedido: %s\n", &getMarcoFromPagina(paginaPedida)->value);
	} else {
		log_info(logger, "No encontre el dato, mandando request a LFS");

		mandarRequestALFS(SELECT, parametros);

		int header = recibirInt(socketLFS, logger);
		if (header != REQUEST) {
			manejoErrorLFS();
			return NULL;
		}

		char* dato = recibirString(socketLFS, logger);

		if (!strcmp(dato, " ")) {
			free(dato);
			manejoErrorLFS();
			return NULL;
		}

		return dato;

	}

}

void actualizoDato(pagina* pagina, char* nuevoValue, int nuevoTimestamp) {
	strcpy(&getMarcoFromPagina(pagina)->value, nuevoValue);
	getMarcoFromPagina(pagina)->timestamp = nuevoTimestamp;

}

char* sacoComillas(char* cadena) {
	int largoCadena = string_length(cadena);
	if (cadena[0] == '\"' && cadena[largoCadena - 1] == '\"') {
		return string_substring(cadena, 1, largoCadena - 2);
	}
	return cadena;
}

int insert(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 3, " ");

	char* tabla = parametrosEnVector[0];
	string_to_upper(tabla);
	int key = atoi(parametrosEnVector[1]);
	char* value = parametrosEnVector[2];

	liberarArrayDeStrings(parametrosEnVector);

	value = sacoComillas(value);
	//value=string_substring_until(value,config_get_int_value(config,"CANT_MAX_CARAC")); //lo corta para que no ocupe mas de 20 caracteres

	int timestamp = time(NULL) / 1000; //TODO: Hacer la adquisicion del timestamp consistente con el LFS

	log_info(logger, "INSERT: Tabla:%s - key:%d - timestamp:%d - value:%s\n",
			tabla, key, timestamp, value);

	segmento* tablaEncontrada = encuentroTablaPorNombre(tabla);
	if (tablaEncontrada == NULL) {
		log_info(logger, "tengo que crear la tabla y el dato");

		segmento* tablaCreada = nuevaTabla(tablaSegmentos, tabla);

		nuevoDato(tablaCreada->tablaDePaginas, 1, key, timestamp, value);
		printf("dato insertado y tabla creada\n");
		return 1;
	} else {
		pagina* datoEncontrado = encuentroDatoPorKey(tablaEncontrada, key);
		if (datoEncontrado != NULL) {
			log_info(logger, "tengo que actualizar el dato");
			actualizoDato(datoEncontrado, value, timestamp);
			printf("dato actualizado\n");
			return 1;
		} else {
			log_info(logger, "tengo que cear el dato");
			nuevoDato(tablaEncontrada->tablaDePaginas, 1, key, timestamp,
					value);
			printf("dato insertado\n");
			return 1;
		}
	}

}

int create(char* parametros) {

	mandarRequestALFS(CREATE, parametros);

	int header = recibirInt(socketLFS, logger);

	if (header != RESPUESTA) {
		manejoErrorLFS();
		return -1;
	}

	int respuesta = recibirInt(socketLFS, logger);

	return respuesta;

}

void mandarRequestALFS(int requestAMandar, char* parametros) {

	request* nuevaRequest = malloc(sizeof(request));
	nuevaRequest->requestEnInt = requestAMandar;
	nuevaRequest->parametros = string_duplicate(parametros);

	enviarRequestConHeader(socketLFS, nuevaRequest, REQUEST);

	free(nuevaRequest->parametros);
	free(nuevaRequest);
}
t_list* describe(char* parametro) {

	mandarRequestALFS(CREATE, parametro);

	int header = recibirInt(socketLFS, logger);

	t_list* metadatas;

	if (header != RESPUESTA) {
		manejoErrorLFS();
		metadatas = list_create();

		metadataTablaLFS* metadata = malloc(sizeof(metadataTablaLFS));
		metadata->nombre = NULL;
		metadata->consistencia = -1;
		metadata->particiones = -1;
		metadata->compactTime = -1;

		list_add(metadatas, metadata);

		return metadatas;
	}

	metadatas = recibirMetadatas(socketLFS, logger);

	metadataTablaLFS* metadataPrueba = list_get(metadatas, 0);

	if (metadataPrueba->compactTime != -1) {
		describirMetadatas(metadatas);
	}

	return metadatas;
}

int drop(char* parametro) {

	void liberoDato(pagina* pag) {

		int nroMarco = ((void*) getMarcoFromPagina(pag) - comienzoMemoria) / tamanioMarco;
		printf("nro de marco a dropear:%d\n", nroMarco);
		marcos[nroMarco].vacio=true;
		printf("cambie valores marco\n");
		free(pag);
	}

	char* tabla = string_duplicate(parametro);
	string_to_upper(tabla);
	segmento* tablaADropear = encuentroTablaPorNombre(tabla);
	if(tablaADropear !=NULL){
		printf("encontre la tabla a dropear, nombre:%s\n",tablaADropear->nombreDeTabla);
		list_destroy_and_destroy_elements(tablaADropear->tablaDePaginas,(void*)liberoDato);
		bool comparoNombreTabla(segmento* segmentoAComparar){
				return strcmp(tablaADropear->nombreDeTabla,segmentoAComparar->nombreDeTabla) == 0;
		}
		list_remove_by_condition(tablaSegmentos,(void*)comparoNombreTabla);
		printf("libere los datos y destruyo tabla paginas\n");

		printf("tabla eliminada\n");

	}
	free(tablaADropear);
	free(tabla);
	mandarRequestALFS(DROP, parametro);

	int header = recibirInt(socketLFS, logger);

	if (header != REQUEST) {
		manejoErrorLFS();
		return 0;
	}

	int respuesta = recibirInt(socketLFS, logger);

	return respuesta;

}


void mandoPaginaComoInsert(pagina* pag){
	registro pagAMandar;
	marco* frame = getMarcoFromPagina(pag);
	pagAMandar.key=frame->key;
	pagAMandar.timestamp = frame->timestamp;
	pagAMandar.value = &frame->value;
	enviarInsert(pagAMandar);
}

void journalPorSegmento(segmento* seg){
	bool estaModificada(pagina* pag){
		return pag->flagModificado;
	}
	t_list* paginasModificadas=list_filter(seg->tablaDePaginas,(void*)estaModificada);
	list_iterate(paginasModificadas,(void*)mandoPaginaComoInsert);
	char* nombreTabla = string_duplicate(seg->nombreDeTabla);
	drop(nombreTabla);
}

void journal() {
	list_iterate(tablaSegmentos,(void*)journalPorSegmento);
}



void journalAutomatico(){
	while(true){

		sleep(config_get_int_value(config,"RETARDO_GOSSIPING"));
	}
}

