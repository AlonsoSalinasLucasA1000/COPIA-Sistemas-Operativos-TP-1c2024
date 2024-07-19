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

void avisar_despertar_kernel()
{
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	buffer->offset = 0;
	buffer->size = 1;
	buffer->stream = malloc(buffer->size);

	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = DESPERTAR;
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


void entradasalida_escuchar_memoria (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_memoria);
			printf("Codigo de operación de memoria recibido\n");
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_memoria,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			if( cod_op == 14 ) //ojo con este hardcodeo, de añadir mas codigos de operacion pueden modificarse
			{
				recv(fd_memoria,&(paquete->buffer->offset), sizeof(uint32_t),0);
			}
			recv(fd_memoria,paquete->buffer->stream, paquete->buffer->size,0);
			switch (cod_op) {
			case STDOUT_TOPRINT:
				//
				char* text_to_print = malloc(paquete->buffer->size);
				text_to_print = paquete->buffer->stream;
				printf("%s\n",text_to_print);
				avisar_despertar_kernel();
				break;
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
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}

void enviar_stdin_to_write_memoria(int* direccionFisica, int* tamanio,char* text)
{
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	buffer->offset = 0;
	buffer->size = sizeof(int)*2 + *tamanio + 1;
	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + buffer->offset,direccionFisica, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,tamanio, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,text,*tamanio+1);

	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = STDIN_TOWRITE;
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
    send(fd_memoria, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void enviar_stdout_to_print_memoria(int* direccionFisica, int* tamanio)
{
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	buffer->offset = 0;
	buffer->size = sizeof(int)*2;
	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + buffer->offset,direccionFisica, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,tamanio, sizeof(int));
    buffer->offset += sizeof(int);

	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = STDOUT_TOPRINT;
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
    send(fd_memoria, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void entradasalida_escuchar_kernel (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);

			printf("Checkpoin1 \n");
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_kernel,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_kernel,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case GENERICA:

				printf("Checkpoin2 \n");
				//hay que deserializar, dado que es solo un int
				int* unidadesDeTrabajo = malloc(sizeof(int));
				unidadesDeTrabajo = paquete->buffer->stream;

				printf("Voy a dormir, reyes, la cantidad de %d unidades de trabajo\n",*unidadesDeTrabajo);
				printf("Voy a dormir: %d\n", (*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO ));
				sleep((*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO)/1000);

				//DEVOLVER AL KERNEL PARA DESPERTAR
				avisar_despertar_kernel();

				break;
			case STDIN:

				//DESEMPAQUETAMOS
				int* direccionFisicaIN = malloc(sizeof(int));
				direccionFisicaIN = paquete->buffer->stream;
				int* tamanioIN = malloc(sizeof(int));
				tamanioIN = paquete->buffer->stream + sizeof(int);
				printf("La direccion física que ha llegado es: %d\n",*direccionFisicaIN);
				printf("La direccion física que ha llegado es: %d\n",*tamanioIN);

				//ESCRIBIMOS
				printf("Vamos a llevar a cabo STDIN\n");
				printf("Ingrese un texto por teclado: \n");
				char* leido;
				leido = readline(">> ");

				//mostramos de vuelta por pantalla
				//guardamos en una nueva variable
				char* text = malloc(*tamanioIN+1);
				strncpy(text,leido,*tamanioIN);
				printf("El texto ingresado fue el siguiente: %s\n",text);

				enviar_stdin_to_write_memoria(direccionFisicaIN,tamanioIN,text);

				free(leido);
				avisar_despertar_kernel();
				break;
			case STDOUT:
				
				//DESEMPAQUETAMOS
				int* direccionFisicaOUT = malloc(sizeof(int));
				direccionFisicaOUT = paquete->buffer->stream;
				int* tamanioOUT = malloc(sizeof(int));
				tamanioOUT = paquete->buffer->stream + sizeof(int);
				printf("La direccion física que ha llegado es: %d\n",*direccionFisicaOUT);
				printf("La direccion física que ha llegado es: %d\n",*tamanioOUT);

				enviar_stdout_to_print_memoria(direccionFisicaOUT, tamanioOUT);
				break;
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
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}

void enviarDatos(int fd_servidor, char** datos, char* tipo_interfaz)
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

	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	//calculamos su tamaño
	buffer->size = sizeof(int) + sizeof(uint32_t)*2 + (to_send->nombre_length) + (to_send->path_length);
    buffer->offset = 0;
	buffer->stream = malloc(buffer->size);

	//movemos los valores al buffer
	memcpy(buffer->stream + buffer->offset, &to_send->fd_cliente, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);

	memcpy(buffer->stream + buffer->offset, &to_send->nombre_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	memcpy(buffer->stream + buffer->offset, to_send->nombre,to_send->nombre_length);
	buffer->offset += to_send->nombre_length;
	
	memcpy(buffer->stream + buffer->offset, &to_send->path_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	memcpy(buffer->stream + buffer->offset, to_send->path,to_send->path_length);//no seria (to_send->path_length+1) ???

	//creamos el paquete
	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));

	
	if( strcmp(tipo_interfaz,"GENERICA") == 0 )
	{

		paquete->codigo_operacion = GENERICA;
	}
	else
	{
		if( strcmp(tipo_interfaz,"STDIN") == 0 )
		{
			paquete->codigo_operacion = STDIN;
		}
		else
		{
			if( strcmp(tipo_interfaz,"STDOUT") == 0 )
			{
				paquete->codigo_operacion = STDOUT;
			}
			else
			{
				if( strcmp(tipo_interfaz,"DIALFS") == 0 )
				{
					paquete->codigo_operacion = DIALFS;
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
    send(fd_servidor, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);	
}

#endif /* ENTRADASALIDA_H_ */