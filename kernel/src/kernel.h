#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <utils/utils.h>
#include <utils/utils.c>

//file descriptors de kernel y los modulos que se conectaran con ella
int fd_kernel;
int fd_cpu_interrupt; 
int fd_cpu_dispatch; //luego se dividira en dos fd, un dispach y un interrupt, por ahora nos es suficiente con este
int fd_memoria;
int fd_entradasalida;
bool cpu_ocupada; //0 si no, 1 si sí

//colas y pid
int procesos_en_new = 0;
int procesos_fin = 0;

sem_t sem; //semaforo mutex region critica cola new
sem_t sem_ready; 
sem_t sem_cant; //semaforo cant de elementos en la cola
sem_t sem_cant_ready;
sem_t sem_mutex_plani_corto; //semaforo oara planificacion FIFO
sem_t sem_mutex_cpu_ocupada; //semaforo para indicar si la cpu esta ocupada

//TODAS LAS QUEUES
int pid = 0;
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_blocked;

//LISTA DE ENTRADAS Y SALIDAS
t_list* listGenericas;
t_list* listStdin;
t_list* listStdout;
t_list* listDialfs;

t_log *kernel_logger; // LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config *kernel_config;
t_log *kernel_logs_obligatorios;// LOGS MINIMOS Y OBLIGATORIOS

char *PUERTO_ESCUCHA;
char *IP_MEMORIA;
char *PUERTO_MEMORIA;
char *IP_CPU;
char *PUERTO_CPU_DISPATCH; 
char *PUERTO_CPU_INTERRUPT;
char *ALGORITMO_PLANIFICACION;
char *QUANTUM; //Da segmentation fault si lo defino como int
char* RECURSOS;
char* INSTANCIAS_RECURSOS;
char* GRADO_MULTIPROGRAMACION; //Da segmentation fault si lo defino como int

void ejecutar_interfaz_generica(char* instruccion, op_code tipoDeInterfaz)
{
	char** instruccion_split = string_split(instruccion," ");
	int* unidadesDeTrabajo = malloc(sizeof(int));
	*unidadesDeTrabajo = atoi(instruccion_split[2]);

/*
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
	buffer->size = sizeof(int);
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset,unidadesDeTrabajo, sizeof(int));
    buffer->offset += sizeof(int);

	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = tipoDeInterfaz;
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
    send(fd_entradasalida, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	*/
	enviarEntero(unidadesDeTrabajo,fd_entradasalida,GENERICA);
}


void ejecutar_interfaz_stdin(char* instruccion, op_code tipoDeInterfaz)
{
	char** instruccion_split = string_split(instruccion," ");
	//IO_STDIN_READ (Interfaz, Registro Direccion, Registro Tamaño)
	
}


