#include <vector>
#include <string>
#include <sstream>
#include <string_view>
#include <stack>
#include <cstring>

#include "ByteCodeBuilder.hpp"

#define Button_Press ((uint16_t)185)
#define Button_LongPress ((uint16_t)1500)
#define Button_VeryLongPress ((uint16_t)2000)

#define Button_Interval_VeryShort ((uint16_t)200)
#define Button_Interval_Short ((uint16_t)500)
#define Button_Interval_Default ((uint16_t)1000)
#define Button_Interval_Long ((uint16_t)1500)
#define Button_Interval_VeryLong ((uint16_t)3000)

static bool is_number(const std::string& s)
{
    for (char c : s) if (!std::isdigit(c)) return false;
    return true;
}

class compile_error : public std::exception
{
private:
    std::string _what;
public:
    compile_error(std::string_view msg) : std::exception(), _what(msg) {}
    virtual const char* what() const noexcept override { return _what.c_str(); }
};

class Compiler
{
private:
    std::ostream& logOut;
    std::istringstream inStream;
    ByteCodeBuilder builder;

    void generatePress(std::stringstream& ss, uint16_t length)
    {
        std::string stopTimeStr;
        ss >> stopTimeStr;

        uint16_t stopTime;
        if (stopTimeStr == "default") stopTime = Button_Interval_Default;
        else if (stopTimeStr == "veryLong") stopTime = Button_Interval_VeryLong;
        else if (stopTimeStr == "long") stopTime = Button_Interval_Long;
        else if (stopTimeStr == "short") stopTime = Button_Interval_Short;
        else if (stopTimeStr == "veryShort") stopTime = Button_Interval_VeryShort;
        else if (is_number(stopTimeStr))
        {
            int num = std::atoi(stopTimeStr.c_str());
            if (num <= 65535) stopTime = num & 0xffff;
            else return error("stopTime should between [0, 65535]");
        }
        else return error("invalid stopTime");

        uint16_t button = 0;
        std::optional<DPadValue> dpad = std::nullopt;

        std::string token;
        while (ss >> token)
        {
            if (token == "a")
                button |= ButtonValue_A;
            else if (token == "b")
                button |= ButtonValue_B;
            else if (token == "x")
                button |= ButtonValue_X;
            else if (token == "y")
                button |= ButtonValue_Y;
            else if (token == "l")
                button |= ButtonValue_L;
            else if (token == "r")
                button |= ButtonValue_R;
            else if (token == "zl")
                button |= ButtonValue_ZL;
            else if (token == "zr")
                button |= ButtonValue_ZR;
            else if (token == "sl")
                button |= ButtonValue_LClick;
            else if (token == "sr")
                button |= ButtonValue_RClick;
            else if (token == "plus")
                button |= ButtonValue_Start;
            else if (token == "minus")
                button |= ButtonValue_Select;
            else if (token == "home")
                button |= ButtonValue_Home;
            else if (token == "capture")
                button |= ButtonValue_Capture;
            else if (token == "right")
            {
                if (dpad.has_value())
                    return error("invalid input");
                dpad = DPadValue_Right;
            }
            else if (token == "rightUp")
            {
                if (dpad.has_value())
                    return error("invalid input");
                dpad = DPadValue_TopRight;
            }
            else if (token == "rightDown")
            {
                if (dpad.has_value())
                    return error("invalid input");
                dpad = DPadValue_BottomRight;
            }
            else if (token == "left")
            {
                if (dpad.has_value())
                    return error("invalid input");
                dpad = DPadValue_Left;
            }
            else if (token == "leftUp")
            {
                if (dpad.has_value())
                    return error("invalid input");
                dpad = DPadValue_TopLeft;
            }
            else if (token == "leftDown")
            {
                if (dpad.has_value())
                    return error("invalid input");
                dpad = DPadValue_BottomLeft;
            }
            else if (token == "up")
            {
                if (dpad.has_value())
                    return error("invalid input");
                dpad = DPadValue_Top;
            }
            else if (token == "down")
            {
                if (dpad.has_value())
                    return error("invalid input");
                dpad = DPadValue_Bottom;
            }
            else return error("invalid input");
        }

        builder.NewPress((ButtonValue)button, length, dpad);
        builder.NewPress((ButtonValue)0, stopTime);
    }

    struct ForDesc
    {
        uint16_t blockStart;
        uint8_t i;
        int16_t loopCount;
    };

    std::stack<ForDesc> forStack;
    void generateFor(std::stringstream& ss)
    {
        std::string token;
        ss >> token;

        int16_t loopCount;
        if (is_number(token))
        {
            int num = std::atoi(token.c_str());
            if (num >= 0) loopCount = num & 0xffff;
            else return error("loopCount should between [0, 32768]");
        }
        else if (token == "unlimited")
            loopCount = -1;
        else return error("invalid loopCount");

        if (loopCount == -1)
        {
            uint16_t pc = builder.PC();

            forStack.push(ForDesc{
                pc,
                0,
                -1
            });
        }
        else if (loopCount == 0)
        {
            forStack.push(ForDesc{
                0,
                0,
                0
            });
        }
        else
        {
            std::optional<uint8_t> iAddress = builder.NewVariable(2);
            if (!iAddress.has_value())
                error("memory overflow");

            builder.NewSet(0, iAddress.value());
            uint16_t pc = builder.PC();

            forStack.push(ForDesc{
                pc,
                iAddress.value(),
                loopCount
            });
        }
    }

    void generateEnd()
    {
        if (forStack.empty())
            return error("mismatch end");
        ForDesc lastDesc = forStack.top();
        forStack.pop();

        if (lastDesc.loopCount == -1)
        {
            builder.NewJumpIf(JumpMode_Always, lastDesc.blockStart);
        }
        else if (lastDesc.loopCount == 0)
        {
        }
        else
        {
            builder.NewAdd(lastDesc.i, static_cast<int16_t>(1), lastDesc.i);
            builder.NewCompare(lastDesc.i, lastDesc.loopCount);
            builder.NewJumpIf(JumpMode_LT, lastDesc.blockStart);
        }
    }

    void error(std::string_view msg)
    {
        logOut << msg << std::endl;
        throw compile_error(msg);
    }

public:
    Compiler(const std::string& source, std::ostream& logOut, std::ostream& code)
        : inStream(source), logOut(logOut), builder(code)
    {
    }

    bool Build()
    {
        std::string line;
        try
        {
            while (std::getline(inStream, line))
            {
                std::stringstream ss(line);
                std::string action;
                ss >> action;

                if (action == "press")
                    generatePress(ss, Button_Press);
                else if (action == "veryLongPress")
                    generatePress(ss, Button_VeryLongPress);
                else if (action == "longPress")
                    generatePress(ss, Button_LongPress);
                else if (action == "for")
                    generateFor(ss);
                else if (action == "end")
                    generateEnd();
                else if (action == "keepLive") {}
                else
                    error("invalid action");
            }
        }
        catch (compile_error& e)
        {
            return false;
        }
        return true;
    }
};

bool CompileOld(std::ostream& log, const std::string& script, std::ostream& code)
{
    Compiler compiler(script, log, code);
    return compiler.Build();
}