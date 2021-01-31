#include <iostream>
#include <Vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glfw/glfw3.h>

using vec3 = glm::vec3;
using mat4 = glm::mat4;
using uint = unsigned int;
using ulint = unsigned long long;

#include "Translations.h"
#include "GridBase.h"
#include "RendererBase.h"
#include "Global.h"

#include "Manager.h"



//MapFileManager TODO!!!!!

MapFileManager::MapFileManager() {
	global->mapFileManager = this;
}


void MapFileManager::loadMap(uint fileId)
{
}

std::vector<char> MapFileManager::getGridData(uint id)
{
	return std::vector<char>();
}

std::vector<Anchor> MapFileManager::getAnchors()
{
	return std::vector<Anchor>();
}

void MapFileManager::openFile(std::string url)
{
}

void MapFileManager::saveChange()
{
}



ulint MapFileManager::getGridDataLocation(uint id)
{
	return ulint();
}

std::vector<char> MapFileManager::getGridDataFromLocation(ulint offset)
{
	return std::vector<char>();
}



//WorldManager

WorldManager::WorldManager() {
	global->worldManager = this;
}

void WorldManager::loadMap(uint fileId) {
	global->mapFileManager->loadMap(fileId);

	grids_count = 0; //TODO: Load grids count to here
	worldGenRng = new SeededRng(global->rng.next()); //TODO: Load world seed to here

	for (uint i : LOOKUPSIZE) {
		if (grids_count < i) {
			lookupTableSizeLevel = i;
			break;
		}
	}

	gridIdLookups.reserve(LOOKUPSIZE[lookupTableSizeLevel]);
	for (uint i = 0; i < LOOKUPSIZE[lookupTableSizeLevel]; i++) {
		gridIdLookups.push_back(0);
	}

	std::vector<Anchor> a = global->mapFileManager->getAnchors();
	if (a.size() == 0) {
		anchors.push_back(Player(0, 0, 0, vec3(0.0f), vec3(0.0f)));
		global->main = &anchors[0];
	}
	else {
		anchors = a;
	}
}

void WorldManager::startWorld()
{
	for (Anchor& a : anchors) {
		loadGrid(a.gridType, a.gridId);
	}
}

GridBase* WorldManager::getGrid(uint gridType, uint gridDataId)
{
	if (gridIdLookups[gridDataId] == 0) loadGrid(gridType, gridDataId);
	return grids[gridIdLookups[gridDataId] - 1];
}

GridBase* WorldManager::getGridNoLoad(uint gridType, uint gridDataId)
{
	if (gridIdLookups[gridDataId] == 0) return nullptr;
	return grids[gridIdLookups[gridDataId] - 1];
}

bool WorldManager::loadGrid(uint gT, uint gdId)
{
	GridBase* g;
	switch (gT) {
	case 0:
		g = (GridBase*) new GridGalaxies(gdId);
		break;
	case 1:
		g = (GridBase*) new GridGalaxy(gdId);
		break;
	default:
		throw;
	}

	//Add the grid object to active grids entries
	long int lId = -1; //loaded grid id
	for (uint i = 0; i <= grids.size(); i++) {
		if (i == grids.size()) {
			lId = i;
			grids.push_back(g);
			break;
		}
		if (grids[i] == nullptr) {
			lId = (long int)i;
			grids[i] = g;
			break;
		}
	}
	if (lId == -1) throw; //TODO: Delete object too, and cout
	if (gridIdLookups.size() < gdId) { //Check if array needs expending

		for (uint i : LOOKUPSIZE) {
			if (gdId < i) {
				lookupTableSizeLevel = i;
				break;
			}
		}

		gridIdLookups.reserve(LOOKUPSIZE[lookupTableSizeLevel]);
		for (uint i = uint(gridIdLookups.size()); i < LOOKUPSIZE[lookupTableSizeLevel]; i++) {
			gridIdLookups.push_back(0);
		}
	}
	gridIdLookups[gdId] = lId+1;

	auto gData = global->mapFileManager->getGridData(gdId);

	if (gData.size() == 0) {
		if (gdId >= grids_count)
			grids_count = gdId + 1; //TODO: Protencal problem with deleting grids if it is allowed in the future
		g->newData();
		return true;
	}
	else {
		g->loadData(gData);
		return false;
	}
}

