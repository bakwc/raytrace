#include <iostream>
#include <vector>
#include <entt/entt.hpp>

#include "linmath.hpp"

#include "mtlpp.hpp"

class MetalRaytracer {
public:
    MetalRaytracer(entt::registry& registry, int width, int height);
    void Update();
    void* RawData() {
        float* outData = static_cast<float*>(OutBuffer.GetContents());
        return outData;
    }
private:
    entt::registry& Registry;
    int Width;
    int Height;
    std::string KernelSource;
    std::vector<float> OutputData;
    mtlpp::Device Device;
    mtlpp::Library Library;
    mtlpp::Function ProcessFunction;
    mtlpp::ComputePipelineState ComputePipelineState;
    mtlpp::CommandQueue CommandsQueue;
    mtlpp::Buffer InBuffer;
    mtlpp::Buffer OutBuffer;
};
