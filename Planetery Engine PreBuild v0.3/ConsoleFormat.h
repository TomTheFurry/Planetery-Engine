// Format word for console out

#define FORMAT_START       "\033["
#define FORMAT_MID         ";"
#define FORMAT_END         "m"
#define FORMAT_RESET       "\033[;0m"
#define COLOR_BLACK        30
#define COLOR_RED          31
#define COLOR_GREEN        32
#define COLOR_YELLOW       33
#define COLOR_BLUE         34
#define COLOR_MAGENTA      35
#define COLOR_CYAN         36
#define COLOR_WHITE        37
#define COLOR_8BIT(v)      38, 5, v
#define COLOR_RGB(r, g, b) 38, 2, r, g, b
#define COLOR_DEFAULT      39
#define BRIGHT             60 +
#define BACKGROUND         10 +
#define BOLD               108
#define FAINT              108
#define ITALIC             108
#define UNDERLINE          4
#define BLINK              108
#define CROSS_OUT          108
#define END                20 +
#define RESET_ALL          0
inline std::string format(const std::initializer_list<char> list) {
    std::string str{FORMAT_START};
    str += std::to_string(int(*list.begin()));
    for (auto it = list.begin() + 1; it != list.end(); it++) {
        str += FORMAT_MID;
        str += std::to_string(int(*it));
    }
    return (str += FORMAT_END);
}
#pragma once
