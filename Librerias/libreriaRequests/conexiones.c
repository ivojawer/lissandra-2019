#include "requests.h"

int seRecibioBien(int respuesta, t_log* logger) {
	if (respuesta < 0) {

		log_error(logger, "Hubo un problema en el recibo de algo.");

		return 0;
	}
	return 1;
}

int conectarseAServidor(char* ip, int puerto) {

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(ip);
	direccionServidor.sin_port = htons(puerto);

	int conexion = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(conexion, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("No me pude conectar");
		return -1;
	}
	return conexion;
}

int crearServidor(int puerto) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(puerto);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("Fallo el bind");
		return -1;
	}

	listen(servidor, 100);

	return servidor;
}

void enviarIntConHeader(int aQuien, int intAEnviar, int header) {
	void* paquete = malloc(sizeof(int) + sizeof(int));
	memcpy(paquete, &header, sizeof(int));
	memcpy(paquete + sizeof(int), &intAEnviar, sizeof(int));
	send(aQuien, paquete, sizeof(int) + sizeof(int), 0);
	free(paquete);
}

void enviarVariosIntsConHeader(int aQuien, t_list* intsAEnviar, int header) {
	int cantidadInts = list_size(intsAEnviar);

	int tamanioPaquete = cantidadInts * sizeof(int) + sizeof(int);

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));

	int ultimaPosicionPaquete = sizeof(int);

	for (int i = 0; i < cantidadInts; i++) {
		int* unInt = list_get(intsAEnviar, i);
		memcpy(paquete + ultimaPosicionPaquete, unInt, sizeof(int));
		ultimaPosicionPaquete += sizeof(int);
	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);
}

void enviarStringConHeader(int aQuien, char* string, int header) {
	void* paquete;

	int tamanioString = (strlen(string) + 1) * sizeof(char);

	int tamanioEnvio = sizeof(int) + sizeof(int) + tamanioString;

	paquete = malloc(tamanioEnvio);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete + sizeof(int), &tamanioString, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int), string, tamanioString);

	send(aQuien, paquete, tamanioEnvio, 0);

	free(paquete);
}

void enviarStringConHeaderEId(int aQuien, char* string, int header, int id) {
	void* paquete;

	int tamanioString = (strlen(string) + 1) * sizeof(char);

	int tamanioEnvio = sizeof(int) + sizeof(int) + sizeof(int) + tamanioString;

	paquete = malloc(tamanioEnvio);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete + sizeof(int), &id, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int), &tamanioString, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int) + sizeof(int), string,
			tamanioString);

	send(aQuien, paquete, tamanioEnvio, 0);

	free(paquete);
}

void enviarRequestConHeader(int aQuien, request* requestAEnviar, int header) {
	char* stringAEnviar = requestStructAString(requestAEnviar);

	enviarStringConHeader(aQuien, stringAEnviar, header);

	free(stringAEnviar);

}

void enviarRequestConHeaderEId(int aQuien, request* requestAEnviar, int header,
		int id) {

	char* stringAEnviar = requestStructAString(requestAEnviar);

	void* paquete;

	int tamanioString = (strlen(stringAEnviar) + 1) * sizeof(char);

	int tamanioEnvio = sizeof(int) + sizeof(int) + sizeof(int) + tamanioString;

	paquete = malloc(tamanioEnvio);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete + sizeof(int), &id, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int), &tamanioString, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int) + sizeof(int), stringAEnviar,
			tamanioString);

	send(aQuien, paquete, tamanioEnvio, 0);

	free(stringAEnviar);
	free(paquete);

}

void enviarMetadatasConHeaderEId(int aQuien, t_list* metadatas, int header,
		int id) {
	int cantidadMetadatas = list_size(metadatas);

	int tamanioPaquete = sizeof(int) + sizeof(int) + sizeof(int); //Se pone "= sizeof(int)" por el int de cantidadMetadatas, el header e id que va a ir al principio

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		tamanioPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));
	memcpy(paquete + sizeof(int), &id, sizeof(int));
	memcpy(paquete + sizeof(int) + sizeof(int), &cantidadMetadatas,
			sizeof(int));

	int ultimaPosicionDelPaquete = sizeof(int) + sizeof(int) + sizeof(int);

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		memcpy(paquete + ultimaPosicionDelPaquete, &tamanioNombre, sizeof(int));

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int),
				elemento->nombre, tamanioNombre);

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre,
				&elemento->consistencia, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int), &elemento->particiones, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int) + sizeof(int), &elemento->compactTime,
				sizeof(int));

		ultimaPosicionDelPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);
}

