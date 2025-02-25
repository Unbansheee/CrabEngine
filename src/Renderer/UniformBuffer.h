#pragma once
#include "DynamicUniformBuffer.h"

template <typename T>
class UniformBuffer {
public:
    static_assert(std::is_trivially_copyable_v<T>, "UniformBuffer only supports trivially copyable types.");

    UniformBuffer(WGPUDevice device)
    {
        InternalBuffer = new DynamicUniformBuffer(device, sizeof(T));
    }

    ~UniformBuffer() {
        delete InternalBuffer;
    }

    void SetData(const T& data) {
        Data = data;
        InternalBuffer->Write(0, &data, sizeof(T));
        InternalBuffer->Upload(InternalBuffer->GetDevice().getQueue());
    }

    const T& GetData() { return Data; }
    
    wgpu::Buffer GetBuffer() const { return InternalBuffer->GetBuffer(); }
private:
    T Data;
    DynamicUniformBuffer* InternalBuffer;
};
