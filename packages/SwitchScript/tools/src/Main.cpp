#include <qapplication.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <signal.h>
#include <vm_limits.h>
#include <vm.h>
#include "MainWindow.hpp"
#include "Compiler.hpp"

extern "C"
{
	uint32_t _VM_MilliSeconds(void);
	void _VM_State_Init(JoystickState* state);
}

static volatile bool g_Stop = false;

void signalHandler(int sig) {
	if (sig == SIGINT) {
		std::cout << "Stop!" << std::endl;
		g_Stop = true;
	}
}

int EmulatorMain(int argc, char** argv) {
	if (argc < 2)
		return -1;

	const char* scriptPath = argv[1];
	if (!std::filesystem::exists(scriptPath))
		return -2;

	auto scriptSize = std::filesystem::file_size(scriptPath);
	if (scriptSize > VMExeSize)
		return -3;

	std::ifstream fin(scriptPath, std::ios::binary);

	VM_Init();

	std::vector<uint8_t> code(scriptSize);
	fin.read((char*)code.data(), scriptSize);
	VM_StartLoadProgram();
	VM_LoadProgram(code.data(), (uint16_t)scriptSize, 0);
	VM_EndLoadProgram();

	JoystickState lastState;
	_VM_State_Init(&lastState);

	uint32_t lastTime = 0;

	VM_Start();

	signal(SIGINT, signalHandler);
	while (true) {
		if (g_Stop)
			VM_Stop();

		VM_Update();

		if (VM_IsTerminated())
			break;

		if (VM_WaitingForSignal()) {
			std::cout << "Input signal (int16_t): ";

			int16_t value;
			std::cin >> value;

			VM_Signal(value);
		}

		JoystickState* state = VM_State();

		if (memcmp(state, &lastState, sizeof(JoystickState)) != 0) {
			uint32_t currentTime = _VM_MilliSeconds();
			std::cout << "State changed after " << currentTime - lastTime << "ms at time " << currentTime << "ms: ";
			lastTime = currentTime;

			std::vector<const char*> pressedButtons;
			if (state->Button & ButtonValue_A)
				pressedButtons.push_back("A");
			if (state->Button & ButtonValue_B)
				pressedButtons.push_back("B");
			if (state->Button & ButtonValue_X)
				pressedButtons.push_back("X");
			if (state->Button & ButtonValue_Y)
				pressedButtons.push_back("Y");
			if (state->Button & ButtonValue_L)
				pressedButtons.push_back("L");
			if (state->Button & ButtonValue_R)
				pressedButtons.push_back("R");
			if (state->Button & ButtonValue_ZL)
				pressedButtons.push_back("ZL");
			if (state->Button & ButtonValue_ZR)
				pressedButtons.push_back("ZR");
			if (state->Button & ButtonValue_LClick)
				pressedButtons.push_back("SL");
			if (state->Button & ButtonValue_RClick)
				pressedButtons.push_back("SR");
			if (state->Button & ButtonValue_Start)
				pressedButtons.push_back("+");
			if (state->Button & ButtonValue_Select)
				pressedButtons.push_back("-");
			if (state->Button & ButtonValue_Home)
				pressedButtons.push_back("Home");
			if (state->Button & ButtonValue_Capture)
				pressedButtons.push_back("Capture");

			switch (state->DPad)
			{
			case DPadValue_Bottom:
				pressedButtons.push_back("Down");
				break;
			case DPadValue_BottomLeft:
				pressedButtons.push_back("LeftDown");
				break;
			case DPadValue_BottomRight:
				pressedButtons.push_back("RightDown");
				break;
			case DPadValue_Top:
				pressedButtons.push_back("Up");
				break;
			case DPadValue_TopLeft:
				pressedButtons.push_back("LeftUp");
				break;
			case DPadValue_TopRight:
				pressedButtons.push_back("RightUp");
				break;
			case DPadValue_Right:
				pressedButtons.push_back("Right");
				break;
			case DPadValue_Left:
				pressedButtons.push_back("Left");
				break;
			default:
				break;
			}

			for (const char* s : pressedButtons)
				std::cout << s << " ";

			std::cout << std::endl;

			memcpy(&lastState, state, sizeof(JoystickState));
		}
	}

	return 0;
}

int UploaderMain(int argc, char** argv)
{
	return 0;
}

int CompilerMain(int argc, char** argv)
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

int ClientMain(int argc, char** argv)
{
	QApplication app(argc, argv);

	MainWindow w;
	w.show();

	return app.exec();
}

int main(int argc, char** argv)
{
	ClientMain(argc, argv);
}