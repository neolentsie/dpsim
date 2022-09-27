# Makefile
JULIA ?= julia

ROOT_DIR:=$(shell pwd)
TARGET="JuliaLibCompiled"

.PHONY: all, default

default: plugin.so

all: julialib.so wrapper.o plugin.so

julialib.so:
	$(JULIA) --threads=auto --startup-file=no JuliaLib/build/build.jl $(TARGET)

wrapper.o:
	gcc -c -fPIC -I$(ROOT_DIR)/JuliaLibCompiled/include -I$(ROOT_DIR)/../../../include dpsim_wrapper.c

plugin.so: wrapper.o
	gcc -shared -o juliaplugin.so dpsim_wrapper.o  -L$(ROOT_DIR)/JuliaLibCompiled/lib -ljulialib

test_app: plugin.so
	gcc test_app.c -o test_app -I$(ROOT_DIR)/../../../include -ldl