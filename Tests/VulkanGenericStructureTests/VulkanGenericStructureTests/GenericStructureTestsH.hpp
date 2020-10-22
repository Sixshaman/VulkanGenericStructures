#pragma once

class GenericStructureTestsH
{
public:
	GenericStructureTestsH();
	~GenericStructureTestsH();

public:
	void DoTests();

private:
	void TestGenericStructure();
	void TestStructureBlob();
	void TestGenericStructureChain();
	void TestStructureChainBlob();
};