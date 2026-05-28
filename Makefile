# =====================================================================
# Makefile - Projeto ED 25/26 "Gestao de Caixas de um Supermercado"
#
# Estrutura:
#   include/  -> ficheiros .h
#   src/      -> ficheiros .c
#   data/     -> ficheiros de dados (.txt)
#   obj/      -> ficheiros objeto (.o)  [gerado]
#   bin/      -> executavel             [gerado]
#
# Utilizacao (Windows: mingw32-make ; Linux/Mac: make):
#   make         -> compila o projeto (cria bin/supermercado)
#   make run     -> compila e executa
#   make clean   -> apaga os ficheiros gerados
# =====================================================================

CC      := gcc
CFLAGS  := -Wall -Wextra -g -Iinclude

SRCDIR  := src
OBJDIR  := obj
BINDIR  := bin

# ---- comandos que dependem do sistema operativo ----
ifeq ($(OS),Windows_NT)
    EXE   := .exe
    MKDIR  = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
    RMDIR  = if exist "$(subst /,\,$1)" rmdir /s /q "$(subst /,\,$1)"
    DEL    = del /q
    RUNCMD = $(subst /,\,$(TARGET))
else
    EXE   :=
    MKDIR  = mkdir -p $1
    RMDIR  = rm -rf $1
    DEL    = rm -f
    RUNCMD = ./$(TARGET)
endif

TARGET  := $(BINDIR)/supermercado$(EXE)

# Lista automaticamente os fontes e os objetos correspondentes
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
HEADERS := $(wildcard include/*.h)

# Alvo por omissao
all: $(TARGET)

# Liga todos os objetos no executavel final
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# Compila cada .c em .o (recompila se o .c ou qualquer .h mudar)
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Cria as pastas de saida, se ainda nao existirem
$(OBJDIR):
	$(call MKDIR,$(OBJDIR))
$(BINDIR):
	$(call MKDIR,$(BINDIR))

# Compila e executa (a partir da raiz, para encontrar a pasta data/)
run: all
	$(RUNCMD)

# Apaga os ficheiros gerados
clean:
	$(call RMDIR,$(OBJDIR))
	$(call RMDIR,$(BINDIR))
	-$(DEL) historico.csv resultado.txt

.PHONY: all run clean
