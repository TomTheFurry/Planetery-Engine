#include <Vector>

#include "Translations.h"

#include "GridBase.h"


using vec3 = glm::vec3;
using mat4 = glm::mat4;
using uint = unsigned int;
using ulong = unsigned long long;

GridBase::GridBase() : TranslationBasic() {}

//GridBase* GridBase::getUpper() {}

//GridBase* GridBase::loadUpper() {}
void GridBase::update() {}
void GridBase::unload() {}
void GridBase::loadData(std::vector<char> data) {}
void GridBase::newData() {}