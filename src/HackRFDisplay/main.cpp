#include <string>
#include <iostream>
#include "hackrf.h"


extern "C" {
	int notifyMain();
}

int main(void)
{
	try 
	{
		initHackRf();

		notifyMain();

		closeHackRf();
	}
	catch (std::string e) 
	{
		std::cout << e;
	}

	return 0;
}

