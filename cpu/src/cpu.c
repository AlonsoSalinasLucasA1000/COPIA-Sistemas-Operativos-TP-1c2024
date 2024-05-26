#include <stdlib.h>
#include <stdio.h>
#include <cpu.h>

int main(int argc, char* argv[]) {


cpu_logger = log_create(".//tp.log", "log_cliente", true, LOG_LEVEL_INFO);
	if (cpu_logger == NULL)
	{
		perror("Algo paso con el log. No se pudo crear.");
		exit(EXIT_FAILURE);
	}

//cambiar la ruta del archivo config a una abreviatura
t_config *cpu_config = config_create("src/cpu.config");
if (cpu_config == NULL)
{
	perror("Error al crear el config.");
	exit(EXIT_FAILURE);
}

    

    IP_MEMORIA = config_get_string_value (cpu_config , "IP_MEMORIA" );
    PUERTO_MEMORIA = config_get_string_value (cpu_config , "PUERTO_MEMORIA" );
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value (cpu_config , "PUERTO_ESCUCHA_DISPATCH" );
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value (cpu_config , "PUERTO_ESCUCHA_INTERRUPT" );
    CANTIDAD_ENTRADAS_TLB = config_get_int_value (cpu_config , "CANTIDAD_ENTRADAS_TLB" );
    ALGORITMO_TLB = config_get_string_value (cpu_config , "ALGORITMO_TLB" );
    
log_info(cpu_logger, "Leí la ip: %s y puerto: %s\n", IP_MEMORIA, PUERTO_MEMORIA);

//inicia servidor cpu   SE DEBE INICIAR EL SERVIDOR DEL OTRO PUERTO DE CPU, EL PUERTO INTERRUPT!!!
fd_cpu_dispach = iniciar_servidor (PUERTO_ESCUCHA_DISPATCH, cpu_logger, "INICIADO EL CPU");

fd_cpu_interrupt = iniciar_servidor (PUERTO_ESCUCHA_INTERRUPT, cpu_logger, "INICIADO EL CPU");

//conestarse a memoria como cliente
log_info(cpu_logger,"Intentando conexión con memoria");
fd_memoria = crear_conexion (IP_MEMORIA, PUERTO_MEMORIA,"Memoria");
//log_info (cpu_logger, "Conectado a memoria exitosamente.");
handshakeClient(fd_memoria, 1);


//esperar conexion de kernel
log_info (cpu_logger, "Esperando a conectar con Kernel.");
fd_kernel = esperar_cliente (fd_cpu_dispach, cpu_logger, "KERNEL");
handshakeServer(fd_kernel);

log_info (cpu_logger, "Esperando a conectar con Kernel.");
fd_kernel = esperar_cliente (fd_cpu_interrupt, cpu_logger, "KERNEL");
handshakeServer(fd_kernel);

//escuchar mensajes de Kernel
pthread_t hilo_kernel;
pthread_create (&hilo_kernel, NULL, (void*)cpu_escuchar_kernel, NULL);
pthread_detach (hilo_kernel);

//escuchar mensajes de memoria
pthread_t hilo_memoria;
pthread_create (&hilo_memoria, NULL, (void*)cpu_escuchar_memoria, NULL);
pthread_join (hilo_memoria, NULL);
//si el ultimo hilo se desacopla el programa termina, JOIN HACE QUE EL PROGRAMA NO TERMINE HASTA QUE EL ULTIMO HILO FINALICE

liberar_config(cpu_config);
liberar_logger(cpu_logger);
liberar_conexion(fd_cpu_interrupt);
liberar_conexion(fd_cpu_dispach);
liberar_conexion(fd_memoria);
liberar_conexion(fd_kernel);

return (EXIT_SUCCESS);
}


