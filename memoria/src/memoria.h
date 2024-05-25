#ifndef MEMORIA_H_
#define MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<utils/utils.h>
#include <utils/utils.c>

//file descriptors de memoria y los modulos que se conectaran con ella
int fd_memoria;
int fd_kernel;
int fd_cpu;
int fd_entradasalida;


t_log* memoria_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* memoria_config;

char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;



void memoria_escuchar_cpu (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_cpu);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(memoria_logger, "El cliente cpu se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(memoria_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
}


void memoria_escuchar_entradasalida (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_entradasalida);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(memoria_logger, "El cliente EntradaSalida se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(memoria_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
}

void iterator(char* value) 
{
	log_info(memoria_logger,"%s", value);
}

char* leerArchivo(FILE* file)
{
	fseek(file,0,SEEK_END);

	int tamanioArchivo = ftell(file);

	rewind(file);

	char* contenido = malloc((tamanioArchivo + 1) * sizeof(char) );
	if( contenido == NULL )
	{
		printf("Error al intentar reservar memoria");
		return NULL;
	}

	size_t leidos = fread(contenido, sizeof(char), tamanioArchivo, file);
	if( leidos < tamanioArchivo )
	{
		printf("No se pudo leer el contenido del archivo");
		free(contenido);
		return NULL;
	}

	contenido[tamanioArchivo] = '\0';
	return contenido;
}

void abrir_archivo(char* path)
{
	FILE* file = fopen(path,"r");
	if( file == NULL )
	{
		printf("Error al abrir archivo, sorry");
	}
	char* content = leerArchivo(file);
	char** newContent = string_split(content,"\n");

	for(int i=0; newContent[i] != '\0'; i++)
	{
		printf("%s\n",newContent[i]);
	}

    free(content);
	fclose(file);
}

void memoria_escuchar_kernel (){
		bool control_key = 1;
			t_list* lista;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);
			printf("Recibi codigo de operacion\n");
			//debemos extraer el resto, primero el tamaño y luego el contenido
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			
			paquete->buffer = malloc(sizeof(t_newBuffer));

			recv(fd_kernel,&(paquete->buffer->size),sizeof(uint32_t),0);
			printf("Recibimos tamaño\n");
			
			paquete->buffer->stream = malloc(paquete->buffer->size);

			recv(fd_kernel,paquete->buffer->stream, paquete->buffer->size,0);

			printf("recibi stream\n");
			
			switch (cod_op) {
			case MENSAJE:
				//
				printf("Ejecute este mensaje MENSAJE jaja");
				break;
			case PAQUETE:
			//FUNCIONES PARA CUANDO RECIBAMOS PAQUETE
				//lista = recibir_paquete(fd_kernel);
				//log_info(memoria_logger, "Me llegaron los siguientes valores:\n");
				//list_iterate(lista, (void*) iterator);
				// //extraigp el elemento lista y muestro sus parametros
				// ProcesoMemoria* random = list_get(lista,0);
				// printf("El pid recibido es %d", random->PID);
				// printf("El path recibido es %s", random->path);
			//crea conecccion cuando ingresamos INICIAR_PROCESO
			//ABRIR ARCHIVO PSEUDOCODIGO
			ProcesoMemoria* nuevoProceso = deserializar_proceso_memoria(paquete->buffer);
			printf("El PID que recibi es: %d", nuevoProceso->PID);
			printf("El PID que recibi es: %s", nuevoProceso->path);
			      //char* path = "archivopseudocodigo";
			      //abrir_archivo(path);
			free(nuevoProceso);
			break;
			case -1:
				log_error(memoria_logger, "El cliente kernel se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(memoria_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
			//liberar memoria
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
			cod_op = 0;
		}
}




#endif /* MEMORIA_H_ */