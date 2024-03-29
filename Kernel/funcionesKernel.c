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
extern sem_t sem_cambioMemoriaEC;
extern sem_t sem_borradoMemoria;
extern sem_t sem_refreshConfig;
extern sem_t sem_operacionesTotales;
extern t_log* logger;
extern t_log* loggerSigiloso;
extern int idInicial;
extern int proximaMemoriaEC;
extern int sleepEjecucion;
extern int quantum;
extern int sleepEjecucion;
extern int intervaloDeRefreshMetadata;
extern int operacionesTotales;
extern int retardoGossiping;
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

//			moverScript(nuevoScript->idScript, colaNEW, listaEXIT);
			return -1;

		}

	}

	sem_init(&nuevoScript->semaforoDelScript, 0, 0);

	list_add(colaNEW, nuevoScript); //Esto es puramente por formalidad del TP

	char* textoALoggear = string_new();
	string_append(&textoALoggear, string_itoa(nuevoScript->idScript));
	string_append(&textoALoggear, ": NEW->READY");
	loggearCyanClaro(logger, textoALoggear);
	free(textoALoggear);

	moverScript(nuevoScript->idScript, colaNEW, colaREADY);

	liberarRequest(nuevaRequest);

	sem_post(&sem_disponibleColaREADY);

	return 1;

}

void journal() {

	sem_wait(&sem_borradoMemoria);

	sem_wait(&sem_operacionesTotales);
	operacionesTotales++;
	sem_post(&sem_operacionesTotales);

	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (laMemoriaTieneConsistencias(unaMemoria)) //TODO: Marca, va a todas
				{

			if (unaMemoria->estaViva) {
				enviarInt(unaMemoria->socket, OP_JOURNAL);
				sem_wait(&sem_operacionesTotales);
				unaMemoria->estadisticas.operacionesTotalesEnMemoria++;
				sem_post(&sem_operacionesTotales);

			}

		}
	}

	sem_post(&sem_borradoMemoria);

	loggearAzulClaro(logger,
			"Se mando JOURNAL a todas las memorias conocidas.");
}

