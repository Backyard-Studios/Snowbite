#include <iostream>
#include <Test.h>

uint32_t TestFunction(const uint32_t a, const uint32_t b)
{
	std::cout << "TestFunction(" << a << ", " << b << ")" << '\n';
	return a + b;
}