void enviarMetadatasConHeader(int aQuien, t_list* metadatas, int header) {

	int cantidadMetadatas = list_size(metadatas);

	int tamanioPaquete = sizeof(int) + sizeof(int); //Se pone "= sizeof(int)" por el int de cantidadMetadatas y el header que va a ir al principio

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		tamanioPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));
	memcpy(paquete + sizeof(int), &cantidadMetadatas, sizeof(int));

	int ultimaPosicionDelPaquete = sizeof(int) + sizeof(int);

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		memcpy(paquete + ultimaPosicionDelPaquete, &tamanioNombre, sizeof(int));

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int),
				elemento->nombre, tamanioNombre);

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre,
				&elemento->consistencia, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int), &elemento->particiones, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int) + sizeof(int), &elemento->compactTime,
				sizeof(int));

		ultimaPosicionDelPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

void enviarSeedsConHeader(int aQuien, t_list* seeds, int header) {

	int cantidadSeeds = list_size(seeds);

	int tamanioPaquete = sizeof(int) + sizeof(int); //Se pone "= sizeof(int)" por el int de cantidadSeeds y el header que va a ir al principio

	for (int i = 0; i < list_size(seeds); i++) {

		seed* elemento = list_get(seeds, i);

		int tamanioIp = (strlen(elemento->ip) + 1) * sizeof(char);

		tamanioPaquete += sizeof(int) + tamanioIp + sizeof(int);
	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));
	memcpy(paquete + sizeof(int), &cantidadSeeds, sizeof(int));

	int ultimaPosicionDelPaquete = sizeof(int) + sizeof(int);

	for (int i = 0; i < list_size(seeds); i++) {

		seed* elemento = list_get(seeds, i);

		int tamanioIp = (strlen(elemento->ip) + 1) * sizeof(char);

		memcpy(paquete + ultimaPosicionDelPaquete, &tamanioIp, sizeof(int));

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int), elemento->ip,
				tamanioIp);

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioIp,
				&elemento->puerto, sizeof(int));

		ultimaPosicionDelPaquete += sizeof(int) + tamanioIp + sizeof(int);

	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

void enviarRegistroConHeaderEId(int aQuien, registro* unRegistro, int header,
		int id) {

	int tamanioValue = strlen(unRegistro->value) + 1;
	int tamanioTimestamp = sizeof(typeof(unRegistro->timestamp));
	int tamanioKey = sizeof(typeof(unRegistro->key));

	int tamanioPaquete = sizeof(int) + sizeof(int) + tamanioTimestamp
			+ tamanioKey + sizeof(int) + tamanioValue;

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete+sizeof(int),&id,sizeof(int));

	memcpy(paquete+sizeof(int)+sizeof(int),&unRegistro->timestamp,tamanioTimestamp);

	memcpy(paquete+sizeof(int)+sizeof(int)+tamanioTimestamp,&unRegistro->key,tamanioKey);

	memcpy(paquete+sizeof(int)+sizeof(int)+tamanioTimestamp+tamanioKey,&tamanioValue,sizeof(int));

	memcpy(paquete+sizeof(int)+sizeof(int)+tamanioTimestamp+tamanioKey+sizeof(int),unRegistro->value,tamanioValue);

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

registro* recibirRegistro(int deQuien, t_log* logger) {

	registro* elRegistro = malloc(sizeof(registro));

	int tamanioTimestamp = sizeof(typeof(elRegistro->timestamp));

	void* bufferTimestamp = malloc(tamanioTimestamp);

	recv(deQuien, bufferTimestamp, tamanioTimestamp, MSG_WAITALL);

	memcpy(&elRegistro->timestamp, bufferTimestamp, tamanioTimestamp);

	free(bufferTimestamp);

	int tamanioKey = sizeof(typeof(elRegistro->timestamp));

	void* bufferKey = malloc(tamanioKey);

	recv(deQuien, bufferKey, tamanioKey, MSG_WAITALL);

	memcpy(&elRegistro->key, bufferKey, tamanioKey);

	free(bufferKey);

	elRegistro->value = recibirString(deQuien, logger);

	return elRegistro;

}

