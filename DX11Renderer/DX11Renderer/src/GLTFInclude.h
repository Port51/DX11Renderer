#pragma once

#define TINYGLTF_IMPLEMENTATION

// Don't include anything from stb_image, as that's used elsewhere and can only be included once
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
// Don't try to load any images
#define TINYGLTF_NO_EXTERNAL_IMAGE
// Load it ourselves
#define TINYGLTF_NO_INCLUDE_JSON
// Use C++14 for better performance
#define TINYGLTF_USE_CPP14

#include "./vendor/json/json.hpp"
#include "./vendor/tinygltf/tiny_gltf.h"

using namespace tinygltf;