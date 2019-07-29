#include "funcionesBaseKernel.h"

extern t_log* logger;
extern t_list* listaTablas;
extern t_list* listaTablas;
extern t_list* listaEXEC;
extern t_list* memorias;
extern t_list* seedsMemorias;
extern t_list* seedsSiendoCreadas;
extern sem_t sem_actualizacionMetadatas;
extern sem_t sem_borradoMemoria;
extern sem_t sem_cambioMemoriaEC;
extern sem_t sem_seedSiendoCreada;
extern sem_t sem_operacionesTotales;
extern int proximaMemoriaEC;
extern int operacionesTotales;

int manejarRespuestaDeMemoria(script* elScript, request* laRequest, int memoria) {

	void loggearErrorDeEjecucion(int tipoRespuesta) {
		if (tipoRespuesta == NO_EXISTE) {
			log_error(logger,
					"%s: No se pudo encontrar el dato. (Enviado a %i)",
					requestStructAString(laRequest), memoria);
		} else if (tipoRespuesta == ERROR) {
			log_error(logger, "%s: No se pudo realizar. (Enviado a %i)",
					requestStructAString(laRequest), memoria);
		} else if (tipoRespuesta == TABLA_NO_EXISTE) {
			log_error(logger, "%s: La tabla ya no existe. (Enviado a %i)",
					requestStructAString(laRequest), memoria);

		}
	}


	int respuesta;
	memcpy(&respuesta, elScript->resultadoDeEnvio, sizeof(int));

	switch (respuesta) {
	case ERROR:
	case NO_EXISTE:
	case TABLA_NO_EXISTE: {

		agregarOperacionFallidaAMemoria(memoria, laRequest->requestEnInt);

		loggearErrorDeEjecucion(respuesta);

		if (respuesta == TABLA_NO_EXISTE) {

			removerMetadataDeUnRequest(laRequest);

		}
		break;
	}

	case MEM_LLENA: {
		char* textoALoggear = string_new();
		string_append(&textoALoggear, "La memoria ");
		string_append(&textoALoggear, string_itoa(memoria));
		string_append(&textoALoggear,
				" esta llena, se va a enviar un JOURNAL.");
		loggearAzulClaro(logger, textoALoggear);
		free(textoALoggear);

		journalAUnaMemoria(memoria);

		sem_wait(&sem_borradoMemoria);

		int index = encontrarPosicionDeMemoria(memoria);

		if (index == -1) {
			sem_post(&sem_borradoMemoria);
			free(elScript->resultadoDeEnvio);
			return ERROR;
		}

		memoriaEnLista* laMemoria = list_get(memorias,
				encontrarPosicionDeMemoria(memoria));

		enviarRequestConHeaderEId(laMemoria->socket, laRequest, REQUEST,
				elScript->idScript);

		//TODO: Marca, el JOURNAL intermedio cuenta como una operacion mas (Esta suma de operacion es el JOURNAL, no de la request enviada).
		sem_wait(&sem_operacionesTotales);
		operacionesTotales++;
		sem_post(&sem_operacionesTotales);

		laMemoria->estadisticas.operacionesTotalesEnMemoria++;

		list_add(laMemoria->scriptsEsperando, &elScript->idScript);

		sem_post(&sem_borradoMemoria);

		sem_wait(&elScript->semaforoDelScript);

		sem_wait(&laMemoria->sem_cambioScriptsEsperando);
		sacarScriptDeEspera(elScript->idScript, laMemoria);
		sem_post(&laMemoria->sem_cambioScriptsEsperando);

		respuesta = manejarRespuestaDeMemoria(elScript, laRequest, memoria);

		elScript->resultadoDeEnvio = malloc(1); //Para que no rompa en el free de despues
		break;
	}
	default: {
		switch (laRequest->requestEnInt) {
		case SELECT: {

			int tamanioString;

			memcpy(&tamanioString, elScript->resultadoDeEnvio + sizeof(int),
					sizeof(int));

			char* resultadoSelect = malloc(tamanioString);

			memcpy(resultadoSelect,
					elScript->resultadoDeEnvio + sizeof(int) + sizeof(int),
					tamanioString);

			char* textoALoggear = string_new();
			string_append(&textoALoggear, requestStructAString(laRequest));
			string_append(&textoALoggear, ": ");
			string_append(&textoALoggear, resultadoSelect);
			string_append(&textoALoggear, "(Enviado a ");
			string_append(&textoALoggear, string_itoa(memoria));
			string_append(&textoALoggear, ")");
			loggearVerde(logger, textoALoggear);
			free(textoALoggear);

			agregarSelectCompletoAMemoria(memoria);
			break;

		}

		case DESCRIBE: {
			t_list* metadatas;

			memcpy(&metadatas, elScript->resultadoDeEnvio + sizeof(int),
					sizeof(t_list*));

			if (esDescribeGlobal(laRequest)) {
				actualizarMetadatas(metadatas);
				sem_wait(&sem_actualizacionMetadatas);
				describirMetadatas(metadatas);
				sem_post(&sem_actualizacionMetadatas);
			} else {
				metadataTablaLFS* laMetadata = list_get(metadatas, 0);
				describirUnaMetadata(laMetadata);
				agregarUnaMetadata(laMetadata);

				list_destroy(metadatas);
			}
		}
			/* no break */

		default: {

			if (laRequest->requestEnInt == CREATE) {

				agregarUnaMetadataEnString(laRequest->parametros);

			}

			if (laRequest->requestEnInt == INSERT) {

				agregarInsertCompletoAMemoria(memoria);

			}

			char* textoALoggear = string_new();
			string_append(&textoALoggear, requestStructAString(laRequest));
			string_append(&textoALoggear, ": Se pudo realizar. (Enviado a ");
			string_append(&textoALoggear, string_itoa(memoria));
			string_append(&textoALoggear, ")");
			loggearVerdeClaro(logger, textoALoggear);
			free(textoALoggear);
		}

		}
	}

	}

	if (elScript->resultadoDeEnvio != NULL) {
		free(elScript->resultadoDeEnvio);
	}
	if (respuesta == ERROR) { //TODO: Marca, si se hace una operacion sobre una tabla que ya no existe, no se corta
		return ERROR;
	} else {
		return TODO_BIEN; //Si la respuesta fuese NO_EXISTE, no se corta la ejecucion
	}

}

