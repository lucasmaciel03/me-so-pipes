# me-so-pipes

## 📝 Descrição

Projeto de desenvolvimento para o trabalho de Sistemas Operativos, focado no uso de pipes para comunicação entre processos.

## ⚙️ Pré-requisitos

- 🖥️ Multipass (para macOS)
- 🐧 Ubuntu 24.04 LTS

## 🚀 Instalação e Configuração

### 1️⃣ Criar a Máquina Virtual

Crie uma máquina virtual com Ubuntu 24.04 LTS usando o Multipass:

```bash
multipass launch 24.04 --name so-projeto --cpus 2 --mem 2G --disk 10G
```

### 2️⃣ Instalar Ferramentas Necessárias

Aceda à VM e instale as ferramentas:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential make gcc gdb valgrind git vim
```

| Ferramenta | Função                  |
|------------|-------------------------|
| gcc       | Compilador C           |
| make      | Sistema de compilação  |
| gdb       | Depurador              |
| valgrind  | Detetor de fugas de memória |
| git       | Controlo de versões    |
| vim       | Editor de texto        |

### 3️⃣ Clonar o Projeto

Clone o repositório na VM:

```bash
git clone https://github.com/lucasmaciel03/me-so-pipes.git
```

### 4️⃣ Criar Estrutura de Pastas

Navegue até ao diretório do projeto e crie a estrutura:

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

## 🧱 Compilação

### 🛠️ Makefile Base

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

| Ação                  | Comando          |
|-----------------------|------------------|
| Compilar o projeto   | `make`          |
| Limpar ficheiros gerados | `make clean`   |

## 💻 Código

### 🧩 Servidor (Versão Mensagens)

O servidor cria um FIFO nomeado (`/tmp/exec_fifo`) e aguarda mensagens dos clientes. Lê as mensagens e imprime-as no terminal.

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

### 💬 Cliente

O cliente lê uma mensagem do utilizador e envia-a para o servidor através do FIFO.

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

### ⚙️ Servidor (Versão Comandos)

Esta versão do servidor recebe comandos e executa-os num processo filho, registando o resultado.

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

## ▶️ Utilização

O projeto inclui duas versões do servidor:

- Uma que apenas imprime as mensagens recebidas.
- Outra que executa comandos.

### 🔹 Versão de Mensagens (Servidor Inicial)

1. Compilar os executáveis:
   ```bash
   make
   ```

2. Iniciar o servidor (num terminal):
   ```bash
   ./build/server
   ```

3. Executar o cliente (noutro terminal):
   ```bash
   ./build/client
   ```
   Introduza uma mensagem quando solicitado.

O servidor receberá e mostrará a mensagem enviada pelo cliente.

### 🔹 Versão de Comandos (Servidor Atualizado)

1. Compile os executáveis (mesmo processo).

2. Inicie o servidor (se ainda não estiver a correr).

3. Execute o cliente com um comando:
   ```bash
   ./build/client "ls -l"
   ```

O servidor receberá o comando, executá-lo-á e mostrará a saída e o respetivo código de término.

## 🪵 Registos (Logs)

O servidor regista os comandos executados e os respetivos códigos de saída num ficheiro de log localizado em:

`logs/server.log`

Cada entrada inclui o comando completo e o resultado da execução.

Para visualizar os registos:

```bash
cat logs/server.log
```

Os registos são anexados ao ficheiro, permitindo acompanhar o histórico completo de execuções.
