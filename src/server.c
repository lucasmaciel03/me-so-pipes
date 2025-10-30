#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

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

    printf("[Servidor] A aguardar comandos no FIFO %s ...\n", FIFO_PATH);

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
            buffer[bytes] = '\0';

            printf("[Servidor] Comando recebido: '%s'\n", buffer);

            // Quebrar a string em tokens (programa + argumentos)
            char *args[32];
            int i = 0;
            char *token = strtok(buffer, " ");
            while (token != NULL && i < 31) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            if (args[0] == NULL) {
                printf("[Servidor] Nenhum comando válido recebido.\n");
                continue;
            }

            // Cria processo filho
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                continue;
            }

            if (pid == 0) {
                // Processo filho: executa o comando
                printf("[Servidor:Filho] A executar '%s'...\n", args[0]);
                execvp(args[0], args);
                perror("[Servidor:Filho] Erro no execvp");
                exit(EXIT_FAILURE);
            } else {
                // Processo pai: espera pelo filho
                int status;
                waitpid(pid, &status, 0);

                if (WIFEXITED(status)) {
                    int exit_code = WEXITSTATUS(status);
                    printf("[Servidor] Comando '%s' terminou com código %d\n", args[0], exit_code);
                } else {
                    printf("[Servidor] O processo terminou de forma anormal.\n");
                }
            }

        } else if (bytes == 0) {
            printf("[Servidor] Cliente terminou a escrita. A reabrir FIFO...\n");
            close(fd);
            fd = open(FIFO_PATH, O_RDONLY);
        } else {
            perror("read");
            break;
        }
    }

    close(fd);
    unlink(FIFO_PATH);
    return 0;
}
