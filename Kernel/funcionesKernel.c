#include "funcionesKernel.h"

extern t_list* colaNEW;
extern t_list* colaREADY;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern t_list* tiemposInsert;
extern t_list* tiemposSelect;
extern t_list* memorias;
extern sem_t sem_cambioId;
extern sem_t sem_disponibleColaREADY;
extern sem_t sem_cambioSleepEjecucion;
extern t_log* logger;
extern int idInicial;
extern int proximaMemoriaEC;
extern int sleepEjecucion;
extern script* scriptRefreshMetadata;

int crearScript(request* nuevaRequest) {

	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	script* nuevoScript = malloc(sizeof(script));

	sem_wait(&sem_cambioId);
	nuevoScript->idScript = idInicial;
	idInicial++;
	sem_post(&sem_cambioId);

	nuevoScript->lineasLeidas = 0;

	if (nuevaRequest->requestEnInt == RUN) {

		char* direccion = scriptConRaiz(nuevaRequest->parametros);

		nuevoScript->direccionScript = direccion;
		nuevoScript->esPorConsola = 0;
	}

	else {

		nuevoScript->esPorConsola = 1;

		if ((crearArchivoParaRequest(nuevoScript, nuevaRequest)) == -1) {
			log_error(logger, "Hubo un error en la creacion del request");

			moverScript(nuevoScript->idScript, colaNEW, listaEXIT);
			return -1;

		}

	}

	sem_init(&nuevoScript->semaforoDelScript, 0, 0);

	list_add(colaNEW, nuevoScript); //Esto es puramente por formalidad del TP

	log_info(logger, "%i%s", nuevoScript->idScript, ": NEW->READY");

	moverScript(nuevoScript->idScript, colaNEW, colaREADY);

	liberarRequest(nuevaRequest);

	sem_post(&sem_disponibleColaREADY);

	return 1;

}

void journal() //TODO: Semaforo?
{
	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* unaMemoria = list_get(memorias, i);
		enviarInt(OP_JOURNAL, unaMemoria->socket);
	}

	log_info(logger,
			"Se ha mandado el JOURNAL a todas las memorias conocidas.");
}

int ejecutarRequest(request* requestAEjecutar, script* elScript) {

	switch (requestAEjecutar->requestEnInt) {

	case JOURNAL: {
		journal();
		return 1;
	}
	case RUN: {
		return crearScript(requestAEjecutar);
	}
	case METRICS: {
		metrics(0);
		return 1;
	}

	case ADD: {
		return add(requestAEjecutar->parametros);
	}

	}

	if (unaMemoriaCualquiera() == -1) //No hay memorias
			{
		log_error(logger,
				"No hay memorias conectadas para ejecutar la request %i.",
				requestAEjecutar->requestEnInt);
		return -1;
	}

	int memoria;

	if (requestAEjecutar->requestEnInt == CREATE
			|| requestAEjecutar->requestEnInt == DESCRIBE) { //Preguntar esto
		memoria = unaMemoriaCualquiera();

	} else {

		memoria = determinarAQueMemoriaEnviar(requestAEjecutar);
	}

	if (memoria == -1) {
		log_error(logger,
				"No se conoce una memoria que pueda ejecutar la request %i.",
				requestAEjecutar->requestEnInt);
		return -1;
	}

	memoriaEnLista* laMemoria = list_get(memorias,
			encontrarPosicionDeMemoria(memoria));

	time_t tiempoInicial = time(NULL);

	sem_wait(&sem_cambioSleepEjecucion);
	int tiempoDeSleep = sleepEjecucion;
	sem_post(&sem_cambioSleepEjecucion);

	sleep(tiempoDeSleep);

	enviarRequestConHeaderEId(laMemoria->socket, requestAEjecutar, REQUEST,
			elScript->idScript);

	sem_wait(&elScript->semaforoDelScript);

	int respuesta = manejarRespuestaDeMemoria(elScript,requestAEjecutar,memoria);

	time_t tiempoFinal = time(NULL);

	insertarTiempo(tiempoInicial, tiempoFinal, requestAEjecutar->requestEnInt);

	return respuesta;
}