void kernel_escuchar_cpu ()
{
	bool control_key = 1;
	while (control_key) {
	//recibimos operacion y guardamos todo en un paquete
			int cod_op = recibir_operacion(fd_cpu_dispatch);

			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_cpu_dispatch,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_cpu_dispatch,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case PROCESOFIN:
			//recibo el proceso de la CPU
			    PCB* proceso = deserializar_proceso_cpu(paquete->buffer);
				proceso->estado = EXIT;
				printf("Recibimos el proceso con el pid: %d\n",proceso->PID);
				printf("Recibimos el proceso con el AX: %d\n",proceso->registro.AX);//cambios
				printf("Recibimos el proceso con el BX: %d\n",proceso->registro.BX);//cambios
				printf("Recibimos el proceso con el CX: %d\n",proceso->registro.CX);//cambios
				printf("Recibimos el proceso con el DX: %d\n",proceso->registro.DX);//cambios
				printf("Recibimos el proceso con el EAX: %d\n",proceso->registro.EAX);//cambios
				printf("Recibimos el proceso con el EBX: %d\n",proceso->registro.EBX);//cambios
				printf("Recibimos el proceso con el ECX: %d\n",proceso->registro.ECX);//cambios
				printf("Recibimos el proceso con el EDX: %d\n",proceso->registro.EDX);//cambios
				printf("Recibimos el proceso con el SI: %d\n",proceso->registro.SI);//cambios
				printf("Recibimos el proceso con el DI: %d\n",proceso->registro.DI);//cambios
				printf("Este proceso ha terminado\n");

				enviarPCB(proceso,fd_memoria,PROCESOFIN);

				free(proceso->path);
				free(proceso);
				
				sem_wait(&sem_mutex_cpu_ocupada);
				cpu_ocupada = false;
				sem_post(&sem_mutex_cpu_ocupada);				
			//	
                break;
			case PROCESOIO:

				PCB* proceso_io = deserializar_proceso_cpu(paquete->buffer);
				proceso_io->estado = BLOCKED;
				printf("Recibimos el proceso con el pid: %d\n",proceso_io->PID);
				printf("Recibimos el proceso con el AX: %d\n",proceso_io->registro.AX);//cambios
				printf("Recibimos el proceso con el BX: %d\n",proceso_io->registro.BX);//cambios
				printf("Recibimos el proceso con el CX: %d\n",proceso_io->registro.CX);//cambios
				printf("Recibimos el proceso con el DX: %d\n",proceso_io->registro.DX);//cambios
				printf("Recibimos el proceso con el EAX: %d\n",proceso_io->registro.EAX);//cambios
				printf("Recibimos el proceso con el EBX: %d\n",proceso_io->registro.EBX);//cambios
				printf("Recibimos el proceso con el ECX: %d\n",proceso_io->registro.ECX);//cambios
				printf("Recibimos el proceso con el EDX: %d\n",proceso_io->registro.EDX);//cambios
				printf("Recibimos el proceso con el SI: %d\n",proceso_io->registro.SI);//cambios
				printf("Recibimos el proceso con el DI: %d\n",proceso_io->registro.DI);//cambios
				printf("Este SE HA BLOQUEADO\n");

				//lo ideal seria tambien agregarlo a una cola de la interfaz de cada proceso
				queue_push(cola_blocked, proceso_io);

				sem_wait(&sem_mutex_cpu_ocupada);
				cpu_ocupada = false;
				sem_post(&sem_mutex_cpu_ocupada);		

				break;
			case FIN_DE_QUANTUM:

				PCB* proceso_fin_de_quantum = deserializar_proceso_cpu(paquete->buffer);
				proceso_fin_de_quantum->estado = READY;
				printf("Recibimos el proceso con el pid: %d\n",proceso_fin_de_quantum->PID);
				printf("Recibimos el proceso con el AX: %d\n",proceso_fin_de_quantum->registro.AX);//cambios
				printf("Recibimos el proceso con el BX: %d\n",proceso_fin_de_quantum->registro.BX);//cambios
				printf("Recibimos el proceso con el CX: %d\n",proceso_fin_de_quantum->registro.CX);//cambios
				printf("Recibimos el proceso con el DX: %d\n",proceso_fin_de_quantum->registro.DX);//cambios
				printf("Recibimos el proceso con el EAX: %d\n",proceso_fin_de_quantum->registro.EAX);//cambios
				printf("Recibimos el proceso con el EBX: %d\n",proceso_fin_de_quantum->registro.EBX);//cambios
				printf("Recibimos el proceso con el ECX: %d\n",proceso_fin_de_quantum->registro.ECX);//cambios
				printf("Recibimos el proceso con el EDX: %d\n",proceso_fin_de_quantum->registro.EDX);//cambios
				printf("Recibimos el proceso con el SI: %d\n",proceso_fin_de_quantum->registro.SI);//cambios
				printf("Recibimos el proceso con el DI: %d\n",proceso_fin_de_quantum->registro.DI);//cambios
				printf("SE HA ACABADO EL QUANTUM DE ESTE PROCESO\n");

				//lo ideal seria tambien agregarlo a una cola de la interfaz de cada proceso
				sem_wait(&sem_ready);   // mutex hace wait
				queue_push(cola_ready,proceso_fin_de_quantum);	//agrega el proceso a la cola de ready
  				sem_post(&sem_ready); 
				sem_post(&sem_cant_ready);  // mutex hace wait
				
				sem_wait(&sem_mutex_cpu_ocupada);
				cpu_ocupada = false;
				sem_post(&sem_mutex_cpu_ocupada);	
				break;
			case GENERICA:
				
				Instruccion_io* instruccion_io_gen = deserializar_instruccion_io(paquete->buffer);
				printf("La instruccion es: %s\n",instruccion_io_gen->instruccion);
				printf("El PID de la instruccion es: %d\n",instruccion_io_gen->proceso.PID);

				//asumiendo un unico elemento en la lista
				//EntradaSalida* io_gen = list_get(listGenericas,0);

				//como por ahora tenemos un único elemento, ejecutamos directamente

				//ejecutar funcion para enviar a dormir a la interfaz
				printf("Checkpoin1 \n");
				ejecutar_interfaz_generica(instruccion_io_gen->instruccion,GENERICA);
				//
				break;
			case STDIN:
				
				Instruccion_io* instruccion_io_stdin = deserializar_instruccion_io(paquete->buffer);
				printf("La instruccion es: %s\n",instruccion_io_stdin->instruccion);
				printf("El PID de la instruccion es: %d\n",instruccion_io_stdin->proceso.PID);
				//ejecutar funcion para la interfaz
				printf("Checkpoin1 \n");
				ejecutar_interfaz_stdin(instruccion_io_stdin->instruccion,STDIN);
				//
				break;
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(kernel_logger, "Desconexion de cpu.");
				control_key = 0;
			default:
				log_warning(kernel_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}

void kernel_escuchar_entradasalida ()
{
	bool control_key = 1;
	while (control_key) {

			int cod_op = recibir_operacion(fd_entradasalida);

			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_entradasalida,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_entradasalida,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case GENERICA:

				EntradaSalida* new_io_generica = deserializar_entrada_salida(paquete->buffer);
				printf("Llego una IO cuyo nombre es: %s\n",new_io_generica->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_generica->path);
				
				list_add(listGenericas,new_io_generica);
			    break;
			case STDIN:
				EntradaSalida* new_io_stdin = deserializar_entrada_salida(paquete->buffer);
				printf("Llego una IO cuyo nombre es: %s\n",new_io_stdin->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_stdin->path);
				
				list_add(listGenericas,new_io_stdin);			
			    break;
			case STDOUT:
				EntradaSalida* new_io_stdout = deserializar_entrada_salida(paquete->buffer);
				printf("Llego una IO cuyo nombre es: %s\n",new_io_stdout->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_stdout->path);
				
				list_add(listGenericas,new_io_stdout);			
			    break;
			case DIALFS:
				EntradaSalida* new_io_dialfs = deserializar_entrada_salida(paquete->buffer);
				printf("Llego una IO cuyo nombre es: %s\n",new_io_dialfs->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_dialfs->path);
				
				list_add(listGenericas,new_io_dialfs);
			    break;
			case DESPERTAR:
			//sacamos al proceso de la cola de blocked y lo añadimos a IO
				printf("La IO ha despertado :O\n");
				PCB* proceso_awaken = queue_pop(cola_blocked);
				proceso_awaken->estado = READY;
				//meter en ready
				sem_wait(&sem_ready);   // mutex hace wait
				queue_push(cola_ready,proceso_awaken);	//agrega el proceso a la cola de ready
    			sem_post(&sem_ready); 
				sem_post(&sem_cant_ready);
			    break;
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(kernel_logger, "El cliente EntradaSalida se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(kernel_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}

void kernel_escuchar_memoria ()
{
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
				log_error(kernel_logger, "Desconexion de memoria.");
				control_key = 0;
			default:
				log_warning(kernel_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}	
}

void iniciar_proceso(char* path)
{
	PCB* pcb = malloc(sizeof(PCB));
	if ( pcb == NULL )
	{
		printf("Error al crear pcb");
	}

	//inicializo el PCB del proceso
	pid++;
	pcb->PID = pid;
	pcb->PC = 0;
	pcb->quantum = 0;
	pcb->registro.AX = 0;//cambios
	pcb->registro.BX = 0;//cambios
	pcb->registro.CX = 0;//cambios
	pcb->registro.DX = 0;//cambios
	pcb->registro.EAX = 0;//cambios
	pcb->registro.EBX = 0;//cambios
	pcb->registro.ECX = 0;//cambios
	pcb->registro.EDX = 0;//cambios
	pcb->registro.DI = 0;//cambios
	pcb->registro.SI = 0;//cambios
	pcb->estado = NEW;
	pcb->path = string_duplicate(path); //consejo

	//agrego el pcb a la cola new
	
	sem_wait(&sem);
	queue_push(cola_new,pcb);	
    // Señalar (incrementar) el semáforo
    sem_post(&sem);
	sem_post(&sem_cant);

	//procesos_en_new++;
	
	log_info (kernel_logs_obligatorios, "Se crea el proceso %d en NEW, funcion iniciar proceso\n", pcb->PID);
}

void atender_instruccion (char* leido)
{
    char** comando_consola = string_split(leido, " ");
	//printf("%s\n",comando_consola[0]);

    if((strcmp(comando_consola[0], "INICIAR_PROCESO") == 0))
	{ 
        iniciar_proceso(comando_consola[1]);  
    }else if(strcmp(comando_consola [0], "FINALIZAR_PROCESO") == 0){
    }else if(strcmp(comando_consola [0], "DETENER_PLANIFICACION") == 0){
    }else if(strcmp(comando_consola [0], "INICIAR_PLANIFICACION") == 0){
    }else if(strcmp(comando_consola [0], "MULTIPROGRAMACION") == 0){ 
    }else if(strcmp(comando_consola [0], "PROCESO_ESTADO") == 0){
    }else if(strcmp(comando_consola [0], "HELP") == 0){
    }else if(strcmp(comando_consola [0], "PRINT") == 0){
    }else{   
        log_error(kernel_logger, "Comando no reconocido, pero que paso el filtro ???");  
        exit(EXIT_FAILURE);
    }
    string_array_destroy(comando_consola); 
}

void validarFuncionesConsola(char* leido)
{
	 char** valorLeido = string_split(leido, " ");
	 //printf("%s\n",valorLeido[0]);

	 if(strcmp(valorLeido[0], "EJECUTAR_SCRIPT") == 0)
	 {
		printf("Comando válido\n");
	 }
	 else
	 {
		if(strcmp(valorLeido[0], "INICIAR_PROCESO") == 0)
	    {
		    printf("Comando válido\n");
			atender_instruccion(leido);
	    }
		else
		{
			if(strcmp(valorLeido[0], "FINALIZAR_PROCESO") == 0)
	        {  
		         printf("Comando válido\n");
	        }
			else
			{
				if(strcmp(valorLeido[0], "DETENER_PLANIFICACION") == 0)
	            {  
		             printf("Comando válido\n");
	            }
				else
				{
					if(strcmp(valorLeido[0], "INICIAR_PLANIFICACION") == 0)
	                {  
		                printf("Comando válido\n");
	                }
					else
					{
						if(strcmp(valorLeido[0], "MULTIPROGRAMACION") == 0)
	                    {  
		                       printf("Comando válido\n");
	                    }
						else
						{
							if(strcmp(valorLeido[0], "PROCESO_ESTADO") == 0)
	                        {  
		                       printf("Comando válido\n");
	                        }
						}
					}
				}
			}
		}
	 }
	 string_array_destroy(valorLeido);
}

void consolaInteractiva()
{
	char* leido;
	leido = readline("> ");
	
	while( strcmp(leido,"") != 0 )
	{
		validarFuncionesConsola(leido);
		free(leido);
		leido = readline("> ");
	}
}

void enviarProcesoMemoria (ProcesoMemoria* proceso, int socket_servidor)
{
    //Creamos un Buffer
    t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
    buffer->size = sizeof(uint32_t)*2 + proceso->path_length +1;//MODIFCADO
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset, &proceso->PID, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);

    // Para el nombre primero mandamos el tamaño y luego el texto en sí:
    memcpy(buffer->stream + buffer->offset, &proceso->path_length, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, proceso->path, proceso->path_length);
    
	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = PAQUETE;
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

void informar_memoria_nuevo_procesoNEW()
{
	//CREAMOS BUFFER
	//POR EL MOMENTO ESTAMOS HARCODEANDO para intentar mandar algo a memoria
	ProcesoMemoria* proceso = malloc(sizeof(ProcesoMemoria));

	PCB* pcb = queue_pop(cola_new);//
	proceso->path = malloc(strlen(pcb->path)+1);
	//proceso->path = pcb->path;
	strcpy(proceso->path, pcb->path);
	proceso->PID = pcb->PID;
	proceso->path_length = strlen(pcb->path)+1;
	proceso->TablaDePaginas = list_create();
	
	enviarProcesoMemoria(proceso,fd_memoria);
	
	sem_wait(&sem_ready);   // mutex hace wait
	queue_push(cola_ready,pcb);	//agrega el proceso a la cola de ready
    sem_post(&sem_ready); 
	sem_post(&sem_cant_ready);  // mutex hace wait
	
}

void finalizar_proceso ()
{
}

void mover_procesos_ready(int grado_multiprogramacion)
{
	//CONSULTA. TENDREMOS QUE USAR SEMAFOROS?
    //Cantidad de procesos en las colas de NEW y READY
    int cantidad_procesos_new = queue_size(cola_new);
    int cantidad_procesos_ready = queue_size(cola_ready);

    //El grado de multiprogramación lo permite?
    if (cantidad_procesos_ready < grado_multiprogramacion)
    {
        // Mover procesos de la cola de NEW a la cola de READY
        while (cantidad_procesos_new > 0 && cantidad_procesos_ready < grado_multiprogramacion)
        {
            // Seleccionar el primer proceso de la cola de NEW y los borramos de la cola
            PCB* proceso_nuevo = queue_pop(cola_new);
           // queue_pop(cola_new);

            // Cambiar el estado del proceso a READY
            proceso_nuevo->estado = READY;

            // Agregar el proceso a la cola de READY
		

			queue_push(cola_ready, proceso_nuevo);
            cantidad_procesos_ready++;
			
		
           

            // Reducir la cantidad de procesos en la cola de NEW
            cantidad_procesos_new--;
        }
    }
    else
    {
        printf("El grado de multiprogramación máximo ha sido alcanzado. %d procesos permanecerán en la cola de NEW.\n",cantidad_procesos_new);
    }
}

void planificador_largo_plazo()
{
    // //Obtenemos el grado de multiprogramación especificado por el archivo de configuración
    // //int grado_multiprogramacion = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");
	// int grado_multiprogramacion = 20;
	// //pregunto constantemente
	// while(1)
	// {
	// 	//hay nuevos procesos en new?
	// 	if( procesos_en_new > 0 )
	// 	{
	// 		//avisar a memoria
	// 		//USAR QUEUE Pop
	while(1)
	{
		//SEMAFOROS (PRODUCTOR-CONSUMIDOR)  consumidor de la cola de NEW

		//if( queue_size(cola_new) > 0)
		//{	
		    sem_wait(&sem_cant);   // mutex hace wait
		    sem_wait(&sem);   // mutex hace wait

			informar_memoria_nuevo_procesoNEW();
			
			sem_post(&sem);   // mutex hace signal

			//sem_post(&sem_cant_ready); //avisamos al planificador que hay un nuevo proceso listo
			
		//}

	}			
	// 	}
	// 	else
	// 	{
	// 		//algun proceso terminó?
	// 		if(procesos_fin > 0 )
	// 		{
	// 			//
	// 			printf("esta parte es la del fin del proceso");
	// 		}
	// 	}
	// 	//luego de esto muevo los procesos a ready mientras pueda
	// 	//mover_procesos_ready(grado_multiprogramacion);
	// 	sleep(1);
	//}	
}

void enviar_pcb_a_cpu()
{
	sem_wait(&sem_mutex_cpu_ocupada);
	while( cpu_ocupada != false )
	{
		sem_post(&sem_mutex_cpu_ocupada);
		sleep(1);
		sem_wait(&sem_mutex_cpu_ocupada);
	}
	sem_post(&sem_mutex_cpu_ocupada);
	//Reservo memoria para enviarla
	PCB* to_send = malloc(sizeof(PCB));

	sem_wait(&sem_cant_ready);   // mutex hace wait
	sem_wait(&sem_ready);   // mutex hace wait

	PCB* pcb_cola = queue_pop(cola_ready); //saca el proceso de la cola de ready
			
	sem_post(&sem_ready); // mutex hace signal
	

	to_send->PID = pcb_cola->PID;
	to_send->PC = pcb_cola->PC;
	to_send->quantum = pcb_cola->quantum;
	to_send->estado = EXEC;
	to_send->registro = pcb_cola->registro;//cambios
	/*to_send->registro.BX = pcb_cola->registro.BX;//cambios
	to_send->registro.CX = pcb_cola->registro.CX;//cambios
	to_send->registro.DX = pcb_cola->registro.DX;//cambios*/

	to_send->path = string_duplicate( pcb_cola->path);

	printf("Enviaremos un proceso\n");
	enviarPCB(to_send, fd_cpu_dispatch,PROCESO);
	sem_wait(&sem_mutex_cpu_ocupada);
	cpu_ocupada = true;
	sem_post(&sem_mutex_cpu_ocupada);
}


void interrumpir_por_quantum()
{
	printf("Entre a dormir un poco para el quantum\n");
	int quantumAUsar = atoi(QUANTUM);
	printf("El quantum sera: %d\n",quantumAUsar);
	sleep(quantumAUsar/1000);
	//MANDAR A MEMORIA FIN DE QUANTUM POR DISPATCH
	int* enteroRandom = malloc(sizeof(int));
	*enteroRandom = 0;
 	printf("Me desperte uwu\n");
	enviarEntero(enteroRandom,fd_cpu_interrupt,FIN_DE_QUANTUM);
}


void planificador_corto_plazo()
{

		if(strcmp(ALGORITMO_PLANIFICACION,"FIFO")==0)
		{
			printf("Planificare por FIFO\n");
			while(1)
			{
				//PLANIFICAR POR FIFO
				sem_wait(&sem_mutex_plani_corto);
				enviar_pcb_a_cpu();
				sem_post(&sem_mutex_plani_corto);
			}
		}
		
		if(strcmp(ALGORITMO_PLANIFICACION,"RR")==0)
		{
			printf("Planificare por RR\n");
			//PLANIFICAR POR RR
			while(1)
			{
				sem_wait(&sem_mutex_plani_corto);
				enviar_pcb_a_cpu();
				sem_post(&sem_mutex_plani_corto);
				interrumpir_por_quantum();
			}
		}
		if(strcmp(ALGORITMO_PLANIFICACION,"VRR")==0)
		{

			//PLANIFICAR POR VRR
		}

}
	

#endif /* KERNEL_H_ */
