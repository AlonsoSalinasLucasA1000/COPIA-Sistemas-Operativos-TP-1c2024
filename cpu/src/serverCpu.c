int err;

struct addrinfo hints, *server_info;

memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

err = getaddrinfo(NULL, "45007", &hints, &server_info);

int fd_escucha = socket(server_info->ai_family,
                        server_info->ai_socktype,
                        server_info->ai_protocol);

freeaddrinfo(server_info);

err = bind(fd_escucha, server_info->ai_addr, server_info->ai_addrlen);
err = listen(fd_escucha, SOMAXCONN);

int fd_conexion = accept(fd_escucha, NULL, NULL);

size_t bytes;

int32_t handshake;
int32_t resultOk = 0;
int32_t resultError = -1;

bytes = recv(fd_conexion, &handshake, sizeof(int32_t), MSG_WAITALL);
if (handshake == 1) {
    bytes = send(fd_conexion, &resultOk, sizeof(int32_t), 0);
} else {
    bytes = send(fd_conexion, &resultError, sizeof(int32_t), 0);
}

close(fd_conexion);
close(fd_escucha);