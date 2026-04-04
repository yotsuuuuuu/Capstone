#include "VulkanRenderer.h"
#include "Debug.h"
PipelineInfo VulkanRenderer::CreateGraphicsPipeline(VkDescriptorSetLayout descriptorSetLayout, const char* vertFile, const char* fragFile,
    const char* tessCtrlFile, const char* tessEvalFile, const char* geomFile) {

    PipelineInfo graphicsPipeInfo;

    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;
    std::vector<char> tessCtrlCode;
    std::vector<char> tessEvalCode;
    std::vector<char> geomCode;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;
    VkShaderModule tessCtrlModule = VK_NULL_HANDLE;
    VkShaderModule tessEvalModule = VK_NULL_HANDLE;
    VkShaderModule geomModule = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{}; 
    VkPipelineShaderStageCreateInfo tessCtrlStageInfo{};
    VkPipelineShaderStageCreateInfo tessEvalStageInfo{};
    VkPipelineShaderStageCreateInfo geomStageInfo{};

    if (vertFile != nullptr && fragFile != nullptr){
        vertShaderCode = readFile(vertFile);
        fragShaderCode = readFile(fragFile);
        vertShaderModule = createShaderModule(vertShaderCode);
        fragShaderModule = createShaderModule(fragShaderCode);
    }

    if(tessCtrlFile != nullptr && tessEvalFile != nullptr){
        tessCtrlCode = readFile(tessCtrlFile);
        tessEvalCode = readFile(tessEvalFile);
        tessCtrlModule = createShaderModule(tessCtrlCode);
        tessEvalModule = createShaderModule(tessEvalCode); 
    }

    if(geomFile != nullptr){
        geomCode = readFile(geomFile);
        geomModule = createShaderModule(geomCode);
       
    }

    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    tessCtrlStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessCtrlStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    tessCtrlStageInfo.module = tessCtrlModule;
    tessCtrlStageInfo.pName = "main";

    tessEvalStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessEvalStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    tessEvalStageInfo.module = tessEvalModule;
    tessEvalStageInfo.pName = "main";

    geomStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    geomStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    geomStageInfo.module = geomModule;
    geomStageInfo.pName = "main";


    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    if(vertShaderModule != VK_NULL_HANDLE) shaderStages.push_back(vertShaderStageInfo);
    if(tessCtrlModule != VK_NULL_HANDLE) shaderStages.push_back(tessCtrlStageInfo);
    if(tessEvalModule != VK_NULL_HANDLE) shaderStages.push_back(tessEvalStageInfo);
    if(geomModule != VK_NULL_HANDLE) shaderStages.push_back(geomStageInfo);
    if(fragShaderModule != VK_NULL_HANDLE) shaderStages.push_back(fragShaderStageInfo);


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    range.offset = 0;
    range.size = sizeof(ModelMatrixPushConst);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &range;
    //pipelineLayoutInfo.flags = VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;


    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &graphicsPipeInfo.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

     

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<int>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = graphicsPipeInfo.pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeInfo.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    
    if (fragShaderModule) vkDestroyShaderModule(device, fragShaderModule, nullptr);
    if (fragShaderModule) vkDestroyShaderModule(device, vertShaderModule, nullptr);
    if (tessCtrlModule) vkDestroyShaderModule(device, tessCtrlModule, nullptr);
    if (tessEvalModule) vkDestroyShaderModule(device, tessEvalModule, nullptr);
    if (geomModule) vkDestroyShaderModule(device, geomModule, nullptr);

    return graphicsPipeInfo;
}

