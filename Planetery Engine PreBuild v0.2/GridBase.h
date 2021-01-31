#pragma once

#include <Vector>

#include <glm/glm.hpp>

#include "Translations.h"
#include "utility.h"

/*
Grid Object handles loading and making grid (or world).
Example: GridPlanet, GridStarSystem, GridGalaxy, GridGalaxies

What it needs to do:

Creation:
	Remember to choose which TranslationsClass to use. default: TranslationBase
	Spawned by upper grid from enter range
	Spawned by player loading in, needs to also spawn upper grid in that case

Game logic:
1 Try load from data, if failed,
2. Generate grid data
3. Save grid data and unload subgrid on exit range (Unload)
4. Create subgrid on enter subgrid range
5. Unload subgrid on exit subgrid range
6. Note: subgrid update() should only be called using efficient ticker module (Classes located in the bottom)

Game display:
Not sure???
Store if it needs rendering??? And also update related grids if needed?

*/

using vec3 = glm::vec3;
using mat4 = glm::mat4;
using uint = unsigned int;
using ulint = unsigned long long;



//Virtual Class!!!!
class GridBase : public TranslationBasic
{
public:
	//Id in storage
	uint type = uint(-1);
	uint gdId = 0;
	bool hasUpperGrid = false;
	uint ugdId = 0;

    SeededRng* gridRng = nullptr;

	GridBase();
	
	//virtual GridBase* getUpper(); //getUpper GridBase (Does not auto load into memory)
	//virtual GridBase* loadUpper(); //Same as GridBase but also force load into memory
	virtual void update(); //For override. Remember to callback _update()
	virtual void unload(); //For override. Remember to callback _unload()
	virtual void loadData(std::vector<char> data);
	virtual void newData();
};