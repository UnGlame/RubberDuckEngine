#pragma once

// This must be included before GLFW to prevent redefinition of APIENTRY
#include <Windows.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdlib>
#include <memory>
#include <cassert>
#include <vector>
#include <utility>
#include <algorithm>
#include <sstream>
#include <cstdint>