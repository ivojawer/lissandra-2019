#include "funcionesKernel.h"

extern t_list* colaNEW;
extern t_list* colaREADY;
extern t_list* listaTablas;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern int idInicial;
extern sem_t sem_cambioId;
extern sem_t sem_disponibleColaREADY;
sem_t sem_multiprocesamiento;
extern t_log* logger;



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

		if ( (crearArchivoParaRequest(nuevoScript,nuevaRequest)) == -1)
		{
			log_error(logger,"Hubo un error en la creacion del request");

			moverScript(nuevoScript->idScript,colaNEW,listaEXIT);
			return;

		}

	}
	moverScript(nuevoScript->idScript,colaNEW,colaREADY);

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

		if (!existeArchivo(requestAEjecutar->parametros)) {

			return -1;
		}
		crearScript(requestAEjecutar);

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

	//sleep(2); //No olvidar sacar esto jesus NO OLVIDAR

	//int criterio = criterioDeTabla(devolverTablaDeRequest(request));

	//por aca enviar :)

	return 1;
}

void matarTodo() //TODO
{
}

void metrics() //TODO
{

}

void status() //TODO
{
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
