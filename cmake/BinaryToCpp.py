if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input", required=True)
    parser.add_argument("-o", "--output", required=True)
    parser.add_argument("-s", "--symbol", required=True)
    parser.add_argument("-n", "--namespace", required=True)
    args = parser.parse_args()

    with open(args.input, "rb") as f:
        content = bytearray(f.read())

    with open(args.output, "w") as f:
        f.writelines([
            "#pragma once\n",
            "#include <array>\n",
            f"namespace {args.namespace}\n",
            "{\n",
            f"\tconstexpr std::array<std::byte, {len(content)}> {args.symbol}\n",
            "\t{\n"
        ] + [ f"\t\tstd::byte{{{hex(x)}}},\n" for x in content ] + [
            "\t};\n"
            "}"
        ])