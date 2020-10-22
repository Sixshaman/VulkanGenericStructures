#include "GenericStructureTestsH.hpp"
#include <vulkan/vulkan.h>
#include <vector>

#define vgs vgs_h
#include "..\..\..\Include\VulkanGenericStructures.h"

GenericStructureTestsH::GenericStructureTestsH()
{
}

GenericStructureTestsH::~GenericStructureTestsH()
{
}

void GenericStructureTestsH::DoTests()
{
	TestGenericStructure();
	TestStructureBlob();
	TestGenericStructureChain();
	TestStructureChainBlob();
}

void GenericStructureTestsH::TestGenericStructure()
{
	//Common tests
	VkInstanceCreateInfo               instanceCreateInfo;
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;

	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_MAX_ENUM; //Deliberately wrong

	instanceCreateInfo.pNext = &debugReportCallbackCreateInfo;

	//Data references should be saved
	vgs::GenericStruct genericInstanceCreateInfo = vgs::TransmuteTypeToSType(instanceCreateInfo);
	assert(genericInstanceCreateInfo.GetStructureData() == reinterpret_cast<std::byte*>(&instanceCreateInfo));
	assert(genericInstanceCreateInfo.GetSType()         == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	assert(genericInstanceCreateInfo.GetPNext()         == &debugReportCallbackCreateInfo);

	vgs::GenericStruct genericDebugReportCallbackCreateInfo(debugReportCallbackCreateInfo);
	assert(genericDebugReportCallbackCreateInfo.GetStructureData() == reinterpret_cast<std::byte*>(&debugReportCallbackCreateInfo));
	assert(genericDebugReportCallbackCreateInfo.GetSType()         == VK_STRUCTURE_TYPE_MAX_ENUM); //Regular struct creating SHOULD NOT mess with original sType
	assert(genericInstanceCreateInfo.GetPNext()                    == genericDebugReportCallbackCreateInfo.GetStructureData());

	VkInstanceCreateInfo&               instanceCreateInfo2            = genericInstanceCreateInfo.GetDataAs<VkInstanceCreateInfo>();
	VkDebugReportCallbackCreateInfoEXT& debugReportCallbackCreateInfo2 = genericDebugReportCallbackCreateInfo.GetDataAs<VkDebugReportCallbackCreateInfoEXT>();
	
	assert(instanceCreateInfo2.pNext == &debugReportCallbackCreateInfo);
	assert(instanceCreateInfo.pNext  == &debugReportCallbackCreateInfo2);

	//Can change structure data from the value of GetDataAs
	instanceCreateInfo2.enabledLayerCount = 1;
	assert(instanceCreateInfo.enabledLayerCount == 1);

	//Operator= test
	vgs::GenericStruct genericInstanceCreateInfo2 = instanceCreateInfo;
	assert(genericInstanceCreateInfo2.GetStructureData() == reinterpret_cast<std::byte*>(&instanceCreateInfo));
	assert(genericInstanceCreateInfo2.GetSType()         == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	assert(genericInstanceCreateInfo2.GetPNext()         == &debugReportCallbackCreateInfo);

	//Emplacement test
	std::vector<vgs::GenericStruct> genericStructs;
	genericStructs.emplace_back(instanceCreateInfo);
	genericStructs.emplace_back(debugReportCallbackCreateInfo);

	assert(genericStructs[0].GetPNext()         == genericStructs[1].GetStructureData());
	assert(genericStructs[0].GetStructureData() == reinterpret_cast<std::byte*>(&instanceCreateInfo));

	//Copy constructor test
	std::vector<vgs::GenericStruct> genericStructs2;
	genericStructs2.push_back(genericInstanceCreateInfo);
	genericStructs2.push_back(genericDebugReportCallbackCreateInfo);

	assert(genericStructs2[0].GetPNext()         == genericStructs2[1].GetStructureData());
	assert(genericStructs2[0].GetStructureData() == reinterpret_cast<std::byte*>(&instanceCreateInfo));
}

void GenericStructureTestsH::TestStructureBlob()
{
	//Common tests
	VkInstanceCreateInfo               instanceCreateInfo;
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;

	instanceCreateInfo.pNext            = &debugReportCallbackCreateInfo;
	debugReportCallbackCreateInfo.pNext = nullptr;

	//Data should be different, but other contents, including pNext, should be left untouched
	vgs::StructureBlob instanceCreateInfoBlob(instanceCreateInfo);
	assert(instanceCreateInfoBlob.GetStructureData() != reinterpret_cast<std::byte*>(&instanceCreateInfo));
	assert(instanceCreateInfoBlob.GetSType()         == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	assert(instanceCreateInfoBlob.GetPNext()         == nullptr);

	vgs::StructureBlob debugReportCallbackCreateInfoBlob(debugReportCallbackCreateInfo);
	assert(debugReportCallbackCreateInfoBlob.GetStructureData() != reinterpret_cast<std::byte*>(&debugReportCallbackCreateInfo));
	assert(debugReportCallbackCreateInfoBlob.GetSType()         == VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT);
	assert(debugReportCallbackCreateInfoBlob.GetPNext()         == nullptr);

	//Changes in blob shouldn't change the original
	VkInstanceCreateInfo& instanceCreateInfo2 = instanceCreateInfoBlob.GetDataAs<VkInstanceCreateInfo>();

	instanceCreateInfo2.enabledExtensionCount = 3;
	assert(instanceCreateInfo.enabledExtensionCount != 3);

	instanceCreateInfo.enabledExtensionCount = 4;
	assert(instanceCreateInfo2.enabledExtensionCount != 4);

	//We should be able to safely delete all local variables after we've made a blob
	vgs::StructureBlob instanceCreateInfoBlob2;
	{
		VkInstanceCreateInfo localInstanceCreateInfo;
		localInstanceCreateInfo.enabledLayerCount     = 1;
		localInstanceCreateInfo.enabledExtensionCount = 2;

		VkDebugReportCallbackCreateInfoEXT localDebugReportCallbackCreateInfo;
		localInstanceCreateInfo.pNext = &localDebugReportCallbackCreateInfo;

		instanceCreateInfoBlob2 = vgs::StructureBlob(localInstanceCreateInfo);
	}

	//Assignment operator should zero out pNext pointer too
	VkInstanceCreateInfo& copiedInstanceCreateInfo = instanceCreateInfoBlob2.GetDataAs<VkInstanceCreateInfo>();
	assert(copiedInstanceCreateInfo.enabledLayerCount     == 1);
	assert(copiedInstanceCreateInfo.enabledExtensionCount == 2);
	assert(copiedInstanceCreateInfo.sType                 == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	assert(copiedInstanceCreateInfo.pNext                 == nullptr);

	//Emplace test
	std::vector<vgs::StructureBlob> genericStructs;
	genericStructs.emplace_back(instanceCreateInfo);
	genericStructs.emplace_back(debugReportCallbackCreateInfo);

	assert(genericStructs[0].GetPNext() == nullptr);
	assert(genericStructs[1].GetPNext() == nullptr);

	//Copy constructor test
	std::vector<vgs::StructureBlob> genericStructs2;
	genericStructs2.push_back(instanceCreateInfoBlob);
	genericStructs2.push_back(debugReportCallbackCreateInfoBlob);

	//PNext should be zeroed out
	assert(genericStructs2[0].GetPNext() == nullptr);
	assert(genericStructs2[1].GetPNext() == nullptr);
}

void GenericStructureTestsH::TestGenericStructureChain()
{
	//Base test
	VkPhysicalDeviceVulkan11Features                    vulkan11Features;
	VkPhysicalDeviceImagelessFramebufferFeatures        imagelessFramebufferFeatures;
	VkPhysicalDeviceUniformBufferStandardLayoutFeatures uniformBufferStandardLayoutFeatures;
	VkPhysicalDeviceVulkanMemoryModelFeatures           vulkanMemoryModelFeatures;

	//Test generic structures too
	VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeaturesBaseStruct;
	vgs::StructureBlob meshShaderFeatures(meshShaderFeaturesBaseStruct);

	VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV deviceGeneratedCommandsFeaturesBaseStruct;
	deviceGeneratedCommandsFeaturesBaseStruct.pNext = nullptr;
	vgs::GenericStruct deviceGeneratedCommandsFeatures = vgs::TransmuteTypeToSType(deviceGeneratedCommandsFeaturesBaseStruct); //sType should be already initialized in GenericStruct

	vgs::GenericStructureChain<VkPhysicalDeviceFeatures2> physicalDeviceFeatures2Chain;
	physicalDeviceFeatures2Chain.AppendToChain(vulkan11Features);
	physicalDeviceFeatures2Chain.AppendToChain(imagelessFramebufferFeatures);
	physicalDeviceFeatures2Chain.AppendToChain(uniformBufferStandardLayoutFeatures);
	physicalDeviceFeatures2Chain.AppendToChain(vulkanMemoryModelFeatures);

	physicalDeviceFeatures2Chain.AppendToChainGeneric(meshShaderFeatures);
	physicalDeviceFeatures2Chain.AppendToChainGeneric(deviceGeneratedCommandsFeatures);

	auto& physicalDeviceFeatures2 = physicalDeviceFeatures2Chain.GetChainHead();

	assert(physicalDeviceFeatures2.pNext              == &vulkan11Features);
	assert(vulkan11Features.pNext                     == &imagelessFramebufferFeatures);
	assert(imagelessFramebufferFeatures.pNext         == &uniformBufferStandardLayoutFeatures);
	assert(uniformBufferStandardLayoutFeatures.pNext  == &vulkanMemoryModelFeatures);
	assert(vulkanMemoryModelFeatures.pNext            == meshShaderFeatures.GetStructureData());
	assert(meshShaderFeatures.GetPNext()              == deviceGeneratedCommandsFeatures.GetStructureData());
	assert(deviceGeneratedCommandsFeatures.GetPNext() == nullptr);

	//Test obtaining chain link data
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceVulkan11Features>()                    == &vulkan11Features);
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceImagelessFramebufferFeatures>()        == &imagelessFramebufferFeatures);
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>() == &uniformBufferStandardLayoutFeatures);
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceVulkanMemoryModelFeatures>()           == &vulkanMemoryModelFeatures);
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceMeshShaderFeaturesNV>()                == reinterpret_cast<VkPhysicalDeviceMeshShaderFeaturesNV*>(meshShaderFeatures.GetStructureData()));
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV>()   == reinterpret_cast<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV*>(deviceGeneratedCommandsFeatures.GetStructureData()));
	
	//Test sType initialization
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceVulkan11Features>().sType                    == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceImagelessFramebufferFeatures>().sType        == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>().sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceVulkanMemoryModelFeatures>().sType           == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceMeshShaderFeaturesNV>().sType                == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV>().sType   == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV);

	//Test structure filling
	VkBaseOutStructure* currStruct = reinterpret_cast<VkBaseOutStructure*>(&physicalDeviceFeatures2);
	while(currStruct != nullptr)
	{
		switch(currStruct->sType)
		{
		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2:
			((VkPhysicalDeviceFeatures2*)(currStruct))->features.geometryShader     = true;
			((VkPhysicalDeviceFeatures2*)(currStruct))->features.tessellationShader = true;
			break;

		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
			((VkPhysicalDeviceVulkan11Features*)(currStruct))->multiviewGeometryShader     = true;
			((VkPhysicalDeviceVulkan11Features*)(currStruct))->multiviewTessellationShader = true;
			break;

		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES:
			((VkPhysicalDeviceImagelessFramebufferFeatures*)(currStruct))->imagelessFramebuffer = true;
			break;

		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES:
			((VkPhysicalDeviceUniformBufferStandardLayoutFeatures*)(currStruct))->uniformBufferStandardLayout = true;
			break;

		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES:
			((VkPhysicalDeviceVulkanMemoryModelFeatures*)(currStruct))->vulkanMemoryModel            = true;
			((VkPhysicalDeviceVulkanMemoryModelFeatures*)(currStruct))->vulkanMemoryModelDeviceScope = true;
			break;

		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV:
			((VkPhysicalDeviceMeshShaderFeaturesNV*)(currStruct))->meshShader = true;
			((VkPhysicalDeviceMeshShaderFeaturesNV*)(currStruct))->taskShader = true;
			break;

		case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV:
			((VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV*)(currStruct))->deviceGeneratedCommands = true;
			break;

		default:
			break;
		}

		currStruct = currStruct->pNext;
	}

	assert(physicalDeviceFeatures2Chain.GetChainHead().features.geometryShader);
	assert(physicalDeviceFeatures2Chain.GetChainHead().features.tessellationShader);
	assert(vulkan11Features.multiviewGeometryShader);
	assert(vulkan11Features.multiviewTessellationShader);
	assert(imagelessFramebufferFeatures.imagelessFramebuffer);
	assert(uniformBufferStandardLayoutFeatures.uniformBufferStandardLayout);
	assert(vulkanMemoryModelFeatures.vulkanMemoryModel);
	assert(vulkanMemoryModelFeatures.vulkanMemoryModelDeviceScope);
	assert(meshShaderFeatures.GetDataAs<VkPhysicalDeviceMeshShaderFeaturesNV>().meshShader);
	assert(meshShaderFeatures.GetDataAs<VkPhysicalDeviceMeshShaderFeaturesNV>().taskShader);
	assert(deviceGeneratedCommandsFeatures.GetDataAs<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV>().deviceGeneratedCommands);
}

