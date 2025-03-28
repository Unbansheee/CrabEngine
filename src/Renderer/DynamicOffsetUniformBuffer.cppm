module;

#pragma once

export module Engine.GFX.DynamicOffsetUniformBuffer;
import Engine.Math;
import Engine.WGPU;
import Engine.Assert;

export template<typename T>
class DynamicOffsetUniformBuffer
{
public:
    DynamicOffsetUniformBuffer() = default;
    
    DynamicOffsetUniformBuffer(wgpu::Device device, uint32_t uniformCount = 1024) : internalDevice(device)
    {
        Initialize(device, uniformCount);
    }

    void Initialize(wgpu::Device device, uint32_t uniformCount = 1024)
    {
        internalDevice = device;
        wgpu::SupportedLimits supportedLimits;
        device.getLimits(&supportedLimits);
        wgpu::Limits deviceLimits = supportedLimits.limits;
        stride = MathUtils::Align(sizeof(T), deviceLimits.minUniformBufferOffsetAlignment);

        wgpu::BufferDescriptor descriptor = wgpu::Default;
        descriptor.size = stride * uniformCount;
        descriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        descriptor.mappedAtCreation = false;
        descriptor.label = "DynamicOffsetUniformBuffer";
        buffer = internalDevice.createBuffer(descriptor);

        initialized = true;
        
        mappedData = new uint8_t[(size_t)stride * uniformCount];
    }

    ~DynamicOffsetUniformBuffer()
    {
        if (buffer)
            buffer.release();
        delete[] mappedData;
    }

    uint32_t Write(const T& value)
    {
        Assert::Verify(initialized, "initialized == true", "Buffer not initialized before use");
        auto offset = currentCount * stride;
        memcpy(mappedData + offset, &value, sizeof(T));
        currentCount ++;
        return offset;
    }

    void Upload(wgpu::Queue queue)
    {
        Assert::Verify(initialized, "initialized == true", "Buffer not initialized before use");
        auto offset = currentCount * stride;
        queue.writeBuffer(buffer, 0, mappedData, offset);
        currentCount = 0;
    }

    uint32_t GetBindingSize() {return sizeof(T);}
    uint32_t GetStride() {return stride;}

    wgpu::Buffer GetInternalBuffer() { return buffer; }
    
private:
    bool initialized = false;
    wgpu::Device internalDevice = nullptr;
    wgpu::Buffer buffer = nullptr;
    uint32_t stride = 0;
    uint32_t currentCount = 0;
    uint8_t* mappedData;
};
