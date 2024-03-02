#include "output.h"
#include <cstdio>
#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

namespace util {

bool Output::IsPty() {
#ifdef _WIN32
    static bool pty = _isatty(_fileno(stdout)) != 0;
#else
    static bool pty = isatty(fileno(stdout)) != 0;
#endif
    return pty;
}

} // namespace util