int criterioDeTabla(char* nombreTabla) {
	sem_wait(&sem_actualizacionMetadatas);
	for (int i = 0; i < list_size(listaTablas); i++) {
		metadataTablaLFS* tabla = list_get(listaTablas, i);

		if (!strcmp(tabla->nombre, nombreTabla)) {
			sem_post(&sem_actualizacionMetadatas);
			return tabla->consistencia;
		}
	}
	sem_post(&sem_actualizacionMetadatas);

	return -1;
}

char* devolverTablaDeRequest(request* unaRequest) {

	char** parametros = string_split(unaRequest->parametros, " ");

	char* tabla = string_duplicate(parametros[0]);

	liberarArrayDeStrings(parametros);

	return tabla;

}

int encontrarPosicionDeMemoria(int memoriaAEncontrar) {

	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* memoria = list_get(memorias, i);

		if (memoria->nombre == memoriaAEncontrar) {
			return i;
		}
	}

	return -1;
}

int memoriaECSiguiente(int memoriaInicialEC) {

	int posicionMemoriaInicialEnLista = encontrarPosicionDeMemoria(
			memoriaInicialEC);

	for (int i = posicionMemoriaInicialEnLista + 1; i < list_size(memorias);
			i++) { // De memoriaEC a fin de lista

		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->consistencias[EC] == EC) {
			return unaMemoria->nombre;
		}
	}

	if (posicionMemoriaInicialEnLista == -1) //El for anterior recorre toda la lista
			{
		return -1;
	}

	for (int i = 0; i < posicionMemoriaInicialEnLista; i++) { //De comienzo de lista a memoriaEC
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->consistencias[EC] == EC) {
			return unaMemoria->nombre;
		}
	}

	return memoriaInicialEC; //Si no encuentra otra devuelve la misma
}

int determinarAQueMemoriaEnviar(request* unaRequest) { //Synceado por afuera

	int criterio = criterioDeTabla(devolverTablaDeRequest(unaRequest));

	if (criterio == -1) {
		log_error(logger, "No se encontro la tabla %s en las metadatas.",
				devolverTablaDeRequest(unaRequest));
		return -1;
	}

	switch (criterio) {

	case SC: {
		for (int i = 0; i < list_size(memorias); i++) {
			memoriaEnLista* unaMemoria = list_get(memorias, i);
			if (unaMemoria->consistencias[SC] == SC && unaMemoria->estaViva) {
				return unaMemoria->nombre;
			}
		}
		return -1;
	}

	case EC: {

		int memoriaAux = proximaMemoriaEC;

		proximaMemoriaEC = memoriaECSiguiente(proximaMemoriaEC);

		return memoriaAux;

	}

	case SHC: {

		int key = 0; //Una key random

		if (unaRequest->requestEnInt == SELECT
				|| unaRequest->requestEnInt == INSERT) {
			char** keyConBasuras = string_split(unaRequest->parametros, " ");
			key = atoi(keyConBasuras[1]);

			liberarArrayDeStrings(keyConBasuras);
		}

		return memoriaHash(key);
	}

	}

	return -1;

}

