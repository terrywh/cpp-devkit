#ifndef DEVKIT_COLOR_H_
#define DEVKIT_COLOR_H_
#include <string>
#include <format>

namespace util {

class Output {
    static bool IsPty();
public:
    template <int C, class T>
    static std::string Color(T&& v) {
        return IsPty()
            ? std::format("\x1b[38;5;{}m{}\x1b[0m", C, std::forward<T>(v))
            : std::format("{}", std::forward<T>(v));
    }
    template <class T>
    static std::string Fatal(T&& v) {
        return Color<161, T>(std::forward<T>(v));
    }
    template <class T>
    static std::string Warning(T&& v) {
        return Color<208, T>(std::forward<T>(v));
    }
    template <class T>
    static std::string Info(T&& v) {
        return Color<154, T>(std::forward<T>(v));
    }
    template <class T>
    static std::string Debug(T&& v) {
        return Color<195, T>(std::forward<T>(v));
    }
    template <class T>
    static std::string Trace(T&& v) {
        return Color<250, T>(std::forward<T>(v));
    }
};

} // namespace util

#endif // DEVKIT_COLOR_H_
