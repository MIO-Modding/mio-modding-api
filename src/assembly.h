#include <modding_api.h>
extern "C" {
	//Allowing an assembly method to be called by c++
	//(it will warn it cant be found, idk how to fix that, it just works though)
	void test_print();

	//Creating a c++ method to be called by assembly
	void AssemblyPrint(char* text) {
		LogMessage(text);
	}
}