int unaMemoriaCualquiera() //TODO: Marca, una cualquiera es realmente cualquiera
{
	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* unaMemoria = list_get(memorias, 0);

		if (laMemoriaTieneConsistencias(unaMemoria) && unaMemoria->estaViva) {
			return unaMemoria->nombre;
		}
	}

	return -1;

}

int memoriaHash(int key) {
	int cantidadMemoriasSHC = 0;

	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->consistencias[SHC] == SHC) {
			cantidadMemoriasSHC++;
		}
	}

	if (cantidadMemoriasSHC == 0) {
		return -1;
	}

	int numeroMemoriaResultante = (key % cantidadMemoriasSHC) + 1; //Las memorias empiezan por la memoria 1, asi que se pone +1

	return numeroMemoriaResultante;
}

void matarMemoria(int nombreMemoria) {
	//Esto virtualmente borra la memoria del sistema, no la borra completamente para que
	//las cosas que la estan usando en el momento de borrarla no rompan pero que se enteren que se borro.

	sem_wait(&sem_borradoMemoria);
	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->nombre == nombreMemoria) {

			list_remove(memorias, i);

			if (unaMemoria->consistencias[SHC] == SHC) { //Journal a todas las SHC

				for (int d = 0; d < list_size(memorias); d++) {

					memoriaEnLista* unaMemoria = list_get(memorias, d);

					if (unaMemoria->consistencias[SHC] == SHC
							&& unaMemoria->estaViva) {
						enviarInt(unaMemoria->socket, OP_JOURNAL);

					}

				}
			}

			if (unaMemoria->nombre == proximaMemoriaEC) {

				sem_wait(&sem_cambioMemoriaEC);
				proximaMemoriaEC = memoriaECSiguiente(proximaMemoriaEC);

				if (unaMemoria->nombre == proximaMemoriaEC) //Si es la unica EC
						{
					proximaMemoriaEC = -1;
				}
				sem_post(&sem_cambioMemoriaEC);
			}

			unaMemoria->estaViva = 0;
//			free(unaMemoria->ip); es referencia a la seed, que tiene que estar siempre

			close(unaMemoria->socket);
			sem_wait(&unaMemoria->sem_cambioScriptsEsperando);
			while (list_size(unaMemoria->scriptsEsperando) != 0) {

				int* nombreScript = list_remove(unaMemoria->scriptsEsperando,
						0);

				int respuesta = ERROR;

				script* elScript = encontrarScriptEnLista(*nombreScript,
						listaEXEC);

				elScript->resultadoDeEnvio = malloc(sizeof(int));
				memcpy(elScript->resultadoDeEnvio, &respuesta, sizeof(int));

				sem_post(&elScript->semaforoDelScript);
			}
			sem_post(&unaMemoria->sem_cambioScriptsEsperando);

			sem_post(&sem_borradoMemoria);

			return;
		}
	}

	sem_post(&sem_borradoMemoria);
}

int seedYaEstaConectada(seed* unaSeed) {

	sem_wait(&sem_borradoMemoria);

	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (!unaMemoria->estaViva) {

			continue;
		}

		if ((!strcmp(unaMemoria->ip, unaSeed->ip))
				&& (unaSeed->puerto == unaMemoria->puerto)) {
			sem_post(&sem_borradoMemoria);
			return 1;
		}
	}
	sem_post(&sem_borradoMemoria);
	return 0;
}

int seedYaExiste(seed* unaSeed) { //Sincro por afuera
	for (int i = 0; i < list_size(seedsMemorias); i++) {
		seed* otraSeed = list_get(seedsMemorias, i);
		if ((!strcmp(otraSeed->ip, unaSeed->ip))
				&& unaSeed->puerto == otraSeed->puerto) {
			return 1;
		}
	}
	return 0;
}

