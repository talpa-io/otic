import sys


def main():
    with open("otic.c") as f:
        dotc = f.read()
    with open("otic.h") as f:
        doth = f.read()
    with open("internal.h") as f:
        internal = f.read()
    dotc = dotc.replace('#include "otic.h"\n', doth, 1)
    dotc = dotc.replace('#include "internal.h"\n', internal, 1)
    with open("otic_all.h", "w") as f:
        f.write(dotc)


if __name__ == "__main__":
    main()
