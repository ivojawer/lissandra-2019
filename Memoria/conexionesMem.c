#include "conexionesMem.h"

extern t_log* logger;
extern t_list* seeds;
int socketKernel;
int socketLFS;

void primeraConexionKernel() {
	t_config* config = config_create(DIRCONFIG);
	int puerto = config_get_int_value(config, "PUERTO_ESCUCHA");

	int socketServidor = crearServidor(puerto);

	socketKernel = accept(socketServidor, (void*) NULL, NULL);

	t_list* listaInts = list_create();

	int nombreModulo = MEMORIA;

	int nombreMemoria = config_get_int_value(config, "MEMORY_NUMBER");

	config_destroy(config);

	list_add(listaInts, &nombreModulo);
	list_add(listaInts, &nombreMemoria);

	enviarVariosIntsConHeader(listaInts, HANDSHAKE, socketKernel);

	list_destroy(listaInts);

	int operacion = recibirInt(socketKernel, logger);

	if (operacion != HANDSHAKE) {
		log_error(logger,
				"Se envio un handshake al kernel y se devolvio otra cosa.");
		return;
	}

	int modulo = recibirInt(socketKernel, logger);

	if (modulo != KERNEL) {
		log_error(logger, "Se conecto algo que no es el kernel.");
		return;
	}

	comunicacionConKernel();

}

void manejoErrorKernel() {
	log_error(logger,
			"Se recibio del kernel algo incorrecto, se va a cerrar la conexion.");
	close(socketKernel);
}

void comunicacionConKernel() {
	while (1) {
		int operacion = recibirInt(socketKernel, logger);

		switch (operacion) {
		case REQUEST: {
			int id = recibirInt(socketKernel, logger);

			if (id == -1) {
				manejoErrorKernel();
				return;
			}

			request* unaRequest = recibirRequest(socketKernel, logger);

			if (unaRequest->requestEnInt == -1) {
				manejoErrorKernel();
				return;
			}

			//Hacer la request
			break;

		}
		case GOSSIPING: {
			enviarSeedsConHeader(socketKernel, seeds, GOSSIPING);
			continue;
		}
		case JOURNAL: {
			journal();
			continue;
		}
		default: {
			manejoErrorKernel();
			return;
		}

		}
	}

}

char* ponerComillas(char* string){
	printf("hola\n");
	char* cadenaConComillas = malloc((string_length(string)+2)*sizeof(char));
	*cadenaConComillas='"';
	strcpy(&cadenaConComillas[1],string);
	cadenaConComillas[string_length(string)+1]='"';
	printf("con comillas:%s\n",cadenaConComillas);
	return cadenaConComillas;
}

void enviarInsert(registro reg){

	int lengthKey = snprintf( NULL, 0, "%d", reg.key );
	char* keyEnString = malloc( lengthKey + 1 );
	snprintf( keyEnString, lengthKey + 1, "%d", reg.key );

	int lengthTS = snprintf( NULL, 0, "%d", reg.timestamp );
	char* tsEnString = malloc( lengthTS + 1 );
	snprintf( tsEnString, lengthTS + 1, "%d", reg.key );

	char* parametros = "";
	string_append(parametros,keyEnString);
	string_append_with_format(parametros," ",reg.value);
	string_append_with_format(parametros," ",tsEnString);

	free(keyEnString);
	free(tsEnString);

	enviarStringConHeader(socketLFS,parametros,INSERT);

}

void conexionLFS() {
	t_config* config = config_create(DIRCONFIG);
	socketLFS = conectarseAServidor(config_get_string_value(config,"IP_FS"),config_get_int_value(config,"PUERTO_FS"));
}
