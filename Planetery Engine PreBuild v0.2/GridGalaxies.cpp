
#include <iostream>
#include <algorithm>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include "GridGalaxies.h"

/*
* 1 unit (GU) is ~ 100,000 light years
* Max speed: 2 GU
* Max Acceleration: 0.002 GU/t (200 SU/t)
*/



//Galaxy

Galaxy::Galaxy(uint seed, GalaxyCluster* gc, uint id) {
	galaxyRng = new SeededRng(seed);
	Galaxy::gc = gc;
	offset = vec3(
		galaxyRng->normal(0.0f, gc->grid->galaxyDeviation),
		galaxyRng->normal(0.0f, gc->grid->galaxyDeviation),
		galaxyRng->normal(0.0f, gc->grid->galaxyDeviation)
	);
	double r, g, b;
	bv = galaxyRng->next(-0.4f, 2.0f);
	bv2rgb(r, g, b, bv);
	color = vec3(r, g, b);
	pole = glm::normalize(vec3(
		galaxyRng->next(-1.0f, 1.0f),
		galaxyRng->next(-1.0f, 1.0f),
		galaxyRng->next(-1.0f, 1.0f)
	));
	type = galaxyRng->next() % 4;
	radius = abs(galaxyRng->normal(1.0f, 0.5f)) + 0.1f;
	vboPos = 0;
	genId = id;
}



//GalaxyCluster

GalaxyCluster::GalaxyCluster(uint seed, GridGalaxies* gl) {
	clusterRng = new SeededRng(seed);

	centrePosition = vec3(
		clusterRng->normal(0.0f, gl->clusterPosDeviation),
		clusterRng->normal(0.0f, gl->clusterPosDeviation),
		clusterRng->normal(0.0f, gl->clusterPosDeviation));

	numOfGalaxy = clusterRng->next(gl->clusterSizeMin, gl->clusterSizeMax);

	maxDistance = 0.0f;
	galaxyObject.reserve(numOfGalaxy);
	grid = gl;

	for (uint i = 0; i < numOfGalaxy; i++) {
		galaxyObject.push_back(new Galaxy(clusterRng->next(), this, grid->genIds));
		grid->genIds++;
		maxDistance = glm::max(glm::length(galaxyObject[i]->offset), maxDistance);
	}
}

void GalaxyCluster::onEnterRange() {

}

void GalaxyCluster::onExitRange() {

}

void GalaxyCluster::onUpdateInRange() {

	for (Galaxy* gx : galaxyObject) {
		if (glm::distance(grid->TranslationTypeA::gPos(),
			centrePosition + gx->offset) < gx->radius * 1.0) {
			if (gx->visable == true) {
				gx->visable = false;
				gx->renderUpdate = true;
				renderUpdate = true;
				grid->renderUpdate = true;
				grid->loadGalaxy(gx);
			}
		}
		else {
			if (gx->visable == false) {
				gx->visable = true;
				gx->renderUpdate = true;
				renderUpdate = true;
				grid->renderUpdate = true;
				grid->unloadGalaxy(gx);
			}
		}
	}

}

void GalaxyCluster::onUpdate() {

}



//GridGalaxies

float GridGalaxies::calUpdateTick(vec3 obj, vec3 mov, vec3 speed, float rad, float maxAcc, float maxSpeed) {
	if (glm::distance(obj, mov) <= rad) return 0.0f;
	float m = abs(maxAcc);
	float tx = 0;
	if (abs(mov.x - obj.x) > rad) {
		if (mov.x > obj.x) {
			m = -m;
			obj.x += rad;
		}
		else obj.x -= rad;
		tx = std::max({
			(-speed.x + sqrt(pow(speed.x, 2) + 2 * m * (obj.x - mov.x))) / m,
			(-speed.x - sqrt(pow(speed.x, 2) + 2 * m * (obj.x - mov.x))) / m,
			abs(mov.x - obj.x) / maxSpeed });
	}
	if (!(tx >= 0)) {
		tx = 0;
	}
	m = abs(maxAcc);
	float ty = 0;
	if (abs(mov.y - obj.y) > rad) {
		if (mov.y > obj.y) {
			m = -m;
			obj.y += rad;
		}
		else obj.y -= rad;
		ty = std::max({
			(-speed.y + sqrt(pow(speed.y, 2) + 2 * m * (obj.y - mov.y))) / m,
			(-speed.y - sqrt(pow(speed.y, 2) + 2 * m * (obj.y - mov.y))) / m,
			abs(mov.y - obj.y) / maxSpeed });
	}
	if (!(ty >= 0)) {
		ty = 0;
	}
	m = abs(maxAcc);
	float tz = 0;
	if (abs(mov.z - obj.z) > rad) {
		if (mov.z > obj.z) {
			m = -m;
			obj.z += rad;
		}
		else obj.z -= rad;
		tz = std::max({
			(-speed.z + sqrt(pow(speed.z, 2) + 2 * m * (obj.z - mov.z))) / m,
			(-speed.z - sqrt(pow(speed.z, 2) + 2 * m * (obj.z - mov.z))) / m,
			abs(mov.z - obj.z) / maxSpeed });
	}
	if (!(tz >= 0)) {
		tz = 0;
	}
	float r = std::max({ tx, ty, tz });
	return r;
}

