# MIT License
#
# Copyright (c) 2020 Sixshaman
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import urllib.request
import re
from bs4 import BeautifulSoup

header_license = """\
/********************************************************************************
MIT License

Copyright (c) 2020 Sixshaman

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
********************************************************************************/

"""

header_start_h = """\
#ifndef VULKAN_GENERIC_STRUCTURES_H
#define VULKAN_GENERIC_STRUCTURES_H

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <cassert>

namespace vgs
{

using VulkanStructureType = VkStructureType;

template<typename VkStruct>
constexpr VulkanStructureType ValidStructureType = VK_STRUCTURE_TYPE_MAX_ENUM;
"""

header_start_hpp = """\
#ifndef VULKAN_GENERIC_STRUCTURES_HPP
#define VULKAN_GENERIC_STRUCTURES_HPP

#include <vulkan/vulkan.hpp>
#include <vector>
#include <unordered_map>
#include <cassert>

namespace vgs
{

using VulkanStructureType = vk::StructureType;

template<typename VkStruct>
constexpr VulkanStructureType ValidStructureType = VkStruct::structureType;
"""

header_stype_init_h = """\
inline void InitSType(void* ptr, ptrdiff_t offset, VulkanStructureType value)
{
	memcpy((std::byte*)ptr + offset, &value, sizeof(VulkanStructureType));
}
"""

header_stype_init_hpp = """\
inline void InitSType(void* ptr, ptrdiff_t offset, VulkanStructureType value)
{
}
"""

