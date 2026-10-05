#include <iostream>
#include <App/app.h>

// This module basically just adds a main function to the static library and packs it into and executable

int main(int argc, char* argv[]) {
	Application::Run(argc, argv);
	return 0;
}