uint WorldManager::newGrid(uint gT)
{
	uint gdId = grids_count;

	GridBase* g;
	switch (gT) {
	case 0:
		g = (GridBase*) new GridGalaxies(gdId);
		break;
	case 1:
		g = (GridBase*) new GridGalaxy(gdId);
		break;
	default:
		throw;
	}

	//Add the grid object to active grids entries
	long long lId = -1; //loaded grid id
	for (uint i = 0; i <= grids.size(); i++) {
		if (i == grids.size()) {
			lId = i;
			grids.push_back(g);
			break;
		}
		if (grids[i] == nullptr) {
			lId = (long long)i;
			grids[i] = g;
			break;
		}
	}
	if (lId == -1) throw; //TODO: Delete object too, and cout
	if (gridIdLookups.size() < gdId) { //Check if array needs expending

		for (uint i : LOOKUPSIZE) {
			if (gdId < i) {
				lookupTableSizeLevel = i;
				break;
			}
		}

		gridIdLookups.reserve(LOOKUPSIZE[lookupTableSizeLevel]);
		for (uint i = uint(gridIdLookups.size()); i < LOOKUPSIZE[lookupTableSizeLevel]; i++) {
			gridIdLookups.push_back(0);
		}
	}
	gridIdLookups[gdId] = lId+1;
	
	grids_count++;
	g->newData();
	return gdId;
}

void WorldManager::tickAnchor() {
	for (Anchor& a : anchors) {
		uint gdId = a.gridId;
		if (gridIdLookups[gdId] == 0) {
			loadGrid(a.gridType, a.gridId);
		}
		grids[gridIdLookups[gdId]-1]->update();
	}
}

uint WorldManager::getGdIdByAnchor(Anchor* a, uint gType) {
	if (gType > a->gridType) return uint(-1);
	uint result = a->gridId;
	if (gType == 0) return 0;
	for (uint i = a->gridType; i >= gType; i--) {
		auto g = getGridNoLoad(i, result);
		if (!g->hasUpperGrid) return uint(-1);
		result = g->ugdId;
	}
	return result;
}



// RenderManager

RenderManager::RenderManager() {
	target = nullptr;
}

void RenderManager::setRenderTarget(Anchor* t) {
	target = t;
	if (renderers.size() != 0) {
		for (auto e : renderers) {
			delete e;
		}
		renderers.clear();
	}
	switch (t->gridType) //Break in case is NOT needed!
	{
	case 0:
		renderers.push_back((GridBaseRenderer*)new GridGalaxiesRenderer(t));
	case 1:
		renderers.push_back((GridBaseRenderer*)new GridGalaxyRenderer(t));
	}
}

void RenderManager::addRenderer(uint gType) {
	switch (gType) //Break in case is needed!
	{
	case 0:
		renderers.push_back((GridBaseRenderer*)new GridGalaxiesRenderer(target));
		break;
	case 1:
		renderers.push_back((GridBaseRenderer*)new GridGalaxyRenderer(target));
		break;
	}
}

void RenderManager::renderTarget() {
	if (target == nullptr) return;
	checkForRendererType();

	for (auto r : renderers) {
		r->prerender();
		r->render();
	}
}

void RenderManager::checkForRendererType() {
	if (renderers.size() == 0) return;
	auto r = renderers[renderers.size() - 1];
	if (r->needZoomIn()) {
		addRenderer(renderers.size());
		checkForRendererType();
	} else if (r->needZoomOut()) {
		delete r;
		renderers.pop_back();
		checkForRendererType();
	}
}
