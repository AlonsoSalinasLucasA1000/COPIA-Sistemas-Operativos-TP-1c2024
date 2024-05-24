#include <utils/utils.h>

int crear_conexion(char *ip, char* puerto, char* nameServ)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int resultado = getaddrinfo(ip, puerto, &hints, &server_info);
		if (resultado != 0){
			printf ("Error en getaddrinfo: %s", gai_strerror (resultado));
			exit (-1);
		}

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
					server_info->ai_socktype,
					server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	int connect_resultado = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
		if (connect_resultado == 0) {
			printf ("El cliente se conecto al servidor correctamente a %s.\n",nameServ);
		}
		else {
			printf ("Error al conectar servidor %s\n", nameServ);
		}

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(char* puerto, t_log* un_log, char* msj_server)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int resultado = getaddrinfo(NULL, puerto, &hints, &servinfo);
		 if (resultado != 0){
			log_error (un_log, "Error en getaddrinfo: %s", gai_strerror (resultado));
			exit (-1);
		 }
	
	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
					servinfo->ai_socktype,
					servinfo->ai_protocol);
	
	// Asociamos el socket a un puerto
	int bind_resultado = bind (socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	if (bind_resultado != 0){
		herror ("Fallo el bind");
		exit (-3);
	}
					

	// Escuchamos las conexiones entrantes
	listen (socket_servidor, SOMAXCONN);


	freeaddrinfo(servinfo);
	log_info(un_log, "Server: %s:", msj_server);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* un_log, char* msj)
{
	int socket_cliente;
	socket_cliente = accept (socket_servidor, NULL, NULL );
	log_info(un_log, "Se conecto el cliente: %s", msj);

	return socket_cliente;
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

void handshakeClient(int fd_servidor, int32_t handshake)
{
	int result;

	send(fd_servidor,&handshake,sizeof(int32_t),0);
	recv(fd_servidor,&result,sizeof(int32_t),0);

	if( result == 0 )
		printf("Handshake Success\n");
	else
		printf("Handshake Failure\n");
}

void handshakeServer(int fd_client)
{
	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	recv(fd_client, &handshake,sizeof(int32_t),MSG_WAITALL);
	switch (handshake){
		case 1: //CPU
				send(fd_client, &resultOk,sizeof(int32_t),0);
				break;
		case 2: //Kernel
				send(fd_client, &resultOk,sizeof(int32_t),0);
				break;
		case 3: //IO
				send(fd_client, &resultOk,sizeof(int32_t),0);
				break;
		default: //ERROR
				send(fd_client, &resultError,sizeof(int32_t),0);
				break;
	}
}

void liberar_conexion(int socket)
{
    close(socket);
}

void liberar_logger(t_log* logger)
{
	log_destroy(logger);
	free(logger);
}
void liberar_config(t_config* config)
{
	config_destroy(config);
	free(config);
}

void crear_buffer(t_buffer* un_buffer)
{
	un_buffer = malloc(sizeof(t_buffer));
	un_buffer->size = 0;
	un_buffer->stream = NULL;
}

void cargar_string_al_buffer(t_buffer* un_buffer, char* un_string)
{
	un_buffer->stream = realloc(un_buffer->stream, un_buffer->size + (strlen(un_string) + 1) +sizeof(int));

	memcpy(un_buffer->stream + un_buffer->size,  strlen(un_string) + 1, sizeof(int));
	memcpy(un_buffer->stream + un_buffer->size + sizeof(int), un_string, (strlen(un_string) + 1));

	un_buffer->size +=  (strlen(un_string) + 1) + sizeof(int) ;
}


