#pragma once

#include <glm/glm.hpp>
#include <Vector>
#include <unordered_map>

#include "Global.h"
#include "ShaderProgram.h"
#include "Anchors.h"
#include "GridBase.h"
#include "RendererBase.h"

/* ALL NEW GRIDS CLASS AND RENDERER CLASS FILES NEEDS TO BE REGISTERED HERE*/
#include "GridGalaxies.h"
#include "GridGalaxy.h"



class MapFileManager
{
public:

	MapFileManager();

	void* mapFile; //Will be a file stream
	ulint mapSeed;
	uint anchorsCount;

	//Lookup table for grid data

	/*
	File format:
		file header(64): //NO COMPRESSION
			
			[ulint]identifier(4), [uint]saveFileFormatVersion(2), [uint]gameVersion(2),
			[uint]gameSubVersion(2), [uint]compressionMethod(2), [ulint]compressionKey???(4),
			[ullint]saveFileSizeUncompressed(8),
			unused(40)

		saveFile:(max of 2^64 * 8 bypes, more then enough) //Can be compressed
			savefile header(512):
				
				[ullint]seed(8),
				[uint]gridsCount(2), [ulint]lookupTableSizeInBytesOfEight(2)

			Grids Lookup Table(determinded by lookupTableSizeInBytesOfEight):
				
				[ullint]gridId[0]DataPositionOffsetInByte(8),
				[ullint]gridId[1]DataPositionOffsetInByte(8),
				[ullint]gridId[2]DataPositionOffsetInByte(8),
				[ullint]gridId[3]DataPositionOffsetInByte(8),
				.
				.
				.

			GridsData Start:

				Grid:
					[uint]typeAndStuff...
	
	*/

	void loadMap(uint fileId);

	std::vector<char> getGridData(uint id); //return the data of the grid. Throws DataNotFoundExpection if it does not exist
	std::vector<Anchor> getAnchors(); //return all anchors
protected:

	void openFile(std::string url);
	void saveChange();

	std::vector<ulint> gridDataOffsetLookup;
	ulint gridDataOffsetStart;

	ulint getGridDataLocation(uint id);
	std::vector<char> getGridDataFromLocation(ulint offset);


};



class WorldManager
{
public:

	const uint MAX_GRIDS_COUNT = 65536; //max uint number - 1
	const uint LOOKUPSIZE[14] = { 8,16,32,64,128,256,512,1024,2048,4096,8192,16384,36728,65536 }; //For init
	uint lookupTableSizeLevel = 0;

	uint grids_count = 0; //total grids number
	std::vector<uint> gridIdLookups; // REMEMBER TO -1!! change this will change the max num of loaded grids. Currently 65536
	
	std::vector<GridBase*> grids;
	
	std::vector<Anchor> anchors;

	SeededRng* worldGenRng = nullptr;

	WorldManager(); //Init

	//Change this for init world gen
	void loadMap(uint fileId);

	void startWorld();

	//Change this to change overall world structure

	GridBase* getGrid(uint gridType, uint gridDataId); //Get grid and create it if not exist
	GridBase* getGridNoLoad(uint gridType, uint gridDataId); //Get grid

	bool loadGrid(uint gridType, uint gridDataId); //return bool isGridNewGrid
	void* newGridArgs = nullptr; //A hack for passing grid creation data when calling newGrid.
	uint newGrid(uint gridType); //return gdId of the new grid

	void tickAnchor();

	uint getGdIdByAnchor(Anchor* anchor, uint gridType);
	
};



class RenderManager
{
public:
	Anchor* target;
	std::vector<GridBaseRenderer*> renderers;
	RenderManager();

	void setRenderTarget(Anchor* t);
	void addRenderer(uint gridType);

	void renderTarget();

	void checkForRendererType();


};