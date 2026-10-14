#include <common.h>

#include <App/app.h>

// This module basically just adds a main function to the static library and packs it into and executable

int main(int argc, char* argv[])
{
	// Skip first argument because it is just the program name
	std::vector<std::string_view> arguments;
	for (int i = 1; i < argc; i++) {
		arguments.push_back(argv[i]);
	}
	App::Run(arguments);
	return 0;
}