int ejecutarRequest(request* requestAEjecutar, script* elScript) {

	sem_wait(&sem_refreshConfig);
	int tiempoDeSleep = sleepEjecucion / 1000;
	sem_post(&sem_refreshConfig);

	sleep(tiempoDeSleep); //TODO: Marca, el sleep esta al principio de la ejecucion

	char* textoALoggear = string_new();
	string_append(&textoALoggear, "Se va a ejecutar la request ");
	string_append(&textoALoggear, requestStructAString(requestAEjecutar));
	loggearCyanClaro(logger, textoALoggear);
	free(textoALoggear);

	switch (requestAEjecutar->requestEnInt) {

	case JOURNAL: {
		journal();
		return TODO_BIEN;
	}
	case RUN: {
		crearScript(requestAEjecutar);
		return TODO_BIEN;
	}
	case METRICS: {
		metrics(0);
		return TODO_BIEN;
	}

	case ADD: {
		add(requestAEjecutar->parametros);
		return TODO_BIEN;
	}
	}

	if (requestAEjecutar->requestEnInt == DROP
			|| requestAEjecutar->requestEnInt == SELECT
			|| requestAEjecutar->requestEnInt == INSERT) {

		char** parametros = string_split(requestAEjecutar->parametros, " ");
		char* tabla = string_duplicate(parametros[0]);

		liberarArrayDeStrings(parametros);

		if (!existeTabla(tabla)) {
			log_error(logger, "No se encontro la tabla %s.", tabla);
			free(tabla);
			return ERROR;
		}
		free(tabla);

	}

	if (requestAEjecutar->requestEnInt == CREATE) {
		char** parametros = string_split(requestAEjecutar->parametros, " ");
		char* tabla = string_duplicate(parametros[0]);

		liberarArrayDeStrings(parametros);

		if (existeTabla(tabla)){

			log_error(logger, "La tabla %s ya existe.", tabla);
			free(tabla);
			return TODO_BIEN;
		}

		free(tabla);

	}

	sem_wait(&sem_borradoMemoria);

	if (unaMemoriaCualquiera() == -1) //No hay memorias
			{
		log_error(logger,
				"No hay memorias conectadas para ejecutar la request %s.",
				requestStructAString(requestAEjecutar));
		sem_post(&sem_borradoMemoria);
		return TODO_BIEN; //TODO: Marca, Si no hay memorias conectadas sigue la ejecucion
	}

	int memoria;

	if (requestAEjecutar->requestEnInt == CREATE
			|| requestAEjecutar->requestEnInt == DESCRIBE) { //TODO: Marca, create y describe van a cualquiera
		memoria = unaMemoriaCualquiera();

	} else {

		memoria = determinarAQueMemoriaEnviar(requestAEjecutar);
	}

	if (memoria == -1) {
		log_error(logger,
				"No se conoce una memoria que pueda ejecutar la request %s.",
				requestStructAString(requestAEjecutar));
		sem_post(&sem_borradoMemoria);
		return TODO_BIEN; //TODO: Marca, Si no hay memorias conectadas sigue la ejecucion
	}

	memoriaEnLista* laMemoria = list_get(memorias,
			encontrarPosicionDeMemoria(memoria));

	time_t tiempoInicial = time(NULL);

	if (!laMemoria->estaViva) {
		time_t tiempoFinal = time(NULL);

		insertarTiempo(tiempoInicial, tiempoFinal,
				requestAEjecutar->requestEnInt);
		sem_post(&sem_borradoMemoria);
		return TODO_BIEN; //TODO: Marca, si la memoria muere sigue la ejecucion.
	}

	enviarRequestConHeaderEId(laMemoria->socket, requestAEjecutar, REQUEST,
			elScript->idScript);

	sem_wait(&sem_operacionesTotales);
	operacionesTotales++;
	laMemoria->estadisticas.operacionesTotalesEnMemoria++;
	sem_post(&sem_operacionesTotales);

	list_add(laMemoria->scriptsEsperando, &elScript->idScript);

	sem_post(&sem_borradoMemoria);

	sem_wait(&elScript->semaforoDelScript);

	sem_wait(&laMemoria->sem_cambioScriptsEsperando);
	sacarScriptDeEspera(elScript->idScript, laMemoria);
	sem_post(&laMemoria->sem_cambioScriptsEsperando);

	int respuesta = manejarRespuestaDeMemoria(elScript, requestAEjecutar,
			memoria);

	respuesta = TODO_BIEN; //Si se llego a mandar la request, entonces no puede cortar el script

	if (requestAEjecutar->requestEnInt == DROP) {
		removerUnaMetadata(requestAEjecutar->parametros);

	}

	time_t tiempoFinal = time(NULL);

	insertarTiempo(tiempoInicial, tiempoFinal, requestAEjecutar->requestEnInt);

	return respuesta;
}

