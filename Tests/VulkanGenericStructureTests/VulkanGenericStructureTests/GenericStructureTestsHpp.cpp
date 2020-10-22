#include "GenericStructureTestsHpp.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>

#define vgs vgs_hpp
#include "..\..\..\Include\VulkanGenericStructures.hpp"

GenericStructureTestsHpp::GenericStructureTestsHpp()
{
}

GenericStructureTestsHpp::~GenericStructureTestsHpp()
{
}

void GenericStructureTestsHpp::DoTests()
{
	TestGenericStructure();
	TestStructureBlob();
	TestGenericStructureChain();
	TestStructureChainBlob();
}

void GenericStructureTestsHpp::TestGenericStructure()
{
	//Common tests
	vk::InstanceCreateInfo               instanceCreateInfo;
	vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;

	instanceCreateInfo.pNext = &debugReportCallbackCreateInfo;

	vgs::GenericStruct genericInstanceCreateInfo(instanceCreateInfo);
	assert(genericInstanceCreateInfo.GetStructureData() == reinterpret_cast<std::byte*>(&instanceCreateInfo));
	assert(genericInstanceCreateInfo.GetSType()         == vk::StructureType::eInstanceCreateInfo);
	assert(genericInstanceCreateInfo.GetPNext()         == &debugReportCallbackCreateInfo);

	vgs::GenericStruct genericDebugReportCallbackCreateInfo(debugReportCallbackCreateInfo);
	assert(genericDebugReportCallbackCreateInfo.GetStructureData() == reinterpret_cast<std::byte*>(&debugReportCallbackCreateInfo));
	assert(genericDebugReportCallbackCreateInfo.GetSType()         == vk::StructureType::eDebugReportCallbackCreateInfoEXT);
	assert(genericInstanceCreateInfo.GetPNext()                    == genericDebugReportCallbackCreateInfo.GetStructureData());

	vk::InstanceCreateInfo&               instanceCreateInfo2            = genericInstanceCreateInfo.GetDataAs<vk::InstanceCreateInfo>();
	vk::DebugReportCallbackCreateInfoEXT& debugReportCallbackCreateInfo2 = genericDebugReportCallbackCreateInfo.GetDataAs<vk::DebugReportCallbackCreateInfoEXT>();
	
	assert(instanceCreateInfo2.pNext == &debugReportCallbackCreateInfo);
	assert(instanceCreateInfo.pNext  == &debugReportCallbackCreateInfo2);

	instanceCreateInfo2.enabledLayerCount = 1;
	assert(instanceCreateInfo.enabledLayerCount == 1);

	//Operator= test
	vgs::GenericStruct genericInstanceCreateInfo2 = instanceCreateInfo;
	assert(genericInstanceCreateInfo2.GetStructureData() == reinterpret_cast<std::byte*>(&instanceCreateInfo));
	assert(genericInstanceCreateInfo2.GetSType()         == vk::StructureType::eInstanceCreateInfo);
	assert(genericInstanceCreateInfo2.GetPNext()         == &debugReportCallbackCreateInfo);

	//Copy constructor test
	std::vector<vgs::GenericStruct> genericStructs;
	genericStructs.emplace_back(instanceCreateInfo);
	genericStructs.emplace_back(debugReportCallbackCreateInfo);

	assert(genericStructs[0].GetPNext()         == genericStructs[1].GetStructureData());
	assert(genericStructs[0].GetStructureData() == reinterpret_cast<std::byte*>(&instanceCreateInfo));
}

