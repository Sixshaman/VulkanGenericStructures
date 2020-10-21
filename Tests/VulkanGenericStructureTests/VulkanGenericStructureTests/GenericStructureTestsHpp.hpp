#pragma once

class GenericStructureTestsHpp
{
public:
	GenericStructureTestsHpp();
	~GenericStructureTestsHpp();

public:
	void DoTests();

private:
	void TestStructureBlob();
	void TestGenericStructureChain();
	void TestStructureChainBlob();
};