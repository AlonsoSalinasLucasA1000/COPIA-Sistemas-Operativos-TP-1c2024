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

sem_t protect_list_procesos;

t_log* memoria_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* memoria_config;
t_list* listProcesos; 
t_list* listMarcos;

t_list* listStdin;
t_list* listStdout;
t_list* listDialfs;

char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;

void* espacio_usuario;
void* espacio_pagina;

t_list* generarListaDeMarcos(t_list* listMarcos)
{
	t_list* to_return = list_create();
	//agregamos -1 en todos los marcos para indicar que están libres
	int i = 0;
	while( i < (TAM_MEMORIA / TAM_PAGINA))
	{
		int* valor = malloc(sizeof(int));
		*valor = -1;
		list_add(to_return,valor);
		i++;
	}
	return to_return;
}

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

int cantiMarcosLibresMemoria(t_list* listMarcos)
{
    int contador = 0;
    for(int i = 0; i < list_size(listMarcos); i++) {
        int* elemento = list_get(listMarcos, i);
        if(elemento != NULL && *elemento == -1) {
            contador++;
        }
    }
    return contador;
}

int dividir_y_redondear_arriba(int num1, int num2) 
{
    if (num2 == 0) {
        // Manejar división por cero
        return -1;
    }
    double division = (double)num1 / num2;
    return (int)ceil(division);
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

			printf("Recibi el codigo operacion\n");

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
				
				sem_wait(&protect_list_procesos);
				ProcesoMemoria* datos = encontrarProceso(listProcesos,proceso->PID);
				sem_post(&protect_list_procesos);
				printf("Los datos encontrados son los siguientes pid: %d\n",datos->PID);
				printf("Los datos encontrados son los siguientes path: %s\n",datos->path);

				//leemos la linea indicada por el PC
			    char* instruccion = abrir_archivo(datos->path, proceso->PC); 
				printf("Enviaremos a la cpu: %s\n",instruccion); 
				//enviamos, sin antes dormir el tiempo esperado
				printf("Me voy a mimir, buenas noches\n");
				sleep(RETARDO_RESPUESTA/1000);
				enviar_mensaje_cpu_memoria(instruccion,fd_cpu,MENSAJE);
				break;

			case RESIZE:
				PCB* proceso_resize = deserializar_proceso_cpu(paquete->buffer);
				printf("Los datos recibidos de CPU para HACER RESIZE son pid: %d\n",proceso->PID);
				printf("Los datos recibidos de CPU para HACER RESIZE pc: %d\n",proceso->PC);
				
				sem_wait(&protect_list_procesos);
				ProcesoMemoria* datos_resize = encontrarProceso(listProcesos,proceso_resize->PID);
				sem_post(&protect_list_procesos);
				printf("Los datos encontrados son los siguientes pid: %d\n",datos_resize->PID);
				printf("Los datos encontrados son los siguientes path: %s\n",datos_resize->path);

				printf("Voy a asignarle marcos al proceso\n");
				//veamos el tamaño actual de la lista del proceso
				//dejamos especificado en el path length el tamaño de resize que queremos
				
				//cuantas paginas necesita?
				int paginas_requeridas = dividir_y_redondear_arriba(proceso_resize->path_length,TAM_PAGINA);
				int tam_actual_tabla = list_size(datos_resize->TablaDePaginas);
				if( tam_actual_tabla < paginas_requeridas )
				{
					if( cantiMarcosLibresMemoria(listMarcos) >= paginas_requeridas )
					{
					//Asignar marcos
						int i = 0;
						while( i < list_size(listMarcos) && paginas_requeridas != 0 )
						{
							//aquellos elementos que sean -1 se asignan hasta acabar con la cantidad de resizes pedidos
							int* marco = list_get(listMarcos,i);
							if( *marco == -1 )
							{
								//reemplazo dicha casilla con el pid del proceso
								uint32_t* pid = malloc(sizeof(u_int32_t));
								*pid = proceso_resize->PID;
								list_replace(listMarcos,i,pid);
								paginas_requeridas--;
							}
							i++; 
						}
						printf("LLEGUE HASTA ACA EN LA PARTE DE ASIGNAR MARCOS1");
						printf("Pude asignarle, la lista de marcos quedó así:\n");
						int j = 0;
						while( j < list_size(listMarcos) )
						{
							int* number = list_get(listMarcos,j);
							if (number != NULL) 
							{
								printf("El marco numero %d\n",j);
								printf("Esta asignado al proceso %d\n",*number);
							} 
							else 
							{
								printf("El marco numero %d no está asignado a ningún proceso",j);
							}
							j++;
							printf("-----------------\n");					
						}
						printf("LLEGUE HASTA ACA EN LA PARTE DE ASIGNAR MARCOS2");
						for(int k = 0; k < list_size(listMarcos); k++)
						{
							int* marco = list_get(listMarcos,k);
							if( *marco == proceso->PID )
							{
								//añado el marco a la tabla de paginas del proceso
								int* to_add = malloc(sizeof(int));
								*to_add = k;
								list_add(datos_resize->TablaDePaginas,to_add);
							}
						}

						printf("LLEGUE HASTA ACA EN LA PARTE DE ASIGNAR MARCOS3");
						//pude asignar los marcos, la lista del proceso quedó así:
						int l = 0;
						while( l < list_size(datos_resize->TablaDePaginas) )
						{
							int* number = list_get(datos_resize->TablaDePaginas,l);
							if (number != NULL) 
							{
								printf("la pagina numero %d\n",l);
								printf("Esta asignado al marco %d\n",*number);
							} 
							else 
							{
								printf("explosion\n");
							}
							l++;
							printf("-----------------\n");					
						}

						//EXITOSA ASIGNACION
						int hola = 20;
						char* to_send = malloc(hola);
						strncpy(to_send, "vegeta", hola - 1);
						to_send[hola - 1] = '\0';
						enviar_mensaje_cpu_memoria(instruccion,fd_cpu,ASIGNACION_CORRECTA);
						free(to_send);
					}
					else
					{
						//OUT of memory
						int hola = 20;
						char* to_send = malloc(hola);
						strncpy(to_send, "vegeta", hola - 1);
						to_send[hola - 1] = '\0';
						enviar_mensaje_cpu_memoria(instruccion,fd_cpu,OUT_OF_MEMORY);
						free(to_send);
					}
				}
				else
				{
					//reducir
					//Ahora el ser menor la cantidad de paginas debo ir quitando paginas de la tabla y liberando el marco
					int i = list_size(datos_resize->TablaDePaginas)-1;
					while( i+1 != paginas_requeridas )
					{
						//me muevo al ultimo elemento de la tabla de paginas y obtengo el marco asignado
						int* marco_asignado = list_get(datos_resize->TablaDePaginas, i);
						//libero el marco
						int* liberado = malloc(sizeof(int));
						*liberado = (-1);
						list_replace(listMarcos,*marco_asignado,liberado);

						//saco el elemento de la tabla
						list_remove(datos_resize->TablaDePaginas,i);
						i--;
					}

					printf("Pude asignarle, la lista de marcos quedó así:\n");
					int j = 0;
					while( j < list_size(listMarcos) )
					{
						int* number = list_get(listMarcos,j);
						if (number != NULL) 
						{
							printf("El marco numero %d\n",j);
							printf("Esta asignado al proceso %d\n",*number);
						} 
						else 
						{
							printf("El marco numero %d no está asignado a ningún proceso",j);
						}
						j++;
						printf("-----------------\n");					
					}

					//pude asignar los marcos, la lista del proceso quedó así:
					int l = 0;
					while( l < list_size(datos_resize->TablaDePaginas) )
					{
						int* number = list_get(datos_resize->TablaDePaginas,l);
						if (number != NULL) 
						{
							printf("la pagina numero %d\n",l);
							printf("Esta asignado al marco %d\n",*number);
						} 
						else 
						{
							printf("explosion\n");
						}
						l++;
						printf("-----------------\n");					
					}

					//EXITOSA ASIGNACION, o exitosa reduccion
					int hola = 20;
					char* to_send = malloc(hola);
					strncpy(to_send, "vegeta", hola - 1);
					to_send[hola - 1] = '\0';
					enviar_mensaje_cpu_memoria(instruccion,fd_cpu,ASIGNACION_CORRECTA);
					free(to_send);

				}
				break;
			case LECTURA:
				
				void* copy_stream = paquete->buffer->stream;
				printf("He recibido un pedido de LECTURA\n");
				int* direccionFisica = malloc(sizeof(int));
				int* tamanioDato = malloc(sizeof(int));		//puede que sea int*
				memcpy(direccionFisica,copy_stream,sizeof(int));
				copy_stream += sizeof(int);
				memcpy(tamanioDato,copy_stream,sizeof(int));

				printf("Recibimos la direccion fisica: %d\n",*direccionFisica);
				printf("Recibimos la del tamaño del dato: %d\n",*tamanioDato);


				//OBTENER LO ALMACENADO EN LA DIRECCION FISICA
				int* datoObtenido = malloc(sizeof(int));
				memcpy(datoObtenido,espacio_usuario + *direccionFisica,*tamanioDato);
				printf("Enviaremos %d\n",*datoObtenido);
				enviarEntero(datoObtenido,fd_cpu,LECTURA);
				//
				break;
			/*
			case ESCRITURA_CADENA:
				void* copia_stream = paquete->buffer->stream;
				printf("He recibido un pedido de ESCRITURA_CADENA\n");
				int* direccionFisica = malloc(sizeof(int));
				int* tamanioCadena = malloc(sizeof(int)); 
				char* cadena = malloc(sizeof(char));
				
				memcpy(direccionFisica, copia_stream, sizeof(int));
				copia_stream += sizeof(int);
				memcpy(tamanioCadena, copia_stream, sizeof(int));
				copia_stream += sizeof(int);
				memcpy(cadena, copia_stream, *tamanioCadena);
				
				//OBTENER LO ALMACENADO EN LA DIRECCION FISICA
				memmove (espacio_usuario + *direccionFisica, cadena, *tamanioCadena);

				void* to_show = malloc(*tamanioCadena);
				memcpy(to_show,espacio_usuario + *direccionFisica,*tamanioCadena);

				char* show = (char*)to_show;
				printf("Mostrar lo escrito %s",show);

Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings. 
extern void *memmove (void *__dest, const void *__src, size_t __n)
     __THROW __nonnull ((1, 2));


			
				break;  
			*/
			case ESCRITURA_NUMERICO:

				void* copy_stream_e_n = paquete->buffer->stream;
				printf("He recibido un pedido de ESCRITURA_NUMERICO\n");
				int* direccionFisica_e_n = malloc(sizeof(int));
				int* valor = malloc(sizeof(int));
				memcpy(direccionFisica_e_n,copy_stream_e_n,sizeof(int));
				copy_stream_e_n += sizeof(int);
				memcpy(valor,copy_stream_e_n,sizeof(int));
				
				printf("Recibimos la siguiente direccion: %d\n",*direccionFisica_e_n);//BX
				printf("Recibimos el siguiente valor a escribir: %d\n",*valor);//AX
				memmove(espacio_usuario + *direccionFisica_e_n, valor, sizeof(int));

				void* to_show_n = malloc(sizeof(int));
				memcpy(to_show_n,espacio_usuario + *direccionFisica_e_n, sizeof(int));

				int* show_n = (int*)to_show_n;
				printf("Mostrar lo escrito %d\n",*show_n);

				int hola = 20;
				char* to_send = malloc(hola);
				strncpy(to_send, "vegeta", hola - 1);
				to_send[hola - 1] = '\0';
				enviar_mensaje_cpu_memoria(to_send,fd_cpu,ESCRITO);
				free(to_send);
				
				break;
			case PAQUETE:
				//
				break;
			case MARCO:
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

			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_entradasalida,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_entradasalida,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case STDIN:

				EntradaSalida* new_io_stdin = deserializar_entrada_salida(paquete->buffer);
				printf("Llego una IO cuyo nombre es: %s\n",new_io_stdin->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_stdin->path);
				
				list_add(listStdin,new_io_stdin);
				//
				break;
			case STDOUT:

				EntradaSalida* new_io_stdout = deserializar_entrada_salida(paquete->buffer);
				printf("Llego una IO cuyo nombre es: %s\n",new_io_stdout->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_stdout->path);
				
				list_add(listStdout,new_io_stdout);
				//
				break;
			case DIALFS:

				EntradaSalida* new_io_dialfs = deserializar_entrada_salida(paquete->buffer);
				printf("Llego una IO cuyo nombre es: %s\n",new_io_dialfs->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_dialfs->path);
				
				list_add(listStdin,new_io_dialfs);
				//
				break;
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
		//to_ret = newContent[PC];
		if (to_ret == NULL) {
        printf("Error al asignar memoria para to_ret\n");
        // Manejar el error apropiadamente
    	}
    	
		strcpy(to_ret, newContent[PC]); // Copiar el contenido de newContent[PC] a to_ret
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
					//hay que liberar los marcos
					int i = 0;
					while( i < list_size(listMarcos) )
					{
						int* got = list_get(listMarcos,i);
						if( *got == proceso->PID )
						{
							int* to_add = malloc(sizeof(int));
							*to_add = (-1);
							list_replace(listMarcos,i,to_add);
						}
						i++;
					}

					printf("Pude asignarle, la lista de marcos quedó así:\n");
					int j = 0;
					while( j < list_size(listMarcos) )
					{
						int* number = list_get(listMarcos,j);
						if (number != NULL) 
						{
							printf("El marco numero %d\n",j);
							printf("Esta asignado al proceso %d\n",*number);
						} 
						else 
						{
							printf("El marco numero %d no está asignado a ningún proceso",j);
						}
						j++;
						printf("-----------------\n");					
					}
					
					sem_wait(&protect_list_procesos);
					ProcesoMemoria* dato = encontrarProceso(listProcesos,proceso->PID);
					sem_post(&protect_list_procesos);
					printf("Borraremos [CONFIRMADO] el proceso con pid: %d\n", dato->PID);
					list_destroy(dato->TablaDePaginas);
					free(dato->path);
					free(dato);
					printf("Proceso borrado con exito\n");
					break;
			case PAQUETE:
				ProcesoMemoria* nuevoProceso = deserializar_proceso_memoria(paquete->buffer);
				nuevoProceso->TablaDePaginas = list_create();//creamos la lista de páginas
				if(nuevoProceso != NULL){ 
					//añadimos el proceso a la lista de procesos que posee la memoria, además le creamos un array con su tabla de páginas
					//printf("Me voy a mimir, buenas noches\n");
					//sleep(RETARDO_RESPUESTA); INCLUIR EL SLEEP ACA HARIA QUE CAMBIASEMOS GRAN PARTE DE LA SINCRONIZACION ENTRE MEMORIA Y KERNEL EN LA CREACION DE PROCESOS
					sem_wait(&protect_list_procesos);
					list_add(listProcesos, nuevoProceso);
					sem_post(&protect_list_procesos);
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