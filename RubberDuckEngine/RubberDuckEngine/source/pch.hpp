#pragma once

// This must be included before GLFW to prevent redefinition of APIENTRY
#include <Windows.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <utility>
#include <vector>