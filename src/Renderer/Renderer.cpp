#include "Renderer.h"

wgpu::TextureView Renderer::GetCurrentTextureView()
{
    if (m_currentSurfaceView == nullptr)
    {
        m_currentSurfaceView = GetNextSurfaceTextureView();
    }
    return m_currentSurfaceView;
}

wgpu::TextureView Renderer::GetNextSurfaceTextureView() const
{
    // Get the surface texture
    
    wgpu::SurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
        return nullptr;
    }
    wgpu::Texture texture = surfaceTexture.texture;

    // Create a view for this surface texture
    wgpu::TextureViewDescriptor viewDescriptor;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = texture.getFormat();
    viewDescriptor.dimension = wgpu::TextureViewDimension::_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = wgpu::TextureAspect::All;
    wgpu::TextureView targetView = texture.createView(viewDescriptor);
#ifndef WEBGPU_BACKEND_WGPU
    // We no longer need the texture, only its view
    // (NB: with wgpu-native, surface textures must not be manually released)
    wgpuTextureRelease(surfaceTexture.texture);
#endif // WEBGPU_BACKEND_WGPU

    return targetView;
}
