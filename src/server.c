#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#define FIFO_PATH "/tmp/exec_fifo"
#define LOG_FILE "logs/server.log"

void append_log(const char *line) {
    int log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("[SERVER] Erro ao abrir o ficheiro de log    ");
        return;
    }

    // Escreve a linha no ficheiro
    ssize_t written = write(log_fd, line, strlen(line));
    if (written == -1) {
        perror("[SERVER] Erro ao escrever no ficheiro de log");
    }

    close(log_fd);
}

int main(void) {
    int fd;
    char buffer[256];

    // Garante que a pasta de logs existe
    mkdir("logs", 0755);

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

            // Divide a string em tokens (programa + argumentos)
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

                char log_entry[512];
                if (WIFEXITED(status)) {
                    int exit_code = WEXITSTATUS(status);
                    snprintf(log_entry, sizeof(log_entry), "%s", args[0]);
                    for (int j=1; args[j] != NULL; j++) {
                        strncat(log_entry, " ", sizeof(log_entry) - strlen(log_entry) -1);
                        strncat(log_entry, args[j], sizeof(log_entry) - strlen(log_entry) -1);
                    }
                    strncat(log_entry, "; exit status: ", sizeof(log_entry) - strlen(log_entry) -1);

                    char code[10];
                    snprintf(code, sizeof(code), "%d\n", exit_code);
                    strncat(log_entry, code, sizeof(log_entry) - strlen(log_entry) -1);


                    printf("[Servidor] %s", log_entry);
                    append_log(log_entry);
                } else {
                    snprintf(log_entry, sizeof(log_entry), "%s; terminou de forma anormal\n", args[0]);
                    printf("[Servidor] %s", log_entry);
                    append_log(log_entry);
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
