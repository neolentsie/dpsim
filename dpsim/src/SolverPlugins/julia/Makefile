# Makefile
JULIA ?= julia

ROOT_DIR:=$(shell pwd)
TARGET="JuliaMNACompiled"

.PHONY: all, default

default: plugin.so

all: juliamna.so wrapper.o plugin.so

juliamna.so: JuliaMNA/build/build.jl JuliaMNA/src/JuliaMNA.jl JuliaMNA/src/mna_solver.jl
	$(JULIA) --threads=auto --startup-file=no JuliaMNA/build/build.jl $(TARGET)

wrapper.o:
	gcc -c -fPIC -I$(ROOT_DIR)/JuliaMNACompiled/include -I$(ROOT_DIR)/../../../include dpsim_wrapper.c

plugin.so: wrapper.o
	gcc -shared -o juliamna.so dpsim_wrapper.o  -L$(ROOT_DIR)/JuliaMNACompiled/lib -ljuliamna
