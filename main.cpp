#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080	// Cambia a 8080 si no corres como root

int main() {
	int server_fd, client_fd;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);

	// Crear el socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0) {
		perror("socket failed");
		return 1;
	}

	// Configurar la dirección del servidor
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	// Bind al puerto
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		return 1;
	}

	// Escuchar por conexiones
	if (listen(server_fd, 10) < 0) {
		perror("listen failed");
		return 1;
	}

	std::cout << "Servidor escuchando en el puerto " << PORT << "...\n";

	// Aceptar una conexión
	client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
	if (client_fd < 0) {
		perror("accept failed");
		return 1;
	}

	// Leer la petición del cliente (ignoramos el contenido)
	char buffer[3000] = {0};
	read(client_fd, buffer, 3000);
	std::cout << "Petición recibida:\n" << buffer << "\n";

	// Crear la respuesta
	const char* http_response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: 48\r\n"
		"\r\n"
		"<html><body><h1>Hola mundo</h1></body></html>";

	// Enviar la respuesta
	write(client_fd, http_response, strlen(http_response));

	// Cerrar conexiones
	close(client_fd);
	close(server_fd);

	return 0;
}

