#include "funcionesKernel.h"

extern sem_t sem_disponibleColaREADY;
extern sem_t sem_multiprocesamiento;
extern t_list* colaREADY;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern int quantum;
extern t_log* logger;

void planificadorREADYAEXEC() {

	while (1) {

		//Si no puede pasar de READY a EXIT entonces esto va bien

		sem_wait(&sem_disponibleColaREADY);
		sem_wait(&sem_multiprocesamiento);

		script* scriptAExec = list_get(colaREADY, 0); //El 0 siempre va a ser el mas viejo en la lista

		moverScript(scriptAExec->idScript, colaREADY, listaEXEC);

		pthread_t h_EXEC;
		pthread_create(&h_EXEC, NULL, (void *) planificadorEXEC,
				scriptAExec->idScript);
		pthread_detach(h_EXEC);

	}

}

void planificadorEXEC(int IdScript) {

	int index = encontrarScriptEnLista(IdScript, listaEXEC);

	script* scriptEXEC = list_get(listaEXEC, index);
	for (int i = 0; i < quantum; i++) {

		char* linea = leerLinea(scriptEXEC->direccionScript,
				scriptEXEC->lineasLeidas);

		if (!strcmp(linea, "error")) {
			log_error(logger, "%s%i",
					"Hubo un error abriendo el archivo del script ",
					scriptEXEC->idScript);
			moverScript(scriptEXEC->idScript, listaEXEC, listaEXIT);
			free(linea);
			break;
		}

		int resultado = ejecutarRequest(linea);

		if (resultado == -1) {
			log_error(logger, "%s%i",
					"Hubo un error en la ejecucion del script ",
					scriptEXEC->idScript);
			moverScript(scriptEXEC->idScript, listaEXEC, listaEXIT);
			free(linea);
			break;
		}


		scriptEXEC->lineasLeidas++;

		char* proximaLinea = leerLinea(scriptEXEC->direccionScript,
				scriptEXEC->lineasLeidas+1);

		if (!strcmp(proximaLinea,"fin"))
		{
			log_info(logger,"%s%i","Termino de ejecutar exitosamente el script ",IdScript);
			moverScript(scriptEXEC->idScript, listaEXEC, listaEXIT);
			break;
		}

		free(proximaLinea);
		free(linea);

		//Semaforo por si cambia el quantum?
	}

	sem_post(&sem_multiprocesamiento);

}
