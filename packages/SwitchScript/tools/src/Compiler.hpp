#pragma once
#include <ostream>
#include <string>
#include <vector>

bool CompileOld(std::ostream& log, const std::string& script, std::ostream& code);
std::vector<std::string> LexerParseTokens(const std::string& source);