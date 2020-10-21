#pragma once

class GenericStructureTestsH
{
public:
	GenericStructureTestsH();
	~GenericStructureTestsH();

public:
	void DoTests();

private:
	void TestStructureBlob();
	void TestGenericStructureChain();
	void TestStructureChainBlob();
};