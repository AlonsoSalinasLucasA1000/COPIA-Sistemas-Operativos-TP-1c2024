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
t_list* listProcesos; 

char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;

char* abrir_archivo(char* path, int PC);

ProcesoMemoria* encontrarProceso(t_list* lista, uint32_t pid)
{
	ProcesoMemoria* ret;
	int i = 0;
	while( i < list_size(lista) )
	{
		ProcesoMemoria* got = list_get(lista,i);
		if( got->PID == pid)
		{
			ret = got;
		}
		i++;
	}
	return ret;
}

void memoria_escuchar_cpu (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_cpu);

			
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_cpu,&(paquete->buffer->size),sizeof(uint32_t),0);
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_cpu,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case MENSAJE: 
				//
				break;
			case PROCESO:
				//
				printf("------------------------------\n");
				//desearializamos lo recibido
				
				PCB* proceso = deserializar_proceso_cpu(paquete->buffer);
				printf("Los datos recibidos de CPU son pid: %d\n",proceso->PID);
				printf("Los datos recibidos de CPU son pc: %d\n",proceso->PC);
				
				//REVISAR, PORQUE SOLO RETORNA EL PRIMER ELEMENTO DE LA LISTA. DE TENER VARIOS PROCESOS DENTRO DE ELLA, ESTARÍA REGRESANDO SIEMPRE EL MISMO.
				//ProcesoMemoria* datos = list_get(listProcesos,0);
				ProcesoMemoria* datos = encontrarProceso(listProcesos,proceso->PID);
				printf("Los datos encontrados son los siguientes pid: %d\n",datos->PID);
				printf("Los datos encontrados son los siguientes path: %s\n",datos->path);

				//leemos la linea indicada por el PC
			    char* instruccion = abrir_archivo(datos->path, proceso->PC); 
				printf("Enviaremos a la cpu: %s\n",instruccion); 
				//enviamos
				enviar_mensaje_cpu_memoria(instruccion,fd_cpu);

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
			//Liberando los paquetes solucionamos los errores presentados anteriormente
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
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
				log_error(memoria_logger, "El cliente EntradaSalida se desconecto. Terminando servidor\n");
				control_key = 0;
			default:
				log_warning(memoria_logger,"Operacion desconocida. No quieras meter la pata\n");
				break;
			}
		}
}

//No definido exactamente
void atender_entrada_salida_memoria(int fd_cliente_entrada_salida)
{
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_cliente_entrada_salida);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				printf("Nada, por ahora.\n");
				control_key = 0;
			default:
				printf("Nada, por ahora.\n");
				break;
			}
		}
}

//creo un hilo por cada conexion nueva de E/S
void esperar_entrada_salida_memoria()
{
	while (1) 
	{
     pthread_t thread;
     int *fd_conexion_ptr = malloc(sizeof(int));
     *fd_conexion_ptr = accept(fd_memoria, NULL, NULL);
	 handshakeServer(*fd_conexion_ptr);
     pthread_create(&thread,NULL,(void*)atender_entrada_salida_memoria,fd_conexion_ptr);
     pthread_detach(thread);
    }
}

void iterator(char* value) 
{
	log_info(memoria_logger,"%s", value);
}

char* leerArchivo(FILE* file)
{
	fseek(file,0,SEEK_END); //

	int tamanioArchivo = ftell(file);

	rewind(file);

	char* contenido = malloc((tamanioArchivo + 1) * sizeof(char) );  
	if( contenido == NULL )
	{
		printf("Error al intentar reservar memoria\n");
		return NULL;
	}

	size_t leidos = fread(contenido, sizeof(char), tamanioArchivo, file);
	if( leidos < tamanioArchivo )
	{
		printf("No se pudo leer el contenido del archivo\n");
		free(contenido);
		return NULL;
	}

	contenido[tamanioArchivo] = '\0';

	return contenido;
}

int calcularCantidadDeInstrucciones(char* content)
{
	int i = 0;
	int cantInstrucciones = 0;
	while( i < strlen(content) )
	{
		if( content[i] == '\n' )
		{
			cantInstrucciones++;
		}
		i++;
	}
	return cantInstrucciones+1;
}

char* abrir_archivo(char* path, int PC)
{
	FILE* file = fopen(path,"r");
	if( file == NULL )
	{
		printf("Error al abrir archivo, sorry\n");
	}
	char* content = leerArchivo(file);
	char** newContent = string_split(content,"\n");

	int cantInstrucciones = calcularCantidadDeInstrucciones(content);
	printf("La cantidad de instrucciones que tiene es: %d\n", cantInstrucciones);

	char* to_ret;

	if( PC < cantInstrucciones )
	{
		to_ret = malloc(strlen(newContent[PC])+1);
		to_ret = newContent[PC];
	}
	else
	{
		to_ret = "";
	}

    free(content);
	fclose(file);
	return to_ret;
}

void memoria_escuchar_kernel (){
		bool control_key = 1;
		
		
			//t_list* lista;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);

			//debemos extraer el resto, primero el tamaño y luego el contenido
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_kernel,&(paquete->buffer->size),sizeof(uint32_t),0);			
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_kernel,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case MENSAJE:
				//
					printf("Hemos recibido un mensaje del kernel\n");
					break;
			case PROCESOFIN:
			//recibimos la instruccion de finalizar un proceso
		            printf("Hemos recibido la orden de eliminar un proceso que ha finalizado\n");
					PCB* proceso = deserializar_proceso_cpu(paquete->buffer);
					
					printf("Borraremos el proceso con pid: %d\n",proceso->PID);

					ProcesoMemoria* dato = encontrarProceso(listProcesos,proceso->PID);
					printf("Borraremos [CONFIRMADO] el proceso con pid: %d\n", dato->PID);
					free(dato->path);
					free(dato);
					printf("Proceso borrado con exito\n");
					break;
			case PAQUETE:
				ProcesoMemoria* nuevoProceso = deserializar_proceso_memoria(paquete->buffer);
				if(nuevoProceso != NULL){ 
					//list proceso no se aniade correctamente a la lista
					list_add(listProcesos, nuevoProceso);
					printf("El PID que recibi es: %d\n", nuevoProceso->PID);
					printf("El PATH que recibi es: %s\n", nuevoProceso->path);
				} else{
					printf("No se pudo deserializar\n");
				}
			break;
			case -1:
				log_error(memoria_logger, "El cliente kernel se desconecto. Terminando servidor\n");
				control_key = 0;

			default:
				log_warning(memoria_logger,"Operacion desconocida. No quieras meter la pata\n");
				break;
			}
			//liberar memoria
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
			//cod_op = 0;
		}

}




#endif /* MEMORIA_H_ */