#include "funcionesKernel.h"

extern t_list* colaNEW;
extern t_list* colaREADY;
extern t_list* listaTablas;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern int idInicial;
extern sem_t sem_cambioId;
extern sem_t sem_disponibleColaREADY;
extern sem_t sem_multiprocesamiento;
extern t_log* logger;
extern t_list* tiemposInsert;
extern t_list* tiemposSelect;

void crearScript(request* nuevaRequest) {
	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	script* nuevoScript = malloc(sizeof(script));

	sem_wait(&sem_cambioId);
	nuevoScript->idScript = idInicial;
	idInicial++;
	sem_post(&sem_cambioId);

	list_add(colaNEW, nuevoScript); //Esto es puramente por formalidad del TP

	nuevoScript->lineasLeidas = 0;

	if (nuevaRequest->requestEnInt == RUN) {

		char* direccion = scriptConRaiz(nuevaRequest->parametros);

		nuevoScript->direccionScript = malloc(sizeof(direccion));

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

	log_info(logger,"%i%s",nuevoScript->idScript,": NEW->READY");

	moverScript(nuevoScript->idScript, colaNEW, colaREADY);

	liberarRequest(nuevaRequest);

	sem_post(&sem_disponibleColaREADY);

}

int ejecutarRequest(request* requestAEjecutar) {

	switch (requestAEjecutar->requestEnInt) {

	case JOURNAL: {
		;
		//Hacer el journal
		return -1;
	}

	case RUN: {
		;
		//Preguntar
		return 1;
	}

	case METRICS: {
		;
		metrics();
		return 1;
	}

	case ADD: {
		;
		//Hacer el add
		return -1;
	}

	}

	if (esDescribeGlobal(requestAEjecutar)) {
		//Hacer el describe global
		return -1;
	}

	//int criterio = criterioDeTabla(devolverTablaDeRequest(request));

	time_t tiempoInicial = time(NULL);

	sleep(2); //No olvidar sacar esto jesus NO OLVIDAR

	//Esperar y eso a la respuesta

	time_t tiempoFinal = time(NULL);

	insertarTiempo(tiempoInicial, tiempoFinal, requestAEjecutar->requestEnInt);

	return 1;
}

void matarTodo() //TODO
{
}

void metrics() //TODO: Faltan los memory loads
{

	t_list* tiemposSelectAux = filterCasero_esMasNuevoQue30Segundos(tiemposSelect);
	t_list* tiemposInsertAux = filterCasero_esMasNuevoQue30Segundos(tiemposInsert);

	int promedioSelect = promedioDeTiemposDeOperaciones(tiemposSelectAux);
	int promedioInsert = promedioDeTiemposDeOperaciones(tiemposInsertAux);

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

	list_destroy(tiemposSelectAux);
	list_destroy(tiemposInsertAux);

	//limpiarListasTiempos();

}

void loggearMetrics() //TODO
{

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
