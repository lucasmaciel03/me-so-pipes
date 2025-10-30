#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define FIFO_PATH "/tmp/exec_fifo"
int main(void) {
    int fd;
    char message[256];

    printf("[CLIENT] Introduz uma mensagem: ");
    if (fgets(message, sizeof(message), stdin) == NULL) {
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    message[strcspn(message, "\n")] = 0; // Remove newline character

    // Abre o FIFO para escrita
    fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
      perror("open");
      exit(EXIT_FAILURE);
    }

    // Escreve a mensagem no FIFO
    ssize_t written = write(fd, message, strlen(message) + 1);
    if (written == -1) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("[CLIENT] Mensagem enviada: %s\n", message);

    close(fd);
    return 0;
}
