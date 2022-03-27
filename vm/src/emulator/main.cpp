#include <fstream>
#include <filesystem>

#include <vm.h>
#include <vm/limits.h>

int main(int argc, char** argv)
{
	if (argc < 2)
		return -1;

	const char* scriptPath = argv[1];
	if (!std::filesystem::exists(scriptPath))
		return -2;

	auto scriptSize = std::filesystem::file_size(scriptPath);
	if (scriptSize > ExeSize)
		return -3;

	std::ifstream fin(scriptPath, std::ios::binary);

	uint8_t* code = VM_InitForLoad();
	fin.read((char*)code, scriptSize);

	VM_Start();
	while (true)
	{
		if (VM_IsTerminated())
			break;

		VM_Update();
	}

	return 0;
}