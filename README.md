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

### Código do Servidor

O servidor cria um FIFO nomeado (`/tmp/exec_fifo`) e aguarda mensagens dos clientes. Ele lê as mensagens e as imprime no console.

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

## Uso

Para executar o projeto:

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