header_end = """\

//==========================================================================================================================

//Base interface for StructureBlob(owning type-erased structure) and GenericStructureView(non-owning type-erased structure)
class GenericStructBase
{
protected:
	GenericStructBase();
	GenericStructBase(std::byte* data, size_t dataSize, ptrdiff_t pNextOffset, ptrdiff_t sTypeOffset);
	~GenericStructBase();

public:
	template<typename Struct>
	Struct& GetDataAs();

	std::byte* GetStructureData() const;
	size_t     GetStructureSize() const;

	ptrdiff_t GetPNextOffset() const;
	void*     GetPNext()       const;

	ptrdiff_t           GetSTypeOffset() const;
	VulkanStructureType GetSType()       const;

protected:
	std::byte* StructureData;
	size_t     StructureSize;

	ptrdiff_t PNextPointerOffset;
	ptrdiff_t STypeOffset;
};

inline GenericStructBase::GenericStructBase(): StructureData(nullptr), StructureSize(0), PNextPointerOffset(0), STypeOffset(0)
{
}

inline GenericStructBase::GenericStructBase(std::byte* data, size_t dataSize, ptrdiff_t pNextOffset, ptrdiff_t sTypeOffset): StructureData(data), StructureSize(dataSize), PNextPointerOffset(pNextOffset), STypeOffset(sTypeOffset)
{
}

inline GenericStructBase::~GenericStructBase()
{
}

template<typename Struct>
inline Struct& GenericStructBase::GetDataAs()
{
	assert(GetStructureSize() == sizeof(Struct));
	assert(StructureData != nullptr);

	Struct* structureData = reinterpret_cast<Struct*>(StructureData);
	return *structureData;
}

inline std::byte* GenericStructBase::GetStructureData() const
{
	assert(GetStructureSize() != 0);

	return StructureData;
}

inline size_t GenericStructBase::GetStructureSize() const
{
	return StructureSize;
}

inline ptrdiff_t GenericStructBase::GetPNextOffset() const
{
	return PNextPointerOffset;
}

inline void* GenericStructBase::GetPNext() const
{
	assert(PNextPointerOffset + sizeof(void*) <= GetStructureSize());

	void* pNext = nullptr;
	memcpy(&pNext, StructureData + PNextPointerOffset, sizeof(void*));

	return pNext;
}

inline ptrdiff_t GenericStructBase::GetSTypeOffset() const
{
	return STypeOffset;
}

inline VulkanStructureType GenericStructBase::GetSType() const
{
	assert(STypeOffset + sizeof(VulkanStructureType) <= GetStructureSize());

	VulkanStructureType sType;
	memcpy(&sType, StructureData + STypeOffset, sizeof(VulkanStructureType));

	return sType;
}

//==========================================================================================================================

//Non-owning version of a generic structure
class GenericStruct: public GenericStructBase
{
public:
	template<typename Struct>
	GenericStruct(Struct& structure);

	//The copy constructor should be template-specialized, because <const GenericStructureView&> can be passed as a <Struct&>
	template<>
	GenericStruct(const GenericStruct& right);
	GenericStruct& operator=(const GenericStruct& right);
};

template<typename Struct>
inline GenericStruct::GenericStruct(Struct& structure): GenericStructBase(reinterpret_cast<std::byte*>(&structure), sizeof(Struct), offsetof(Struct, pNext), offsetof(Struct, sType))
{
}

template<>
inline GenericStruct::GenericStruct(const GenericStruct& right): GenericStructBase(right.StructureData, right.StructureSize, right.PNextPointerOffset, right.STypeOffset)
{
}

inline GenericStruct& GenericStruct::operator=(const GenericStruct& right)
{
	StructureData      = right.StructureData;
	StructureSize      = right.StructureSize;
	STypeOffset        = right.STypeOffset;
	PNextPointerOffset = right.PNextPointerOffset;

	return *this;
}

//Creates a GenericStruct and automatically fills in sType (a side effect which is undesireable in constructors)
template<typename Struct>
inline GenericStruct TransmuteTypeToSType(Struct& structure)
{
	InitSType(&structure, offsetof(Struct, sType), ValidStructureType<Struct>);
	return GenericStruct(structure);
}

//==========================================================================================================================

//Owning version of a generic structure
class StructureBlob: public GenericStructBase
{
public:
	StructureBlob();

	StructureBlob(const StructureBlob& right);
	StructureBlob& operator=(const StructureBlob& right);

	template<typename Struct>
	StructureBlob(const Struct& structure);

private:
	std::vector<std::byte> StructureBlobData;
};

inline StructureBlob::StructureBlob()
{
	PNextPointerOffset = 0;
	STypeOffset        = 0;

	StructureData = nullptr;
	StructureSize = 0;
}

template<typename Struct>
inline StructureBlob::StructureBlob(const Struct& structure)
{
	static_assert(std::is_trivially_destructible<Struct>::value, "Structure blob contents must be trivially destructible");

	PNextPointerOffset = offsetof(Struct, pNext);
	STypeOffset        = offsetof(Struct, sType);

	StructureBlobData.resize(sizeof(Struct));
	memcpy(StructureBlobData.data(), &structure, sizeof(Struct));

	StructureData = StructureBlobData.data();
	StructureSize = StructureBlobData.size();

	//Init sType and set pNext to null
	VulkanStructureType structureType = ValidStructureType<Struct>;
	InitSType(StructureBlobData.data(), STypeOffset, structureType);

	void* nullPNext = nullptr;
	memcpy(StructureBlobData.data() + PNextPointerOffset, &nullPNext, sizeof(void*));
}

inline StructureBlob::StructureBlob(const StructureBlob& right)
{
	*this = right;
}

inline StructureBlob& StructureBlob::operator=(const StructureBlob& right)
{
	StructureBlobData.assign(right.StructureBlobData.begin(), right.StructureBlobData.end());

	StructureData = StructureBlobData.data();
	StructureSize = StructureBlobData.size();

	STypeOffset        = right.STypeOffset;
	PNextPointerOffset = right.PNextPointerOffset;

	assert(PNextPointerOffset + sizeof(void*) <= StructureBlobData.size());

	//Zero out PNext
	memset(StructureBlobData.data() + PNextPointerOffset, 0, sizeof(void*));

	return *this;
}

//==========================================================================================================================

//Base class for a generic structure chain, hiding chain link type info
template<typename HeadType>
class GenericStructureChainBase
{
protected:
	GenericStructureChainBase();
	~GenericStructureChainBase();

public:
	HeadType& GetChainHead();

	template<typename Struct>
	Struct& GetChainLinkDataAs();

public:
	GenericStructureChainBase(const GenericStructureChainBase& rhs) = delete;
	GenericStructureChainBase& operator=(const GenericStructureChainBase& rhs) = delete;

protected:
	std::vector<std::byte*> StructureDataPointers;
	std::vector<ptrdiff_t>  PNextPointerOffsets;
	std::vector<ptrdiff_t>  STypeOffsets;

	std::unordered_map<VulkanStructureType, size_t> StructureDataIndices;
};

template<typename HeadType>
inline GenericStructureChainBase<HeadType>::GenericStructureChainBase()
{
}

template<typename HeadType>
inline GenericStructureChainBase<HeadType>::~GenericStructureChainBase()
{
}

template<typename HeadType>
inline HeadType& GenericStructureChainBase<HeadType>::GetChainHead()
{
	assert(StructureDataPointers.size() > 0);
	assert(StructureDataPointers[0] != nullptr);

	HeadType* head = reinterpret_cast<HeadType*>(StructureDataPointers[0]);
	return *head;
}

template<typename HeadType>
template<typename Struct>
inline Struct& GenericStructureChainBase<HeadType>::GetChainLinkDataAs()
{
	Struct* structPtr = reinterpret_cast<Struct*>(StructureDataPointers[StructureDataIndices.at(ValidStructureType<Struct>)]);
	return *structPtr;
}

//==========================================================================================================================

//Generic structure chain, non-owning version
template<typename HeadType>
class GenericStructureChain: public GenericStructureChainBase<HeadType>
{
	using GenericStructureChainBase<HeadType>::StructureDataPointers;
	using GenericStructureChainBase<HeadType>::STypeOffsets;
	using GenericStructureChainBase<HeadType>::PNextPointerOffsets;
	using GenericStructureChainBase<HeadType>::StructureDataIndices;

public:
	GenericStructureChain();
	GenericStructureChain(HeadType& head);
	~GenericStructureChain();
	
	//Clears everything EXCEPT head
	void Clear();

	template<typename Struct>
	void AppendToChain(Struct& next);

	void AppendToChainGeneric(GenericStructBase& nextBlobData);

public:
	GenericStructureChain(const GenericStructureChain& rhs) = delete;
	GenericStructureChain& operator=(const GenericStructureChain& rhs) = delete;

private:
	void AppendDataToChain(void* dataPtr, size_t sTypeOffset, size_t pNextOffset, VulkanStructureType sType);

protected:
	HeadType HeadData;
};

template<typename HeadType>
inline GenericStructureChain<HeadType>::GenericStructureChain()
{
	//Init HeadData's sType and pNext
	InitSType(&HeadData, offsetof(HeadData, sType), ValidStructureType<HeadType>);
	HeadData.pNext = nullptr;

	StructureDataPointers.push_back(reinterpret_cast<std::byte*>(&HeadData));
	STypeOffsets.push_back(offsetof(HeadType, sType));
	PNextPointerOffsets.push_back(offsetof(HeadType, pNext));

	StructureDataIndices[ValidStructureType<HeadType>] = 0;
}

template<typename HeadType>
inline GenericStructureChain<HeadType>::GenericStructureChain(HeadType& head)
{
	HeadData = head;

	InitSType(&HeadData, offsetof(HeadData, sType), ValidStructureType<HeadType>);
	HeadData.pNext = nullptr;

	//Head is always the first pointer
	StructureDataPointers.push_back(reinterpret_cast<std::byte*>(&HeadData));
	STypeOffsets.push_back(offsetof(HeadType, sType));
	PNextPointerOffsets.push_back(offsetof(HeadType, pNext));

	StructureDataIndices[ValidStructureType<HeadType>] = 0;
}

template<typename HeadType>
inline GenericStructureChain<HeadType>::~GenericStructureChain()
{
}

template<typename HeadType>
inline void GenericStructureChain<HeadType>::Clear()
{
	//Just reset the pointers
	StructureDataPointers.clear();
	PNextPointerOffsets.clear();

	StructureDataIndices.clear();

	HeadData.pNext = nullptr;

	StructureDataPointers.push_back(&HeadData);
	STypeOffsets.push_back(offsetof(HeadType, sType));
	PNextPointerOffsets.push_back(offsetof(HeadType, pNext));

	StructureDataIndices[ValidStructureType<HeadType>] = 0;
}

template<typename HeadType>
template<typename Struct>
inline void GenericStructureChain<HeadType>::AppendToChain(Struct& next)
{
	AppendDataToChain(&next, offsetof(Struct, sType), offsetof(Struct, pNext), ValidStructureType<Struct>);
}

template<typename HeadType>
inline void GenericStructureChain<HeadType>::AppendToChainGeneric(GenericStructBase& nextBlobData)
{
	AppendDataToChain(nextBlobData.GetStructureData(), nextBlobData.GetSTypeOffset(), nextBlobData.GetPNextOffset(), nextBlobData.GetSType());
}

template<typename HeadType>
inline void GenericStructureChain<HeadType>::AppendDataToChain(void* dataPtr, size_t sTypeOffset, size_t pNextOffset, VulkanStructureType sType)
{
	std::byte* prevLastStruct  = StructureDataPointers.back();
	ptrdiff_t  prevPNextOffset = PNextPointerOffsets.back();

	StructureDataPointers.push_back(reinterpret_cast<std::byte*>(dataPtr));
	STypeOffsets.push_back(sTypeOffset);
	PNextPointerOffsets.push_back(pNextOffset);

	std::byte* currLastStructPtr = StructureDataPointers.back();
	InitSType(dataPtr, sTypeOffset, sType);                                           //Set sType of the current struct
	memcpy(prevLastStruct + prevPNextOffset, &currLastStructPtr, sizeof(std::byte*)); //Set pNext pointer of the previous struct

	StructureDataIndices[sType] = StructureDataPointers.size() - 1;
}

//==========================================================================================================================

//Generic structure chain, owning version
template<typename HeadType> 
class StructureChainBlob: public GenericStructureChainBase<HeadType>
{
	using GenericStructureChainBase<HeadType>::StructureDataPointers;
	using GenericStructureChainBase<HeadType>::STypeOffsets;
	using GenericStructureChainBase<HeadType>::PNextPointerOffsets;
	using GenericStructureChainBase<HeadType>::StructureDataIndices;

public:
	StructureChainBlob();
	StructureChainBlob(const HeadType& head);
	~StructureChainBlob();
	   
	//Clears everything EXCEPT head
	void Clear();

	template<typename Struct>
	void AppendToChain(const Struct& next);

	void AppendToChainGeneric(const GenericStructBase& nextBlobData);

public:
	StructureChainBlob(const StructureChainBlob& rhs)            = delete;
	StructureChainBlob& operator=(const StructureChainBlob& rhs) = delete;

private:
	void AppendDataToBlob(const std::byte* data, size_t dataSize, const void* dataPNext, ptrdiff_t dataSTypeOffset, ptrdiff_t dataPNextOffset, VulkanStructureType sType);

private:
	std::vector<std::byte> StructureChainBlobData;
};

template<typename HeadType>
inline StructureChainBlob<HeadType>::StructureChainBlob()
{
	static_assert(std::is_trivially_destructible<HeadType>::value, "All members of the structure chain blob must be trivially destructible");

	//Store head in the blob
	StructureChainBlobData.resize(sizeof(HeadType));

	HeadType head;
	InitSType(&head, offsetof(HeadType, sType), ValidStructureType<HeadType>);
	head.pNext = nullptr;

	memcpy(StructureChainBlobData.data(), &head, sizeof(HeadType));

	StructureDataPointers.push_back(StructureChainBlobData.data());
	STypeOffsets.push_back(offsetof(HeadType, sType));
	PNextPointerOffsets.push_back(offsetof(HeadType, pNext));

	StructureDataIndices[ValidStructureType<HeadType>] = 0;
}

template<typename HeadType>
inline StructureChainBlob<HeadType>::StructureChainBlob(const HeadType& head)
{
	static_assert(std::is_trivially_destructible<HeadType>::value, "All members of the structure chain blob must be trivially destructible");

	//Store head in the blob
	StructureChainBlobData.resize(sizeof(HeadType));

	memcpy(StructureChainBlobData.data(), &head, sizeof(HeadType));

	StructureDataPointers.push_back(StructureChainBlobData.data());
	STypeOffsets.push_back(offsetof(HeadType, sType));
	PNextPointerOffsets.push_back(offsetof(HeadType, pNext));

	VulkanStructureType headSType = ValidStructureType<HeadType>;
	void*               headPNext = nullptr;

	InitSType(StructureDataPointers.back(), STypeOffsets.back(), headSType);
	memcpy(StructureDataPointers.back() + PNextPointerOffsets.back(), &headPNext, sizeof(void*));

	StructureDataIndices[ValidStructureType<HeadType>] = 0;
}

template<typename HeadType>
inline StructureChainBlob<HeadType>::~StructureChainBlob()
{
}

template<typename HeadType>
inline void StructureChainBlob<HeadType>::Clear()
{
	//Save the head
	HeadType& headData = GenericStructureChainBase<HeadType>::GetChainHead();
	std::vector<std::byte> oldHead(sizeof(HeadType));
	memcpy(oldHead.data(), &headData, sizeof(HeadType));

	//Clear everything
	StructureChainBlobData.clear();

	StructureDataPointers.clear();
	STypeOffsets.clear();
	PNextPointerOffsets.clear();

	StructureDataIndices.clear();

	//Reinit 
	StructureChainBlobData.resize(sizeof(HeadType));

	memcpy(StructureChainBlobData.data(), oldHead.data(), sizeof(HeadType));

	StructureDataPointers.push_back(StructureChainBlobData.data());
	STypeOffsets.push_back(offsetof(HeadType, sType));
	PNextPointerOffsets.push_back(offsetof(HeadType, pNext));

	StructureDataIndices[ValidStructureType<HeadType>] = 0;
}

template<typename HeadType>
template<typename Struct>
inline void StructureChainBlob<HeadType>::AppendToChain(const Struct& next)
{
	static_assert(std::is_trivially_destructible<Struct>::value, "All members of the structure chain blob must be trivially destructible");

	AppendDataToBlob((const std::byte*)(&next), sizeof(Struct), next.pNext, offsetof(Struct, sType), offsetof(Struct, pNext), ValidStructureType<Struct>);
}

template<typename HeadType>
inline void StructureChainBlob<HeadType>::AppendToChainGeneric(const GenericStructBase& nextBlobData)
{
	AppendDataToBlob(nextBlobData.GetStructureData(), nextBlobData.GetStructureSize(), nextBlobData.GetPNext(), nextBlobData.GetSTypeOffset(), nextBlobData.GetPNextOffset(), nextBlobData.GetSType());
}

template<typename HeadType>
inline void StructureChainBlob<HeadType>::AppendDataToBlob(const std::byte* data, size_t dataSize, const void* dataPNext, ptrdiff_t dataSTypeOffset, ptrdiff_t dataPNextOffset, VulkanStructureType sType)
{
	size_t prevDataSize   = StructureChainBlobData.size();
	size_t nextDataOffset = prevDataSize;

	//Copy all current structures to the new chain, and append new structure
	std::vector<std::byte> newStructureChainData(prevDataSize + dataSize);
	memcpy(newStructureChainData.data(),                StructureChainBlobData.data(), prevDataSize);
	memcpy(newStructureChainData.data() + prevDataSize, data,                          dataSize);

	//Initialize sType
	InitSType(newStructureChainData.data() + prevDataSize, dataSTypeOffset, sType);

	//Rebuild StructureDataPointers
	std::vector<ptrdiff_t> structureDataOffsets(StructureDataPointers.size());
	for(size_t i = 0; i < StructureDataPointers.size(); i++)
	{
	    structureDataOffsets[i] = (StructureDataPointers[i] - &StructureChainBlobData[0]);
	}

	StructureDataPointers.clear();
	for(size_t i = 0; i < structureDataOffsets.size(); i++)
	{
	    StructureDataPointers.push_back(newStructureChainData.data() + structureDataOffsets[i]);
	}

	StructureDataPointers.push_back(newStructureChainData.data() + nextDataOffset);
	STypeOffsets.push_back(dataSTypeOffset);
	PNextPointerOffsets.push_back(dataPNextOffset);

	//Invalidate pNext pointers
	for(size_t i = 0; i < PNextPointerOffsets.size() - 1; i++)
	{
	    void** currPPNext = (void**)(StructureDataPointers[i] + PNextPointerOffsets[i]);
	    memcpy(currPPNext, &StructureDataPointers[i + 1], sizeof(void*));
	}

	//Invalidate the last pNext pointer with the provided one
	std::byte* pLastStruct = StructureDataPointers.back();
	memcpy(pLastStruct + dataPNextOffset, &dataPNext, sizeof(void*));

	//Only use move semantics, because copy semantics will make pNext pointers invalid once again
	StructureChainBlobData = std::move(newStructureChainData);

	//Make sure all pNext point to inside of StructureChainBlobData. The last pointer can point to whatever the user specified
	for(size_t i = 0; i < PNextPointerOffsets.size() - 1; i++)
	{
	    void* pNextPointer = nullptr;
	    memcpy(&pNextPointer, StructureDataPointers[i] + PNextPointerOffsets[i], sizeof(void*)); //Init the pointer data

	    assert(pNextPointer >= &StructureChainBlobData[0] && pNextPointer < (&StructureChainBlobData[0] + StructureChainBlobData.size())); //Move semantics should never break pNext pointers, they should always point to inside the blob
	}

	StructureDataIndices[sType] = StructureDataPointers.size() - 1;
}

}

#endif
"""

