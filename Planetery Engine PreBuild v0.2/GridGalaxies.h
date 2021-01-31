#pragma once

#include <Vector>
#include <list>

#include <glm/glm.hpp>

#include "GridBase.h"
#include "RendererBase.h"
#include "Global.h"
#include "GridGalaxy.h"

//prediclaration
class GalaxyCluster;
class GridGalaxies;



class Galaxy {
public:
    Galaxy(uint seed, GalaxyCluster* gc, uint id);
    uint genId;
    SeededRng* galaxyRng;
    vec3 offset;
    vec3 color;
    vec3 pole;
    uint type;
    float radius;
    float bv;
    bool renderUpdate = true;
    ulint vboPos;
    bool visable = true;
    GalaxyCluster* gc;
};



class GalaxyCluster {
public:
    const float BOUNDERY_BUFFER = 10.0f;

    GalaxyCluster(uint seed, GridGalaxies* gl);

    GridGalaxies* grid;
    std::vector<Galaxy*> galaxyObject;

    vec3 centrePosition;
    int numOfGalaxy;
    SeededRng* clusterRng;

    float maxDistance = 0.0f;
    int updateTick = 0;
    bool inRange = false;

    void onEnterRange();
    void onExitRange();
    void onUpdateInRange();
    void onUpdate();
    bool renderUpdate = true;
};



class GridGalaxies : public TranslationTypeA, public GridBase
{
public:
    const int numOfCluster = 10000;
    const int clusterSizeMin = 1;
    const int clusterSizeMax = 20;
    const float clusterPosDeviation = 800.0f;

    const float galaxyDeviation = 10.0f;

    static float calUpdateTick(vec3 obj, vec3 mov, vec3 speed, float rad, float maxAcc, float maxSpeed);

    GridGalaxies(uint gridId); //grid with data
    
    virtual void newData();
    virtual void loadData(std::vector<char> data);

    virtual void update(); //For override. Remember to callback _update()
    virtual void unload(); //For override. Remember to callback _unload()

    void aAcc(vec3 acc);

    void loadGalaxy(Galaxy* gx);
    void unloadGalaxy(Galaxy* gx);
    bool renderUpdate = true;

    uint tick = 0;
    uint genIds = 0;
    std::vector<GalaxyCluster*> clusters;
    std::list<GalaxyCluster*> inRange;
    std::unordered_map<uint, uint> genIdToGridIdLookup;
    std::list<uint> childGdIds;

};



/*-----------------Render Data------------------*/



struct GalaxyData { //ALL DATA MUST BE 4 BYTES SIZE ONLY
    float posX;   //0-3
    float posY;   //4-7
    float posZ;   //8-11
    float colorR; //12-15
    float colorG; //16-19
    float colorB; //20-23
    float poleX;  //24-27
    float poleY;  //28-31
    float poleZ;  //32-35
    uint type;    //36-39
    float radius; //40-43   Note: if equals 0, it means disabled.
}; //total 44 Bytes, 11 units



class GridGalaxiesRenderer : ShaderProgram, public GridBaseRenderer
{
public:
    const uint DATASIZE = sizeof(GalaxyData) / sizeof(float);

    GridGalaxiesRenderer(Anchor* a);
    std::vector<float> data;

    uint vaoId;
    uint vboId;

    void initData();

    bool needZoomIn();
    bool needZoomOut();

    void prerender();

    void render();

};