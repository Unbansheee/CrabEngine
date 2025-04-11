module;
#pragma once
//#include "webgpu/webgpu.hpp"
#include <cstdint>

export module Engine.GFX.DynamicUniformBuffer;
import Engine.WGPU;

export constexpr inline uint32_t Align(uint32_t value, uint32_t step) {
    uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
    return step * divide_and_ceil;
}

export class DynamicUniformBuffer {
public:
    DynamicUniformBuffer(wgpu::Device device, uint32_t size) : internalDevice(device)
    {
        Init(device, size);
    }

    ~DynamicUniformBuffer()
    {
        Clear();
    }
    
    void Init(wgpu::Device device, uint32_t size) {
        internalDevice = device;
        wgpu::Limits supportedLimits;
        device.getLimits(&supportedLimits);
        wgpu::Limits deviceLimits = supportedLimits;
        alignment = deviceLimits.minUniformBufferOffsetAlignment;
        auto s = Align(size, deviceLimits.minUniformBufferOffsetAlignment);
        
        wgpu::BufferDescriptor descriptor = wgpu::Default;
        descriptor.size = (uint64_t)s;
        descriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        buffer = device.createBuffer({
            descriptor
        });
        mappedData = new uint8_t[size];
    }

    void Clear()
    {
        delete[] mappedData;
        buffer = {};
    }

    wgpu::Device GetDevice() { return internalDevice; }
    
    uint32_t Allocate(uint32_t size) {
        //uint32_t offset = Align(currentOffset, alignment);
        //currentOffset = offset + (size);
        //return offset; // Return offset for this allocation
        return 0;
    }

    void Write(uint32_t offset, const void* data, uint32_t size) {
        memcpy(mappedData + offset, data, size);
    }

    void Upload(wgpu::Queue queue) {
        queue.writeBuffer(*buffer, 0, mappedData, buffer->getSize());
    }

    wgpu::raii::Buffer GetBuffer() {return buffer;}
    
private:
    wgpu::raii::Buffer buffer{};
    uint8_t* mappedData = nullptr;
    uint32_t alignment = 0;
    wgpu::Device internalDevice;
};