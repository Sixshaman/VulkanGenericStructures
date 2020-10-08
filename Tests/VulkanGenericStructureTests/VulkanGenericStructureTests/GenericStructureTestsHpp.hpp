#pragma once

class GenericStructureTestsHpp
{
public:
	GenericStructureTestsHpp();
	~GenericStructureTestsHpp();

public:
	void DoTests();

private:
	void TestGenericStructure();
	void TestStructureBlob();
	void TestGenericStructureChain();
	void TestStructureChainBlob();
};