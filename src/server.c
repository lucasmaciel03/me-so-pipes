#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define FIFO_PATH "/tmp/exec_fifo"


int main(void) {
    int fd;
    char buffer[256];

    // Cria o FIFO se não existir
    if (mkfifo(FIFO_PATH, 0666) == -1) {
      if (errno != EEXIST) {
          perror("mkfifo");
          exit(EXIT_FAILURE);
      }
    }

    printf("[SERVER] A aguardar mensagens no FIFO %s ...\n", FIFO_PATH);

    // Abre o FIFO para leitura
    fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
      perror("open");
      exit(EXIT_FAILURE);
    }

    // Loop principal
    while (1) {
      ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
      if (bytes > 0) {
        buffer[bytes] = '\0'; // Null-terminate the string
        printf("[SERVER] Mensagem recebida: %s\n", buffer);
      } else if (bytes == 0) {
        printf("[SERVER] Cliente terminou a conexão.\n");
        close(fd);
        fd = open(FIFO_PATH, O_RDONLY); // Reopen FIFO for new clients
      } else {
        perror("read");
        break;
      }
    }
    
    close(fd);
    unlink(FIFO_PATH); // Remove o FIFO ao sair
    return 0;
}