void GenericStructureTestsHpp::TestStructureBlob()
{
	//Common tests
	vk::InstanceCreateInfo               instanceCreateInfo;
	vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;

	instanceCreateInfo.pNext            = &debugReportCallbackCreateInfo;
	debugReportCallbackCreateInfo.pNext = nullptr;

	//Data should be different, and pNext should be nulled out
	vgs::StructureBlob instanceCreateInfoBlob(instanceCreateInfo);
	assert(instanceCreateInfoBlob.GetStructureData() != reinterpret_cast<std::byte*>(&instanceCreateInfo));
	assert(instanceCreateInfoBlob.GetSType()         == vk::StructureType::eInstanceCreateInfo);
	assert(instanceCreateInfoBlob.GetPNext()         == nullptr);

	vgs::StructureBlob debugReportCallbackCreateInfoBlob(debugReportCallbackCreateInfo);
	assert(debugReportCallbackCreateInfoBlob.GetStructureData() != reinterpret_cast<std::byte*>(&debugReportCallbackCreateInfo));
	assert(debugReportCallbackCreateInfoBlob.GetSType()         == vk::StructureType::eDebugReportCallbackCreateInfoEXT);
	assert(debugReportCallbackCreateInfoBlob.GetPNext()         == nullptr);

	//Changes in blob shouldn't change the original
	vk::InstanceCreateInfo& instanceCreateInfo2 = instanceCreateInfoBlob.GetDataAs<vk::InstanceCreateInfo>();

	instanceCreateInfo2.enabledExtensionCount = 3;
	assert(instanceCreateInfo.enabledExtensionCount != 3);

	instanceCreateInfo.enabledExtensionCount = 4;
	assert(instanceCreateInfo2.enabledExtensionCount != 4);

	//We should be able to safely delete all local variables after we've made a blob
	vgs::StructureBlob instanceCreateInfoBlob2;
	{
		vk::InstanceCreateInfo localInstanceCreateInfo;
		localInstanceCreateInfo.enabledLayerCount     = 1;
		localInstanceCreateInfo.enabledExtensionCount = 2;

		vk::DebugReportCallbackCreateInfoEXT localDebugReportCallbackCreateInfo;
		localInstanceCreateInfo.pNext = &localDebugReportCallbackCreateInfo;

		instanceCreateInfoBlob2 = vgs::StructureBlob(localInstanceCreateInfo);
	}

	//Assignment operator should zero out pNext pointer too
	vk::InstanceCreateInfo& copiedInstanceCreateInfo = instanceCreateInfoBlob2.GetDataAs<vk::InstanceCreateInfo>();
	assert(copiedInstanceCreateInfo.enabledLayerCount     == 1);
	assert(copiedInstanceCreateInfo.enabledExtensionCount == 2);
	assert(copiedInstanceCreateInfo.sType                 == vk::StructureType::eInstanceCreateInfo);
	assert(copiedInstanceCreateInfo.pNext                 == nullptr);

	//Emplace test
	std::vector<vgs::StructureBlob> genericStructs;
	genericStructs.emplace_back(instanceCreateInfo);
	genericStructs.emplace_back(debugReportCallbackCreateInfo);

	//PNext should be zeroed out
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

void GenericStructureTestsHpp::TestGenericStructureChain()
{
	//Base test
	vk::PhysicalDeviceVulkan11Features                    vulkan11Features;
	vk::PhysicalDeviceImagelessFramebufferFeatures        imagelessFramebufferFeatures;
	vk::PhysicalDeviceUniformBufferStandardLayoutFeatures uniformBufferStandardLayoutFeatures;
	vk::PhysicalDeviceVulkanMemoryModelFeatures           vulkanMemoryModelFeatures;

	//Test generic structures too
	vk::PhysicalDeviceMeshShaderFeaturesNV meshShaderFeaturesBaseStruct;
	vgs::StructureBlob meshShaderFeatures(meshShaderFeaturesBaseStruct);

	vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV deviceGeneratedCommandsFeaturesBaseStruct;
	vgs::GenericStruct deviceGeneratedCommandsFeatures(deviceGeneratedCommandsFeaturesBaseStruct);

	vgs::GenericStructureChain<vk::PhysicalDeviceFeatures2> physicalDeviceFeatures2Chain;
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
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceVulkan11Features>()                    == &vulkan11Features);
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceImagelessFramebufferFeatures>()        == &imagelessFramebufferFeatures);
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceUniformBufferStandardLayoutFeatures>() == &uniformBufferStandardLayoutFeatures);
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceVulkanMemoryModelFeatures>()           == &vulkanMemoryModelFeatures);
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceMeshShaderFeaturesNV>()                == reinterpret_cast<vk::PhysicalDeviceMeshShaderFeaturesNV*>(meshShaderFeatures.GetStructureData()));
	assert(&physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV>()   == reinterpret_cast<vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV*>(deviceGeneratedCommandsFeatures.GetStructureData()));

	//Test structure filling
	vk::BaseOutStructure* currStruct = reinterpret_cast<vk::BaseOutStructure*>(&physicalDeviceFeatures2);
	while(currStruct != nullptr)
	{
		switch(currStruct->sType)
		{
		case vk::StructureType::ePhysicalDeviceFeatures2:
			((vk::PhysicalDeviceFeatures2*)(currStruct))->features.geometryShader     = true;
			((vk::PhysicalDeviceFeatures2*)(currStruct))->features.tessellationShader = true;
			break;

		case vk::StructureType::ePhysicalDeviceVulkan11Features:
			((vk::PhysicalDeviceVulkan11Features*)(currStruct))->multiviewGeometryShader     = true;
			((vk::PhysicalDeviceVulkan11Features*)(currStruct))->multiviewTessellationShader = true;
			break;

		case vk::StructureType::ePhysicalDeviceImagelessFramebufferFeatures:
			((vk::PhysicalDeviceImagelessFramebufferFeatures*)(currStruct))->imagelessFramebuffer = true;
			break;

		case vk::StructureType::ePhysicalDeviceUniformBufferStandardLayoutFeatures:
			((vk::PhysicalDeviceUniformBufferStandardLayoutFeatures*)(currStruct))->uniformBufferStandardLayout = true;
			break;

		case vk::StructureType::ePhysicalDeviceVulkanMemoryModelFeatures:
			((vk::PhysicalDeviceVulkanMemoryModelFeatures*)(currStruct))->vulkanMemoryModel            = true;
			((vk::PhysicalDeviceVulkanMemoryModelFeatures*)(currStruct))->vulkanMemoryModelDeviceScope = true;
			break;

		case vk::StructureType::ePhysicalDeviceMeshShaderFeaturesNV:
			((vk::PhysicalDeviceMeshShaderFeaturesNV*)(currStruct))->meshShader = true;
			((vk::PhysicalDeviceMeshShaderFeaturesNV*)(currStruct))->taskShader = true;
			break;

		case vk::StructureType::ePhysicalDeviceDeviceGeneratedCommandsFeaturesNV:
			((vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV*)(currStruct))->deviceGeneratedCommands = true;
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
	assert(meshShaderFeatures.GetDataAs<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV>().deviceGeneratedCommands);
}

void GenericStructureTestsHpp::TestStructureChainBlob()
{
	//Base test
	vgs::StructureChainBlob<vk::PhysicalDeviceFeatures2> physicalDeviceFeatures2Chain;

	vk::PhysicalDeviceVulkan11Features             vulkan11Features;
	vk::PhysicalDeviceImagelessFramebufferFeatures imagelessFramebufferFeatures;

	{
		//Test local variables
		vk::PhysicalDeviceUniformBufferStandardLayoutFeatures uniformBufferStandardLayoutFeatures;
		vk::PhysicalDeviceVulkanMemoryModelFeatures           vulkanMemoryModelFeatures;

		//Test generic structures too
		vk::PhysicalDeviceMeshShaderFeaturesNV meshShaderFeaturesBaseStruct;
		vgs::StructureBlob meshShaderFeatures(meshShaderFeaturesBaseStruct);

		vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV deviceGeneratedCommandsFeaturesBaseStruct;
		vgs::GenericStruct deviceGeneratedCommandsFeatures(deviceGeneratedCommandsFeaturesBaseStruct);

		physicalDeviceFeatures2Chain.AppendToChain(vulkan11Features);
		physicalDeviceFeatures2Chain.AppendToChain(imagelessFramebufferFeatures);
		physicalDeviceFeatures2Chain.AppendToChain(uniformBufferStandardLayoutFeatures);
		physicalDeviceFeatures2Chain.AppendToChain(vulkanMemoryModelFeatures);

		physicalDeviceFeatures2Chain.AppendToChainGeneric(meshShaderFeatures);
		physicalDeviceFeatures2Chain.AppendToChainGeneric(deviceGeneratedCommandsFeatures);

		auto& physicalDeviceFeatures2 = physicalDeviceFeatures2Chain.GetChainHead();

		//Test structure filling
		vk::BaseOutStructure* currStruct = reinterpret_cast<vk::BaseOutStructure*>(&physicalDeviceFeatures2);
		while(currStruct != nullptr)
		{
			switch(currStruct->sType)
			{
			case vk::StructureType::ePhysicalDeviceFeatures2:
				((vk::PhysicalDeviceFeatures2*)(currStruct))->features.geometryShader     = true;
				((vk::PhysicalDeviceFeatures2*)(currStruct))->features.tessellationShader = true;
				break;

			case vk::StructureType::ePhysicalDeviceVulkan11Features:
				((vk::PhysicalDeviceVulkan11Features*)(currStruct))->multiviewGeometryShader     = true;
				((vk::PhysicalDeviceVulkan11Features*)(currStruct))->multiviewTessellationShader = true;
				break;

			case vk::StructureType::ePhysicalDeviceImagelessFramebufferFeatures:
				((vk::PhysicalDeviceImagelessFramebufferFeatures*)(currStruct))->imagelessFramebuffer = true;
				break;

			case vk::StructureType::ePhysicalDeviceUniformBufferStandardLayoutFeatures:
				((vk::PhysicalDeviceUniformBufferStandardLayoutFeatures*)(currStruct))->uniformBufferStandardLayout = true;
				break;

			case vk::StructureType::ePhysicalDeviceVulkanMemoryModelFeatures:
				((vk::PhysicalDeviceVulkanMemoryModelFeatures*)(currStruct))->vulkanMemoryModel            = true;
				((vk::PhysicalDeviceVulkanMemoryModelFeatures*)(currStruct))->vulkanMemoryModelDeviceScope = true;
				break;

			case vk::StructureType::ePhysicalDeviceMeshShaderFeaturesNV:
				((vk::PhysicalDeviceMeshShaderFeaturesNV*)(currStruct))->meshShader = true;
				((vk::PhysicalDeviceMeshShaderFeaturesNV*)(currStruct))->taskShader = true;
				break;

			case vk::StructureType::ePhysicalDeviceDeviceGeneratedCommandsFeaturesNV:
				((vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV*)(currStruct))->deviceGeneratedCommands = true;
				break;

			default:
				break;
			}

			currStruct = currStruct->pNext;
		}
	}

	//Test reassignment too
	vulkan11Features             = physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceVulkan11Features>();
	imagelessFramebufferFeatures = physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceImagelessFramebufferFeatures>();

	vk::PhysicalDeviceFeatures2&                           testPhysicalDeviceFeatures2             = physicalDeviceFeatures2Chain.GetChainHead();
	vk::PhysicalDeviceUniformBufferStandardLayoutFeatures& testUniformBufferStandardLayoutFeatures = physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceUniformBufferStandardLayoutFeatures>();
	vk::PhysicalDeviceVulkanMemoryModelFeatures&           testVulkanMemoryModelFeatures           = physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceVulkanMemoryModelFeatures>();
	vk::PhysicalDeviceMeshShaderFeaturesNV&                testMeshShaderFeatures                  = physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceMeshShaderFeaturesNV>();
	vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV&   testDeviceGeneratedCommandsFeatures     = physicalDeviceFeatures2Chain.GetChainLinkDataAs<vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV>();

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