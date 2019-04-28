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



void crearScript(char* request) {
	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	script* nuevoScript = malloc(sizeof(script));

	sem_wait(&sem_cambioId);
	nuevoScript->idScript = idInicial;
	idInicial++;
	sem_post(&sem_cambioId);

	list_add(colaNEW, nuevoScript); //Esto es puramente por formalidad del TP

	nuevoScript->lineasLeidas = 0;

	if (devolverSoloRequest(request) == RUN) {

		char* direccion = scriptConRaiz(devolverSoloParametros(request));

		nuevoScript->direccionScript = malloc(sizeof(direccion));

		nuevoScript->direccionScript = direccion;
		nuevoScript->esPorConsola = 0;
	}

	else {

		nuevoScript->esPorConsola = 1;

		if ( (crearArchivoParaRequest(nuevoScript,request)) == -1)
		{
			log_error(logger,"Hubo un error en la creacion del request");

			moverScript(nuevoScript->idScript,colaNEW,listaEXIT);
			return;

		}

	}
	moverScript(nuevoScript->idScript,colaNEW,colaREADY);

	free(request);

	sem_post(&sem_disponibleColaREADY);

}


int ejecutarRequest(char* request) {

	int requestEnInt = devolverSoloRequest(request);

	switch (requestEnInt) {

	case JOURNAL: {
		;
		//Hacer el journal
		return -1;
	}

	case RUN: {
		;
		char** requestYParametros = string_split(request, " ");

		if (!existeArchivo(requestYParametros[1])) {

			liberarArrayDeStrings(requestYParametros);
			return -1;
		}
		crearScript(request);

		liberarArrayDeStrings(requestYParametros);
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

	if (esDescribeGlobal(request)) {
		//Hacer el describe global
		return -1;
	}

	sleep(2); //No olvidar sacar esto jesus NO OLVIDAR

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
