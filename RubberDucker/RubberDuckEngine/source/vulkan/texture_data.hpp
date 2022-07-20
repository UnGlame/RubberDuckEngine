#pragma once
#include <stbi/stb_image.h>

namespace RDE
{
namespace Vulkan
{

struct TextureData {
  stbi_uc *data;
  int texWidth;
  int texHeight;
  int texChannels;
};
} // namespace Vulkan
} // namespace RDE
