#include "VulkanRenderer.h"

Sampler2D VulkanRenderer::Create2DTextureImage(const char* textureFile) {

    Sampler2D texture2D;
    SDL_Surface* im = IMG_Load(textureFile);
    SDL_Surface* image = SDL_ConvertSurface(im, SDL_PIXELFORMAT_RGBA32);
    VkDeviceSize imageSize = image->w * image->h * 4; /// RGBA only please
    SDL_DestroySurface(im);

    BufferMemory stagingBuffer{};
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer.bufferID, stagingBuffer.bufferMemoryID);

    void* data;
    vkMapMemory(device, stagingBuffer.bufferMemoryID, 0, imageSize, 0, &data);
    memcpy(data, image->pixels, static_cast<size_t>(imageSize)); /// memcpy wants a 32 bit not a 64 bit int thus the type cast
    vkUnmapMemory(device, stagingBuffer.bufferMemoryID);


    createImage(image->w, image->h, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture2D.image, texture2D.imageDeviceMemory);

    transitionImageLayout(texture2D.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer.bufferID, texture2D.image, static_cast<uint32_t>(image->w), static_cast<uint32_t>(image->h));
    transitionImageLayout(texture2D.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer.bufferID, nullptr);
    vkFreeMemory(device, stagingBuffer.bufferMemoryID, nullptr);
    SDL_DestroySurface(image);

    createTextureImageView(texture2D);
    createTextureSampler(texture2D);
    return texture2D;
}


void VulkanRenderer::CreateSampler(VkSampler& sampler, VkFilter filter, VkSamplerAddressMode samplerMode,
    VkBorderColor borderColor, VkBool32 compare,VkBool32 anisotropy)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;
	samplerInfo.addressModeU = samplerMode;
    samplerInfo.addressModeV = samplerMode;
    samplerInfo.addressModeW = samplerMode;
    if (anisotropy == VK_TRUE) {
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    }
    else {
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
    }
	samplerInfo.borderColor = borderColor;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;   
    samplerInfo.compareEnable = compare;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}


/// Configure how this image is to be sampled - this is same a the openGl texture filtering
void VulkanRenderer::createTextureSampler(Sampler2D& texture2D) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &texture2D.sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void VulkanRenderer::createTextureImageView(Sampler2D& texture2D) {
    texture2D.imageView = createImageView(texture2D.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}


/// Defines the way the shader looks at the image
VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    return imageView;
}

void VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    vkBindImageMemory(device, image, imageMemory, 0);
}

void VulkanRenderer::DestroySampler2D(Sampler2D& texture2D) {
    vkDestroyImageView(device, texture2D.imageView, nullptr);
    vkDestroyImage(device, texture2D.image, nullptr);
    vkFreeMemory(device, texture2D.imageDeviceMemory, nullptr);
    vkDestroySampler(device, texture2D.sampler, nullptr);  
}

