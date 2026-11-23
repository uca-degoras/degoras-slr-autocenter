// The one translation unit holding the stb implementations (see thirdparty/CMakeLists.txt).
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_easy_font.h"
