#include "Compiler.hpp"
#include "ByteCodeBuilder.hpp"

enum class LexerState
{
	None,
	Operator,
	Number,
	Symbol,
	Unknown
};

static inline LexerState lexerState(LexerState lastState, char c)
{
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		return lastState == LexerState::Number ? LexerState::Unknown : LexerState::Symbol;
	if (c == '*' || c == '+' || c == '-' || c == '/' || c == '%' || c == ';' || c == ':' ||
		c == '>' || c == '<' || c == '=' || c == '(' || c == ')' || c == '!' || c == '|' || c == '&')
		return LexerState::Operator;
	if (c >= '0' && c <= '9')
		return lastState == LexerState::Symbol ? LexerState::Symbol : LexerState::Number;
	if (c == '.')
		return lastState == LexerState::Number ? LexerState::Number : LexerState::Unknown;
	
	if (c == '\r' || c == '\n' || c == '\t' || c == ' ')
		return LexerState::None;

	return LexerState::Unknown;
}

std::vector<std::string> LexerParseTokens(const std::string& source)
{
	std::vector<std::string> result;
	LexerState state = LexerState::None;
	size_t charIndex = 0;

	std::string token = "";
	while (true)
	{
		char c = source[charIndex];
		if (c == 0)
			break;

		LexerState nextState = lexerState(state, c);
		bool match = false;
		if (nextState != LexerState::None)
			match = true;

		if (match)
		{
			charIndex += 1;
			if (state != nextState)
			{
				if (token.size() != 0)
					result.push_back(token);
				token.clear();
			}
			state = nextState;
			token.push_back(c);
		}
		else
		{
			charIndex += 1;
			if (token.size() != 0)
				result.push_back(token);
			token.clear();
			state = LexerState::None;
		}
	}

	return result;
}