request* recibirRequest(int deQuien, t_log* logger) {
	char* requestEnString = recibirString(deQuien, logger);

	request* requestNuevo = crearStructRequest(requestEnString);

	free(requestEnString);

	return requestNuevo;

}

char* recibirString(int deQuien, t_log* logger) {

	int tamanioString = recibirInt(deQuien, logger);

	if (tamanioString == -1) {
		return " ";
	}

	void* bufferString = malloc(tamanioString);

	int respuesta = recv(deQuien, bufferString, tamanioString,
	MSG_WAITALL);

	if (!seRecibioBien(respuesta, logger)) {
		return " ";
	}

	char* stringRecibido = malloc(tamanioString);

	memcpy(stringRecibido, bufferString, tamanioString);

	free(bufferString);

	return stringRecibido;
}

int recibirInt(int deQuien, t_log* logger) {
	void* bufferInt = malloc(sizeof(int));
	int respuesta = recv(deQuien, bufferInt, sizeof(int),
	MSG_WAITALL);

	if (!seRecibioBien(respuesta, logger)) {

		free(bufferInt);
		return -1;
	}

	int intRecibido;
	memcpy(&intRecibido, bufferInt, sizeof(int));

	free(bufferInt);
	return intRecibido;
}

t_list* recibirMetadatas(int deQuien, t_log* logger) {
	int cantidadMetadatas = recibirInt(deQuien, logger);

	t_list* metadatas = list_create();

	if (cantidadMetadatas == -1)
	{
		return metadatas;
	}

	for (int i = 0; i < cantidadMetadatas; i++) {
		metadataTablaLFS* metadata = malloc(sizeof(metadataTablaLFS));

		metadata->nombre = recibirString(deQuien, logger);
		metadata->consistencia = recibirInt(deQuien, logger);
		metadata->particiones = recibirInt(deQuien, logger);
		metadata->compactTime = recibirInt(deQuien, logger);

		list_add(metadatas, metadata);
	}

	return metadatas;
}

t_list* recibirSeeds(int deQuien, t_log* logger) {
	int cantidadSeeds = recibirInt(deQuien, logger);

	t_list* seeds = list_create();

	for (int i = 0; i < cantidadSeeds; i++) {

		seed* unaSeed = malloc(sizeof(metadataTablaLFS));

		unaSeed->ip = recibirString(deQuien, logger);
		unaSeed->puerto = recibirInt(deQuien, logger);

		list_add(seeds, unaSeed);
	}

	return seeds;
}

//----------------------------------------------------------------------
//Funciones sin header, por si alguna razon son necesarias:

void enviarInt(int aQuien, int intAEnviar) {
	void* paquete = malloc(sizeof(int));
	memcpy(paquete, &intAEnviar, sizeof(int));
	send(aQuien, paquete, sizeof(int), 0);
	free(paquete);
}

void enviarString(char* string, int aQuien) {
	void* paquete;

	int tamanioString = (strlen(string) + 1) * sizeof(char);

	int tamanioEnvio = sizeof(int) + tamanioString;

	paquete = malloc(tamanioEnvio);

	memcpy(paquete, &tamanioString, sizeof(int));

	memcpy(paquete + sizeof(int), string, tamanioString);

	send(aQuien, paquete, tamanioEnvio, 0);

	free(paquete);
}

void enviarMetadatas(t_list* metadatas, int aQuien) {

	int cantidadMetadatas = list_size(metadatas);

	int tamanioPaquete = sizeof(int); //Se pone "= sizeof(int)" por el int de cantidadMetadatas que va a ir al principio

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		tamanioPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &cantidadMetadatas, sizeof(int));

	int ultimaPosicionDelPaquete = sizeof(int);

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		memcpy(paquete + ultimaPosicionDelPaquete, &tamanioNombre, sizeof(int));

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int),
				elemento->nombre, tamanioNombre);

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre,
				&elemento->consistencia, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int), &elemento->particiones, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int) + sizeof(int), &elemento->compactTime,
				sizeof(int));

		ultimaPosicionDelPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

void enviarRequest(int aQuien, request* requestAEnviar) {
	char* stringAEnviar = requestStructAString(requestAEnviar);

	enviarString(stringAEnviar, aQuien);

	free(stringAEnviar);
}
