#include "ByteCodeBuilder.hpp"

template<typename T>
static inline void streamWrite(std::ostream& stream, T buffer)
{
	stream.write((const char*)&buffer, sizeof(T));
}

ByteCodeBuilder::ByteCodeBuilder(std::ostream& code)
	: m_Code(code), m_OriginPos(code.tellp()), m_UsedHeap{ false }
{
}

std::optional<uint8_t> ByteCodeBuilder::NewVariable(uint8_t size)
{
	for (size_t i = 0; i < m_UsedHeap.size(); i++)
	{
		bool succeed = true;
		for (uint8_t j = 0; j < size; j++)
		{
			if (m_UsedHeap[i + j])
			{
				succeed = false;
				break;
			}
		}

		if (succeed)
		{
			for (uint8_t j = 0; j < size; j++)
				m_UsedHeap[i + j] = true;

			return (uint8_t)i;
		}
	}
	return std::nullopt;
}

void ByteCodeBuilder::DestroyVariable(uint8_t var)
{
	auto it = m_VariableMap.find(var);
	if (it == m_VariableMap.end())
		return;
	
	size_t size = it->second;
	for (size_t i = 0; i < size; i++)
		m_UsedHeap[i + var] = false;

	m_VariableMap.erase(it);
}

uint16_t ByteCodeBuilder::PC()
{
	return (uint16_t)(m_Code.tellp() - m_OriginPos);
}

void ByteCodeBuilder::NewSetButton(ButtonValue flags, std::optional<DPadValue> dpad)
{
	VMCommand command = { 0 };
	command.opCode = Op_SetButton;
	if (dpad.has_value())
	{
		command.desc.setButtonOp.setDPad = 1;
		command.desc.setButtonOp.dpad = dpad.value();
	}
	else
	{
		command.desc.setButtonOp.setDPad = 0;
	}
	uint16_t extend = (uint16_t)flags;

	streamWrite(m_Code, command);
	streamWrite(m_Code, extend);
}

void ByteCodeBuilder::NewUnsetButton(ButtonValue flags, bool unsetDpad)
{
	VMCommand command = { 0 };
	command.opCode = Op_UnsetButton;
	command.desc.unsetButtonOp.unsetDPad = unsetDpad ? 1 : 0;
	uint16_t extend = (uint16_t)flags;

	streamWrite(m_Code, command);
	streamWrite(m_Code, extend);
}

void ByteCodeBuilder::NewJumpIf(JumpMode mode, uint16_t target)
{
	VMCommand command = { 0 };
	command.opCode = Op_JumpIf;
	command.desc.jumpIfOp.mode = mode;

	uint16_t extend = (uint16_t)target;

	streamWrite(m_Code, command);
	streamWrite(m_Code, extend);
}

static void GenerateALUCommand(std::ostream& stream, Op op, std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress)
{
	VMCommand command = { 0 };
	command.opCode = op;
	command.desc.aluOp.isLeftConst = a.index() == 1 ? 1 : 0;
	command.desc.aluOp.isRightConst = b.index() == 1 ? 1 : 0;

	streamWrite(stream, command);
	if (std::holds_alternative<uint8_t>(a)) {
		streamWrite(stream, std::get<uint8_t>(a));
	}
	else if (std::holds_alternative<int16_t>(a)) {
		streamWrite(stream, std::get<int16_t>(a));
	}

	if (std::holds_alternative<uint8_t>(b)) {
		streamWrite(stream, std::get<uint8_t>(b));
	}
	else if (std::holds_alternative<int16_t>(b)) {
		streamWrite(stream, std::get<int16_t>(b));
	}

	streamWrite(stream, resultAddress);
}

void ByteCodeBuilder::NewAdd(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress)
{
	GenerateALUCommand(m_Code, Op_Add, a, b, resultAddress);
}

void ByteCodeBuilder::NewSub(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress)
{
	GenerateALUCommand(m_Code, Op_Sub, a, b, resultAddress);
}