void metrics(int loggear) {

	t_list* tiemposSelectAux = filterCasero_esMasNuevoQue30Segundos(
			tiemposSelect);
	t_list* tiemposInsertAux = filterCasero_esMasNuevoQue30Segundos(
			tiemposInsert);

	float promedioSelect = promedioDeTiemposDeOperaciones(tiemposSelectAux);
	float promedioInsert = promedioDeTiemposDeOperaciones(tiemposInsertAux);

	if (loggear) {

		log_info(loggerSigiloso, "----METRICS----");
		if (promedioSelect == -1) {
			log_info(loggerSigiloso, "Read latency: ---");
		} else {
			log_info(loggerSigiloso, "Read latency: %f", promedioSelect);
		}

		if (promedioInsert == -1) {
			log_info(loggerSigiloso, "Write latency: ---");
		} else {
			log_info(loggerSigiloso, "Write latency: %f", promedioInsert);
		}

		log_info(loggerSigiloso, "Reads: %i", list_size(tiemposSelectAux));
		log_info(loggerSigiloso, "Writes: %i", list_size(tiemposInsertAux));

		sem_wait(&sem_operacionesTotales);
		log_info(loggerSigiloso, "Operaciones totales: %i", operacionesTotales);
		sem_post(&sem_operacionesTotales);

		sem_wait(&sem_borradoMemoria);

		for (int i = 0; i < list_size(memorias); i++) {
			memoriaEnLista* unaMemoria = list_get(memorias, i);

			if (unaMemoria->estaViva) {
				log_info(loggerSigiloso,
						"--Memoria %i-- \nInserts Completos: %i \nInserts Fallidos: %i \nSelects Completos: %i \nSelects Fallidos: %i \nOperaciones totales: %i",
						unaMemoria->nombre,
						unaMemoria->estadisticas.insertsCompletos,
						unaMemoria->estadisticas.insertsFallidos,
						unaMemoria->estadisticas.selectsCompletos,
						unaMemoria->estadisticas.selectsFallidos,
						unaMemoria->estadisticas.operacionesTotalesEnMemoria);
			}

		}

		sem_post(&sem_borradoMemoria);

	} else {
		printf("\n\n----METRICS----");
		if (promedioSelect == -1) {
			printf("\nRead latency: ---");
		} else {
			printf("\nRead latency: %f", promedioSelect);
		}

		if (promedioInsert == -1) {
			printf("\nWrite latency: ---");
		} else {
			printf("\nWrite latency: %f", promedioInsert);
		}

		printf("\nReads: %i", list_size(tiemposSelectAux));
		printf("\nWrites: %i", list_size(tiemposInsertAux));

		sem_wait(&sem_operacionesTotales);
		printf("\nOperaciones totales: %i", operacionesTotales);
		sem_post(&sem_operacionesTotales);

		sem_wait(&sem_borradoMemoria);
		for (int i = 0; i < list_size(memorias); i++) {
			memoriaEnLista* unaMemoria = list_get(memorias, i);

			if (unaMemoria->estaViva) {
				printf(
						"\n\n--Memoria %i-- \nInserts Completos: %i \nInserts Fallidos: %i \nSelects Completos: %i \nSelects Fallidos: %i \nOperaciones totales: %i",
						unaMemoria->nombre,
						unaMemoria->estadisticas.insertsCompletos,
						unaMemoria->estadisticas.insertsFallidos,
						unaMemoria->estadisticas.selectsCompletos,
						unaMemoria->estadisticas.selectsFallidos,
						unaMemoria->estadisticas.operacionesTotalesEnMemoria);
			}
		}
		sem_post(&sem_borradoMemoria);
		printf("\n\n");

	}

	list_destroy(tiemposSelectAux);
	list_destroy(tiemposInsertAux);

	limpiarListasTiempos();
}

void metricsAutomatico() {

	while (1) {
		sleep(30);
		metrics(1);
	}
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
	printf("\n\n------------\n\n");
	sem_wait(&sem_refreshConfig);
	printf("QUANTUM: %i\n", quantum);
	printf("SLEEP EJECUCION: %i\n", sleepEjecucion);
	printf("REFRESH METADATA: %i\n", intervaloDeRefreshMetadata);
	sem_post(&sem_refreshConfig);
	printf("Memorias conectadas: ");
	sem_wait(&sem_borradoMemoria);

	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->estaViva) {
			printf("%i ", unaMemoria->nombre);
		}

	}
	sem_post(&sem_borradoMemoria);
	printf("\n\n");
}

int add(char* chocloDeCosas) {

	char** consistenciaYMemoriaEnArray = string_n_split(chocloDeCosas, 4, " ");

	int nombreMemoria = atoi(consistenciaYMemoriaEnArray[1]);
	int consistencia = queConsistenciaEs(consistenciaYMemoriaEnArray[3]);

	sem_wait(&sem_borradoMemoria);

	int posicionMemoria = encontrarPosicionDeMemoria(nombreMemoria);

	if (posicionMemoria == -1) {
		printf("%s%i%s", "No se pudo encontrar la memoria ", nombreMemoria,
				"\n");
		liberarArrayDeStrings(consistenciaYMemoriaEnArray);
		sem_post(&sem_borradoMemoria);

		return -1;
	}
	memoriaEnLista* memoria = list_get(memorias, posicionMemoria);

	if (!memoria->estaViva) {
		liberarArrayDeStrings(consistenciaYMemoriaEnArray);
		sem_post(&sem_borradoMemoria);
		return -1;
	}

	if (memoria->consistencias[consistencia] == consistencia) //Si ya estaba asociada al criterio
			{
		log_info(logger, "La memoria %i ya estaba asociada al criterio %s",
				nombreMemoria, consistenciaYMemoriaEnArray[3]);

		liberarArrayDeStrings(consistenciaYMemoriaEnArray);
		sem_post(&sem_borradoMemoria);
		return 1;
	}
	memoria->consistencias[consistencia] = consistencia; //La consistencia esta en la misma posicion del numero que lo representa (0,1 o 2);

	if (consistencia == SHC) { //Journal a todas las SHC

		for (int i = 0; i < list_size(memorias); i++) {

			memoriaEnLista* unaMemoria = list_get(memorias, i);

			if (unaMemoria->estaViva && unaMemoria->consistencias[SHC] == SHC) {

				enviarInt(unaMemoria->socket, OP_JOURNAL);

				sem_wait(&sem_operacionesTotales);
				unaMemoria->estadisticas.operacionesTotalesEnMemoria++;
				operacionesTotales++;
				sem_post(&sem_operacionesTotales);

			}

		}
	}

	if ((consistencia == EC) && (proximaMemoriaEC == -1)) {
		sem_wait(&sem_cambioMemoriaEC);
		proximaMemoriaEC = memoria->nombre;
		sem_post(&sem_cambioMemoriaEC);
	}

	log_info(logger, "Se agrego la memoria %i al criterio %s", nombreMemoria,
			consistenciaYMemoriaEnArray[3]);

	liberarArrayDeStrings(consistenciaYMemoriaEnArray);

	sem_post(&sem_borradoMemoria);

	return 1;

}

