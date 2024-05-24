#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <utils/utils.h>
#include <utils/utils.c>

//file descriptors de kernel y los modulos que se conectaran con ella
int fd_kernel;
int fd_cpu_interrupt; 
int fd_cpu_dispach; //luego se dividira en dos fd, un dispach y un interrupt, por ahora nos es suficiente con este
int fd_memoria;
int fd_entradasalida;

//colas y pid
int procesos_en_new = 0;
int procesos_fin = 0;

int pid = 0;
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_blocked;

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

//semaforos
//sem_t planificador = 0;

void kernel_escuchar_cpu ()
{
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_cpu_dispach);
			switch (cod_op) {
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
		}	
}

void kernel_escuchar_entradasalida ()
{
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
				log_error(kernel_logger, "El cliente EntradaSalida se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(kernel_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
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
	pcb->registro.AX = 0;
	pcb->registro.BX = 0;
	pcb->registro.CX = 0;
	pcb->registro.DX = 0;
	pcb->estado = NEW;
	pcb->path = path;

	//agrego el pcb a la cola new
	queue_push(cola_new,pcb);
	log_info (kernel_logs_obligatorios, "Se crea el proceso %d en NEW", pcb->PID);
	
	//sem_signal(&planificador);
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

void informar_memoria_nuevo_proceso()
{
	//deberemos informarle a la memoria de un nuevo proceso
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
            PCB* proceso_nuevo = queue_peek(cola_new);
            queue_pop(cola_new);

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
    //Obtenemos el grado de multiprogramación especificado por el archivo de configuración
    int grado_multiprogramacion = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");

	//pregunto constantemente
	while(1)
	{
		//hay nuevos procesos en new?
		if( procesos_en_new > 0 )
		{
			//avisar a memoria
			//USAR QUEUE PEEK
			informar_memoria_nuevo_proceso();			
		}
		else
		{
			//algun proceso terminó?
			if(procesos_fin > 0 )
			{
				//
			}
		}
		//luego de esto muevo los procesos a ready mientras pueda
		mover_procesos_ready(grado_multiprogramacion);
		sleep(1);
	}	
}

#endif KERNEL_H_ 
