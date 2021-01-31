#include "MultilineString.h"
#include "ConsoleFormat.h"

//BUG: Unkown issue with "\n newline missing" warning with logger. Not sure where the bug is.

MultilineString::MultilineString(const MultilineString& ms) : _data(ms._data) {}
MultilineString::MultilineString(MultilineString&& ms) noexcept : _data(std::move(ms._data)) {}
MultilineString::MultilineString() : _data() {}
MultilineString::MultilineString(const std::string& str, size_t lineLimit) : _data() {
    size_t pos = 0;
    size_t newPos;
    while ((newPos = str.find(STD_NEXTLINE_CHAR, pos)) != std::string::npos) {
        size_t strLen = newPos-pos;
        pos = newPos;
        for (; strLen>lineLimit; strLen -= lineLimit) {
            _data.emplace_back(str.substr(pos-strLen, lineLimit));
        }
        _data.emplace_back(str.substr(pos-strLen, strLen));
        pos++;
    }
    if (pos != str.length()) {
        _data.emplace_back(str.substr(pos)+format({COLOR_RED})+"\\n"+format({COLOR_DEFAULT}));
    }
}
MultilineString::MultilineString(const std::vector<std::string>& strlist) : _data(strlist) {}
MultilineString::MultilineString(std::vector<std::string>&& strlist) : _data(std::move(strlist)) {}
MultilineString::operator std::string() const {
    std::string a{};
    for (const auto& line : _data) {
        a.append(line);
        a.push_back(STD_NEXTLINE_CHAR);
    }
    return a;
}
MultilineString& MultilineString::operator<<(MultilineString&& join) {
    _data.insert(_data.end(), std::make_move_iterator(join._data.begin()), std::make_move_iterator(join._data.end()));
    return *this;
}
std::string& MultilineString::getLine(size_t l) {
    return _data[l];
}
const std::string& MultilineString::getLine(size_t l) const {
    return _data[l];
}
size_t MultilineString::countLines() const {
    return _data.size();
}
void MultilineString::padLeft(const std::string& str) {
    for (auto& line : _data) {
        line.insert(0, str);
    }
}
void MultilineString::padRight(const std::string& str) {
    for (auto& line : _data) {
        line.append(str);
    }
}
std::ostream& operator<<(std::ostream& out, const MultilineString& str) {
    for (auto& line : str._data) {
        out << line << STD_NEXTLINE_CHAR;
    }
    return out;
}