int laMemoriaTieneConsistencias(memoriaEnLista* unaMemoria) {
	if (!unaMemoria->estaViva) {
		return -1;
	}

	if (unaMemoria->consistencias[SC] == -1
			&& unaMemoria->consistencias[EC] == -1
			&& unaMemoria->consistencias[SHC] == -1) {
		return 0;
	}
	return 1;
}

int existeTabla(char* nombreTabla) {
	return criterioDeTabla(nombreTabla) + 1;
}

int memoriaEstaSiendoCreada(seed* unaSeed) {
	sem_wait(&sem_seedSiendoCreada);
	for (int i = 0; i < list_size(seedsSiendoCreadas); i++) {

		seed* otraSeed = list_get(seedsSiendoCreadas, i);

		if ((!strcmp(otraSeed->ip, unaSeed->ip))
				&& unaSeed->puerto == otraSeed->puerto) {
			sem_post(&sem_seedSiendoCreada);
			return 1;
		}

	}
	sem_post(&sem_seedSiendoCreada);
	return 0;
}

void sacarSeedDeMemoriasEnCreacion(seed* unaSeed) {
	sem_wait(&sem_seedSiendoCreada);
	for (int i = 0; i < list_size(seedsSiendoCreadas); i++) {

		seed* otraSeed = list_get(seedsSiendoCreadas, i);

		if ((!strcmp(otraSeed->ip, unaSeed->ip))
				&& unaSeed->puerto == otraSeed->puerto) {

			list_remove(seedsSiendoCreadas, i);

			sem_post(&sem_seedSiendoCreada);
			return;
		}

	}
	sem_post(&sem_seedSiendoCreada);

}

void agregarOperacionFallidaAMemoria(int numeroMemoria, int operacion) {
	if (operacion == INSERT) {
		agregarInsertFallidoAMemoria(numeroMemoria);
	} else if (operacion == SELECT) {
		agregarSelectFallidoAMemoria(numeroMemoria);
	}
}

void agregarInsertCompletoAMemoria(int numeroMemoria)

{
	sem_wait(&sem_borradoMemoria);

	int index = encontrarPosicionDeMemoria(numeroMemoria);

	if (index != -1) {
		memoriaEnLista* laMemoria = list_get(memorias,
				encontrarPosicionDeMemoria(numeroMemoria));

		laMemoria->estadisticas.insertsCompletos++;

	}

	sem_post(&sem_borradoMemoria);
}

void agregarInsertFallidoAMemoria(int numeroMemoria) {
	sem_wait(&sem_borradoMemoria);

	int index = encontrarPosicionDeMemoria(numeroMemoria);

	if (index != -1) {
		memoriaEnLista* laMemoria = list_get(memorias,
				encontrarPosicionDeMemoria(numeroMemoria));

		laMemoria->estadisticas.insertsFallidos++;

	}

	sem_post(&sem_borradoMemoria);
}

void agregarSelectFallidoAMemoria(int numeroMemoria) {
	sem_wait(&sem_borradoMemoria);

	int index = encontrarPosicionDeMemoria(numeroMemoria);

	if (index != -1) {
		memoriaEnLista* laMemoria = list_get(memorias,
				encontrarPosicionDeMemoria(numeroMemoria));

		laMemoria->estadisticas.selectsFallidos++;

	}

	sem_post(&sem_borradoMemoria);
}

void agregarSelectCompletoAMemoria(int numeroMemoria) {
	sem_wait(&sem_borradoMemoria);

	int index = encontrarPosicionDeMemoria(numeroMemoria);

	if (index != -1) {
		memoriaEnLista* laMemoria = list_get(memorias,
				encontrarPosicionDeMemoria(numeroMemoria));

		laMemoria->estadisticas.selectsCompletos++;

	}

	sem_post(&sem_borradoMemoria);
}

void journalAUnaMemoria(int numeroMemoria) {
	sem_wait(&sem_borradoMemoria);

	for (int i = 0; i < list_size(memorias); i++) {
		memoriaEnLista* unaMemoria = list_get(memorias, i);

		if (unaMemoria->nombre == numeroMemoria) //TODO: Marca, va a todas
				{

			if (unaMemoria->estaViva) {
				unaMemoria->estadisticas.operacionesTotalesEnMemoria++;
				enviarInt(unaMemoria->socket, OP_JOURNAL);
			}

		}

	}

	sem_post(&sem_borradoMemoria);
}
