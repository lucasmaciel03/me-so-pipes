# me-so-pipes

## Descrição

Projeto de desenvolvimento para o trabalho de Sistemas Operacionais, focado no uso de pipes para comunicação entre processos.

## Pré-requisitos

- Multipass (para MacOS)
- Ubuntu 24.04 LTS

## Instalação e Configuração

### 1. Criar a Máquina Virtual

Crie uma máquina virtual com Ubuntu 24.04 LTS usando o Multipass:

```bash
multipass launch 24.04 --name so-projeto --cpus 2 --mem 2G --disk 10G
```

### 2. Instalar Ferramentas Necessárias

Acesse a VM e instale as ferramentas:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential make gcc gdb valgrind git vim
```

Ferramentas instaladas:

- `gcc`: Compilador C
- `make`: Sistema de build
- `gdb`: Debugger
- `valgrind`: Detector de fugas de memória
- `git`: Controlo de versões
- `vim`: Editor de texto

### 3. Clonar o Projeto

Clone o repositório na VM:

```bash
git clone https://github.com/lucasmaciel03/me-so-pipes.git
```

### 4. Criar Estrutura de Pastas

Navegue para o diretório do projeto e crie a estrutura:

```bash
cd ~/me-so-pipes
mkdir src logs build tests
touch src/client.c src/server.c Makefile
```

Estrutura final:

```
.
├── LICENSE
├── Makefile
├── README.md
├── build/
├── logs/
├── src/
│   ├── client.c
│   └── server.c
└── tests/
```

## Compilação

### Configurar o Makefile

O Makefile base está configurado da seguinte forma:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2

all: build/server build/client

build/server: src/server.c
    $(CC) $(CFLAGS) -o $@ $^

build/client: src/client.c
    $(CC) $(CFLAGS) -o $@ $^

clean:
    rm -rf build/* logs/* /tmp/exec_fifo /tmp/log_fifo_*
```

Para compilar o projeto:

```bash
make
```

Para limpar os arquivos gerados:

```bash
make clean
```

## Código

### Código do Servidor (Envio de Mensagens)

O servidor cria um FIFO nomeado (`/tmp/exec_fifo`) e aguarda mensagens dos clientes. Ele lê as mensagens e as imprime no console. (Versão inicial para mensagens)

```c
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
```

### Código do Cliente

O cliente lê uma mensagem do utilizador via entrada padrão e a envia para o servidor através do FIFO.

```c
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
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = 0; // Remove newline character

    // Abre o FIFO para escrita
    fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Escreve a mensagem no FIFO
    write(fd, message, strlen(message) + 1);
    printf("[CLIENT] Mensagem enviada: %s\n", message);

    close(fd);
    return 0;
}
```

### Código do Servidor (Versão Comandos)

Esta versão do servidor não apenas recebe os comandos, mas também os executa em um processo filho, reportando o resultado.

```c
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

    printf("[SERVER] A aguardar comandos no FIFO %s ...\n", FIFO_PATH);

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

            printf("[SERVER] Comando recebido: %s\n", buffer);

            // Break a string em token (programa + argumentos
            char *args[32];
            int i = 0;
            char *token = strtok(buffer, " ");
            while (token != NULL && i < 31) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL; // Null-terminate the args array

            if (args[0] == NULL) {
                printf("[SERVER] Nenhum comando válido recebido.\n");
                continue;
            }

            // Cria um processo filho para executar o comando
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                continue;
            }

            if (pid == 0) {
                // Processo filho: executa o comando
                printf("[SERVER] A executar '%s'...\n", args[0]);
                execvp(args[0], args);
                perror("[SERVER] Erro no execvp");
                exit(EXIT_FAILURE);
            } else {
                // Processo pai: espera o filho terminar
                int status;
                waitpid(pid, &status, 0);

                if(WIFEXITED(status)) {
                    int exit_code = WEXITSTATUS(status);
                    printf("[SERVER] Comando '%s' terminou com código %d\n", args[0], exit_code);
                } else {
                    printf("[SERVER] Comando '%s' terminou de forma anormal\n", args[0]);
                }
            }

        } else if (bytes == 0) {
            printf("[SERVER] Cliente terminou a conexão. A reabrir FIFO... \n");
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
```

## Uso

O projeto inclui duas versões do servidor: uma que apenas imprime as mensagens recebidas e outra que executa os comandos.

### Versão Mensagens (Servidor Inicial):

1. Compile os executáveis:

   ```bash
   make
   ```

2. Em um terminal, inicie o servidor:

   ```bash
   ./build/server
   ```

3. Em outro terminal, execute o cliente:
   ```bash
   ./build/client
   ```
   Introduza uma mensagem quando solicitado.

O servidor receberá e mostrará a mensagem enviada pelo cliente.

### Versão Comandos (Servidor Atualizado):

1. Compile os executáveis (mesmo processo).

2. Inicie o servidor (se não estiver rodando).

3. Execute o cliente com um comando:
   ```bash
   ./build/client "ls -l"
   ```

O servidor receberá, executará o comando e mostrará a saída e o código de saída.

## Logs

O servidor regista os comandos executados e os respetivos códigos de saída num ficheiro de log localizado em `logs/server.log`. Cada entrada inclui o comando completo e o resultado da execução.

Para visualizar os logs:

```bash
cat logs/server.log
```

Os logs são anexados ao ficheiro, permitindo acompanhar o histórico de execuções.