void metrics(int loggear) //TODO: Faltan los memory loads
{

	t_list* tiemposSelectAux = filterCasero_esMasNuevoQue30Segundos(
			tiemposSelect);
	t_list* tiemposInsertAux = filterCasero_esMasNuevoQue30Segundos(
			tiemposInsert);

	int promedioSelect = promedioDeTiemposDeOperaciones(tiemposSelectAux);
	int promedioInsert = promedioDeTiemposDeOperaciones(tiemposInsertAux);

	if (loggear) {

		log_info(logger, "\n\n----METRICS----");
		if (promedioSelect == -1) {
			log_info(logger, "\n\nRead latency: ---");
		} else {
			log_info(logger, "%s%i", "\n\nRead latency: ", promedioSelect);
		}

		if (promedioInsert == -1) {
			log_info(logger, "\n\nWrite latency: ---");
		} else {
			log_info(logger, "%s%i", "\n\nWrite latency: ", promedioInsert);
		}

		log_info(logger, "%s%i", "\n\nReads: ", list_size(tiemposSelectAux));
		log_info(logger, "%s%i", "\n\nWrites: ", list_size(tiemposInsertAux));

	} else {
		printf("\n\n----METRICS----");
		if (promedioSelect == -1) {
			printf("\n\nRead latency: ---");
		} else {
			printf("%s%i", "\n\nRead latency: ", promedioSelect);
		}

		if (promedioInsert == -1) {
			printf("\n\nWrite latency: ---");
		} else {
			printf("%s%i", "\n\nWrite latency: ", promedioInsert);
		}

		printf("%s%i", "\n\nReads: ", list_size(tiemposSelectAux));
		printf("%s%i", "\n\nWrites: ", list_size(tiemposInsertAux));
		printf("\n\n");
	}

	list_destroy(tiemposSelectAux);
	list_destroy(tiemposInsertAux);

	limpiarListasTiempos();

}

void status() {
	printf("\n\n----NEW----\n\n");
	mostrarListaScripts(colaNEW);
	printf("\n\n----READY----\n\n");
	mostrarListaScripts(colaREADY);
	printf("\n\n----EXEC----\n\n");
	mostrarListaScripts(listaEXEC);
	printf("\n\n----EXIT----\n\n");
	mostrarListaScripts(listaEXIT);
	printf("\n\n");
}

int add(char* chocloDeCosas) {

	char** consistenciaYMemoriaEnArray = string_n_split(chocloDeCosas, 4, " ");

	int nombreMemoria = atoi(consistenciaYMemoriaEnArray[1]);
	int consistencia = queConsistenciaEs(consistenciaYMemoriaEnArray[3]);

	int posicionMemoria = encontrarPosicionDeMemoria(nombreMemoria);

	if (posicionMemoria == -1) {
		printf("%s%i%s", "No se pudo encontrar la memoria ", nombreMemoria,
				"\n");
		return -1;
	}
	memoriaEnLista* memoria = list_get(memorias, posicionMemoria);

//TODO: Semaforo para cuando se usa?
	memoria->consistencias[consistencia] = consistencia; //La consistencia esta en la misma posicion del numero que lo representa (0,1 o 2);

//TODO: Semaforo???
	if ((consistencia == EC) && (proximaMemoriaEC == -1)) {
		proximaMemoriaEC = memoria->nombre;
	}

	log_info(logger, "Se agrego la memoria %i al criterio %s", nombreMemoria,
			consistenciaYMemoriaEnArray[3]);

	liberarArrayDeStrings(consistenciaYMemoriaEnArray);

	return 1;

}

void refreshMetadatas() {

	request* requestMetadata = crearStructRequest("DESCRIBE");

	while (1) {
		t_config* config = config_create(DIRCONFIG);
		int intervaloDeRefresh = config_get_int_value(config,
				"METADATA_REFRESH");
		config_destroy(config);

		sleep(intervaloDeRefresh);

		int memoria = unaMemoriaCualquiera();

		if (memoria == -1) {
			continue;
		}

		memoriaEnLista* laMemoria = list_get(memorias,
				encontrarPosicionDeMemoria(memoria));

		enviarRequestConHeaderEId(laMemoria->socket, requestMetadata,
		REQUEST, scriptRefreshMetadata->idScript);

		t_list* metadatas;

		sem_wait(&scriptRefreshMetadata->semaforoDelScript);

		memcpy(&metadatas,
				scriptRefreshMetadata->resultadoDeEnvio + sizeof(int),
				sizeof(t_list*));

		actualizarMetadatas(metadatas);

	}

}

void refreshSleep() { //TODO: Arreglar esto
	int elConfig = inotify_init();

	inotify_add_watch(elConfig, DIRCONFIG, IN_MODIFY);

	int bufferSize = sizeof(struct inotify_event);

	while (1) {

		struct inotify_event* descriptor = malloc(sizeof(struct inotify_event));

		read(elConfig, descriptor, bufferSize);

		t_config* config = config_create(DIRCONFIG);

		sem_wait(&sem_cambioSleepEjecucion);

		sleepEjecucion = config_get_int_value(config, "SLEEP_EJECUCION");

		sem_post(&sem_cambioSleepEjecucion);

		config_destroy(config);

		log_info(logger, "Se cambio el sleep");

		free(descriptor);

	}

}

