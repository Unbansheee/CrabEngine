
export module Engine.WGPU;
export import <webgpu/webgpu.h>;
export import <webgpu/webgpu.hpp>;
export import <webgpu/webgpu-raii.hpp>;

export enum WGPUBindingType {
    Undefined,
    Buffer,
    Texture,
    StorageTexture,
    Sampler
};


export namespace wgpu {
    constexpr uint64_t STRLEN = WGPU_STRLEN;
    constexpr uint64_t WHOLE_SIZE = WGPU_WHOLE_SIZE;

}