GridGalaxies::GridGalaxies(uint gridId) : TranslationTypeA(), GridBase() {
	type = 0;
	gdId = gridId;
	sMaxAcc(0.002f);
	sMaxVol(2.0f);
}

void GridGalaxies::newData() {
	gridRng = new SeededRng(global->worldManager->worldGenRng->next());
	childGdIds.clear();

	for (int i = 0; i < numOfCluster; i++) {
		GalaxyCluster* cluster = new GalaxyCluster(gridRng->next(), this);
		clusters.push_back(cluster);
	}

	genIdToGridIdLookup.clear();
}

void GridGalaxies::loadData(std::vector<char> data) {
	//load rng data and genIdToGridIdLookup table and childGdIds
}

void GridGalaxies::update() {
	if (global->playerLevel == 0) {
		TranslationTypeA::update();

		vec3 pos = TranslationTypeA::gPos();
		vec3 vol = TranslationTypeA::gVol();
		//Update Glaaxies Cluster with calUpdateTick() method
		uint counter = 0;
		tick += 1;
		auto it = std::vector<GalaxyCluster*>::iterator();
		for (it = clusters.begin(); it != clusters.end(); it++) {
			if ((**it).updateTick < tick || (**it).inRange) {
				counter++;
				if (!(**it).inRange && glm::distance((**it).centrePosition, pos) < (**it).maxDistance) {
					(**it).inRange = true;
					(**it).onEnterRange();
					inRange.push_back(*it);
				}
				else if ((**it).inRange && glm::distance((**it).centrePosition, pos) >= (**it).maxDistance) {
					(**it).inRange = false;
					(**it).onExitRange();
					inRange.remove(*it);
				}
				if (!(**it).inRange) {
					(**it).onUpdate();
					float ts = calUpdateTick((**it).centrePosition, pos, vol, (**it).maxDistance, maxAcc, maxVol);

					int ti = ts;
					if (ti < 0) {
						logger << "ERROR: " << ts << "\n";
					}

					(**it).updateTick = tick + ti;
				}
				else {
					(**it).onUpdateInRange();
				}
			}
		}
		logger << "Total ticked: " << counter << "\n";
	}
	else {
		TranslationTypeA::sVol(vec3(0.0f));
		TranslationTypeA::update();
	}
}

void GridGalaxies::unload() {}

void GridGalaxies::aAcc(vec3 acc)
{
	if (global->playerLevel == 0) {
		TranslationTypeA::aAcc(acc);
	}
}

void GridGalaxies::loadGalaxy(Galaxy* gx) {
	global->playerLevel = 1;
	auto it = genIdToGridIdLookup.find(gx->genId);
	uint gdId;
	global->worldManager->newGridArgs = (void*)gx;
	if (it == genIdToGridIdLookup.end()) {
		gdId = global->worldManager->newGrid(1);
		genIdToGridIdLookup.insert_or_assign(gx->genId, gdId);
	} else {
		gdId = it->second;
		global->worldManager->loadGrid(1, gdId);
	}
	global->worldManager->newGridArgs = nullptr;

	global->main->gridType = 1;
	global->main->gridId = gdId;
	auto gg = (GridGalaxy*)global->worldManager->getGridNoLoad(1, gdId);
	gg->ugdId = GridGalaxies::gdId;
	gg->hasUpperGrid = true;
	vec3 a = vec3(0.0, 0.0, 1.0);
	vec3 b = gx->pole;
	vec3 v = glm::cross(b, a);
	float angle = acos(glm::dot(b, a));
	mat4 rotmat = glm::rotate(mat4(1.0f),angle, v);
	vec3 pos = -(gx->offset + gx->gc->centrePosition - TranslationTypeA::gPos()) * 100000.0f;
	pos = rotmat * vec4(pos,0.0f);
	logger << "Set starting position to: \n";
	printVec3(pos);
	gg->TranslationTypeA::sPos(pos);

	gg->TranslationTypeA::matRot = rotmat * TranslationTypeA::matRot;
	gg->TranslationTypeA::vol = rotmat * vec4(TranslationTypeA::vol * 100000.0f, 0.0f);

	TranslationTypeA::vol = vec3(0.0f);

	childGdIds.push_back(gdId);
}

void GridGalaxies::unloadGalaxy(Galaxy* gx) {
	global->playerLevel = 0;
	global->main->gridType = 0;
	global->main->gridId = gdId;
	vec3 a = vec3(0.0, 0.0, 1.0);
	vec3 b = gx->pole;
	vec3 v = glm::cross(b, a);
	float angle = acos(glm::dot(b, a));
	mat4 rotmat = glm::rotate(mat4(1.0f), angle, v);
	vec3 pos = rotmat * vec4(-(gx->offset + gx->gc->centrePosition - TranslationTypeA::gPos()) * 100000.0f, 1.0f);
	auto gg = (GridGalaxy*)global->worldManager->getGridNoLoad(1, genIdToGridIdLookup[gx->genId]);
	vec3 offset = (gg->TranslationTypeA::pos - pos)/100000.0f;
	mat4 inverseRot = glm::inverse(rotmat);
	offset = inverseRot * vec4(offset, 0.0f);
	TranslationTypeA::pos += offset;
	TranslationTypeA::vol = inverseRot * vec4(gg->TranslationTypeA::vol / 100000.0f, 0.0f);
	childGdIds.remove(genIdToGridIdLookup[gx->genId]);
}



