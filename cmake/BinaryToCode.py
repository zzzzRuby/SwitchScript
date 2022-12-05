import sys
import os
from optparse import OptionParser

def bin2code_gas(input, output, variable, export):
    with open(output, "w") as f2:
        f2.write(".section .data\n")
        if not export:
            f2.write(".hidden " + variable + "\n")
        f2.write(".global " + variable + "\n")
        f2.write(variable + ":\n")
        f2.write("\t.incbin \"" + os.path.relpath(input, os.path.dirname(output)).replace("\\", "/") + "\"\n")

def bin2code_apple(input, output, variable, export):
    with open(output, "w") as f2:
        f2.write(".section __DATA,__data\n")
        if not export:
            f2.write(".private_extern _" + variable + "\n")
        f2.write("_" + variable + ":\n")
        f2.write("\t.incbin \"" + os.path.relpath(input, os.path.dirname(output)).replace("\\", "/") + "\"\n")

def bin2code_c(input, output, variable, export):
    with open(input, "rb") as f1:
        content = bytearray(f1.read())
    with open(output, "w") as f2:
        if not export:
            f2.write("const unsigned char " + variable + "[] = {")
        else:
            f2.write("#ifndef EXPORT\n")
            f2.write("#if defined(_WIN32)\n")
            f2.write("#define EXPORT __declspec(dllexport)\n")
            f2.write("#else\n")
            f2.write("#define EXPORT __attribute__((visibility(\"default\")))\n")
            f2.write("#endif\n")
            f2.write("#endif\n")
            f2.write("EXPORT const unsigned char " + variable + "[] = {")
        for value in content:
            f2.write(hex(value))
            f2.write(",")
        f2.write("};")

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option('--header', dest='header')
    parser.add_option('--code', dest='code')
    parser.add_option('--type', dest='type')
    parser.add_option('--binary', dest='binary')
    parser.add_option('--symbol-name', dest='symbol_name')
    parser.add_option('--export', dest='export', action='store_true', default=False)
    (options, _args) = parser.parse_args(sys.argv)

    des_dir = os.path.dirname(os.path.abspath(options.header))
    if (not os.path.exists(des_dir)):
        os.makedirs(des_dir)
    des_dir = os.path.dirname(os.path.abspath(options.code))
    if (not os.path.exists(des_dir)):
        os.makedirs(des_dir)
    with open(options.header, "w") as f2:
        f2.write("#pragma once\n")
        f2.write("#include <stdint.h>\n")
        f2.write("#if defined(__cplusplus)\n")
        f2.write("extern \"C\" {\n")
        f2.write("#endif\n")
        f2.write("extern const uint8_t " + options.symbol_name + "[];\n")
        f2.write("const size_t " + options.symbol_name + "_size = " + str(os.path.getsize(options.binary)) + ";\n")
        f2.write("#if defined(__cplusplus)\n")
        f2.write("}\n")
        f2.write("#endif\n")

    functionMapping = {
        "gas" : bin2code_gas,
        "c" : bin2code_c,
        "apple" : bin2code_apple
    }
    functionMapping[options.type](options.binary, options.code, options.symbol_name, options.export)