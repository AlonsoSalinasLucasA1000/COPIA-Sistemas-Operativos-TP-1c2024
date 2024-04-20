#include <utils/utils.h>



int crear_conexion(char *ip, char* puerto)
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
			printf ("El cliente se conecto al servidor correctamente.");
		}
		else {
			printf ("Error al conectar cliente.");
		}

	freeaddrinfo(server_info);

	return socket_cliente;
}


int iniciar_servidor(char* puerto, t_log* un_log, char* msj_server)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

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
		error ("Fallo el bind");
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