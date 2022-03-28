#pragma once

#include <ostream>
#include <array>
#include <variant>
#include <map>
#include <optional>

#include <vm.h>

class ByteCodeBuilder
{
private:
	std::array<bool, 256> m_UsedHeap;
	std::ostream& m_Code;
	std::streampos m_OriginPos;
	std::map<uint8_t, size_t> m_VariableMap;
public:
	ByteCodeBuilder(std::ostream& code);

	std::optional<uint8_t> NewVariable(uint8_t size);
	void DestroyVariable(uint8_t var);

	uint16_t PC();

	void NewTerminate();
	void NewSetButton(ButtonValue flags, std::optional<DPadValue> dpad = std::nullopt);
	void NewUnsetButton(ButtonValue flags, bool unsetDpad = false);
	void NewJumpIf(JumpMode mode, uint16_t target);
	void NewAdd(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress);
	void NewSub(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress);
	void NewMul(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress);
	void NewDiv(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress);
	void NewMod(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress);
	void NewCompare(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b);
	void NewSetStick(uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);
	void NewHaltUntilSignal();
	void NewHalt(uint16_t sleepTime);
	void NewNop();
	void NewSet(int16_t value, uint8_t address);
};