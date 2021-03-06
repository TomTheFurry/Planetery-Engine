#pragma once
#include <string>
#include <vector>
#include <iostream>

#define STD_NEXTLINE_CHAR '\n'

class MultilineString
{
  public:
    MultilineString();
    MultilineString(const MultilineString& ms);
    MultilineString(MultilineString&& ms) noexcept;
    MultilineString(const std::string& str, size_t lineLimit = -1);
    MultilineString(const std::vector<std::string>& strlist);
    MultilineString(std::vector<std::string>&& strlist);
    operator std::string() const;
    MultilineString& operator<<(MultilineString&& join);
    std::string& getLine(size_t l);
    const std::string& getLine(size_t l) const;
    size_t countLines() const;
    void padLeft(const std::string& str);
    void padRight(const std::string& str);
    friend std::ostream& operator<<(
      std::ostream& out, const MultilineString& str);
  protected:
    std::vector<std::string> _data;
};