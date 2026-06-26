#!/usr/bin/python3
import argparse
import pathlib
import re
import sys


def process(input, output, symbols):
    """Process input string using defined symbols and write the result to
    output file handle."""

    # A stack of conditions to control echoing input to output.
    conditions = [True]  # Implicit top-level branch is True.

    while True:
        token_found = re.search(r"@([^@]*)@", input, flags=re.DOTALL)

        # No "@...@" token found, this is the terminal state.
        if token_found is None:
            if conditions[0]:
                output.write(input)
            if len(conditions) > 1:
                print("Missing @ENDIF@.", file=sys.stderr)
            return

        # Else echo text between the previous and the current token
        if conditions[0]:
            output.write(input[:token_found.start()])

        # Empty "@@" statement is a quoting way to spell literal "@".
        if token_found.group(1) == "":
            if conditions[0]:
                output.write("@")
            input = input[token_found.end():]
            continue

        # "@IF SYMBOL@ to start a branch
        if_found = re.search(r"IF\s+(.*)", token_found.group(1), flags=re.DOTALL)
        if if_found:
            condition = if_found.group(1)
            condition_matches = condition in symbols and symbols[condition]["boolean"]
            # print("DEBUG: condition={}, matches={}".format(condition,
            #                                                condition_matches))
            # Current False level implies all next levels to be False as an
            # optimization for tests when printing.
            conditions.insert(0, conditions[0] and condition_matches)
            input = input[token_found.end():]
            continue

        # "@ENDIF@ to end a branch
        if token_found.group(1) == "ENDIF":
            if len(conditions) == 1:
                print("Superfluous @ENDIF@.", file=sys.stderr)
            else:
                conditions.pop(0)
            input = input[token_found.end():]
            continue

        # Generic @VARIABLE@ to implement CMake variable expansion.
        # Undefined variables expands to en empty string.
        if token_found.group(1) in symbols:
            if conditions[0]:
                output.write(symbols[token_found.group(1)]["string"])
        input = input[token_found.end():]
        continue


# Defined symbols as a Dict(boolean, string) by the symbol name.
symbols = {}


def add_symbol_from_argument(argument):
    """Parse --define option argument and store it into "symbols"
    dictionary."""

    symbol, *value = argument.split(sep="=", maxsplit=1)
    if symbol == '':
        raise argparse.ArgumentError("A symbol name cannot be an empty string.")

    if not value:
        boolean = True
        value = ['']
    elif value[0].upper() in ("OFF", "FALSE", "0"):
        boolean = False
    else:
        boolean = True
    symbols[symbol] = {'boolean': boolean, 'string': value[0]}


parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description="Expand at-sign--delimented variables and branches of a text.",
    epilog="""
Expressions subject to the expansion:

@IF SYMBOL@         If the SYMBOL's boolean value is to true, a text up
                    to the corresponding @ENDIF@ delimiter, or the end of
                    file, is echoed to the output. Otherwise, the text is
                    discarded. The branches can be nested.
                    There can be more white spaces, including newlines between
                    the IF and SYMBOL.

@ENDIF@             End of the branch starting just after the nearest
                    preceding @IF SYMBOL@ condition.

@SYMBOL@            Expand SYMBOL to its string value. If no value was defined
                    in the --define option, or no such symbol was defined at
                    all it expands to an empty string.

@@                  Expand to a single @ character.

Note that new-line characters adjacent to the @...@ expression are not
part of the expression and will be preserved on the output regardless of the
expansion. To remove the newlines from the output remove them first from the
input.

All the other text is echoed from the input to the output without any change.
""")
parser.add_argument("input", help="an input file to process; default is \
                    standard input",
                    type=pathlib.Path, nargs="?")
parser.add_argument("output", help="a file to write the processed output to; \
                    default is standard output",
                    type=pathlib.Path, nargs="?")
parser.add_argument("--define", help="defines a SYMBOL with the VALUE. \
                    If the VALUE is OFF, FALSE, or 0, the SYMBOL's boolean \
                    value in @IF SYMBOL@ conditions becomes false. \
                    Otherwise, true.",
                    metavar="SYMBOL[=VALUE]",
                    type=add_symbol_from_argument)
args = parser.parse_args()

# Load input
input = args.input.read_text() if args.input else sys.stdin.read()

# Open output and print the processed input there
with args.output.open(mode="w") if args.output else sys.stdout as output:
    process(input, output, symbols)
