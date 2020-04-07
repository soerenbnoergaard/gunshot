infile = "DejaVuSans.ttf"
outfile = infile + ".h"
guard_name = infile.upper().replace(".", "_")
var_name = guard_name.lower()

with open(infile, "rb") as f:
    data = f.read()

h = """
#ifndef {guard:s}
#define {guard:s}

#include <stdint.h>

uint8_t {var:s}[] = {{ {data:s} }};
uint32_t {var:s}_length = {length:d};

#endif
""".format(
    guard = guard_name,
    var = var_name,
    data = ", ".join(["0x{:02x}".format(X) for X in data]),
    length = len(data),
)

with open(outfile, "w") as f:
    f.write(h)