void GenericStructureTestsH::TestStructureChainBlob()
{
	//Base test
	vgs::StructureChainBlob<VkPhysicalDeviceFeatures2> physicalDeviceFeatures2Chain;

	VkPhysicalDeviceVulkan11Features             vulkan11Features;
	VkPhysicalDeviceImagelessFramebufferFeatures imagelessFramebufferFeatures;

	{
		//Test local variables
		VkPhysicalDeviceUniformBufferStandardLayoutFeatures uniformBufferStandardLayoutFeatures;
		VkPhysicalDeviceVulkanMemoryModelFeatures           vulkanMemoryModelFeatures;

		//Test generic structures too
		VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeaturesBaseStruct;
		vgs::StructureBlob meshShaderFeatures(meshShaderFeaturesBaseStruct);

		VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV deviceGeneratedCommandsFeaturesBaseStruct;
		deviceGeneratedCommandsFeaturesBaseStruct.pNext = nullptr;
		vgs::GenericStruct deviceGeneratedCommandsFeatures = vgs::TransmuteTypeToSType(deviceGeneratedCommandsFeaturesBaseStruct);

		physicalDeviceFeatures2Chain.AppendToChain(vulkan11Features);
		physicalDeviceFeatures2Chain.AppendToChain(imagelessFramebufferFeatures);
		physicalDeviceFeatures2Chain.AppendToChain(uniformBufferStandardLayoutFeatures);
		physicalDeviceFeatures2Chain.AppendToChain(vulkanMemoryModelFeatures);

		physicalDeviceFeatures2Chain.AppendToChainGeneric(meshShaderFeatures);
		physicalDeviceFeatures2Chain.AppendToChainGeneric(deviceGeneratedCommandsFeatures);

		auto& physicalDeviceFeatures2 = physicalDeviceFeatures2Chain.GetChainHead();

		//Test structure filling
		VkBaseOutStructure* currStruct = reinterpret_cast<VkBaseOutStructure*>(&physicalDeviceFeatures2);
		while(currStruct != nullptr)
		{
			switch(currStruct->sType)
			{
			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2:
				((VkPhysicalDeviceFeatures2*)(currStruct))->features.geometryShader     = true;
				((VkPhysicalDeviceFeatures2*)(currStruct))->features.tessellationShader = true;
				break;

			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
				((VkPhysicalDeviceVulkan11Features*)(currStruct))->multiviewGeometryShader     = true;
				((VkPhysicalDeviceVulkan11Features*)(currStruct))->multiviewTessellationShader = true;
				break;

			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES:
				((VkPhysicalDeviceImagelessFramebufferFeatures*)(currStruct))->imagelessFramebuffer = true;
				break;

			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES:
				((VkPhysicalDeviceUniformBufferStandardLayoutFeatures*)(currStruct))->uniformBufferStandardLayout = true;
				break;

			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES:
				((VkPhysicalDeviceVulkanMemoryModelFeatures*)(currStruct))->vulkanMemoryModel            = true;
				((VkPhysicalDeviceVulkanMemoryModelFeatures*)(currStruct))->vulkanMemoryModelDeviceScope = true;
				break;

			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV:
				((VkPhysicalDeviceMeshShaderFeaturesNV*)(currStruct))->meshShader = true;
				((VkPhysicalDeviceMeshShaderFeaturesNV*)(currStruct))->taskShader = true;
				break;

			case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV:
				((VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV*)(currStruct))->deviceGeneratedCommands = true;
				break;

			default:
				break;
			}

			currStruct = currStruct->pNext;
		}
	}

	//Test reassignment too
	vulkan11Features             = physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceVulkan11Features>();
	imagelessFramebufferFeatures = physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceImagelessFramebufferFeatures>();

	VkPhysicalDeviceFeatures2&                           testPhysicalDeviceFeatures2             = physicalDeviceFeatures2Chain.GetChainHead();
	VkPhysicalDeviceUniformBufferStandardLayoutFeatures& testUniformBufferStandardLayoutFeatures = physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>();
	VkPhysicalDeviceVulkanMemoryModelFeatures&           testVulkanMemoryModelFeatures           = physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceVulkanMemoryModelFeatures>();
	VkPhysicalDeviceMeshShaderFeaturesNV&                testMeshShaderFeatures                  = physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceMeshShaderFeaturesNV>();
	VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV&   testDeviceGeneratedCommandsFeatures     = physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV>();

	assert(imagelessFramebufferFeatures.pNext             == &testUniformBufferStandardLayoutFeatures);
	assert(testUniformBufferStandardLayoutFeatures.pNext  == &testVulkanMemoryModelFeatures);
	assert(testVulkanMemoryModelFeatures.pNext            == &testMeshShaderFeatures);
	assert(testMeshShaderFeatures.pNext                   == &testDeviceGeneratedCommandsFeatures);
	assert(testDeviceGeneratedCommandsFeatures.pNext      == nullptr);

	//Test sType initialization
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceVulkan11Features>().sType                    == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceImagelessFramebufferFeatures>().sType        == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>().sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceVulkanMemoryModelFeatures>().sType           == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceMeshShaderFeaturesNV>().sType                == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV);
	assert(physicalDeviceFeatures2Chain.GetChainLinkDataAs<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV>().sType   == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV);

	assert(physicalDeviceFeatures2Chain.GetChainHead().features.geometryShader);
	assert(physicalDeviceFeatures2Chain.GetChainHead().features.tessellationShader);
	assert(vulkan11Features.multiviewGeometryShader);
	assert(vulkan11Features.multiviewTessellationShader);
	assert(imagelessFramebufferFeatures.imagelessFramebuffer);
	assert(testUniformBufferStandardLayoutFeatures.uniformBufferStandardLayout);
	assert(testVulkanMemoryModelFeatures.vulkanMemoryModel);
	assert(testVulkanMemoryModelFeatures.vulkanMemoryModelDeviceScope);
	assert(testMeshShaderFeatures.meshShader);
	assert(testMeshShaderFeatures.taskShader);
	assert(testDeviceGeneratedCommandsFeatures.deviceGeneratedCommands);
}

#undef vgs