#include <stdlib.h>
#include <stdio.h>
#include <entradasalida.h>

int main(int argc, char* argv[]) {

/*
//prueba
t_config* a = config_create("/home/utnso/dialfs/Goku.config");
char cantidad_string[32];
sprintf(cantidad_string,"%d",cantidad);
config_set_value(metadata_archivo,"TAMANIO_ARCHIVO",cantidad_string)
config
*/

//Ingresamos por consola los parametros iniciales NOMBRE y path del config
char* datosIniciales;
datosIniciales = readline("> "); //
printf("%s\n",datosIniciales);

char** datosInicialesPartidos = string_split(datosIniciales," ");

free(datosIniciales);
entradasalida_logger = log_create(".//tp.log", "log_cliente", true, LOG_LEVEL_INFO);
if (entradasalida_logger == NULL)
{
	perror("Algo paso con el log. No se pudo crear.");
	exit(EXIT_FAILURE);
}

//cambiar la ruta del archivo config a una abreviatura
t_config *entradasalida_config = config_create(datosInicialesPartidos[1]);
if (entradasalida_config == NULL)
{
	perror("Error al crear el config.");
	exit(EXIT_FAILURE);
}

sem_init(&sem_activacion1,0,0);
sem_init(&sem_activacion2,0,0);
lista_archivos = list_create();

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
pid_actual = malloc(sizeof(int));
    
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

//¿QUIÉN SOY?
printf("Hola soy una I/O y mi nombre es: %s\n",datosInicialesPartidos[0]);
printf("Mi path es el siguiente: %s\n",datosInicialesPartidos[1]);
printf("Mi tipo de interfaz es %s\n",TIPO_INTERFAZ);

printf("Checkpoint 1\n");
//conectarse a memoria como cliente
log_info(entradasalida_logger,"Intentando conexion a memoria");
fd_memoria = crear_conexion (IP_MEMORIA, PUERTO_MEMORIA,"memoria");
//log_info (entradasalida_logger, "Conectado a memoria exitosamente.");
handshakeClient(fd_memoria, 3);
printf("Checkpoint 2\n");


//conestarse a kernel como cliente
log_info (entradasalida_logger, "Intentado conexion a kernel.");
fd_kernel = crear_conexion (IP_KERNEL, PUERTO_KERNEL,"kernel");
//log_info (entradasalida_logger, "Conectado a kernel exitosamente.");
handshakeClient(fd_kernel, 3);

//mandaremos datos a kernel
printf("Enviaremos datos al KERNEL\n");
enviarDatos(fd_kernel,datosInicialesPartidos,TIPO_INTERFAZ);


if( strcmp(TIPO_INTERFAZ,"GENERICA") != 0)
{
	//de no ser generica, debemos enviarle los datos a memoria
	printf("Enviaremos datos a la MEMORIA\n");
	enviarDatos(fd_memoria,datosInicialesPartidos,TIPO_INTERFAZ);
}

path_bloques = malloc(strlen(PATH_BASE_DIALFS));
strcpy(path_bloques,PATH_BASE_DIALFS);
strcat(path_bloques, "/bloques.dat");

//void *mmap(void addr[.length], size_t length, int prot, int flags, int fd, off_t offset);;


//si la interfaz es un dialFS, debemos crearle un archivo de bloques, bitmap y metadata, a medida que se vayan creando archivos
if( strcmp(TIPO_INTERFAZ, "DIALFS") == 0 )
{
	levantarArchivoDeBloques();
	levantarArchivoBitMap();

	//espacio_bit_map = malloc(BLOCK_COUNT/8);
	//bit_map = bitarray_create_with_mode(espacio_bit_map, BLOCK_COUNT/8, LSB_FIRST);

	
	
}



//escuchar mensajes de memoria
pthread_t hilo_memoria;
pthread_create (&hilo_memoria, NULL, (void*)entradasalida_escuchar_memoria, NULL);
pthread_detach (hilo_memoria);
//si el ultimo hilo se desacopla el programa termina, JOIN HACE QUE EL PROGRAMA NO TERMINE HASTA QUE EL ULTIMO HILO FINALICE

//escuchar mensajes de kernel
pthread_t hilo_kernel;
pthread_create (&hilo_kernel, NULL, (void*)entradasalida_escuchar_kernel, NULL);
pthread_detach (hilo_kernel);
//si el ultimo hilo se desacopla el programa termina, JOIN HACE QUE EL PROGRAMA NO TERMINE HASTA QUE EL ULTIMO HILO FINALICE

string_array_destroy(datosInicialesPartidos);

while(1)
{
	sleep(1);
}


liberar_config(entradasalida_config);
liberar_logger(entradasalida_logger);
liberar_conexion(fd_memoria);
liberar_conexion(fd_kernel);

return (EXIT_SUCCESS);
}