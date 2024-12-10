#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define ITERATIONS 1000000000  // Nombre total d'itérations

typedef struct {
    	long start;      // Début de la plage d'itérations
    	long end;        // Fin de la plage d'itérations
    	double result;   // Résultat partiel du thread
	} ThreadData;

// Fonction exécutée par chaque thread
void* benchmark_thread(void* arg) 
{
ThreadData* data = (ThreadData*)arg;
double 	local_result = 0.0;
long	i;

for(i = data->start; i < data->end; i++) {
    local_result += sin(i) * cos(i);  // Charge de calcul
    }

data->result = local_result;
pthread_exit(NULL);
}

// Fonction pour mesurer le temps réel écoulé
double get_time_elapsed(struct timespec start, struct timespec end)
{
return((end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9);
}

int main() 
{
// Récupération de NUMCPUS depuis l'environnement
char 	*num_cpus_env;
int 	i, 
	num_threads;
double 	final_result = 0.0,
	elapsed_time;
struct 	timespec start_time,end_time;
long 	iterations_per_thread;

num_cpus_env = getenv("NUMCPUS");
num_threads = (num_cpus_env != NULL) ? atoi(num_cpus_env) : 1;

if(num_threads <= 0) {
    fprintf(stderr, "La variable NUMCPUS doit être un entier positif.\n");
    return(EXIT_FAILURE);
    }

pthread_t threads[num_threads];
ThreadData thread_data[num_threads];

printf("Démarrage du benchmark multi-cœurs avec %d threads...\n", num_threads);

// Début du chronométrage réel
clock_gettime(CLOCK_MONOTONIC, &start_time);

// Répartition des itérations entre les threads
iterations_per_thread = ITERATIONS / num_threads;
for(i = 0; i < num_threads; i++) {
    thread_data[i].start = i * iterations_per_thread;
    thread_data[i].end = (i == num_threads - 1) ? ITERATIONS : (i + 1) * iterations_per_thread;
    thread_data[i].result = 0.0;

    // Création des threads
    if(pthread_create(&threads[i], NULL, benchmark_thread, &thread_data[i]) != 0) {
        perror("Erreur lors de la création du thread");
        return EXIT_FAILURE;
        }
    }

// Attente de la fin des threads
for(i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
    final_result += thread_data[i].result;  // Agrégation des résultats
    }

// Fin du chronométrage réel
clock_gettime(CLOCK_MONOTONIC, &end_time);

// Calcul du temps écoulé
elapsed_time = get_time_elapsed(start_time, end_time);

// Affichage des résultats
printf("Temps écoulé : %.2f secondes\n", elapsed_time);
printf("Résultat final (pour éviter les optimisations) : %f\n", final_result);

return(0);
}