Sampler2D VulkanRenderer::SkyBoxSampler(std::vector<std::string> paths)
{
    if (paths.size() == 6) {
        throw::std::runtime_error("SkyBox File Paths Not Set");
    }

    Sampler2D CubeSampler;
    SDL_Surface* im0 = IMG_Load(paths[0].c_str());
    SDL_Surface* image0 = SDL_ConvertSurface(im0, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* im1 = IMG_Load(paths[1].c_str());
    SDL_Surface* image1 = SDL_ConvertSurface(im1, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* im2 = IMG_Load(paths[2].c_str());
    SDL_Surface* image2 = SDL_ConvertSurface(im2, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* im3 = IMG_Load(paths[3].c_str());
    SDL_Surface* image3 = SDL_ConvertSurface(im3, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* im4 = IMG_Load(paths[4].c_str());
    SDL_Surface* image4 = SDL_ConvertSurface(im4, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface* im5 = IMG_Load(paths[5].c_str());
    SDL_Surface* image5 = SDL_ConvertSurface(im5, SDL_PIXELFORMAT_RGBA32);

    SDL_DestroySurface(im0);
    SDL_DestroySurface(im1);
    SDL_DestroySurface(im2);
    SDL_DestroySurface(im3);
    SDL_DestroySurface(im4);
    SDL_DestroySurface(im5);

    // Create Cube Image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    // 1024 x 1024 atm the moment
    imageInfo.extent = { static_cast<unsigned int>(image0->w),  static_cast<unsigned int>(image0->h), 1 }; 
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 6;  // six faces
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // important

    if (vkCreateImage(device, &imageInfo, nullptr, &CubeSampler.image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, CubeSampler.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &CubeSampler.imageDeviceMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    vkBindImageMemory(device, CubeSampler.image, CubeSampler.imageDeviceMemory, 0);

    VkDeviceSize imageSize = image0->w * image0->h * 4; /// RGBA only 
    // allocate buffer 
    BufferMemory stagingBuffer{}; 
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer.bufferID, stagingBuffer.bufferMemoryID);

    // change the layout of the dst image
    // transition in a dst to copy over
    CubeImageLayoutTransition(CubeSampler.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT);

    //load image to staging buffer. 
    // copy form buffer to image for each face
    void* data = nullptr;
    VkResult result = vkMapMemory(device, stagingBuffer.bufferMemoryID, 0, imageSize, 0, &data);
    if (result != VK_SUCCESS)
        throw std::runtime_error("map failed");
    //const char* formatName = SDL_GetPixelFormatName(image0->format);
    //printf("format: %s ,pitch:%d ,w: %d,h: %d,pitch xh: %d", formatName, image0->pitch,image0->w, image0->h , image0->pitch * image0->h);
         // dts   source         amount in bytes
    memcpy(data, image0->pixels, static_cast<size_t>(imageSize)); // load image0 +X (right)
    CopyBufferToImage(stagingBuffer.bufferID, CubeSampler.image, image0->w, image0->h, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 0);
    memcpy(data, image1->pixels, static_cast<size_t>(imageSize));  // load image1 -X (left)
    CopyBufferToImage(stagingBuffer.bufferID, CubeSampler.image, image1->w, image1->h, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0);
    memcpy(data, image2->pixels, static_cast<size_t>(imageSize));  // load image2 +Y (up)
    CopyBufferToImage(stagingBuffer.bufferID, CubeSampler.image, image2->w, image2->h, VK_IMAGE_ASPECT_COLOR_BIT, 1, 2, 0);
    memcpy(data, image3->pixels, static_cast<size_t>(imageSize)); // load image3 -Y (donw)
    CopyBufferToImage(stagingBuffer.bufferID, CubeSampler.image, image3->w, image3->h, VK_IMAGE_ASPECT_COLOR_BIT, 1, 3, 0);
    memcpy(data, image4->pixels, static_cast<size_t>(imageSize)); // load image4 +Z (front)
    CopyBufferToImage(stagingBuffer.bufferID, CubeSampler.image, image4->w, image4->h, VK_IMAGE_ASPECT_COLOR_BIT, 1, 4, 0);
    memcpy(data, image5->pixels, static_cast<size_t>(imageSize)); // load image5 -Z (back)
    CopyBufferToImage(stagingBuffer.bufferID, CubeSampler.image, image5->w, image5->h, VK_IMAGE_ASPECT_COLOR_BIT, 1, 5, 0);


    //transition out to sampler
    CubeImageLayoutTransition(CubeSampler.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
    //freeing tempory resources
    vkUnmapMemory(device, stagingBuffer.bufferMemoryID);
    vkDestroyBuffer(device, stagingBuffer.bufferID, nullptr);
    vkFreeMemory(device, stagingBuffer.bufferMemoryID, nullptr);
    SDL_DestroySurface(image0);
    SDL_DestroySurface(image1);
    SDL_DestroySurface(image2);
    SDL_DestroySurface(image3);
    SDL_DestroySurface(image4);
    SDL_DestroySurface(image5);
    // create view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = CubeSampler.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6;

   
    if (vkCreateImageView(device, &viewInfo, nullptr, &CubeSampler.imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    
    // create sampler
    CreateSampler(CubeSampler.sampler, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, VK_FALSE);

    return CubeSampler;
}

