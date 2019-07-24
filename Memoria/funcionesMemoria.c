#include "funcionesMemoria.h"

extern t_log* logger;
extern t_list* tablaSegmentos;
extern t_list* colaDeRequests;
extern sem_t requestsDisponibles;
extern sem_t sem_journal;
extern sem_t sem_refreshConfig;
extern sem_t sem_LFSconectandose;
extern sem_t sem_recepcionLFS;
extern int caracMaxDeValue;
extern int sleepJournal;
extern int sleepGossiping;
extern int retardoAccesoLFS;
extern int retardoAccesoMemoria;
extern int socketKernel;
extern int socketLFS;
extern int idScriptKernel;
extern char* dirConfig;
extern char* tablaSelect;

void ejecutarRequests() {

	while (1) {

		sem_wait(&requestsDisponibles);

		requestConID* requestEID = list_remove(colaDeRequests, 0);

		request* requestAEjecutar = requestEID->laRequest;

		sem_wait(&sem_journal);

		idScriptKernel = requestEID->idKernel;

		switch (requestAEjecutar->requestEnInt) {
		case SELECT: {

			log_info(logger, "Ejecutando SELECT");
			Select(requestAEjecutar->parametros);
			break;
		}

		case INSERT: {

			log_info(logger, "Ejecutando INSERT");
			insert(requestAEjecutar->parametros);
			break;
		}

		case DROP: {
			log_info(logger, "Ejecutando DROP");
			drop(requestAEjecutar->parametros);
			break;
		}
		case CREATE: {
			log_info(logger, "Ejecutando CREATE");
			create(requestAEjecutar->parametros);
			break;
		}
		case DESCRIBE: {
			log_info(logger, "Ejecutando DESCRIBE");
			describe(requestAEjecutar->parametros);
			break;
		}
		case JOURNAL: {

			log_info(logger, "Ejecutando JOURNAL");
			journal();
			break;
		}

		case STATUS:{
			log_info(logger, "Ejecutando STATUS");
			status();
			break;
		}

		}

		idScriptKernel = -1;

		sem_post(&sem_journal);
		printf("termine ciclo ejecucion\n");
		free(requestEID->laRequest->parametros);
		free(requestEID->laRequest);
		free(requestEID);
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

pagina* ultimaPagina(t_list* tablaPaginas) { //para testear
	pagina* lastPagina = list_get(tablaPaginas,
			tablaPaginas->elements_count - 1);
	return lastPagina;
}

void asignoPaginaEnMarco(uint16_t key, unsigned long long timestamp, char* value, void* comienzoMarco) {
	sem_wait(&sem_refreshConfig);
	int sleepMilisegundos = retardoAccesoMemoria / 1000;
	sem_post(&sem_refreshConfig);
	sleep(sleepMilisegundos);


	void* marcoParaKey = mempcpy(comienzoMarco, &timestamp, sizeof(long long int));
	void* marcoParaValue = mempcpy(marcoParaKey, &key, sizeof(uint16_t));
	strcpy(marcoParaValue, value); //maximo carac string
}

void eliminarRegistro(segmento* seg, pagina* pagEnSeg) {
	bool encuentroPaginaPorKey(pagina* pag) {
		return pag == pagEnSeg;
	}
	marcos[pagEnSeg->nroMarco].vacio = true;
	list_remove_by_condition(seg->tablaDePaginas,(void*) encuentroPaginaPorKey);
	free(pagEnSeg);
}

int marcoLRU() {
	bool encontreLRU = false;

	segmento* segmentoLRU = tablaSegmentos->head->data;
	pagina* paginaLRU = segmentoLRU->tablaDePaginas->head->data;
	void menorUltimoUsoPorSegmento(segmento* seg) {
		void menorUltimoUso(pagina* pag) {
			if (pag->ultimoUso < paginaLRU->ultimoUso) {
				paginaLRU = pag;
				segmentoLRU = seg;
				encontreLRU = true;
			}
		}
		list_iterate(seg->tablaDePaginas, (void*) menorUltimoUso);
	}

	list_iterate(tablaSegmentos, (void*) menorUltimoUsoPorSegmento);

	int marcoLRU = paginaLRU->nroMarco;



	if (encontreLRU){
		eliminarRegistro(segmentoLRU, paginaLRU);
		return marcoLRU;
	}
	else
		return MEM_LLENA;
}

int numeroMarcoDondeAlocar() {
	for (int i = 0; i < cantMarcos; i++) {
		if (marcos[i].vacio) {
			log_info(logger, "numero marco libre encontrado:%d", i);
			marcos[i].vacio = false;
			return i;
		}
	}
	int marcoAlocar = marcoLRU();
	if (marcoAlocar != MEM_LLENA){
		marcos[marcoAlocar].vacio = false;
		return marcoAlocar;
	}
	else {
		return MEM_LLENA;
	}

}

pagina* nuevoDato(t_list* tablaPaginas, int flagModificado, uint16_t key,unsigned long long timestamp, char* value) {
	pagina* nuevaPagina = malloc(sizeof(pagina));
	nuevaPagina->ultimoUso =  tiempoActual();
	int nroMarcoAlocar = numeroMarcoDondeAlocar();
	nuevaPagina->nroMarco = nroMarcoAlocar;
	if (nroMarcoAlocar > -1) {
		nuevaPagina->flagModificado = flagModificado;
		asignoPaginaEnMarco(key, timestamp, value,getMarcoFromIndex(nroMarcoAlocar));
		list_add(tablaPaginas, nuevaPagina);
	}
	return nuevaPagina;
}

segmento* encuentroTablaPorNombre(char* nombreTabla) {
	bool comparoNombreTabla(segmento* segmentoAComparar) {
		return strcmp(nombreTabla, segmentoAComparar->nombreDeTabla) == 0;
	}

	return list_find(tablaSegmentos, (void*) comparoNombreTabla);
}

marco* getMarcoFromIndex(int index){
	return comienzoMemoria + index * tamanioMarco;
}
marco* getMarcoFromPagina(pagina* pag) {
	return getMarcoFromIndex(pag->nroMarco);
}

pagina* encuentroDatoPorKey(segmento* tabla, uint16_t key) {
	bool comparoKey(pagina* pag) {
//		printf("key a comparar:%d - key encontrada:%d\n", key, getMarcoFromPagina(pag)->key);
		return getMarcoFromPagina(pag)->key == key;
	}
	return list_find(tabla->tablaDePaginas, (void*) comparoKey);

}

pagina* getPagina(uint16_t key, char* nombreTabla) { //retorna un NULL si no existe la tabla o la pagina

	segmento* tabla = encuentroTablaPorNombre(nombreTabla);
//		printf("tabla pedida:%p\n", tabla);
	if (tabla != NULL) {
//		log_info(logger, "Encontre una tabla con el nombre: %s",
//				tabla->nombreDeTabla);
		sem_wait(&sem_refreshConfig);
		int sleepMilisegundos = retardoAccesoMemoria / 1000;
		sem_post(&sem_refreshConfig);
		sleep(sleepMilisegundos);
		pagina* dato = encuentroDatoPorKey(tabla, key);
//		log_info(logger, "Encontre un dato con el value: %s",
//				&getMarcoFromPagina(dato)->value);

		return dato;
	} else
		return NULL;
}

void actualizoDato(pagina* pagina, char* nuevoValue, unsigned long long nuevoTimestamp) {
	sem_wait(&sem_refreshConfig);
	int sleepMilisegundos = retardoAccesoMemoria/ 1000;
	sem_post(&sem_refreshConfig);
	sleep(sleepMilisegundos);
	pagina->ultimoUso= tiempoActual();
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

void Select(char* parametros) {
	char** parametrosEnVector = string_n_split(parametros, 2, " ");
	char* tabla = string_duplicate(parametrosEnVector[0]);
	string_to_upper(tabla);
	uint16_t key = atoi(parametrosEnVector[1]);
	liberarArrayDeStrings(parametrosEnVector);

	log_info(logger, "Select de tabla: %s - key: %d", tabla, key);

	pagina* paginaPedida = getPagina(key, tabla);

	char* dato;

	if (paginaPedida != NULL) {
		paginaPedida->ultimoUso = tiempoActual();
		dato = string_duplicate(&getMarcoFromPagina(paginaPedida)->value);
		log_info(logger, "Resultado: %s\n", dato);

		if (idScriptKernel) {
			log_info(logger, "Enviando el resultado al kernel");
			enviarStringConHeaderEId(socketKernel, dato, DATO, idScriptKernel);
			free(tabla);
			return;
		}

	} else {
		log_info(logger, "No se encontro el dato, mandando request a LFS");
		tablaSelect = tabla; //tablaSelect es una variable global que se usa en el hilo de recepcion de LFS. Pido disculpas por ser tan hijo de puta y haber hecho esto, pero no hay tiempOOOOO.
		mandarRequestALFS(SELECT, parametros);
//		registro* registroPedido= recibirRegistro(socketLFS, logger);
//		insertInterno(registroPedido->key,registroPedido->value,tabla,registroPedido->timestamp);
//		if (idScriptKernel) {
//			log_info(logger, "Enviando el resultado al kernel");
//			enviarStringConHeaderEId(socketKernel, registroPedido->value, DATO, idScriptKernel);
//			return;
//		}
	}
}

unsigned long long tiempoActual(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

void insert(char* parametros) {
	status();
	char** parametrosEnVector = string_n_split(parametros, 3, " ");

	char* tabla =string_duplicate( parametrosEnVector[0]);
	string_to_upper(tabla);
	uint16_t key = atoi(parametrosEnVector[1]);

	char* value =string_duplicate( parametrosEnVector[2]);

	unsigned long long timestamp = tiempoActual();



	liberarArrayDeStrings(parametrosEnVector);

	value = sacoComillas(value);
	if (string_length(value) > caracMaxDeValue) {
		log_error(logger, "Value excede caracteres maximos");
		if (idScriptKernel) {
			enviarRespuestaAlKernel(idScriptKernel, ERROR);
			log_info(logger, "Enviando ERROR al kernel");
		}
		free(tabla);
		free(value);
		return;

	}

	log_info(logger, "INSERT: Tabla:%s - key:%d - timestamp:%llu - value:%s\n",
					tabla, key, timestamp, value);


	segmento* tablaEncontrada = encuentroTablaPorNombre(tabla);
	if (tablaEncontrada == NULL) {
		log_info(logger, "Se va a cinsertrear la tabla y el dato");
		segmento* tablaCreada = nuevaTabla(tablaSegmentos, tabla);
		pagina* nuevaPagina = nuevoDato(tablaCreada->tablaDePaginas, 1, key,
				timestamp, value);
		if (nuevaPagina->nroMarco == MEM_LLENA) {
			log_error(logger, "MEMORIA FULL");
			free(nuevaPagina);
			if (idScriptKernel) {
				log_info(logger,
						"Avisando al kernel que la memoria esta llena");
				enviarRespuestaAlKernel(idScriptKernel, MEM_LLENA);
			}
			free(tabla);
			free(value);
			return;
		} else {
			log_info(logger, "Se pudo hacer el INSERT");
			if (idScriptKernel) {
				log_info(logger, "Enviando respuesta al kernel");
				enviarRespuestaAlKernel(idScriptKernel, TODO_BIEN);
			}
			free(tabla);
			free(value);
			return;
		}

	} else {
		pagina* datoEncontrado = encuentroDatoPorKey(tablaEncontrada, key);
		if (datoEncontrado != NULL) {
			log_info(logger, "Se va a actualizar el dato ya existente");
			actualizoDato(datoEncontrado, value, timestamp);
			log_info(logger, "Se pudo hacer el INSERT");
			if (idScriptKernel) {
				log_info(logger, "Enviando respuesta al kernel");
				enviarRespuestaAlKernel(idScriptKernel, TODO_BIEN);
			}

			return;
		} else {
			log_info(logger, "Se va a crear el dato");
			pagina* nuevaPagina = nuevoDato(tablaEncontrada->tablaDePaginas, 1,
					key, timestamp, value);
			if (nuevaPagina->nroMarco == MEM_LLENA) {
				log_error(logger, "MEMORIA FULL");
				free(nuevaPagina);
				if (idScriptKernel) {
					log_info(logger,
							"Avisando al kernel que la memoria esta llena");
					enviarRespuestaAlKernel(idScriptKernel, MEM_LLENA);
				}
				return;
			} else {
				log_info(logger, "Se pudo hacer el INSERT");
				if (idScriptKernel) {
					log_info(logger, "Enviando respuesta al kernel");
					enviarRespuestaAlKernel(idScriptKernel, TODO_BIEN);
				}

				return;
			}

		}
	}

}


//si, esto es literalmente una copia de lo que esta arriba solo que sin las respuestas, con el timestamp por parametro Y LO AGREGA SIN EL FLAG MODIFICADO.
void insertInterno(uint16_t key, char* value, char* tabla, unsigned long long timestamp){
	segmento* tablaEncontrada = encuentroTablaPorNombre(tabla);
		if (tablaEncontrada == NULL) {
			log_info(logger, "Se va a crear la tabla y el dato");
			segmento* tablaCreada = nuevaTabla(tablaSegmentos, tabla);
			pagina* nuevaPagina = nuevoDato(tablaCreada->tablaDePaginas, 1, key,
					timestamp, value);
			if (nuevaPagina->nroMarco == MEM_LLENA) {
				log_error(logger, "MEMORIA FULL");
				return;
			} else {
				log_info(logger, "Se pudo hacer el INSERT");
				return;
			}

		} else {
			pagina* datoEncontrado = encuentroDatoPorKey(tablaEncontrada, key);
			if (datoEncontrado != NULL) {
				log_info(logger, "Se va a actualizar el dato ya existente");
				actualizoDato(datoEncontrado, value, timestamp);
				log_info(logger, "Se pudo hacer el INSERT");
				return;
			} else {
				log_info(logger, "Se va a crear el dato");
				pagina* nuevaPagina = nuevoDato(tablaEncontrada->tablaDePaginas, 0,	key, timestamp, value);
				if (nuevaPagina->nroMarco == MEM_LLENA) {
					log_error(logger, "MEMORIA FULL");
					free(nuevaPagina);
					return;
				} else {
					log_info(logger, "Se pudo hacer el INSERT");
					return;
				}

			}
		}

}

void drop(char* parametro) {
	dropInterno(parametro);
	log_info(logger, "Mandando DROP al LFS");
	mandarRequestALFS(DROP, parametro);
}

void dropInterno(char* parametro){
	void liberoDato(pagina* pag) {

			int nroMarco = pag->nroMarco;
			printf("nro de marco a dropear:%d\n", nroMarco);
			marcos[nroMarco].vacio = true;
			printf("cambie disponibilidad marco\n");
			free(pag);
	}
	char* tabla = string_duplicate(parametro);
	string_to_upper(tabla);
	log_info(logger, "DROP de la tabla %s", parametro);
	segmento* tablaADropear = encuentroTablaPorNombre(tabla);
	if (tablaADropear != NULL) {
		list_destroy_and_destroy_elements(tablaADropear->tablaDePaginas,
				(void*) liberoDato);
		bool comparoNombreTabla(segmento* segmentoAComparar) {
			return strcmp(tablaADropear->nombreDeTabla,
					segmentoAComparar->nombreDeTabla) == 0;
		}
		list_remove_by_condition(tablaSegmentos, (void*) comparoNombreTabla);
		log_info(logger, "Se elimino la tabla de la memoria");

	} else {
		log_info(logger, "No existe la tabla en la memoria");
	}
	free(tablaADropear);
	free(tabla);
}

void create(char* parametros) {

	log_info(logger, "CREATE %s", parametros);
	log_info(logger, "Mandando CREATE al LFS");
	mandarRequestALFS(CREATE, parametros);

}

void describe(char* parametro) {

	log_info(logger, "DESCRIBE %s", parametro);
	log_info(logger, "Enviando DESCRIBE al LFS");
	mandarRequestALFS(DESCRIBE, parametro);
}

void mandarRequestALFS(int requestAMandar, char* parametros) {

	sem_wait(&sem_LFSconectandose);
	if (socketLFS == -1) {
		log_error(logger,
				"El LFS no esta conectado, no se puede terminar la REQUEST.");
		if (idScriptKernel) {
			enviarRespuestaAlKernel(idScriptKernel, ERROR);
		}
		sem_post(&sem_LFSconectandose);
		return;
	}
	sem_post(&sem_LFSconectandose);

	request* nuevaRequest = malloc(sizeof(request));
	nuevaRequest->requestEnInt = requestAMandar;
	if (parametros == NULL) {
		nuevaRequest->parametros = string_duplicate(" ");
	} else {
		nuevaRequest->parametros = string_duplicate(parametros);
	}

	sem_wait(&sem_refreshConfig);
	int sleepMilisegundos = retardoAccesoLFS / 1000;
	sem_post(&sem_refreshConfig);
	sleep(sleepMilisegundos);

	enviarRequestConHeader(socketLFS, nuevaRequest, REQUEST);

	free(nuevaRequest->parametros);
	free(nuevaRequest);

	sem_wait(&sem_recepcionLFS);
}

void mandoPaginaComoInsert(pagina* pag) {
	registro* pagAMandar = malloc(sizeof(registro));
	marco* frame = getMarcoFromPagina(pag);
	pagAMandar->key = frame->key;
	pagAMandar->timestamp = frame->timestamp;
	pagAMandar->value = &frame->value;
	enviarRegistroComoInsert(pagAMandar);
	free(pagAMandar);
}

request* pasarPaginaAInsert(pagina* paginaAPasar, char* nombreTabla) {

	request* paginaEnInsert = malloc(sizeof(request));

	paginaEnInsert->requestEnInt = INSERT;

	paginaEnInsert->parametros = string_new();

	marco* frame = getMarcoFromPagina(paginaAPasar);

	int lengthTS = snprintf( NULL, 0, "%llu", frame->timestamp); //TODO: Marca de que se cambio %d por %llu
	char* tsEnString = malloc(lengthTS + 1);
	snprintf(tsEnString, lengthTS + 1, "%llu", frame->timestamp);

	string_append(&paginaEnInsert->parametros, nombreTabla);
	string_append(&paginaEnInsert->parametros, " ");
	string_append(&paginaEnInsert->parametros, string_itoa(frame->key));
	string_append(&paginaEnInsert->parametros, " \"");
	string_append(&paginaEnInsert->parametros, &frame->value);
	string_append(&paginaEnInsert->parametros, "\" ");
	string_append(&paginaEnInsert->parametros, tsEnString);

	free(tsEnString);

	return paginaEnInsert;
}

t_list* journalPorSegmento(segmento* seg) {
//
//	void mostrarPagina(pagina* pag){
//		printf("nro marco:%d-value:%s\n", pag->nroMarco,&getMarcoFromPagina(pag)->value);
//	}
//
//	list_iterate(seg->tablaDePaginas,(void*)mostrarPagina);
	bool estaModificada(pagina* pag) {
		return pag->flagModificado;
	}

	request* pasarPaginaAInsertConNombreDelSegmento(pagina* paginaAPasar) {
		return pasarPaginaAInsert(paginaAPasar, seg->nombreDeTabla);
	}

	t_list* paginasModificadas = list_filter(seg->tablaDePaginas,
			(void*) estaModificada);

	t_list* insertsAMandar = list_map(paginasModificadas,
			(void*) pasarPaginaAInsertConNombreDelSegmento);

	list_destroy(paginasModificadas);

	char* nombreTabla = string_duplicate(seg->nombreDeTabla);
	dropInterno(nombreTabla);
	printf("dropie la tabla:%s\n", nombreTabla);

	return insertsAMandar;
}

void combinarListaInserts(t_list* listaALaQueAgregar, t_list* listaQueAgregar) //Esta funcion destruye listaQueAgregar asi que cuidadoOoOo
{
	if(listaQueAgregar == NULL)
	{
		return;
	}

	while(list_size(listaQueAgregar) != 0)
	{
		request* unaRequest = list_remove(listaQueAgregar,0);
		list_add(listaALaQueAgregar,unaRequest);
	}
	list_destroy(listaQueAgregar);
}

void journal() {
	//TODO: Hay algun problema si las listas son vacias? En haskell no, en C puede ocurrir magia siempre pero no creo.

	sem_wait(&sem_LFSconectandose);
	if (socketLFS == -1) {

		log_error(logger, "El LFS no esta conectado");

		sem_post(&sem_LFSconectandose);
		return;
	}
	sem_post(&sem_LFSconectandose);

	if (tablaSegmentos != NULL && list_size(tablaSegmentos) != 0) { //bueno hay que tener cuidado :)
		t_list* listasDeInserts = list_map(tablaSegmentos,
				(void*) journalPorSegmento);

		if (listasDeInserts != NULL && list_size(listasDeInserts) != 0) {

			t_list* insertsAMandar = list_remove(listasDeInserts, 0);

			while(list_size(listasDeInserts) != 0) //El fold no funcionaba como queria. Tal vez porque soy un pelotudo (re pelotudo) y no lo usaba bien
			{
				t_list* unaListaDeInserts= list_remove(listasDeInserts,0);
				combinarListaInserts(insertsAMandar, unaListaDeInserts);
			}
			list_destroy(listasDeInserts);

			if (insertsAMandar != NULL && list_size(insertsAMandar) != 0) {

				sem_wait(&sem_LFSconectandose);
				if (socketLFS == -1) {
					sem_post(&sem_LFSconectandose);
					return;
				}
				sem_post(&sem_LFSconectandose);

				sem_wait(&sem_refreshConfig);
				int sleepMilisegundos = retardoAccesoLFS / 1000;
				sem_post(&sem_refreshConfig);
				sleep(sleepMilisegundos);

				enviarListaDeRequestsConHeader(socketLFS, insertsAMandar,
				JOURNAL);
				printf("le envie el JOURNAL al LFS\n");
			}

		}

	}

}
void journalAutomatico() {

	while (true) {
		sem_wait(&sem_refreshConfig);
		int sleepMilisegundos = sleepJournal / 1000;
		sem_post(&sem_refreshConfig);
		sleep(sleepMilisegundos);
		sem_wait(&sem_journal);
		log_info(logger, "Ejecutando el JOURNAL automatico");
		journal();
		sem_post(&sem_journal);
	}
}

void refreshConfig() {
	while (true) {

		esperarModificacionDeArchivo(dirConfig);

		sem_wait(&sem_refreshConfig);
		t_config* config = config_create(dirConfig);

		sleepJournal = config_get_int_value(config, "RETARDO_JOURNAL");
		sleepGossiping = config_get_int_value(config, "RETARDO_GOSSIPING");
		retardoAccesoLFS = config_get_int_value(config, "RETARDO_FS");
		retardoAccesoMemoria = config_get_int_value(config, "RETARDO_MEM");

		config_destroy(config);

		sem_post(&sem_refreshConfig);
	}
}

void reconexionLFS() {

	while (1) {
		sleep(5);
		sem_wait(&sem_LFSconectandose);
		int respuesta = primeraConexionLFS();
		sem_post(&sem_LFSconectandose);
		if (respuesta != -1) {
			log_info(logger, "Se conecto el LFS.");
			pthread_t h_respuestaLFS;
			pthread_create(&h_respuestaLFS, NULL, (void *) manejarRespuestaLFS,
			NULL);
			pthread_detach(h_respuestaLFS);
			return;
		}
	}

}


void status(){
	char* statusMarcos[cantMarcos];
	for (int i = 0;i<cantMarcos;i++){
		statusMarcos[i]="FREE";
	}



	void statusPorSegmento(segmento* seg){
		void statusPagina(pagina* pag){
				char* status  = string_new();
				string_append(&status, "PAGINA: M=");
				string_append(&status,string_itoa(pag->flagModificado));
				string_append(&status," K=");
				string_append(&status,string_itoa(getMarcoFromPagina(pag)->key));
				string_append(&status," TS=");
				string_append(&status,string_itoa(getMarcoFromPagina(pag)->timestamp));
				string_append(&status," T=");
				string_append(&status,seg->nombreDeTabla);
				string_append(&status," V=");
				string_append(&status,&getMarcoFromPagina(pag)->value);
				statusMarcos[pag->nroMarco] = status;
			}
		list_iterate(seg->tablaDePaginas,(void*)statusPagina);
	}

	list_iterate(tablaSegmentos, (void*)statusPorSegmento);

	printf("Listado Marcos (M=modificado,K=key,TS=timestamp,T=tabla,V=value)\n");

	for(int i = 0;i<cantMarcos;i++){
		printf("Nro Marco:%d Status:%s\n",i,statusMarcos[i]);
	}

	for(int i = 0;i<cantMarcos;i++){
		if(strcmp(statusMarcos[i],"FREE") != 0) {
			free(statusMarcos[i]);
		}
	}
}

