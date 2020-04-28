#include <iostream>
#include <vector>
#include <OpenCL/opencl.h>
#include <entt/entt.hpp>

#include "linmath.hpp"

class OCLRaytracer {
public:
    OCLRaytracer(entt::registry& registry, int width, int height);
    void Update();
    void* RawData() {
        return &OutputData[0];
    }
private:
    entt::registry& Registry;
    int Width;
    int Height;
    std::string KernelSource;
    std::vector<float> OutputData;
    cl_kernel Kernel;
    cl_device_id DeviceID[2];
    cl_context Context;
    cl_mem Input;
    cl_mem Output;
    cl_command_queue Commands;
};
