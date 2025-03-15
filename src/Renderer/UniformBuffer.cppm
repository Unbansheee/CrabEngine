module;
#pragma once
#include <cassert>

export module uniform_buffer;
import dynamic_uniform_buffer;
import wgpu;

export template <typename T>
class UniformBuffer {
public:
    static_assert(std::is_trivially_copyable_v<T>, "UniformBuffer only supports trivially copyable types.");

    UniformBuffer() = default;

    UniformBuffer(WGPUDevice device)
    {
        Initialize(device);
    }

    ~UniformBuffer() {
        delete InternalBuffer;
    }

    void Initialize(WGPUDevice device)
    {
        InternalBuffer = new DynamicUniformBuffer(device, sizeof(T));
        bInitialized = true;
        SetData(Data);
    }
    
    void SetData(T data) {
        assert(bInitialized);
        Data = data;
        InternalBuffer->Write(0, &data, sizeof(T));
        InternalBuffer->Upload(InternalBuffer->GetDevice().getQueue());
    }

    const T& GetData() { return Data; }
    
    wgpu::Buffer GetBuffer() const
    {
        assert(bInitialized);
        return InternalBuffer->GetBuffer();
    }
private:
    T Data;
    DynamicUniformBuffer* InternalBuffer = nullptr;
    bool bInitialized = false;
};
