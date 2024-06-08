#include <utils/utils.h>

int crear_conexion(char *ip, char *puerto, char *nameServ)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int resultado = getaddrinfo(ip, puerto, &hints, &server_info);
	if (resultado != 0)
	{
		printf("Error en getaddrinfo: %s", gai_strerror(resultado));
		exit(-1);
	}

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	int connect_resultado = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	if (connect_resultado == 0)
	{
		printf("El cliente se conecto al servidor correctamente a %s.\n", nameServ);
	}
	else
	{
		printf("Error al conectar servidor %s\n", nameServ);
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(char *puerto, t_log *un_log, char *msj_server)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int resultado = getaddrinfo(NULL, puerto, &hints, &servinfo);
	if (resultado != 0)
	{
		log_error(un_log, "Error en getaddrinfo: %s.\n", gai_strerror(resultado));
		exit(-1);
	}

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	int bind_resultado = bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	if (bind_resultado != 0)
	{
		herror("Fallo el bind\n");
		exit(-3);
	}

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_info(un_log, "Server: %s:", msj_server);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log *un_log, char *msj)
{
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(un_log, "Se conecto el cliente: %s", msj);

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

/*
void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}
*/

/*
void crear_newBuffer(t_newBuffer *paquete)
{
	paquete = malloc(sizeof(t_newBuffer));
	paquete->offset = 0;
	paquete->size = 0;
	paquete->stream = NULL;
}

void crearBufferProcesoMemoria(t_newBuffer *buffer, ProcesoMemoria *proceso)
{
	// Reservamos espacio de memoria
	//buffer = malloc(sizeof(t_newBuffer));

	// Inicializamos sus campos
	buffer->size = (sizeof(uint32_t) * 2) + proceso->path_length + 1;
	buffer->offset = 0;
	buffer->stream = malloc(buffer->size);

	//void *stream = malloc(buffer->size);

	// copiamos pid
	memcpy(buffer->stream + buffer->offset, &proceso->PID, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	// copiamos tamanio del path
	memcpy(buffer->stream + buffer->offset, &proceso->path_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	// copiamos el path
	memcpy(buffer->stream + buffer->offset, proceso->path, proceso->path_length);

	//buffer->stream = stream;
	// Si usamos memoria dinámica para el path, y no la precisamos más, ya podemos liberarla:
	// free(proceso->path);
}

void rellenarPaqueteConNewBuffer(t_newPaquete *paquete, t_newBuffer *buffer)
{
	// solicitamos memoria para hacer el t_newPaquete
	paquete = malloc(sizeof(t_newPaquete));

	paquete->codigo_operacion = PAQUETE;
	paquete->buffer = malloc(buffer->size);
	paquete->buffer = buffer;
}

void enviarPaqueteConNewBuffer(t_newPaquete *paquete, int socket)
{
	// armamos el stream a enviar
	void *a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t));
	int offset = 0;

	// guardamos el codigo de op.
	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
	offset += sizeof(op_code);
	// mandamos el tamanio del buffer
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	// mandamos el stream.
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	// enviamos
	send(socket, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

	// ACÁ HAY QUE LIBERAR MEMORIA
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
*/

ProcesoMemoria *deserializar_proceso_memoria(t_newBuffer *buffer)
{
	ProcesoMemoria *to_return = malloc(sizeof(ProcesoMemoria));

	void *stream = buffer->stream;

	// deserializamos los campos del buffer, primero el pid y luego el tamanio del path
	memcpy(&(to_return->PID), stream, sizeof(int));
	stream += sizeof(int);
	memcpy(&(to_return->path_length), stream, sizeof(int));
	stream += sizeof(int);
	// deserailizamos el path como tal
	to_return->path = malloc(to_return->path_length);
	memcpy(to_return->path, stream, to_return->path_length);

	return to_return;
}


PCB *deserializar_proceso_cpu(t_newBuffer *buffer)
{
	PCB *to_return = malloc(sizeof(PCB));

	void *stream = buffer->stream;

	// deserializamos los campos del buffer, primero el pid y luego el tamanio del path
	memcpy(&(to_return->PID), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(to_return->PC), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(to_return->quantum), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(to_return->AX), stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&(to_return->BX), stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&(to_return->CX), stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&(to_return->DX), stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&(to_return->estado), stream, sizeof(estado_proceso));
	stream += sizeof(estado_proceso);
	memcpy(&(to_return->path_length), stream, sizeof(int));
	stream += sizeof(uint32_t);
	// deserailizamos el path como tal
	to_return->path = malloc(to_return->path_length);
	memcpy(to_return->path, stream, to_return->path_length);//ERROR AL 3 INTENTO

	return to_return;
}


void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

/*void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}*/

/*t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}*/

/*void agregar_a_paquete(t_paquete* paquete, void* valor, size_t tamanio)
{
	printf("Realojar espacio de memoria?? ");
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));
	printf("Realojar espacio de memoria?? ");

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}*/

/*void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}*/

void* serializar_paquete(t_newPaquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
	desplazamiento+= sizeof(op_code);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, &(paquete->buffer->offset), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);

	return magic;
}

