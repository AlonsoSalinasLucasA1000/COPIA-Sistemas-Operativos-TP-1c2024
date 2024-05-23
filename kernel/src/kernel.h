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



t_log *kernel_logger; // LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config *kernel_config;

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


void kernel_escuchar_cpu (){
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
				log_error(kernel_logger, "Desconexion de cpu.");
				control_key = 0;
			default:
				log_warning(kernel_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}	
}


void kernel_escuchar_entradasalida (){
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


void kernel_escuchar_memoria (){
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

void validarFuncionesConsola(char* leido)
{
	 char** valorLeido = string_split(leido, " ");
	 printf("%s\n",valorLeido[0]);

	 if(strcmp(valorLeido[0], "EJECUTAR_SCRIPT") == 0)
	 {
		printf("Comando válido\n");
	 }
	 else
	 {
		if(strcmp(valorLeido[0], "INICIAR_PROCESO") == 0)
	    {
		    printf("Comando válido\n");
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

void f_iniciar_proceso(t_buffer* un_buffer)
{

}

void atender_instruccion (char* leido){
    char** comando_consola = string_split(leido, " ");
    t_buffer* un_buffer;
	crear_buffer(un_buffer);

    if((strcmp(comando_consola [0], "INICIAR_PROCESO") == 0)
	){
        cargar_string_al_buffer(un_buffer, comando_consola [1] ); //|path]  
        cargar_string_al_buffer(un_buffer, comando_consola [2]); //[size]  
        f_iniciar_proceso(un_buffer); 
    //
    // 
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
#endif /* KERNEL_H_ */