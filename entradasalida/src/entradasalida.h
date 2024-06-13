#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include<stdio.h>
#include<stdlib.h>
#include<utils/utils.h>
#include <utils/utils.c>

//file descriptors de entradasalida y los modulos que se conectaran con el
int fd_entradasalida;
int fd_memoria;
int fd_kernel;

t_log* entradasalida_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* entradasalida_config;

char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;

void entradasalida_escuchar_memoria (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_memoria);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(entradasalida_logger, "Desconexion de memoria.");
				control_key = 0;
			default:
				log_warning(entradasalida_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}	
}


void entradasalida_escuchar_kernel (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(entradasalida_logger, "Desconexion de kernel.");
				control_key = 0;
			default:
				log_warning(entradasalida_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}	
}
/*
void asignarOperacion(op_code* codigo, char* tipo_interfaz)
{
	if( strcmp(tipo_interfaz,"GENERICA") == 0 )
	{
		codigo = GENERICA;
	}
	else
	{
		if( strcmp(tipo_interfaz,"STDIN") == 0 )
		{
			codigo = STDIN;
		}
		else
		{
			if( strcmp(tipo_interfaz,"STDOUT") == 0 )
			{
				codigo = STDOUT;
			}
			else
			{
				if( strcmp(tipo_interfaz,"DIALFS") == 0 )
				{
					codigo = DIALFS;
				}
				else
				{
					codigo = WRONG;
				}
			}
		}
	}
}*/

void enviarDatosKernel(int fd_kernel, char** datos, char* tipo_interfaz)
{
	//creamos el struct a enviar
	EntradaSalida* to_send = malloc(sizeof(EntradaSalida));
	to_send->fd_cliente = 0;
	to_send->nombre_length = strlen(datos[0])+1;
	to_send->nombre = malloc(to_send->nombre_length);
	to_send->nombre = datos[0];
	to_send->path_length = strlen(datos[1])+1;
	to_send->path = malloc(to_send->path_length);
	to_send->path = datos[1];

	printf("vamos a enviar lo siguiente a KERNEL: \n");
	printf("vamos a enviar el nombre: %s \n",datos[0]);
	printf("vamos a enviar el path %s: \n",datos[1]);

	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	//calculamos su tamaño
	buffer->size = sizeof(int) + sizeof(uint32_t)*2 + (to_send->nombre_length+1) + (to_send->path_length+1);
    buffer->offset = 0;
	buffer->stream = malloc(buffer->size);

	//movemos los valores al buffer
	memcpy(buffer->stream + buffer->offset, &to_send->fd_cliente, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);

	memcpy(buffer->stream + buffer->offset, &to_send->nombre_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	memcpy(buffer->stream + buffer->offset, &to_send->nombre, sizeof(to_send->nombre_length+1));
	buffer->offset += to_send->nombre_length+1;
	
	memcpy(buffer->stream + buffer->offset, &to_send->path_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	memcpy(buffer->stream + buffer->offset, &to_send->path, sizeof(to_send->nombre_length+1));

	//creamos el paquete
	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));

	printf("codigo de operacion antes de entrar al if: %d\n",paquete->codigo_operacion);
	
	if( strcmp(tipo_interfaz,"GENERICA") == 0 )
	{

		paquete->codigo_operacion = GENERICA;
		printf("codigo de operacion generica: %d\n",paquete->codigo_operacion);

	}
	else
	{
		if( strcmp(tipo_interfaz,"STDIN") == 0 )
		{
			paquete->codigo_operacion = STDIN;
			printf("codigo de operacion stdin: %d\n",paquete->codigo_operacion);
		}
		else
		{
			if( strcmp(tipo_interfaz,"STDOUT") == 0 )
			{
				paquete->codigo_operacion = STDOUT;
				printf("codigo de operacion stdout: %d\n",paquete->codigo_operacion);
			}
			else
			{
				if( strcmp(tipo_interfaz,"DIALFS") == 0 )
				{
					paquete->codigo_operacion = DIALFS;
					printf("codigo de operacion dialfs: %d\n",paquete->codigo_operacion);
				}
				else
				{
					paquete->codigo_operacion = WRONG;
				}
			}
		}
	}
	
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
    send(fd_kernel, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);	
}




void enviarDatosMemoria(int fd_memoria, char** datos, char* tipo_interfaz)
{
	//creamos el struct a enviar
	EntradaSalida* to_send = malloc(sizeof(EntradaSalida));
	to_send->fd_cliente = 0;
	to_send->nombre_length = strlen(datos[0])+1;
	to_send->nombre = malloc(to_send->nombre_length);
	to_send->nombre = datos[0];
	to_send->path_length = strlen(datos[1])+1;
	to_send->path = malloc(to_send->path_length);
	to_send->path = datos[1];

	printf("vamos a enviar lo siguiente a MEMORIA: \n");
	printf("vamos a enviar el nombre: %s \n",datos[0]);
	printf("vamos a enviar el path %s: \n",datos[1]);

	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	//calculamos su tamaño
	buffer->size = sizeof(int) + sizeof(uint32_t)*2 + (to_send->nombre_length+1) + (to_send->path_length+1);
    buffer->offset = 0;
	buffer->stream = malloc(buffer->size);

	//movemos los valores al buffer
	memcpy(buffer->stream + buffer->offset, &to_send->fd_cliente, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);

	memcpy(buffer->stream + buffer->offset, &to_send->nombre_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	memcpy(buffer->stream + buffer->offset, &to_send->nombre, sizeof(to_send->nombre_length+1));
	buffer->offset += to_send->nombre_length+1;
	
	memcpy(buffer->stream + buffer->offset, &to_send->path_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	memcpy(buffer->stream + buffer->offset, &to_send->path, sizeof(to_send->nombre_length+1));

	//creamos el paquete
	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));

	printf("codigo de operacion antes de entrar al if: %d\n",paquete->codigo_operacion);
	
	if( strcmp(tipo_interfaz,"GENERICA") == 0 )
	{

		paquete->codigo_operacion = GENERICA;
		printf("codigo de operacion generica: %d\n",paquete->codigo_operacion);

	}
	else
	{
		if( strcmp(tipo_interfaz,"STDIN") == 0 )
		{
			paquete->codigo_operacion = STDIN;
			printf("codigo de operacion stdin: %d\n",paquete->codigo_operacion);
		}
		else
		{
			if( strcmp(tipo_interfaz,"STDOUT") == 0 )
			{
				paquete->codigo_operacion = STDOUT;
				printf("codigo de operacion stdout: %d\n",paquete->codigo_operacion);
			}
			else
			{
				if( strcmp(tipo_interfaz,"DIALFS") == 0 )
				{
					paquete->codigo_operacion = DIALFS;
					printf("codigo de operacion dialfs: %d\n",paquete->codigo_operacion);
				}
				else
				{
					paquete->codigo_operacion = WRONG;
				}
			}
		}
	}
	
	paquete->buffer = buffer;
	if (paquete->codigo_operacion != GENERICA){
		//Empaquetamos el Buffer
		void* a_enviar = malloc(buffer->size + sizeof(op_code) + sizeof(uint32_t));
		int offset = 0;
		memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
		offset += sizeof(op_code);
		memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
		//Por último enviamos
		send(fd_memoria, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);
		free(a_enviar);
	}
	// No nos olvidamos de liberar la memoria que ya no usaremos
	
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
    
}


#endif /* ENTRADASALIDA_H_ */