PipelineInfo VulkanRenderer::CreateGraphicsPipeline(std::vector<VkDescriptorSetLayout> descriptorSetLayout, const char* vertFile, const char* fragFile, const char* tessCtrlFile, const char* tessEvalFile, const char* geomFile)
{
    PipelineInfo graphicsPipeInfo;

    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;
    std::vector<char> tessCtrlCode;
    std::vector<char> tessEvalCode;
    std::vector<char> geomCode;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;
    VkShaderModule tessCtrlModule = VK_NULL_HANDLE;
    VkShaderModule tessEvalModule = VK_NULL_HANDLE;
    VkShaderModule geomModule = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    VkPipelineShaderStageCreateInfo tessCtrlStageInfo{};
    VkPipelineShaderStageCreateInfo tessEvalStageInfo{};
    VkPipelineShaderStageCreateInfo geomStageInfo{};

    if (vertFile != nullptr && fragFile != nullptr) {
        vertShaderCode = readFile(vertFile);
        fragShaderCode = readFile(fragFile);
        vertShaderModule = createShaderModule(vertShaderCode);
        fragShaderModule = createShaderModule(fragShaderCode);
    }

    if (tessCtrlFile != nullptr && tessEvalFile != nullptr) {
        tessCtrlCode = readFile(tessCtrlFile);
        tessEvalCode = readFile(tessEvalFile);
        tessCtrlModule = createShaderModule(tessCtrlCode);
        tessEvalModule = createShaderModule(tessEvalCode);
    }

    if (geomFile != nullptr) {
        geomCode = readFile(geomFile);
        geomModule = createShaderModule(geomCode);

    }

    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    tessCtrlStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessCtrlStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    tessCtrlStageInfo.module = tessCtrlModule;
    tessCtrlStageInfo.pName = "main";

    tessEvalStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessEvalStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    tessEvalStageInfo.module = tessEvalModule;
    tessEvalStageInfo.pName = "main";

    geomStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    geomStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    geomStageInfo.module = geomModule;
    geomStageInfo.pName = "main";


    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    if (vertShaderModule != VK_NULL_HANDLE) shaderStages.push_back(vertShaderStageInfo);
    if (tessCtrlModule != VK_NULL_HANDLE) shaderStages.push_back(tessCtrlStageInfo);
    if (tessEvalModule != VK_NULL_HANDLE) shaderStages.push_back(tessEvalStageInfo);
    if (geomModule != VK_NULL_HANDLE) shaderStages.push_back(geomStageInfo);
    if (fragShaderModule != VK_NULL_HANDLE) shaderStages.push_back(fragShaderStageInfo);


    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    range.offset = 0;
    range.size = sizeof(ModelMatrixPushConst);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &range;
    //pipelineLayoutInfo.flags = VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT;


    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &graphicsPipeInfo.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }



    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = graphicsPipeInfo.pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeInfo.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }


    if (fragShaderModule) vkDestroyShaderModule(device, fragShaderModule, nullptr);
    if (fragShaderModule) vkDestroyShaderModule(device, vertShaderModule, nullptr);
    if (tessCtrlModule) vkDestroyShaderModule(device, tessCtrlModule, nullptr);
    if (tessEvalModule) vkDestroyShaderModule(device, tessEvalModule, nullptr);
    if (geomModule) vkDestroyShaderModule(device, geomModule, nullptr);

    return graphicsPipeInfo;
}

