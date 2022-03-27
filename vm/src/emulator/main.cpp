#include <fstream>
#include <filesystem>
#include <signal.h>

#include <vm.h>
#include <vm_limits.h>

static volatile bool g_Stop = false;

void signalHandler(int sig) {
	if (sig == SIGINT) g_Stop = true;
}

int main(int argc, char** argv) {
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

	signal(SIGINT, signalHandler);
	while (true) {
		if (g_Stop)
			VM_Stop();

		VM_Update();

		if (VM_IsTerminated())
			break;
	}

	return 0;
}