def open_vk_spec(url):
	with urllib.request.urlopen(url) as response:
	    spec_data = response.read()
	    return spec_data.decode("utf8")

def parse_stypes(spec_contents):
	spec_soup   = BeautifulSoup(spec_contents, features="xml")

	spec_platform_defines = {}

	spec_platforms_block = spec_soup.find("platforms")
	if spec_platforms_block is not None:
	    spec_platform_tags = spec_platforms_block.find_all("platform")
	    for platform_tag in spec_platform_tags:
	        spec_platform_defines[platform_tag["name"]] = platform_tag["protect"]

	spec_struct_extensions = {}

	extension_blocks = spec_soup.find_all("extension")
	for extension_block in extension_blocks:
	    extension_name = extension_block["name"]

	    extension_define_tag = extension_block.find("enum", {"value": re.compile(".*" + extension_name +".*")})
	    if extension_define_tag is None:
	        continue

	    extension_define = extension_define_tag["name"]
	    if extension_define is None:
	        continue
	    
	    extension_types = []
	    extension_type_tags = extension_block.find_all("type")
	    for type_tag in extension_type_tags:
	        extension_defined_type = type_tag["name"]
	        if extension_defined_type is not None:
	            extension_types.append(extension_defined_type)

	    extension_platform = ""
	    if "platform" in extension_block.attrs:
	        extension_platform = extension_block["platform"]

	    extension_platform_define = ""
	    if extension_platform in spec_platform_defines:
	        extension_platform_define = spec_platform_defines[extension_platform]

	    for extension_type in extension_types:
	        spec_struct_extensions[extension_type] = (extension_name, extension_define, extension_platform_define)

	stypes = []

	struct_blocks = spec_soup.find_all("type", {"category": "struct"})
	for struct_block in struct_blocks:
	    struct_type = struct_block["name"]

	    #Find only structs that have <member> tag with "values" attibute
	    struct_stype_member_tags = [member_tag for member_tag in struct_block.find_all("member") if "values" in member_tag.attrs]
	    if len(struct_stype_member_tags) == 0:
	        continue

	    stype = ""
	    for member_tag in struct_stype_member_tags:
	        name_tag = member_tag.find("name")
	        if name_tag is not None and name_tag.string == "sType":
	            stype = member_tag["values"]
	            break

	    extension_define = ""
	    platform_define  = ""

	    if struct_type in spec_struct_extensions:
	        struct_requires = spec_struct_extensions[struct_type]

	        extension_define = struct_requires[1]
	        platform_define  = struct_requires[2]

	    stypes.append((struct_type, stype, extension_define, platform_define))

	    stypes = sorted(stypes, key=lambda struct_data: struct_data[2])
	    stypes = sorted(stypes, key=lambda struct_data: struct_data[3])

	return stypes

