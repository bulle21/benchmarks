# Définir les variables
CC = gcc
CFLAGS = -Wall -O2 -std=c99
LIBS = -lm
OS = $(shell uname)

# Ajouter les flags spécifiques à l'OS
ifeq ($(OS), Darwin) # macOS
    OPENCL_LIBS = -framework OpenCL
    OPENGL_LIBS = -framework OpenGL -lglfw
else # Linux/WSL
    OPENCL_LIBS = -lOpenCL
    OPENGL_LIBS = -lGL -lglfw
endif

# Programmes à compiler
TARGETS = 1-benchmark 2-benchmark_multicoeurs 3-benchmark_gpu 3-benchmark_gpu-v2

# Règle principale : compiler tous les programmes
all: $(TARGETS)

1-benchmark: 1-benchmark.c
	$(CC) $(CFLAGS) -o 1-benchmark 1-benchmark.c $(LIBS)

2-benchmark_multicoeurs: 2-benchmark_multicoeurs.c
	$(CC) $(CFLAGS) -o 2-benchmark_multicoeurs 2-benchmark_multicoeurs.c $(LIBS) -pthread

3-benchmark_gpu: 3-benchmark_gpu.c
	$(CC) $(CFLAGS) -o 3-benchmark_gpu 3-benchmark_gpu.c $(LIBS) $(OPENCL_LIBS)

3-benchmark_gpu-v2: 3-benchmark_gpu-v2.c
	$(CC) $(CFLAGS) -o 3-benchmark_gpu-v2 3-benchmark_gpu-v2.c $(LIBS) $(OPENCL_LIBS)

benchmark_opengl_2_1_simple: benchmark_opengl_2_1_simple.c
	$(CC) $(CFLAGS) -o benchmark_opengl_2_1_simple benchmark_opengl_2_1_simple.c $(LIBS) $(OPENGL_LIBS)

# Règle pour nettoyer les fichiers générés
clean:
	rm -f $(TARGETS)

# Règle pour tester tous les benchmarks
test: all
	@echo "=== Test 1: CPU mono-thread ==="
	@./1-benchmark
	@echo ""
	@echo "=== Test 2: CPU multi-thread ==="
	@./2-benchmark_multicoeurs
	@echo ""
	@echo "=== Test 3: GPU (version originale) ==="
	@echo "0" | ./3-benchmark_gpu
	@echo ""
	@echo "=== Test 3v2: GPU (version intensive) ==="
	@echo "0" | ./3-benchmark_gpu-v2
