#include <webgpu.h>

int mygputest() {
  //static texture_t low_res_target_texture = {0};

  // Bind group layout
  static WGPUBindGroupLayout bind_group_layout_upscale = NULL;

  WGPUTextureDescriptor texture_desc = {
     .label         = "Low Resolution Target",
     .usage         = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_RenderAttachment,
     .dimension     = WGPUTextureDimension_2D,
     .size          = (WGPUExtent3D) {
       .width              = 320,
       .height             = 240,
       .depthOrArrayLayers = 1,
     },
     .format        = NULL,
     .mipLevelCount = 1,
     .sampleCount   = 1,
   };
  low_res_target_texture.texture
    = wgpuDeviceCreateTexture(wgpu_context->device, &texture_desc);
}
