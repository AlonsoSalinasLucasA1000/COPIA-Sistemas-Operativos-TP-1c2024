#include "server.h"

t_log* logger;

int iniciar_socket(int puertoNum)
{
	char *puerto = malloc(sizeof(char) * 6);
	sprintf(puerto, "%d", puertoNum);
	int socket_servidor;

	struct addrinfo hints, *serv_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int err = getaddrinfo(NULL, puerto, &hints, &serv_info);
	if (err != 0)
	{
		printf("error en greaddr\n");
	}
	// Creamos el socket de escucha del servidor
	socket_servidor = socket(serv_info->ai_family,
                       		 serv_info->ai_socktype,
                       		 serv_info->ai_protocol);
	// Asociamos el socket a un puerto
	bind(socket_servidor, serv_info->ai_addr, serv_info->ai_addrlen);
	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);
	freeaddrinfo(serv_info);
	//log_trace(logger, "Listo para escuchar a mi cliente");

	free(puerto);
	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	printf("Se conecto un cliente!\n");
	size_t bytes;

	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);
	if (handshake == 1) {
		bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
		printf("Handshake OK\n");
	} else {
		bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);
		printf("Handshake ERROR\n");
	}

	close(socket_cliente);
	return socket_cliente;
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	printf("Llegue a `recibir mensaje`");
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	printf("Me llego el mensaje %s", buffer);
//	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}


/*
t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
*/