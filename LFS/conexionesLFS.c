#include "funcionesLFS.h"

extern int tamanioValue;
extern int retardo;
extern t_log* logger;
int socket_memoria;
int socket_cliente;


void manejo_error_memoria(){
	log_error(logger,
			"Se recibio algo incorrecto de Memoria, se cierra la conexion.");
	close(socket_memoria);
}

void comunicacion_con_memoria()
{
	while(1){

		int operacion = recibirInt(socket_cliente, logger);
		switch (operacion) {
		case REQUEST: {

//			int id = recibirInt(socket_cliente, logger); TODO: sacar los id de las respuestas (si los hay)
//			printf("Id request recibido:%d\n",id);
//			if (id == -1) {
//				manejo_error_memoria();
//				return;
//			}

			request* una_request = recibirRequest(socket_cliente, logger);
			if (una_request->requestEnInt == -1) {
				manejo_error_memoria();
				return;
			}

			request* request_para_hilo = malloc(sizeof(request));

			request_para_hilo->parametros = una_request->parametros;
			request_para_hilo->requestEnInt = una_request->requestEnInt;

			list_add(cola_requests, request_para_hilo);
			sem_post(&requests_disponibles);

			break;
		}
		case JOURNAL: {
			int i;
			int nr_registros = recibirInt(socket_cliente, logger);
			t_list *lista_journal = list_create();
			lista_journal = recibirRequests(socket_cliente, logger); //lista de requests *

			for(i = 0; i < nr_registros; i++){
				request* request_procesar = malloc(sizeof(request));
				request_procesar = list_get(lista_journal, i);
				request* request_para_hilo = malloc(sizeof(request));
				request_para_hilo->requestEnInt = INSERT;
				request_para_hilo->parametros = request_procesar->parametros;
				list_add(cola_requests, request_para_hilo);
				sem_post(&requests_disponibles);
			 }
			void liberar_elemento(void *elemento){
				return liberarRequest((request *)elemento);
			}
			list_destroy_and_destroy_elements(lista_journal, liberar_elemento);
			continue;
		}
		default: {
			manejo_error_memoria();
			return;
		}

		}
		sleep(retardo);
	}
}


void aceptar_conexiones()
{
		t_config* config = config_create("../../CONFIG/LFS.config");

		int puerto_servidor = config_get_int_value(config, "PUERTO_ESCUCHA");

		int socket_servidor = crearServidor(puerto_servidor);

		t_list* lista_ints = list_create();

		int nombre_modulo = LFS;

		list_add(lista_ints, &nombre_modulo);
		list_add(lista_ints, &tamanioValue);

		config_destroy(config);

		while (1) {

			sleep(retardo);
			socket_cliente = accept(socket_servidor, (void*) NULL, NULL);

			enviarVariosIntsConHeader(socket_cliente, lista_ints, HANDSHAKE); //HANDSHAKE-LFS-MEMORIA

			int operacion = recibirInt(socket_cliente, logger);

			if (operacion != HANDSHAKE) {
				log_error(logger, "Se envio un handshake y se devolvio otra cosa.");
				close(socket_cliente);
				continue;
			}

			int modulo = recibirInt(socket_cliente, logger);

			switch (modulo){
			case MEMORIA: {
				log_info(logger, "Se conecto una Memoria");
				socket_memoria = socket_cliente;
				pthread_t h_conexion_memoria;
				pthread_create(&h_conexion_memoria, NULL, (void *)comunicacion_con_memoria,
							   NULL);
				pthread_detach(h_conexion_memoria);

				continue;
			}
			default: {
				log_error(logger, "Algo se conecto indebidamente.");
				close(socket_cliente);
				continue;
			}

			}
		}
}

void messageHandler(char* lectura) {
	if (string_is_empty(lectura)) {
		log_info(logger, "No es una request valida, vuelva prontos \n");
		free(lectura);

	}

	char** requestYParametros = string_n_split(lectura, 2, " ");

	int requestEnInt = queRequestEs(requestYParametros[0]);

	if (!esUnaRequestValida(lectura) || requestEnInt == JOURNAL
	|| requestEnInt == ADD || requestEnInt == RUN) { //Si es invalida o es una request que no vale en el LFS

		log_info(logger, "No es una request valida, vuelva prontos \n");

		free(requestYParametros[1]);
		free(requestYParametros[0]);
		free(requestYParametros);
		free(lectura);
		return;

	}

	if (requestYParametros[1] == NULL) { //Para que no rompa en el string_duplicate de funcionesLFS.c
		requestYParametros[1] = (char *) malloc(sizeof(" "));
		strcpy(requestYParametros[1], " ");
	}
	log_info(logger, "mando a ejecutar una request");

	request* requestParaHilo = malloc(sizeof(request));
	requestParaHilo = crearStructRequest(lectura);

	mandarAEjecutarRequest(requestParaHilo);
}