void refreshMetadatas() {

	request* requestMetadata = crearStructRequest("DESCRIBE");

	while (1) {

		sem_wait(&sem_refreshConfig);
		int intervaloDeRefresh = intervaloDeRefreshMetadata / 1000; //TODO: Marca, se considera milisegundos
		sem_post(&sem_refreshConfig);

		sleep(intervaloDeRefresh);
		sem_wait(&sem_borradoMemoria);

		int memoria = unaMemoriaCualquiera();

		if (memoria == -1) {
			sem_post(&sem_borradoMemoria);
			continue;
		}
		loggearAmarillo(logger, "Se va a ejecutar el DESCRIBE automatico.");

		memoriaEnLista* laMemoria = list_get(memorias,
				encontrarPosicionDeMemoria(memoria));

		enviarRequestConHeaderEId(laMemoria->socket, requestMetadata,
		REQUEST, scriptRefreshMetadata->idScript);

		sem_post(&sem_borradoMemoria);

		t_list* metadatas;

		sem_wait(&scriptRefreshMetadata->semaforoDelScript);

		int respuesta;
		memcpy(&respuesta, scriptRefreshMetadata->resultadoDeEnvio,
				sizeof(int));

		if (respuesta == ERROR) {
			log_error(logger, "No se pudo realizar el DESCRIBE automatico");
			continue;
		}

		memcpy(&metadatas,
				scriptRefreshMetadata->resultadoDeEnvio + sizeof(int),
				sizeof(t_list*));

		actualizarMetadatas(metadatas);

		loggearAmarillo(logger, "Se pudo ejecutar el DESCRIBE automatico.");
//		log_info(logger,"Se actualizaron automaticamente las metadatas");

	}

}

void refreshConfig() {

	while (1) {
		esperarModificacionDeArchivo(DIRCONFIG);

		sem_wait(&sem_refreshConfig);

		t_config* config = config_create(DIRCONFIG);

		quantum = config_get_int_value(config, "QUANTUM");
		sleepEjecucion = config_get_int_value(config, "SLEEP_EJECUCION");
		intervaloDeRefreshMetadata = config_get_int_value(config,
				"METADATA_REFRESH");
		retardoGossiping = config_get_int_value(config, "GOSSIP_SLEEP");

		config_destroy(config);

		sem_post(&sem_refreshConfig);

//		log_info(logger, "hice algo");
	}

}

void gossipingAutomatico() {
	while (1) {
		sem_wait(&sem_refreshConfig);
		int retardoGossip = retardoGossiping; //En segundos
		sem_post(&sem_refreshConfig);

		sleep(retardoGossip);
//		log_info(logger,"Se va a empezar a hacer el GOSSIPING");
		conectarseASeedsDesconectadas();

		enviarPeticionesDeGossip();
	}
}

void matarListas() {

	list_destroy(tiemposInsert);
	list_destroy(tiemposSelect);

	matarListaScripts(listaEXIT);
	matarListaScripts(listaEXEC);
	matarListaScripts(colaREADY);
	matarListaScripts(colaNEW);

}

