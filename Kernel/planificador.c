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

		log_info(logger, "%i%s", scriptAExec->idScript, ": READY->EXEC");

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


	for (int i = 0; i < quantum; i++) { //TODO: Ver el tema del quantum, como sigue planificando cuando cambia

		char* linea = leerLinea(scriptEXEC->direccionScript,
				scriptEXEC->lineasLeidas);

		if (!strcmp(linea, "error")) {
			log_error(logger, "%s%i",
					"Hubo un error abriendo el archivo del script ",
					scriptEXEC->idScript);

			free(linea);
			log_info(logger, "%i%s", scriptEXEC->idScript, ": EXEC->EXIT");
			moverScript(scriptEXEC->idScript, listaEXEC, listaEXIT);

			sem_post(&sem_multiprocesamiento);

			if (scriptEXEC->esPorConsola) {
				remove(scriptEXEC->direccionScript);
			}

			return;
		}

		request* requestAEjecutar = crearStructRequest(linea);

		int resultado = ejecutarRequest(requestAEjecutar);

		if (resultado == -1) { //TODO: Expandir los errores

			log_error(logger, "%s%i",
					"Hubo un error en la ejecucion del script ",
					scriptEXEC->idScript);

			liberarRequest(requestAEjecutar);
			free(linea);

			log_info(logger, "%i%s", scriptEXEC->idScript, ": EXEC->EXIT");
			moverScript(scriptEXEC->idScript, listaEXEC, listaEXIT);
			sem_post(&sem_multiprocesamiento);

			if (scriptEXEC->esPorConsola) {
				remove(scriptEXEC->direccionScript);
			}

			return;
		}

		char* proximaLinea = leerLinea(scriptEXEC->direccionScript,
				scriptEXEC->lineasLeidas + 1);

		scriptEXEC->lineasLeidas++;

		if (!strcmp(proximaLinea, "fin")) {
			log_info(logger, "%s%i",
					"Termino de ejecutar exitosamente el script ", IdScript);

			free(proximaLinea); //Aca se solia romper ok
			free(linea);
			liberarRequest(requestAEjecutar);

			log_info(logger, "%i%s", scriptEXEC->idScript, ": EXEC->EXIT");
			moverScript(scriptEXEC->idScript, listaEXEC, listaEXIT);
			sem_post(&sem_multiprocesamiento);

			if (scriptEXEC->esPorConsola) {
				remove(scriptEXEC->direccionScript);
			}

			return;

		}

		free(proximaLinea); //Aca se solia romper ok
		free(linea);

		liberarRequest(requestAEjecutar);

		//TODO: Semaforo por si cambia el quantum?
	}

	log_info(logger, "%i%s", scriptEXEC->idScript, ": EXEC->READY");
	moverScript(scriptEXEC->idScript, listaEXEC, colaREADY);

	sem_post(&sem_disponibleColaREADY);
	sem_post(&sem_multiprocesamiento);

}
