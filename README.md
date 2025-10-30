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
- `valgrind`: Detector de vazamentos de memória
- `git`: Controle de versões
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
