#include "client.h"
#include <errno.h>

void* serializar_paquete(t_paquete* paquete, int bytes)
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
}

int crear_conexion(char* ip, int puertoNum)
{
	struct addrinfo hints;
	struct addrinfo *server_info;
	int err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	printf("Leí la ip: %s y puerto: %d\n", ip, puertoNum);
	char *puerto = malloc(sizeof(char) * 6);
	sprintf(puerto, "%d", puertoNum);

	printf("Leí la ip: %s y puerto: %s\n", ip, puerto);

	err = getaddrinfo(ip, puerto, &hints, &server_info);
	if (err != 0){
		printf ("Error en getaddrinfo: %s", gai_strerror (err));
		exit (-1);
	}
	free(puerto);
	
	int socket_cliente = socket(server_info->ai_family,
                        		server_info->ai_socktype,
                         		server_info->ai_protocol);

	err = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	if (err != 0)
	{
		printf("error en connect\n");
		fprintf (stderr, "Socket closure failed: %s\n", strerror (errno));
		exit (EXIT_FAILURE);
	}

	size_t bytes;
	int32_t handshake = 1;
	int32_t result;
	bytes = send(socket_cliente, &handshake, sizeof(int32_t), 0);
	bytes = recv(socket_cliente, &result, sizeof(int32_t), MSG_WAITALL);

	if (result == 0) {
		printf("Handshake OK\n");
	} else {
		printf("Handshake ERROR\n");
	}
	// IMPLEMENTAR CUANDO SEPAMOS COMO HACER Q ENCUENTRE Y AHI ANDE
	

	//close(socket_cliente);

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	printf("\nEsto mandando este mensaje `%s` al socket %d\n",mensaje, socket_cliente);
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);
	bytes = send(socket_cliente, a_enviar, bytes, 0);
	if (bytes == -1)
	{
		printf("Error al enviar");
		//fprintf (stderr, "Error al enviar por: %s\n", strerror (errno));
		exit(EXIT_FAILURE);
	} else {
		printf("Se enviaron %d bytes\n", bytes);
	}
	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

/********************************************/
/****FUNCIONES QUE HABIA EN EL CLIENT_TO*****/
/********************************************/

void leer_consola(t_log *logger)
{
    char *leido;

    while (1)
    {
        leido = readline("> ");

        if (strcmp(leido, "") == 0)
        {
            break;
        }

        log_info(logger, leido);
        printf("%s/n", leido);

        free(leido);
    }
}

void paquete(int conexion)
{
    // Ahora toca lo divertido!
    char *leido;
    t_paquete *paquete;

    // Leemos y esta vez agregamos las lineas al paquete
    // ¡No te olvides de liberar las líneas y el paquete antes de regresar!
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
    /* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config)
      con las funciones de las commons y del TP mencionadas en el enunciado */       
}