void enviar_mensaje_cpu_memoria(char* mensaje, int socket_cliente)
{
	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_newBuffer));
	paquete->buffer->size = strlen(mensaje) + 1; //
	paquete->buffer->offset = 0;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + sizeof(op_code) + 2*sizeof(u_int32_t);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

t_list *recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while (desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
void eliminar_paquete(t_newPaquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void handshakeClient(int fd_servidor, int32_t handshake)
{
	int result;

	send(fd_servidor, &handshake, sizeof(int32_t), 0);
	recv(fd_servidor, &result, sizeof(int32_t), 0);

	if (result == 0)
		printf("Handshake Success\n");
	else
		printf("Handshake Failure\n");
}

void handshakeServer(int fd_client)
{
	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	recv(fd_client, &handshake, sizeof(int32_t), MSG_WAITALL);
	switch (handshake)
	{
	case 1: // CPU
		send(fd_client, &resultOk, sizeof(int32_t), 0);
		break;
	case 2: // Kernel
		send(fd_client, &resultOk, sizeof(int32_t), 0);
		break;
	case 3: // IO
		send(fd_client, &resultOk, sizeof(int32_t), 0);
		break;
	default: // ERROR
		send(fd_client, &resultError, sizeof(int32_t), 0);
		break;
	}
}

void liberar_conexion(int socket)
{
	close(socket);
}

void liberar_logger(t_log *logger)
{
	log_destroy(logger);
	free(logger);
}
void liberar_config(t_config *config)
{
	config_destroy(config);
	free(config);
}

// void crear_buffer(t_buffer* un_buffer)
// {
// 	un_buffer = malloc(sizeof(t_buffer));
// 	un_buffer->size = 0;
// 	un_buffer->stream = NULL;
// }

// void cargar_string_al_buffer(t_buffer* un_buffer, char* un_string)
// {
// 	un_buffer->stream = realloc(un_buffer->stream, un_buffer->size + (strlen(un_string) + 1) +sizeof(int));

// 	memcpy(un_buffer->stream + un_buffer->size,  strlen(un_string) + 1, sizeof(int));
// 	memcpy(un_buffer->stream + un_buffer->size + sizeof(int), un_string, (strlen(un_string) + 1));

// 	un_buffer->size +=  (strlen(un_string) + 1) + sizeof(int) ;
// }
void enviarPCB (PCB* proceso, int socket_servidor,op_code codigo)
{
    //Creamos un Buffer
    t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
    buffer->size = sizeof(uint32_t)*3 + sizeof(uint8_t)*4 + sizeof(op_code) + (proceso->path_length+1);
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset, &proceso->PID, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);

	memcpy(buffer->stream + buffer->offset, &proceso->PC, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);

	memcpy(buffer->stream + buffer->offset, &proceso->quantum, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);

	memcpy(buffer->stream + buffer->offset, &proceso->AX, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);

	memcpy(buffer->stream + buffer->offset, &proceso->BX, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);

	memcpy(buffer->stream + buffer->offset, &proceso->CX, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);

	memcpy(buffer->stream + buffer->offset, &proceso->DX, sizeof(uint8_t));
    buffer->offset += sizeof(uint8_t);

	memcpy(buffer->stream + buffer->offset, &proceso->estado, sizeof(uint8_t));
    buffer->offset += sizeof(estado_proceso);


    // Para el nombre primero mandamos el tamaño y luego el texto en sí:
    memcpy(buffer->stream + buffer->offset, &proceso->path_length, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, proceso->path, proceso->path_length);
    
	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = codigo;
    paquete->buffer = buffer;

    //Empaquetamos el Buffer
    void* a_enviar = malloc(buffer->size + sizeof(op_code) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    //Por último enviamos
    send(socket_servidor, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}