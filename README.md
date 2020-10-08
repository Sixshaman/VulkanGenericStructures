# VulkanGenericStructures
A small header-only library for chaining structures in Vulkan.

## Building

Execute the command: `python GenerateHeaders.py`

Requires Python 3.3+ with BeautifulSoup and lxml.

## Files

Just copy the file you need to your project directory.

**Include/VulkanGenericStructures.h** — bindings for `vulkan.h`;  
**Include/VulkanGenericStructures.hpp** — bindings for `vulkan.hpp`.

## API

This library uses `namespace vgs` for its parts.

Classes:
- `GenericStruct` — non-owning view of any Vulkan structure that contains `sType` and `pNext` members. Can only be used in scope of the original structure variable.
- `StructureBlob` — owning blob of any Vulkan structure that contains `sType` and `pNext` members. Can be freely passed outside of scope and returned from the functions (with the exception if the structure members reference memory).
- `GenericStructureChain` — non-owning structure chain builder. Provides `AppendToChain` and `AppendToChainGeneric` (for generic structure classes) methods which assigns last `pNext` member of the chain to the provided structure. Can only be used in scope of all structure variables that form it.
- `StructureChainBlob` — owning structure chain builder. Can be freely passed outside of scope and returned from functions (with the exception if the structure members reference memory).

## Usage

### Initializing all members for VkPhysicalDeviceFeatures2 chain

(Assumes all passed structures have initialized `sType`)
```cpp

GenericStructureChain<vk::PhysicalDeviceFeatures2> physicalDeviceFeaturesChain;
physicalDeviceFeaturesChain.AppendToChain(mVulkan11Features);                    //VkPhysicalDeviceVulkan11Features
physicalDeviceFeaturesChain.AppendToChain(mBlendOperationAdvancedFeatures);      //VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT
physicalDeviceFeaturesChain.AppendToChain(mBufferDeviceAddressFeatures);         //VkPhysicalDeviceBufferDeviceAddressFeatures
physicalDeviceFeaturesChain.AppendToChain(mConditionalRenderingFeatures);        //VkPhysicalDeviceConditionalRenderingFeaturesEXT
physicalDeviceFeaturesChain.AppendToChain(mDescriptorIndexingFeatures);          //VkPhysicalDeviceDescriptorIndexingFeatures
physicalDeviceFeaturesChain.AppendToChain(mDeviceGeneratedCommandsFeatures);     //VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV
physicalDeviceFeaturesChain.AppendToChain(mFragmentShaderBarycentricFeatures);   //VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV
physicalDeviceFeaturesChain.AppendToChain(mShadingRateImageFeatures);            //VkPhysicalDeviceShadingRateImageFeaturesNV
physicalDeviceFeaturesChain.AppendToChain(mTexelBufferAlignmentFeatures);        //VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT
physicalDeviceFeaturesChain.AppendToChain(mUniformBufferStandardLayoutFeatures); //VkPhysicalDeviceUniformBufferStandardLayoutFeatures
physicalDeviceFeaturesChain.AppendToChain(mVulkanMemoryModelFeatures);           //VkPhysicalDeviceVulkanMemoryModelFeatures
  
vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeaturesChain.GetChainHead());
```

### Creating a structure chain based on the extensions enabled

No more ridiculous `vulkan.hpp` types such as `vk::StructureChain<vk::Struct1, vk::Struct2, vk::Struct3, vk::Struct1337, vk::Struct18446744073709551616, vk::Struct∞>`, which are very awkward to pass around to functions. No more `unlink` and `relink`. No more `get<>` manually for each entry in the chain.

Now it works like a charm:
```cpp
std::unordered_map<std::string, StructureBlob> extensionBasedStructures;

//Init the map, maybe manually or from a config or whatever
extensionBasedStructures[VK_EXT_DEBUG_REPORT_EXTENSION_NAME]        = StructureBlob(debugReportCallbackParameters);
extensionBasedStructures[VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME] = StructureBlob(validationFeatureParameters);

//...Some other code...

//Actually build VkInstanceCreateInfo chain
StructureChainBlob<VkInstanceCreateInfo> instanceCreateInfoChain;
for(const std::string& extension: requestedExtensions)
{
  if(availableExtensions.contains(extension))
  {
    instanceCreateInfoChain.AppendToChainGeneric(extensionBasedStructures.at(extension));
  }
}

```
