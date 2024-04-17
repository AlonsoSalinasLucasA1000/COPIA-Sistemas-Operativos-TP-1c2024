int err;

struct addrinfo hints, *server_info;

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;

err = getaddrinfo("127.0.0.1", "45007", &hints, &server_info);

int fd_conexion = socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);

freeaddrinfo(server_info);

err = connect(fd_conexion, server_info->ai_addr, server_info->ai_addrlen);

size_t bytes;

int32_t handshake = 1;
int32_t result;

bytes = send(fd_conexion, &handshake, sizeof(int32_t), 0);
bytes = recv(fd_conexion, &result, sizeof(int32_t), MSG_WAITALL);

if (result == 0) {
    // Handshake OK
} else {
    // Handshake ERROR
}