def compile_cpp_header_h(stypes):
	cpp_data = ""

	cpp_data += header_license
	cpp_data += header_start_h

	current_extension_define = ""
	current_platform_define  = ""
	tab_level                = ""
	for stype in stypes:
	    if current_extension_define != stype[2] or current_platform_define != stype[3]:
	        if current_extension_define != "" or current_platform_define != "":
	            cpp_data += "#endif\n"
	            tab_level = ""

	        if stype[2] != "" or stype[3] != "":
	            tab_level = "\t"

	            cpp_data += "\n#if "

	            if stype[2] != "":
	                cpp_data += "defined(" + stype[2] + ")"

	                if stype[3] != "":
	                    cpp_data += " && "

	            if stype[3] != "":
	                cpp_data += "defined(" + stype[3] + ")"

	        current_extension_define = stype[2]
	        current_platform_define  = stype[3]

	    cpp_data += "\n"
	    cpp_data += tab_level + "template<>\n"
	    cpp_data += tab_level + "constexpr VulkanStructureType ValidStructureType<" + stype[0] + "> = " + stype[1] + ";\n"

	if current_extension_define != "" or current_platform_define != "":
	    cpp_data += "#endif\n\n"

	cpp_data += header_stype_init_h
	cpp_data += header_end

	return cpp_data

def compile_cpp_header_hpp(stypes):
	cpp_data = ""

	cpp_data += header_license
	cpp_data += header_start_hpp
	cpp_data += header_stype_init_hpp
	cpp_data += header_end

	return cpp_data

def save_file(contents, filename):
	with open(filename, "w", encoding="utf-8") as out_file:
	    out_file.write(contents)

if __name__ == "__main__":
	spec_text = open_vk_spec("https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/main/xml/vk.xml") #Get it directly from the master branch
	stypes    = parse_stypes(spec_text)

	cpp_header_data_h   = compile_cpp_header_h(stypes)
	cpp_header_data_hpp = compile_cpp_header_hpp(stypes)

	save_file(cpp_header_data_h,   "./Include/VulkanGenericStructures.h")
	save_file(cpp_header_data_hpp, "./Include/VulkanGenericStructures.hpp")