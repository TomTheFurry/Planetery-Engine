#pragma once

#include <Vector>
#include <list>
#include <queue>

#include <glm/glm.hpp>

#include "Global.h"
#include "GridBase.h"
#include "RendererBase.h"
#include "GridGalaxies.h"

/*
* 1 unit (SU) is ~ 1 light year or 0.00001 GU
* Galaxy average size ~ 1 GU or 100,000 SU (1,000 SU thick (with 10,000 SU thick in the middle))
* Planetary system average size ~ 2 SU
* Planetary system average distance ~ 7 SU in Solar System area, ~ 4 SU in Centre of galaxy
* Average num of system ~ 100,000,000
* 1,000,000,000
* 1 GSU is ~ 1,000 SU or 0.01 GU (GSU = Galaxy Sector Unit)
* Galaxy average sector number ~ 100*100*10 (100,000)
* NOTE:
*   All grid pos is defined as in the middle of the grid.
*   And centre of sector(0,0,0) is the middle of the galaxy.
*/

//prediclaration
class GridGalaxy;
class Galaxy;



class Star {
public:
    const float BOUNDERY_BUFFER = 10.0f;

    Star(uint seed, GridGalaxy* gl);

    GridGalaxy* grid;

    vec3 pos;
    vec3 color;
    uint type;
    bool visable = true;
    float brightness;
    SeededRng* starRng;

    float maxDistance = 0.0f;
    int updateTick = 0;
    uint genId;
    bool inRange = false;

    void onEnterRange();
    void onExitRange();
    void onUpdateInRange();
    void onUpdate();

    ulint vboPos;
    bool renderUpdate = true;
};



/*class GalaxySector {
public:
    glm::ivec3 pos;
    GalaxySector(ivec3 pos);

};
*/



class GridGalaxy : public TranslationTypeA, public GridBase
{
public:
    using pqe = pQueueEntry;
    using pqg = std::priority_queue<pqe, std::deque<pqe>, pGreater>;
    int numOfStar = 100000;
    float clusterPosDeviation = 25000.0f;
    float clusterPosDeviationThickness = 5000.0f;

    static float calUpdateTick(vec3 obj, vec3 mov, vec3 speed, float rad, float maxAcc, float maxSpeed);

    GridGalaxy(uint gridId);
    ~GridGalaxy();

    virtual void newData();
    virtual void loadData(std::vector<char> data);

    virtual void update(); //For override. Remember to callback _update()
    virtual void unload(); //For override. Remember to callback _unload()

    void loadStar(Star* star);
    void unloadStar(Star* star);
    bool renderUpdate = true;

    ulint tick = 0;
    uint genIds = 0;
    std::vector<Star*> stars;
    std::list<Star*> inRange;
    pqg updateQueue;
    vec3 mainColor;
    double maxRange;
    Galaxy* ugx;

};



/*-----------------Render Data------------------*/



struct StarData { //ALL DATA MUST BE 4 BYTES SIZE ONLY
    float posX;   //0-3
    float posY;   //4-7
    float posZ;   //8-11
    float colorR; //12-15
    float colorG; //16-19
    float colorB; //20-23
    uint type;    //24-27
    float brightness; //28-31
}; //total 32 Bytes, 8 units

struct StarDataFar { //ALL DATA MUST BE 4 BYTES SIZE ONLY
    float posX;   //0-3
    float posY;   //4-7
    float posZ;   //8-11
    float colorR; //12-15
    float colorG; //16-19
    float colorB; //20-23
    uint type;    //24-27
    float brightness; //28-31
}; //total 32 Bytes, 8 units


class GridGalaxyRenderer : ShaderProgram, public GridBaseRenderer
{
public:
    const uint DATASIZE = sizeof(StarData) / sizeof(float);

    GridGalaxyRenderer(Anchor* a);
    std::vector<float> data;

    uint vaoId;
    uint vboId;

    void initData();

    bool needZoomIn();
    bool needZoomOut();

    void prerender();

    void render();
	ShaderProgram* mode2;
	const float cutout = 500000.0f;
};