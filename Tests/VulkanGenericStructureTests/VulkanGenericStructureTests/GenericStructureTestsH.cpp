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
	TestStructureBlob();
	TestGenericStructureChain();
	TestStructureChainBlob();
}

void GenericStructureTestsH::TestStructureBlob()
{
	//Common tests
	VkInstanceCreateInfo               instanceCreateInfo;
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;

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
	meshShaderFeaturesBaseStruct.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	vgs::StructureBlob meshShaderFeatures(meshShaderFeaturesBaseStruct);

	VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV deviceGeneratedCommandsFeaturesBaseStruct;
	vgs::StructureBlob deviceGeneratedCommandsFeatures(deviceGeneratedCommandsFeaturesBaseStruct);

	vgs::GenericStructureChain<VkPhysicalDeviceFeatures2> physicalDeviceFeatures2Chain;
	physicalDeviceFeatures2Chain.AppendToChain(vulkan11Features);
	physicalDeviceFeatures2Chain.AppendToChain(imagelessFramebufferFeatures);
	physicalDeviceFeatures2Chain.AppendToChain(uniformBufferStandardLayoutFeatures);
	physicalDeviceFeatures2Chain.AppendToChain(vulkanMemoryModelFeatures);

	physicalDeviceFeatures2Chain.AppendToChainGeneric(meshShaderFeatures);
	physicalDeviceFeatures2Chain.AppendToChainGeneric(deviceGeneratedCommandsFeatures);

	auto& physicalDeviceFeatures2 = physicalDeviceFeatures2Chain.GetChainHead();

	assert(physicalDeviceFeatures2.sType             == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
	assert(vulkan11Features.sType                    == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES);
	assert(imagelessFramebufferFeatures.sType        == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES);
	assert(uniformBufferStandardLayoutFeatures.sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES);
	assert(vulkanMemoryModelFeatures.sType           == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES);

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

	vulkan11Features.sType             = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	imagelessFramebufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES;
	{
		//Test local variables
		VkPhysicalDeviceUniformBufferStandardLayoutFeatures uniformBufferStandardLayoutFeatures;
		VkPhysicalDeviceVulkanMemoryModelFeatures           vulkanMemoryModelFeatures;

		uniformBufferStandardLayoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES;
		vulkanMemoryModelFeatures.sType           = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES;

		//Test generic structures too
		VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeaturesBaseStruct;
		meshShaderFeaturesBaseStruct.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
		vgs::StructureBlob meshShaderFeatures(meshShaderFeaturesBaseStruct);

		VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV deviceGeneratedCommandsFeaturesBaseStruct;
		vgs::StructureBlob deviceGeneratedCommandsFeatures(deviceGeneratedCommandsFeaturesBaseStruct);

		physicalDeviceFeatures2Chain.AppendToChain(vulkan11Features);
		physicalDeviceFeatures2Chain.AppendToChain(imagelessFramebufferFeatures);
		physicalDeviceFeatures2Chain.AppendToChain(uniformBufferStandardLayoutFeatures);
		physicalDeviceFeatures2Chain.AppendToChain(vulkanMemoryModelFeatures);

		physicalDeviceFeatures2Chain.AppendToChainGeneric(meshShaderFeatures);
		physicalDeviceFeatures2Chain.AppendToChainGeneric(deviceGeneratedCommandsFeatures);

		auto& physicalDeviceFeatures2 = physicalDeviceFeatures2Chain.GetChainHead();
		physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

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