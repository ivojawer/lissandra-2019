#include "funcionesKernel.h"

extern t_list* colaNEW;
extern t_list* colaREADY;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern int idInicial;
extern sem_t sem_cambioId;
extern sem_t sem_disponibleColaREADY;
extern t_log* logger;
extern t_list* tiemposInsert;
extern t_list* tiemposSelect;
extern t_list* memorias;
extern int proximaMemoriaEC;

void crearScript(request* nuevaRequest) {

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
			return;

		}

	}

	list_add(colaNEW, nuevoScript); //Esto es puramente por formalidad del TP

	log_info(logger, "%i%s", nuevoScript->idScript, ": NEW->READY");

	moverScript(nuevoScript->idScript, colaNEW, colaREADY);

	liberarRequest(nuevaRequest);

	sem_post(&sem_disponibleColaREADY);

}

int ejecutarRequest(request* requestAEjecutar) {

//	if (unaMemoriaCualquiera() == -1) //No hay memorias
//			{
//		return -1;
//	}
//
//	if (esDescribeGlobal(requestAEjecutar)) {
//		enviarRequestAMemoria(requestAEjecutar, unaMemoriaCualquiera());
//	}
//
//	int criterio = criterioDeTabla(devolverTablaDeRequest(requestAEjecutar));
//
//	if (criterio == -1) //No existe la tabla (TODO: Ver si esto se hace antes o que, tambien lo del CREATE)
//			{
//		return -1;
//	}
//
//	int memoria = determinarAQueMemoriaEnviar(criterio);
//
//	if (memoria == -1) // No existe la memoria, lo mismo que arriba
//			{
//		return -1;
//	}

	time_t tiempoInicial = time(NULL);

//	enviarRequestAMemoria(requestAEjecutar, memoria);
//
//
//	int resultado = recibirRespuestaDeMemoria(memoria);
	sleep(1);
//
//	if (resultado == MEMORIA_ERROR) {
//		return -1;
//	}

	time_t tiempoFinal = time(NULL);

	insertarTiempo(tiempoInicial, tiempoFinal, requestAEjecutar->requestEnInt);

	return 1;
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
			log_info(logger,"\n\nRead latency: ---");
		} else {
			log_info(logger,"%s%i", "\n\nRead latency: ", promedioSelect);
		}

		if (promedioInsert == -1) {
			log_info(logger,"\n\nWrite latency: ---");
		} else {
			log_info(logger,"%s%i", "\n\nWrite latency: ", promedioInsert);
		}

		log_info(logger,"%s%i", "\n\nReads: ", list_size(tiemposSelectAux));
		log_info(logger,"%s%i", "\n\nWrites: ", list_size(tiemposInsertAux));



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

void add(char* consistenciaYMemoriaEnString) {

	char** consistenciaYMemoriaEnArray = string_n_split(
			consistenciaYMemoriaEnString, 2, " ");

	int consistencia = atoi(consistenciaYMemoriaEnArray[0]);
	int nombreMemoria = atoi(consistenciaYMemoriaEnArray[1]);

	liberarArrayDeStrings(consistenciaYMemoriaEnArray);

	int posicionMemoria = encontrarPosicionDeMemoria(nombreMemoria);

	if (posicionMemoria == -1) {
		printf("%s%i%s", "No se pudo encontrar la memoria ", nombreMemoria,
				"\n");
		return;
	}

	memoriaEnLista* memoria = list_get(memorias, posicionMemoria);

	//TODO: Semaforo para cuando se usa?
	memoria->consistencias[consistencia] = consistencia; //La consistencia esta en la misma posicion del numero que lo representa (0,1 o 2);

	//TODO: Semaforo???
	if ((consistencia == EC) && (proximaMemoriaEC == -1)) {
		proximaMemoriaEC = memoria->nombre;
	}

}

