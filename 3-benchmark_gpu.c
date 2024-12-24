#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef __APPLE__  // macOS
#include <OpenCL/opencl.h>
#elif defined(__linux__)  // Linux
#include <CL/cl.h>
#else
    #error "Platform not supported!"
#endif

#define ITERATIONS 1000000000 // Nombre d'itérations total
#define WORKGROUP_SIZE 256    // Taille des work-groups

const char *kernelSource = 
    "__kernel void math_benchmark(__global float *output, int iterations) {"
    "   int gid = get_global_id(0);"
    "   float result = 0.0f;"
    "   for (int i = gid; i < iterations; i += get_global_size(0)) {"
    "       result += sin((float)i) * cos((float)i);"
    "   }"
    "   output[gid] = result;"
    "}";

double get_time_elapsed(struct timespec start, struct timespec end) {
    return ((end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9);
}

int main() {
    cl_platform_id platform;
    cl_device_id devices[10];
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem outputBuffer;
    cl_int ret;
cl_uint deviceCount;
    int selectedDevice;

    // Informations sur le GPU
    char deviceName[256];
    size_t deviceNameLen;

    // Chronométrage
    struct timespec start_time, end_time;

    // Récupération de la plateforme OpenCL
    ret = clGetPlatformIDs(1, &platform, NULL);
    if (ret != CL_SUCCESS) {
        printf("Erreur : impossible de récupérer la plateforme OpenCL\n");
        return EXIT_FAILURE;
    }

    // Récupération des appareils disponibles
    ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 10, devices, &deviceCount);
    if (ret != CL_SUCCESS || deviceCount == 0) {
        printf("Erreur : aucun GPU trouvé\n");
        return EXIT_FAILURE;
    }

    printf("GPUs disponibles :\n");
    for (int i = 0; i < deviceCount; i++) {
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, &deviceNameLen);
        printf("%d : %s\n", i, deviceName);
    }

    printf("Sélectionnez le GPU à utiliser (0-%d) : ", deviceCount - 1);
    scanf("%d", &selectedDevice);

    if (selectedDevice < 0 || selectedDevice >= deviceCount) {
        printf("Erreur : sélection invalide\n");
        return EXIT_FAILURE;
    }

    // Récupération du GPU sélectionné
    cl_device_id device = devices[selectedDevice];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(deviceName), deviceName, &deviceNameLen);
    printf("GPU sélectionné : %s\n", deviceName);

    // Création du contexte
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : impossible de créer le contexte OpenCL\n");
        return EXIT_FAILURE;
    }

    // File de commandes
    queue = clCreateCommandQueue(context, device, 0, &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : impossible de créer la file de commandes\n");
        return EXIT_FAILURE;
    }

    // Compilation du kernel
    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : impossible de créer le programme OpenCL\n");
        return EXIT_FAILURE;
    }

    ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de la compilation du kernel\n");
        return EXIT_FAILURE;
    }

    // Création du kernel
    kernel = clCreateKernel(program, "math_benchmark", &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de la création du kernel\n");
        return EXIT_FAILURE;
    }

    // Allocation du buffer de sortie
    size_t numWorkItems = 1024 * WORKGROUP_SIZE;
    float *output = (float *)malloc(numWorkItems * sizeof(float));
    outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numWorkItems * sizeof(float), NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de l'allocation du buffer\n");
        return EXIT_FAILURE;
    }

    // Configuration des arguments du kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &outputBuffer);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de la configuration des arguments du kernel\n");
        return EXIT_FAILURE;
    }

    int iterations = ITERATIONS; // Utilisation d'une variable pour ITERATIONS
    ret = clSetKernelArg(kernel, 1, sizeof(int), &iterations);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de la configuration des arguments du kernel\n");
        return EXIT_FAILURE;
    }

    // Exécution du kernel
    size_t globalWorkSize = numWorkItems;
    size_t localWorkSize = WORKGROUP_SIZE;

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    ret = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de l'exécution du kernel\n");
        return EXIT_FAILURE;
    }

    // Lecture des résultats
    ret = clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, numWorkItems * sizeof(float), output, 0, NULL, NULL);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de la lecture des résultats\n");
        return EXIT_FAILURE;
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // Agrégation des résultats
    double final_result = 0.0;
    for (size_t i = 0; i < numWorkItems; i++) {
        final_result += output[i];
    }

    // Calcul du temps écoulé
    double elapsed_time = get_time_elapsed(start_time, end_time);

    // Affichage des résultats
    printf("Temps GPU : %.2f secondes\n", elapsed_time);
    printf("Résultat final (pour éviter les optimisations) : %f\n", final_result);

    // Libération des ressources
    free(output);
    clReleaseMemObject(outputBuffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}

