#include <stdlib.h>
#include <stdio.h>
#include <entradasalida.h>

int main(int argc, char* argv[]) {


entradasalida_logger = log_create(".//tp.log", "log_cliente", true, LOG_LEVEL_INFO);
	if (entradasalida_logger == NULL)
	{
		perror("Algo paso con el log. No se pudo crear.");
		exit(EXIT_FAILURE);
	}

//cambiar la ruta del archivo config a una abreviatura
	t_config *entradasalida_config = config_create("/home/utnso/tp-2024-1c-Grupo-120/entradasalida/src/entradasalida.config");
	if (entradasalida_config == NULL)
	{
		perror("Error al crear el config.");
		exit(EXIT_FAILURE);
	}

    TIPO_INTERFAZ = config_get_string_value (entradasalida_config , "TIPO_INTERFAZ" );
	TIEMPO_UNIDAD_TRABAJO = config_get_int_value (entradasalida_config , "TIEMPO_UNIDAD_TRABAJO" );;
	IP_KERNEL = config_get_string_value (entradasalida_config , "IP_KERNEL" );
	PUERTO_KERNEL = config_get_string_value (entradasalida_config , "PUERTO_KERNEL" );
	IP_MEMORIA = config_get_string_value (entradasalida_config , "IP_MEMORIA" );
	PUERTO_MEMORIA = config_get_string_value (entradasalida_config , "PUERTO_MEMORIA" );
	PATH_BASE_DIALFS = config_get_string_value (entradasalida_config , "PATH_BASE_DIALFS" );
	BLOCK_SIZE = config_get_int_value (entradasalida_config , "BLOCK_SIZE" );
	BLOCK_COUNT = config_get_int_value (entradasalida_config , "BLOCK_COUNT" );
	RETRASO_COMPACTACION = config_get_int_value (entradasalida_config , "RETRASO_COMPACTACION" );

    
    log_info(entradasalida_logger, "TIPO_INTERFAZ: %s", TIPO_INTERFAZ);
    log_info(entradasalida_logger, "TIEMPO_UNIDAD_TRABAJO: %i", TIEMPO_UNIDAD_TRABAJO);
    log_info(entradasalida_logger, "IP_KERNEL: %s", IP_KERNEL);
    log_info(entradasalida_logger, "PUERTO_KERNEL: %s", PUERTO_KERNEL);
    log_info(entradasalida_logger, "IP_MEMORIA: %s", IP_MEMORIA);
    log_info(entradasalida_logger, "PUERTO_MEMORIA: %s", PUERTO_MEMORIA);
    log_info(entradasalida_logger, "PATH_BASE_DIALFS: %s", PATH_BASE_DIALFS);
    log_info(entradasalida_logger, "BLOCK_SIZE: %i", BLOCK_SIZE);
    log_info(entradasalida_logger, "BLOCK_COUNT: %i", BLOCK_COUNT);
    log_info(entradasalida_logger, "RETRASO_COMPACTACION: %i", RETRASO_COMPACTACION);



//conestarse a memoria como cliente
log_info(entradasalida_logger,"Intentando conexion a memoria");
fd_memoria = crear_conexion (IP_MEMORIA, PUERTO_MEMORIA,"memoria");
//log_info (entradasalida_logger, "Conectado a memoria exitosamente.");
handshakeClient(fd_memoria, 3);

//conestarse a kernel como cliente
log_info (entradasalida_logger, "Intentado conexion a memoria.");
fd_kernel = crear_conexion (IP_KERNEL, PUERTO_KERNEL,"kernel");
//log_info (entradasalida_logger, "Conectado a kernel exitosamente.");
handshakeClient(fd_kernel, 3);

//escuchar mensajes de memoria
pthread_t hilo_memoria;
pthread_create (&hilo_memoria, NULL, (void*)entradasalida_escuchar_memoria, NULL);
pthread_join (hilo_memoria, NULL);
//si el ultimo hilo se desacopla el programa termina, JOIN HACE QUE EL PROGRAMA NO TERMINE HASTA QUE EL ULTIMO HILO FINALICE


//escuchar mensajes de kernel
pthread_t hilo_kernel;
pthread_create (&hilo_kernel, NULL, (void*)entradasalida_escuchar_kernel, NULL);
pthread_join (hilo_kernel, NULL);
//si el ultimo hilo se desacopla el programa termina, JOIN HACE QUE EL PROGRAMA NO TERMINE HASTA QUE EL ULTIMO HILO FINALICE


return (EXIT_SUCCESS);

}