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

#define ITERATIONS 10000000000UL // 10 milliards (10x plus)
#define WORKGROUP_SIZE 256
#define NUM_WORKGROUPS 1152 // Nombre de CUDA cores GTX 760

// Kernel plus complexe pour empêcher les optimisations du compilateur
const char *kernelSource = 
    "__kernel void math_benchmark(__global float *output, ulong iterations) {\n"
    "   int gid = get_global_id(0);\n"
    "   float result = 0.0f;\n"
    "   float x = 0.0f;\n"
    "   \n"
    "   for (ulong i = gid; i < iterations; i += get_global_size(0)) {\n"
    "       x = (float)i * 0.00001f;\n"
    "       float s = sin(x);\n"
    "       float c = cos(x);\n"
    "       float t = tan(x * 0.1f);\n"
    "       result += s * c + t;\n"
    "       result += sqrt(fabs(s)) * c * c;\n"
    "       result += exp(s * 0.01f);\n"
    "       result += log(fabs(c) + 1.0f);\n"
    "       result += powr(fabs(s), 0.3f);\n"
    "   }\n"
    "   output[gid] = result;\n"
    "}\n";

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

    char deviceName[256];
    size_t deviceNameLen;
    cl_ulong globalMem, localMem, maxComputeUnits;

    struct timespec start_time, end_time;

    ret = clGetPlatformIDs(1, &platform, NULL);
    if (ret != CL_SUCCESS) {
        printf("Erreur : impossible de récupérer la plateforme OpenCL\n");
        return EXIT_FAILURE;
    }

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
    if (scanf("%d", &selectedDevice) != 1) {
        printf("Erreur de lecture\n");
        return EXIT_FAILURE;
    }

    if (selectedDevice < 0 || selectedDevice >= deviceCount) {
        printf("Erreur : sélection invalide\n");
        return EXIT_FAILURE;
    }

    cl_device_id device = devices[selectedDevice];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(deviceName), deviceName, &deviceNameLen);
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(globalMem), &globalMem, NULL);
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(localMem), &localMem, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, NULL);
    
    printf("\n=== GPU sélectionné : %s ===\n", deviceName);
    printf("Mémoire globale: %.0f MB\n", globalMem / (1024.0 * 1024.0));
    printf("Compute units: %lu\n", maxComputeUnits);

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : impossible de créer le contexte OpenCL\n");
        return EXIT_FAILURE;
    }

    queue = clCreateCommandQueue(context, device, 0, &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : impossible de créer la file de commandes\n");
        return EXIT_FAILURE;
    }

    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : impossible de créer le programme OpenCL\n");
        return EXIT_FAILURE;
    }

    ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de la compilation du kernel\n");
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char *)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Log:\n%s\n", log);
        free(log);
        return EXIT_FAILURE;
    }

    kernel = clCreateKernel(program, "math_benchmark", &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de la création du kernel\n");
        return EXIT_FAILURE;
    }

    size_t numWorkItems = NUM_WORKGROUPS * WORKGROUP_SIZE;
    float *output = (float *)malloc(numWorkItems * sizeof(float));
    outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, numWorkItems * sizeof(float), NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Erreur : échec de l'allocation du buffer\n");
        return EXIT_FAILURE;
    }

    printf("\nConfiguration:\n");
    printf("  Itérations: %lu (10 milliards)\n", ITERATIONS);
    printf("  Threads: %zu\n", numWorkItems);
    printf("  Itérations/thread: ~%lu\n\n", ITERATIONS / numWorkItems);
    printf("Lancement benchmark GPU intensif...\n\n");

    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &outputBuffer);
    if (ret != CL_SUCCESS) {
        printf("Erreur : configuration arg 0\n");
        return EXIT_FAILURE;
    }

    unsigned long iterations = ITERATIONS;
    ret = clSetKernelArg(kernel, 1, sizeof(unsigned long), &iterations);
    if (ret != CL_SUCCESS) {
        printf("Erreur : configuration arg 1\n");
        return EXIT_FAILURE;
    }

    size_t globalWorkSize = numWorkItems;
    size_t localWorkSize = WORKGROUP_SIZE;

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    ret = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);
    if (ret != CL_SUCCESS) {
        printf("Erreur : exécution kernel (code: %d)\n", ret);
        return EXIT_FAILURE;
    }

    clFinish(queue);
    
    ret = clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, numWorkItems * sizeof(float), output, 0, NULL, NULL);
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double final_result = 0.0;
    for (size_t i = 0; i < numWorkItems; i++) {
        final_result += output[i];
    }

    double elapsed_time = get_time_elapsed(start_time, end_time);

    printf("=== RÉSULTATS ===\n");
    printf("Temps GPU : %.2f secondes\n", elapsed_time);
    printf("Opérations : %.1f milliards\n", (ITERATIONS * 5.0) / 1e9);
    printf("Performance : %.2f GFlops\n", (ITERATIONS * 5.0) / (elapsed_time * 1e9));
    printf("Checksum : %f\n", final_result);

    free(output);
    clReleaseMemObject(outputBuffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
