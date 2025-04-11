module;
#pragma once
#include <cassert>
#include "ReflectionMacros.h"

export module Engine.GFX.UniformBuffer;
import Engine.GFX.DynamicUniformBuffer;
import Engine.WGPU;

export template <typename T>
struct UniformBuffer {
    BEGIN_STRUCT_PROPERTIES(UniformBuffer<T>)
        ADD_NESTED_STRUCT(Data, T)
    END_STRUCT_PROPERTIES

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
    
    void SetData(const T& data) {
        assert(bInitialized);
        Data = T(data);
        InternalBuffer->Write(0, &data, sizeof(T));
        InternalBuffer->Upload(InternalBuffer->GetDevice().getQueue());
    }

    const T& GetData() { return Data; }
    
    wgpu::raii::Buffer GetBuffer() const
    {
        assert(bInitialized);
        return InternalBuffer->GetBuffer();
    }

private:
    T Data;
    DynamicUniformBuffer* InternalBuffer = nullptr;
    bool bInitialized = false;

protected:
    static void StaticOnPropertySet(void* obj, Property& prop) {
        auto buf = static_cast<UniformBuffer<T>*>(obj);
        if (buf->bInitialized) {
            buf->SetData(buf->GetData());
        }
    };
};
