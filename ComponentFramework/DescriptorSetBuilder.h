#ifndef DESCRIPTORSETBUILDER_H
#define DESCRIPTORSETBUILDER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <assert.h>
#include "CoreStructs.h"
/// A descriptor can be thought of as a pointer to a UBO or an image sampler.
/// This information is then available the pipeline.
/// However, unlike a regular pointer, it also contains additional information. 
/// (See my structure below)
/// Descriptors cannot be bound alone (unlike OpenGl)
/// they must be stored in sets - Descriptor Sets. 
/// There can be multiple Descriptor Sets. Each Descriptor Set needs to be given 
/// a unique Set Layout ID



/// Crazy idea: assemble all the raw data I'll need to create a layout, pool and the sets in one master
/// data collection. Map the binding point with the data so the reusing a binding can easily be found, 
/// see unordered_map count().



class DescriptorSetBuilder {
    /// C11 precautions 
    DescriptorSetBuilder(const DescriptorSetBuilder&) = delete;  /// Copy constructor
    DescriptorSetBuilder(DescriptorSetBuilder&&) = delete;       /// Move constructor
    DescriptorSetBuilder& operator = (const DescriptorSetBuilder&) = delete; /// Copy operator
    DescriptorSetBuilder& operator = (DescriptorSetBuilder&&) = delete;      /// Move operator
private:
    VkDevice device;
    /// I'm using a map here because it can detect if the binding location 
    /// is already being used. See add() methods
    std::unordered_map<uint32_t, SingleDescriptorInfoCollection> desciptorsData{};

    uint32_t numberOfDescriptors;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSet;
    uint32_t numSwapChains;

public:
    DescriptorSetBuilder(VkDevice device_);
    ~DescriptorSetBuilder();

    /// Add UBO descriptor
    DescriptorSetBuilder& add(uint32_t binding, VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags, uint32_t count, std::vector<BufferMemory> bufferData);

    /// Add Sampler2D descriptor
    DescriptorSetBuilder& add(uint32_t binding, VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags, uint32_t count, Sampler2D *imageData);
    
    DescriptorSetInfo BuildDescriptorSet(uint32_t numSwapChains_);

    std::vector<VkDescriptorSet>& getDescriptorSet(){ return descriptorSet;}
    VkDescriptorPool getDescriptorPool() { return descriptorPool; }
    VkDescriptorSetLayout  getDescriptorSetLayout(){ return descriptorSetLayout; }

    
private:
    void createLayout();
    void createDescriptorPool();
    void allocateDescriptorSet(); 
    void populateDescriptorSet();
     
};

#endif
