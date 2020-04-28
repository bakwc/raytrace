#include "utils.hpp"
#include "metal_raytracer.hpp"

#include "entities.hpp"

MetalRaytracer::MetalRaytracer(entt::registry& registry, int width, int height)
    : Registry(registry)
{
    Width = width;
    Height = height;

    OutputData.resize(width * height * 3);

    KernelSource = LoadFile("metal_kernel.c");
    if (KernelSource.empty()) {
        throw std::runtime_error("failed to load opencl kernel");
    }

    ns::Array<mtlpp::Device> devices = mtlpp::Device::CopyAllDevices();
    std::cout << "Devices number: " << devices.GetSize() << "\n";
    for (int i = 0; i < devices.GetSize(); ++i) {
        std::cout << devices[i].GetName() << "\n";
    }
    Device = devices[1];

//    Device = mtlpp::Device::CreateSystemDefaultDevice();
    assert(Device);

    ns::Error error;
    Library = Device.NewLibrary(KernelSource.c_str(), mtlpp::CompileOptions(), &error);
    std::cout << error.GetLocalizedDescription().GetCStr() << "\n";
    assert(Library);

    ProcessFunction = Library.NewFunction("processRaytrace");
    assert(ProcessFunction);

    ComputePipelineState = Device.NewComputePipelineState(ProcessFunction, nullptr);
    assert(ComputePipelineState);

    CommandsQueue = Device.NewCommandQueue();
    assert(CommandsQueue);

    const uint32_t dataCount = 1024 * 1024 * 15;

    InBuffer = Device.NewBuffer(sizeof(float) * dataCount, mtlpp::ResourceOptions::StorageModeManaged);
    assert(InBuffer);

    OutBuffer = Device.NewBuffer(sizeof(float) * Width * Height * 3, mtlpp::ResourceOptions::StorageModeManaged);
    assert(OutBuffer);
}

void MetalRaytracer::Update() {
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
        auto view = Registry.view<SphereRenderer, Transform, Material>();
        inputData.push_back(view.size());
        for(auto entity: view) {
            Transform& transform = view.get<Transform>(entity);
            inputData.push_back(transform.Position.X);
            inputData.push_back(transform.Position.Y);
            inputData.push_back(transform.Position.Z);

            SphereRenderer& sphere = view.get<SphereRenderer>(entity);
            inputData.push_back(sphere.Radius);

            Material& material = view.get<Material>(entity);
            inputData.push_back(material.Color.R);
            inputData.push_back(material.Color.G);
            inputData.push_back(material.Color.B);
            inputData.push_back(material.DiffuseCF);
            inputData.push_back(material.AlbedoCF.X);
            inputData.push_back(material.AlbedoCF.Y);
            inputData.push_back(material.AlbedoCF.Z);
            inputData.push_back(material.RefractCF.X);
            inputData.push_back(material.RefractCF.Y);
        }
    }

    float* inData = static_cast<float*>(InBuffer.GetContents());
    for (size_t i = 0; i < inputData.size(); ++i) {
        inData[i] = inputData[i];
    }
    InBuffer.DidModify(ns::Range(0, inputData.size() * sizeof(float)));


    mtlpp::CommandBuffer commandBuffer = CommandsQueue.CommandBuffer();
    assert(commandBuffer);

    mtlpp::ComputeCommandEncoder commandEncoder = commandBuffer.ComputeCommandEncoder();
    commandEncoder.SetBuffer(InBuffer, 0, 0);
    commandEncoder.SetBuffer(OutBuffer, 0, 1);
    commandEncoder.SetComputePipelineState(ComputePipelineState);
    commandEncoder.DispatchThreadgroups(
            mtlpp::Size(Width*4, 1, 1),
            mtlpp::Size(256, 1, 1));
    commandEncoder.EndEncoding();

    mtlpp::BlitCommandEncoder blitCommandEncoder = commandBuffer.BlitCommandEncoder();
    blitCommandEncoder.Synchronize(OutBuffer);
    blitCommandEncoder.EndEncoding();

    commandBuffer.Commit();
    commandBuffer.WaitUntilCompleted();

    float* inData2 = static_cast<float*>(InBuffer.GetContents());
//    std::cout << "input: " << inData2[8] << "\n";

    float* outData = static_cast<float*>(OutBuffer.GetContents());

//    std::cout << "result: " << outData[0] << "\n";

//    // read the data
//    {
//        float* inData = static_cast<float*>(inBuffer.GetContents());
//        float* outData = static_cast<float*>(outBuffer.GetContents());
//        for (uint32_t j=0; j<dataCount; j++)
//            printf("sqr(%g) = %g\n", inData[j], outData[j]);
//    }
}

