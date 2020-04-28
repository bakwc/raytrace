#include "opencl_raytracer.hpp"
#include "utils.hpp"


#include <OpenCL/opencl.h>
#include <iostream>
#include <vector>

#include "entities.hpp"

int DEVICE_NUM = 1;
int DATA_SIZE = 1024;


void dumpDevices() {

    int i, j;
    char* value;
    size_t valueSize;
    cl_uint platformCount;
    cl_platform_id* platforms;
    cl_uint deviceCount;
    cl_device_id* devices;
    cl_uint maxComputeUnits;

    // get all platforms
    clGetPlatformIDs(0, NULL, &platformCount);
    platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, NULL);

    for (i = 0; i < platformCount; i++) {

        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
        devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);

        // for each device print critical attributes
        for (j = 0; j < deviceCount; j++) {

            // print device name
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
            printf("%d. Device: %s\n", j, value);
            free(value);

            // print hardware device version
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
            printf(" %d.%d Hardware version: %s\n", j+1, 1, value);
            free(value);

            // print software driver version
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
            printf(" %d.%d Software version: %s\n", j+1, 2, value);
            free(value);

            // print c version supported by compiler for device
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
            value = (char*) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
            printf(" %d.%d OpenCL C version: %s\n", j+1, 3, value);
            free(value);

            // print parallel compute units
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
                            sizeof(maxComputeUnits), &maxComputeUnits, NULL);
            printf(" %d.%d Parallel compute units: %d\n", j+1, 4, maxComputeUnits);

        }

        free(devices);

    }

    free(platforms);
}


OCLRaytracer::OCLRaytracer(entt::registry& registry, int width, int height)
    : Registry(registry)
{
    //dumpDevices();

    Width = width;
    Height = height;

    OutputData.resize(width * height * 3);

    KernelSource = LoadFile("opencl_kernel.c");
    if (KernelSource.empty()) {
        throw std::runtime_error("failed to load opencl kernel");
    }

    int err;
    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 2, &DeviceID[0], NULL);
    Context = clCreateContext(0, 1, &DeviceID[DEVICE_NUM], NULL, NULL, &err);

    std::cout << "err1: " << err << "\n";

    Commands = clCreateCommandQueue(Context, DeviceID[DEVICE_NUM], 0, &err);

    std::cout << "err2: " << err << "\n";

    std::vector<char*> ptrs;
    ptrs.push_back(&KernelSource[0]);
    cl_program program = clCreateProgramWithSource(Context, 1, (const char **)(&ptrs[0]), NULL, &err);
//    cl_program program = clCreateProgramWithSource(Context, 1, (const char **) & KernelSourceData, NULL, &err);

    std::cout << "err3: " << err << "\n";

    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    Kernel = clCreateKernel(program, "processRaytrace", &err);
    std::cout << "errB: " << err << "\n";

    if (err != 0) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, DeviceID[DEVICE_NUM], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, DeviceID[DEVICE_NUM], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
    }


    Input = clCreateBuffer(Context,  CL_MEM_READ_ONLY,  sizeof(float) * DATA_SIZE, NULL, NULL);
    Output = clCreateBuffer(Context, CL_MEM_WRITE_ONLY, sizeof(float) * OutputData.size(), NULL, NULL);
}


void OCLRaytracer::Update() {

    std::vector<float> inputData;
    inputData.push_back(Width);
    inputData.push_back(Height);

    {
        auto view = Registry.view<Camera, Transform>();
        for(auto entity: view) {
            Transform& transform = view.get<Transform>(entity);
            inputData.push_back(transform.Position.X);
            inputData.push_back(transform.Position.Y);
            inputData.push_back(transform.Position.Z);
            break;
        }
    }

    {
        auto view = Registry.view<LightSource, Transform>();
        for(auto entity: view) {
            Transform& transform = view.get<Transform>(entity);
            inputData.push_back(transform.Position.X);
            inputData.push_back(transform.Position.Y);
            inputData.push_back(transform.Position.Z);
            break;
        }
    }

    {
        auto view = Registry.view<SphereRenderer, Transform>();
        inputData.push_back(view.size());
        for(auto entity: view) {
            Transform& transform = view.get<Transform>(entity);
            inputData.push_back(transform.Position.X);
            inputData.push_back(transform.Position.Y);
            inputData.push_back(transform.Position.Z);

            SphereRenderer& sphere = view.get<SphereRenderer>(entity);
            inputData.push_back(sphere.Radius);
        }
    }

    int err;
    err = clEnqueueWriteBuffer(Commands, Input, CL_TRUE, 0, sizeof(float) * inputData.size(), &inputData[0], 0, NULL, NULL);

    //std::cout << "err4: " << err << "\n";

    clSetKernelArg(Kernel, 0, sizeof(cl_mem), &Input);
    clSetKernelArg(Kernel, 1, sizeof(cl_mem), &Output);
    unsigned int count = DATA_SIZE;

    clSetKernelArg(Kernel, 2, sizeof(unsigned int), &count);
    size_t local;

    size_t groupSize = CL_KERNEL_WORK_GROUP_SIZE;

    clGetKernelWorkGroupInfo(Kernel, DeviceID[DEVICE_NUM], CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);

    size_t global = Width * Height;
    clEnqueueNDRangeKernel(Commands, Kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    clFinish(Commands);

    clEnqueueReadBuffer(Commands, Output, CL_TRUE, 0, sizeof(float) * OutputData.size(), &OutputData[0], 0, NULL, NULL);
    clFinish(Commands);
//    for (int i = 0; i < 15; ++i) {
//        std::cout << OutputData[i] << " ";
//    }
//    std::cout << "\n";
}