void ByteCodeBuilder::NewMul(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress)
{
	GenerateALUCommand(m_Code, Op_Mul, a, b, resultAddress);
}

void ByteCodeBuilder::NewDiv(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress)
{
	GenerateALUCommand(m_Code, Op_Div, a, b, resultAddress);
}

void ByteCodeBuilder::NewMod(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b, uint8_t resultAddress)
{
	GenerateALUCommand(m_Code, Op_Mod, a, b, resultAddress);
}

void ByteCodeBuilder::NewCompare(std::variant<uint8_t, int16_t> a, std::variant<uint8_t, int16_t> b)
{
	VMCommand command = { 0 };
	command.opCode = Op_Compare;
	command.desc.aluOp.isLeftConst = a.index() == 1 ? 1 : 0;
	command.desc.aluOp.isRightConst = b.index() == 1 ? 1 : 0;

	streamWrite(m_Code, command);
	if (std::holds_alternative<uint8_t>(a)) {
		streamWrite(m_Code, std::get<uint8_t>(a));
	}
	else if (std::holds_alternative<int16_t>(a)) {
		streamWrite(m_Code, std::get<int16_t>(a));
	}

	if (std::holds_alternative<uint8_t>(b)) {
		streamWrite(m_Code, std::get<uint8_t>(b));
	}
	else if (std::holds_alternative<int16_t>(b)) {
		streamWrite(m_Code, std::get<int16_t>(b));
	}
}

void ByteCodeBuilder::NewPress(ButtonValue flags, uint16_t length, std::optional<DPadValue> dpad)
{
	VMCommand command = { 0 };
	command.opCode = Op_Press;
	if (dpad.has_value())
	{
		command.desc.pressOp.dpad = dpad.value();
	}
	else
	{
		command.desc.pressOp.dpad = DPadValue_None;
	}
	uint16_t extend = (uint16_t)flags;

	streamWrite(m_Code, command);
	streamWrite(m_Code, extend);
	streamWrite(m_Code, length);
}

void ByteCodeBuilder::NewTerminate()
{
	VMCommand command = { 0 };
	command.opCode = Op_Extern;
	command.desc.externOp.externCode = ExternOp_Terminate;

	streamWrite(m_Code, command);
}

void ByteCodeBuilder::NewNop()
{
	VMCommand command = { 0 };
	command.opCode = Op_Extern;
	command.desc.externOp.externCode = ExternOp_Nop;

	streamWrite(m_Code, command);
}

void ByteCodeBuilder::NewSetStick(uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry)
{
	VMCommand command = { 0 };
	command.opCode = Op_Extern;
	command.desc.externOp.externCode = ExternOp_SetStick;

	streamWrite(m_Code, command);
	streamWrite(m_Code, lx);
	streamWrite(m_Code, ly);
	streamWrite(m_Code, rx);
	streamWrite(m_Code, ry);
}

void ByteCodeBuilder::NewHaltUntilSignal(uint8_t address)
{
	VMCommand command = { 0 };
	command.opCode = Op_Extern;
	command.desc.externOp.externCode = ExternOp_HaltUntilSignal;

	streamWrite(m_Code, command);
	streamWrite(m_Code, address);
}

void ByteCodeBuilder::NewHalt(uint16_t sleepTime)
{
	VMCommand command = { 0 };
	command.opCode = Op_Extern;
	command.desc.externOp.externCode = ExternOp_Halt;

	streamWrite(m_Code, command);
	streamWrite(m_Code, sleepTime);
}

void ByteCodeBuilder::NewSet(int16_t value, uint8_t address)
{
	VMCommand command = { 0 };
	command.opCode = Op_Extern;
	command.desc.externOp.externCode = ExternOp_Set;

	streamWrite(m_Code, command);
	streamWrite(m_Code, address);
	streamWrite(m_Code, value);
}