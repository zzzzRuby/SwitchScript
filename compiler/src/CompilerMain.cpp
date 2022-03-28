#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>

bool CompileOld(std::ostream& log, const std::string& script, std::ostream& code);

int main(int argc, char** argv)
{
	if (argc <= 2)
		return -1;

	const char* inputPath = argv[1];
	const char* outputPath = argv[2];

	if (!std::filesystem::exists(inputPath))
		return -2;

	std::ifstream fin(inputPath);
	std::stringstream sout;
	std::string script((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());

	if (!CompileOld(std::cout, script, sout))
		return -3;

	std::string outBuffer = sout.str();
	std::ofstream fout(outputPath, std::ios::binary);
	fout.write(outBuffer.c_str(), outBuffer.size());

	return 0;
}