#include "GenericStructureTestsH.hpp"
#include "GenericStructureTestsHpp.hpp"

int main(int argc, char* argv[])
{
	GenericStructureTestsH genStructureTestsH;
	genStructureTestsH.DoTests();

	GenericStructureTestsHpp genStructureTestsHpp;
	genStructureTestsHpp.DoTests();
}