PipelineInfo VulkanRenderer::CreateGraphicsPipeline(const std::vector <VkDescriptorSetLayout>& descriptorSetLayout, PipeLineConfig config, std::optional<std::string> vertFile, std::optional<std::string> fragFile, std::optional<std::string> tessCtrlFile, std::optional<std::string> tessEvalFile, std::optional<std::string> geomFile)
{
    PipelineInfo graphicsPipeInfo{};

    std::vector<char> vertShaderCode;
    std::vector<char> fragShaderCode;
    std::vector<char> tessCtrlCode;
    std::vector<char> tessEvalCode;
    std::vector<char> geomCode;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;
    VkShaderModule tessCtrlModule = VK_NULL_HANDLE;
    VkShaderModule tessEvalModule = VK_NULL_HANDLE;
    VkShaderModule geomModule = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    VkPipelineShaderStageCreateInfo tessCtrlStageInfo{};
    VkPipelineShaderStageCreateInfo tessEvalStageInfo{};
    VkPipelineShaderStageCreateInfo geomStageInfo{};


    if (vertFile && fragFile ) {
        vertShaderCode = readFile(vertFile.value());
        fragShaderCode = readFile(fragFile.value());
        vertShaderModule = createShaderModule(vertShaderCode);
        fragShaderModule = createShaderModule(fragShaderCode);
    }

    if (tessCtrlFile && tessEvalFile) {
        tessCtrlCode = readFile(tessCtrlFile.value());
        tessEvalCode = readFile(tessEvalFile.value());
        tessCtrlModule = createShaderModule(tessCtrlCode);
        tessEvalModule = createShaderModule(tessEvalCode);
    }

    if (geomFile) {
        geomCode = readFile(geomFile.value());
        geomModule = createShaderModule(geomCode);

    }

    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    tessCtrlStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessCtrlStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    tessCtrlStageInfo.module = tessCtrlModule;
    tessCtrlStageInfo.pName = "main";

    tessEvalStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessEvalStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    tessEvalStageInfo.module = tessEvalModule;
    tessEvalStageInfo.pName = "main";

    geomStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    geomStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    geomStageInfo.module = geomModule;
    geomStageInfo.pName = "main";

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    if (vertShaderModule != VK_NULL_HANDLE) shaderStages.push_back(vertShaderStageInfo);
    if (tessCtrlModule != VK_NULL_HANDLE) shaderStages.push_back(tessCtrlStageInfo);
    if (tessEvalModule != VK_NULL_HANDLE) shaderStages.push_back(tessEvalStageInfo);
    if (geomModule != VK_NULL_HANDLE) shaderStages.push_back(geomStageInfo);
    if (fragShaderModule != VK_NULL_HANDLE) shaderStages.push_back(fragShaderStageInfo);

    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Vertex::getAttributeDescriptions();
    if (!config.vertexinfo) {
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    }
    else {
        vertexInputInfo = config.vertexinfo.value();
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = config.topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(config.viewPortsize.width);
    viewport.height = static_cast<float>(config.viewPortsize.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = config.viewPortsize;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = config.polygonMode;//  VK_POLYGON_MODE_LINE
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = config.cullMode;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = config.depthBias;
    if (config.depthBias != VK_FALSE) {
        rasterizer.depthBiasConstantFactor = config.depthBiasConstantFactor;
        rasterizer.depthBiasSlopeFactor = config.depthBiasSlopeFactor;
        rasterizer.depthBiasClamp = config.depthBiasClamp;
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = config.depthTestEnable;
    depthStencil.depthWriteEnable = config.depthWriteEnable;
    depthStencil.depthCompareOp = config.depthCompareOp;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    switch (config.blendMode) {
    case PipeLineConfig::BlendMode::OPAQUE: {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        break;
    }
    case  PipeLineConfig::BlendMode::ADDITIVE: {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        break;
    }
    case  PipeLineConfig::BlendMode::ALPHA: {
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        break;
    }
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = (config.Color) ?  1 : 0;
    colorBlending.pAttachments = (config.Color) ? &colorBlendAttachment : nullptr;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    range.offset = 0;
    range.size = sizeof(ModelMatrixPushConst);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &range;       

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &graphicsPipeInfo.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    std::vector<VkDynamicState> dynamicStates;
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    if (config.dynamicViewport) {
        dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    }
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<int>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = graphicsPipeInfo.pipelineLayout;
    pipelineInfo.renderPass = config.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeInfo.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }


    if (vertShaderModule) vkDestroyShaderModule(device, fragShaderModule, nullptr);
    if (fragShaderModule) vkDestroyShaderModule(device, vertShaderModule, nullptr);
    if (tessCtrlModule) vkDestroyShaderModule(device, tessCtrlModule, nullptr);
    if (tessEvalModule) vkDestroyShaderModule(device, tessEvalModule, nullptr);
    if (geomModule) vkDestroyShaderModule(device, geomModule, nullptr);
    

    return graphicsPipeInfo;
}

PipelineInfo VulkanRenderer::CreateComputePipeline(const std::vector <VkDescriptorSetLayout>& descriptorSetLayout, std::optional<std::string> computeFile, std::optional<uint32_t> pushConstSize)
{
    PipelineInfo ComputePipeInfo;

    VkPipelineShaderStageCreateInfo computeInfo{};
    VkShaderModule module;
    if(!computeFile)
        throw std::runtime_error("No File Provided for Compute Shaders");
    if((pushConstSize.has_value() && pushConstSize.value() <= 0))
        throw std::runtime_error("Invalid PushConstant Value");

    module = createShaderModule(readFile(computeFile.value()));

    computeInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeInfo.module = module;
    computeInfo.pName = "main";

    
    VkPushConstantRange range{};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
    if (pushConstSize.has_value() && pushConstSize.value() > 0) {      
        range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        range.offset = 0;
        range.size =  pushConstSize.value();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &range;
    }
    else {
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
    }


    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &ComputePipeInfo.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }


    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.layout = ComputePipeInfo.pipelineLayout;
    computePipelineCreateInfo.stage = computeInfo;
    computePipelineCreateInfo.flags = 0 ;
    computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    computePipelineCreateInfo.basePipelineIndex = -1 ;

    if ( vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &ComputePipeInfo.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }


    vkDestroyShaderModule(device, module, nullptr);
    return ComputePipeInfo;
}





void VulkanRenderer::DestroyPipeline(PipelineInfo pipelineInfo){
    vkDestroyPipelineLayout(device, pipelineInfo.pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipelineInfo.pipeline, nullptr);
}

std::vector<char> VulkanRenderer::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

PipeLineConfig VulkanRenderer::GetMainPassPipeLineConfig()
{
    PipeLineConfig config;

    config.blendMode = PipeLineConfig::BlendMode::OPAQUE;    // Controls how fragment color blends with existing framebuffer color
    config.Color = true;                                     // Enables writing fragment shader output to color attachments
    config.cullMode = VK_CULL_MODE_BACK_BIT;                 // Determines which triangle faces are discarded before rasterization

    config.depthBias = VK_FALSE;                             // Enables polygon depth offset (used to prevent z-fighting e.g. shadows)
    config.depthBiasClamp = 0.0f;                            // Limits maximum depth offset applied
    config.depthBiasConstantFactor = 0.0f;                   // Constant depth offset added to fragments
    config.depthBiasSlopeFactor = 0.0f;                      // Depth offset scaled by polygon slope relative to camera

    config.depthCompareOp = VK_COMPARE_OP_LESS;              // Comparison rule used when testing fragment depth vs depth buffer
    config.depthTestEnable = VK_TRUE;                        // Enables depth testing stage
    config.depthWriteEnable = VK_TRUE;                       // Controls whether fragments update depth buffer

    config.polygonMode = VK_POLYGON_MODE_FILL;               // Controls rasterization style (filled, wireframe, points)

    config.renderPass = hdrInfo.hdrRenderPass;               // Defines which render pass/subpass this pipeline is compatible with

    config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;   // Defines how vertex data is interpreted into primitives

    config.viewPortsize = swapChainExtent;                   // Defines viewport and scissor dimensions used during rasterization

    ///config.vertexinfo;                                    // Would define vertex binding + attribute layout for vertex input stage

    return config;
}

PipeLineConfig VulkanRenderer::GetSwapChainPipeLineConfig()
{
    PipeLineConfig config;

    config.blendMode = PipeLineConfig::BlendMode::OPAQUE;    // Controls how fragment color blends with existing framebuffer color
    config.Color = true;                                     // Enables writing fragment shader output to color attachments
    config.cullMode = VK_CULL_MODE_BACK_BIT;                 // Determines which triangle faces are discarded before rasterization

    config.depthBias = VK_FALSE;                             // Enables polygon depth offset (used to prevent z-fighting e.g. shadows)
    config.depthBiasClamp = 0.0f;                            // Limits maximum depth offset applied
    config.depthBiasConstantFactor = 0.0f;                   // Constant depth offset added to fragments
    config.depthBiasSlopeFactor = 0.0f;                      // Depth offset scaled by polygon slope relative to camera

    config.depthCompareOp = VK_COMPARE_OP_LESS;              // Comparison rule used when testing fragment depth vs depth buffer
    config.depthTestEnable = VK_TRUE;                        // Enables depth testing stage
    config.depthWriteEnable = VK_TRUE;                       // Controls whether fragments update depth buffer

    config.polygonMode = VK_POLYGON_MODE_FILL;               // Controls rasterization style (filled, wireframe, points)

    config.renderPass = renderPass;                          // Defines which render pass/subpass this pipeline is compatible with

    config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;   // Defines how vertex data is interpreted into primitives

    config.viewPortsize = swapChainExtent;                   // Defines viewport and scissor dimensions used during rasterization

    ///config.vertexinfo;                                    // Would define vertex binding + attribute layout for vertex input stage

    return config;
}