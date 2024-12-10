# Définir les variables
CC = gcc
CFLAGS = -Wall -O2 -std=c99
LIBS = -lm
LDFLAGS = 

# Programmes à compiler
TARGETS = 1-benchmark 2-benchmark_multicoeurs 3-benchmark_gpu 

# Règle principale : compiler tous les programmes
all: $(TARGETS)

1-benchmark: 1-benchmark.c
	$(CC) $(CFLAGS) -o 1-benchmark 1-benchmark.c $(LIBS)

# Compilation de benchmark_multicoeurs
2-benchmark_multicoeurs: 2-benchmark_multicoeurs.c
	$(CC) $(CFLAGS) -o 2-benchmark_multicoeurs 2-benchmark_multicoeurs.c $(LIBS) -pthread

# Compilation de benchmark_opengl_2_1_simple
benchmark_opengl_2_1_simple: benchmark_opengl_2_1_simple.c
	$(CC) $(CFLAGS) -o benchmark_opengl_2_1_simple benchmark_opengl_2_1_simple.c $(LIBS) -framework OpenGL -lglfw

# Compilation de benchmark_cuda
#benchmark_cuda: benchmark_cuda.c
	#$(CC) $(CFLAGS) -o benchmark_cuda benchmark_cuda.c $(LIBS) -L/usr/local/cuda/lib -lcudart

3-benchmark_gpu : 3-benchmark_gpu.c
	$(CC) $(CFLAGS) -framework OpenCL 3-benchmark_gpu.c -o 3-benchmark_gpu

# Règle pour nettoyer les fichiers générés
clean:
	rm -f $(TARGETS)