/*-----------------Render Data------------------*/



//GridGalaxiesRenderer

GridGalaxiesRenderer::GridGalaxiesRenderer(Anchor* a) :
	ShaderProgram("shader/stars/galaxies.vert", "shader/stars/galaxies.geom", "shader/stars/galaxies.frag"),
	GridBaseRenderer(a) {
	gdId = global->worldManager->getGdIdByAnchor(a, 0);
	if (gdId == uint(-1)) throw;
}

void GridGalaxiesRenderer::initData() {
	auto g = (GridGalaxies*)global->worldManager->getGrid(0, gdId);
	if (g == nullptr) throw;
	auto buffer = std::list<GalaxyData>();

	logger << g->clusters.size() << "\n";
	for (GalaxyCluster* gc : g->clusters) {

		for (Galaxy* gx : gc->galaxyObject) {
			gx->vboPos = buffer.size() * sizeof(GalaxyData) / sizeof(float);
			vec3 pos = gx->offset + gc->centrePosition;
			auto temp = GalaxyData();
			temp.posX = pos.x;
			temp.posY = pos.y;
			temp.posZ = pos.z;
			auto col = gx->color;
			temp.colorR = col.x;
			temp.colorG = col.y;
			temp.colorB = col.z;
			auto pol = gx->pole;
			temp.poleX = pol.x;
			temp.poleY = pol.y;
			temp.poleZ = pol.z;
			temp.type = gx->type;
			if (gx->visable)
				temp.radius = gx->radius;
			else temp.radius = 0.0f;
			gx->renderUpdate = false;
			buffer.push_back(temp);
		}
		gc->renderUpdate = false;
	}
	g->renderUpdate = false;
	logger << "Setting up vbo data..." << "\n";
	logger << buffer.size() << "\n";

	data.clear();
	data.reserve(buffer.size() * DATASIZE);
	for (auto& bf : buffer) {
		//logger << "[";
		for (uint i = 0; i < DATASIZE; i++) {
			data.push_back(*((float*)(&bf) + i));
			//logger << *((float*)(&bf) + i) << ", ";
		}
		//logger << "], " << "\n";
	}

	logger << "Data Buffer size: " << data.size() << "\n";


	glGenVertexArrays(1, &vaoId);
	glBindVertexArray(vaoId);
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GalaxyData), (void*)0);
	glEnableVertexAttribArray(0); //Position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GalaxyData), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); //Color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GalaxyData), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2); //Pole
	glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(GalaxyData), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3); //Type
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(GalaxyData), (void*)(10 * sizeof(float)));
	glEnableVertexAttribArray(4); //Radius

	logger << "\n" << "vbo data setup completed." << "\n";
}

bool GridGalaxiesRenderer::needZoomIn()
{
	return (anchor->gridType > 0);
}

bool GridGalaxiesRenderer::needZoomOut()
{
	return false;
}

void GridGalaxiesRenderer::prerender()
{
	if (data.size() == 0) {
		initData();
	}
	else {
		auto g = (GridGalaxies*)global->worldManager->getGrid(0, gdId);
		if (g == nullptr) return;

		if (g->renderUpdate) {
			for (auto gc : g->clusters) {
				if (gc->renderUpdate) {
					for (auto gx : gc->galaxyObject) {
						if (gx->renderUpdate) {
							auto pos = gx->offset + gc->centrePosition;
							auto o = gx->vboPos;
							data[o + 0] = pos.x;
							data[o + 1] = pos.y;
							data[o + 2] = pos.z;
							auto col = gx->color;
							data[o + 3] = col.x;
							data[o + 4] = col.y;
							data[o + 5] = col.z;
							auto pol = gx->pole;
							data[o + 6] = pol.x;
							data[o + 7] = pol.y;
							data[o + 8] = pol.z;
							data[o + 9] = *(float*)(&gx->type);
							if (gx->visable)
								data[o + 10] = gx->radius;
							else {
								data[o + 10] = 0.0f;
							}

							glBindBuffer(GL_ARRAY_BUFFER, vboId);
							glBufferSubData(GL_ARRAY_BUFFER, o * sizeof(float), sizeof(GalaxyData), data.data() + o);

							gx->renderUpdate = false;
						}
					}
					gc->renderUpdate = false;
				}
			}
			g->renderUpdate = false;
		}
	}

}

void GridGalaxiesRenderer::render()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	enable();

	GridGalaxies* g = (GridGalaxies*)global->worldManager->getGrid(0, gdId);

	

	setMat4("viewportMatrix", g->gMatView());
	setMat4("projectionMatrix", g->gMatProj());



	glBindVertexArray(vaoId);
	glDrawArrays(GL_POINTS, 0, ulint(data.size()) / DATASIZE);

}