#include "funcionesLFS.h"

extern int tamanioValue;
extern int retardo;
extern t_log* logger;
//int socket_memoria;
//int socket_cliente;

void manejo_error_memoria(int socket_memoria) {
	log_error(logger,
			"Se recibio algo incorrecto de Memoria, se cierra la conexion.");
	close(socket_memoria);
}

void comunicacion_con_memoria(int socket_cliente) {

	while (1) {

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
				manejo_error_memoria(socket_cliente);
				return;
			}

			request* request_para_hilo = malloc(sizeof(request));

			request_para_hilo->parametros = una_request->parametros;
			request_para_hilo->requestEnInt = una_request->requestEnInt;

			mandarAEjecutarRequest(request_para_hilo,socket_cliente);

			break;
		}
		case JOURNAL: {
			int i;
			//int nr_registros = recibirInt(socket_cliente, logger);
			//printf("cantidad de registros a recibir:%d\n",nr_registros);
			t_list *lista_journal = recibirRequests(socket_cliente, logger); //lista de requests *
			if (list_size(lista_journal) != 0) {
//				request* primerRequest = (request*) lista_journal->head->data;
				request* primerRequest = (request*) list_get(lista_journal, 0);
				printf("primer request:%s\n", primerRequest->parametros);
				for (i = 0; i < list_size(lista_journal); i++) {
//					request* request_procesar = malloc(sizeof(request)); -malloc sacado
					request* request_procesar = list_get(lista_journal, i);
					request* request_para_hilo = malloc(sizeof(request));
					request_para_hilo->requestEnInt = INSERT;
					request_para_hilo->parametros =
							request_procesar->parametros;

					mandarAEjecutarRequest(request_para_hilo,-1);
				}
			}

			void liberar_elemento(void *elemento) {
				return liberarRequest((request *) elemento);
			}
			//list_destroy_and_destroy_elements(lista_journal, (void*)liberarRequest); TODO: esta linea hace explotar pero habria que hacer un free en algun lado
			continue;
		}
		default: {
			manejo_error_memoria(socket_cliente);
			return;
		}

		}
//		sleep(retardo);
	}
}

void aceptar_conexiones() {

	t_config* config = config_create("../../CONFIG/LFS.config");

	int puerto_servidor = config_get_int_value(config, "PUERTO_ESCUCHA");

	int socket_servidor = crearServidor(puerto_servidor);

	t_list* lista_ints = list_create();

	int nombre_modulo = LFS;

	list_add(lista_ints, &nombre_modulo);
	list_add(lista_ints, &tamanioValue);

	config_destroy(config);

	while (1) {

//		sleep(retardo);
		int socket_cliente = accept(socket_servidor, (void*) NULL, NULL);

		enviarVariosIntsConHeader(socket_cliente, lista_ints, HANDSHAKE); //HANDSHAKE-LFS-TamValue

		int operacion = recibirInt(socket_cliente, logger);

		if (operacion != HANDSHAKE) {
			log_error(logger, "Se envio un handshake y se devolvio otra cosa.");
			close(socket_cliente);
			continue;
		}

		int modulo = recibirInt(socket_cliente, logger);

		switch (modulo) {
		case MEMORIA: {
			log_info(logger, "Se conecto una Memoria");
			int socket_memoria = socket_cliente;

			//struct para hilo
			struct socket_info *socket_info = malloc(
					sizeof(struct socket_info));
//			socket_info->socket_cliente = socket_cliente;
			socket_info->socket_memoria = socket_memoria;

			pthread_t h_conexion_memoria;
			pthread_create(&h_conexion_memoria, NULL,
					(void *) comunicacion_con_memoria, socket_memoria);
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
