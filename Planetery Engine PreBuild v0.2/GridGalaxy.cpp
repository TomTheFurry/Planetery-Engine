
#include <iostream>
#include <algorithm>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "Global.h"

#include "GridGalaxy.h"

/*
* 1 unit (SU) is ~ 1 light year or 0.00001 GU
* Galaxy average size ~ 1 GU or 100,000 SU (1,000 SU thick (with 10,000 SU thick in the middle))
* Planetary system average size ~ 2 SU
* Planetary system average distance ~ 7 SU in Solar System area, ~ 4 SU in Centre of galaxy
* Average num of system ~ 100,000,000
* 1 GSU is ~ 1,000 SU or 0.01 GU (GSU = Galaxy Sector Unit)
* Galaxy average sector number ~ 100*100*10 (100,000)
* Max speed: 4 SU
* Max acceleration: 0.004 SU
*/



//Star

Star::Star(uint seed, GridGalaxy* gl) {
	starRng = new SeededRng(seed);

	pos = vec3(
		starRng->normal(0.0f, gl->clusterPosDeviation),
		starRng->normal(0.0f, gl->clusterPosDeviation),
		starRng->normal(0.0f, gl->clusterPosDeviationThickness));
	type = (uint)starRng->next(0.0f, 10.0f);
	if (type < 5) {
		color = vec3(
			starRng->next(-0.2f, 0.2f),
			starRng->next(-0.2f, 0.2f),
			starRng->next(-0.2f, 0.2f)) + gl->mainColor;
	} else {
		double r, g, b;
		bv2rgb(r, g, b, starRng->next(-0.4, 2.0));
		color = vec3(r, g, b);
	}
	brightness = starRng->normal(75.0, 50.0);

	maxDistance = 2.0f;
	grid = gl;
	genId = grid->genIds;
	grid->genIds++;
}

void Star::onEnterRange() {

}

void Star::onExitRange() {

}

void Star::onUpdateInRange() {

}

void Star::onUpdate() {

}



//GridGalaxies

float GridGalaxy::calUpdateTick(vec3 obj, vec3 mov, vec3 speed, float rad, float maxAcc, float maxSpeed) {
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

GridGalaxy::GridGalaxy(uint gridId) : TranslationTypeA(), GridBase() {
	type = 1;
	gdId = gridId;
	sMaxAcc(0.8f);
	sMaxVol(400.0f);
	ugx = (Galaxy*)global->worldManager->newGridArgs;
	if (ugx == nullptr) return;
	mainColor = ugx->color;
	clusterPosDeviation *= ugx->radius;
	numOfStar *= ugx->radius;
	maxRange = ugx->radius * 100000.0;
}

GridGalaxy::~GridGalaxy()
{
	auto g = (GridGalaxies*)global->worldManager->getGridNoLoad(0, ugdId);
	g->unloadGalaxy(ugx);
	for (auto st : stars) delete st;
}

void GridGalaxy::newData() {
	logger << "Generating new GridGalaxy...\n";
	gridRng = new SeededRng(global->worldManager->worldGenRng->next());

	std::list<pQueueEntry> q;
	for (uint i = 0; i < numOfStar; i++) {
		Star* star = new Star(gridRng->next(), this);
		stars.push_back(star);
		q.push_back(pQueueEntry(0.0, star));
	}
	logger << "Generating updateQueue...\n";
	updateQueue = pqg(q.begin(), q.end());
	logger << "Generation completed.\n";
}

void GridGalaxy::loadData(std::vector<char> data) {
	//load rng data
}

void GridGalaxy::update() {
	global->worldManager->getGridNoLoad(0, ugdId)->update();
	TranslationTypeA::update();

	vec3 pos = TranslationTypeA::gPos();
	vec3 vol = TranslationTypeA::gVol();

	if (glm::length(pos) > maxRange) {
		unload();
		return;
	}

	//Update Star with calUpdateTick() method
	uint counter = 0;
	tick += 1;
	pqe e = updateQueue.top();
	while (e.order < tick) {
		Star* star = (Star*)e.pointer;
		counter++;
		if (glm::distance(star->pos, pos) < star->maxDistance) {
			if (!star->inRange) {
				star->inRange = true;
				star->onEnterRange();
				inRange.push_back(star);
			}
			star->onUpdateInRange();
			updateQueue.pop();
			e.order = double(tick) + 1;
			updateQueue.push(e);
		} else {
			if (star->inRange) {
				star->inRange = false;
				star->onExitRange();
				inRange.remove(star);
			}
			star->onUpdate();
			double timeout = calUpdateTick(star->pos, pos, vol, star->maxDistance, maxAcc, maxVol);
			if (timeout <= 1) {
				if (timeout < 0) {
					logger << "ERROR: Timeout calculation result negative: " << timeout;
				}
				updateQueue.pop();
				e.order = double(tick) + 1;
				updateQueue.push(e);
			} else {
				updateQueue.pop();
				e.order = double(tick) + timeout;
				updateQueue.push(e);
			}
		}
		
		e = updateQueue.top();
	}
	logger << "Total ticked: " << counter << " at tick " << tick << "\n";
}

void GridGalaxy::unload() {
	delete this;
}

void GridGalaxy::loadStar(Star* gx) {

	//load star When in range
}

void GridGalaxy::unloadStar(Star* gx) {

}



/*-----------------Render Data------------------*/



//GridGalaxiesRenderer

GridGalaxyRenderer::GridGalaxyRenderer(Anchor* a) :
	ShaderProgram("shader/stars/galaxy.vert", "shader/stars/galaxy.geom", "shader/stars/galaxy.frag"),
	GridBaseRenderer(a) {
	mode2 = new ShaderProgram("shader/stars/galaxy2.vert", "shader/stars/galaxy2.frag");
}

void GridGalaxyRenderer::initData() {
	auto g = (GridGalaxy*)global->worldManager->getGrid(1, gdId);
	if (g == nullptr) throw;
	auto buffer = std::list<StarData>();

	logger << "Init GridGalaxyRenderer with size " << g->stars.size() << "\n";
	for (Star* st : g->stars) {
		st->vboPos = buffer.size() * sizeof(StarData) / sizeof(float);
		vec3 pos = st->pos;
		auto temp = StarData();
		temp.posX = pos.x;
		temp.posY = pos.y;
		temp.posZ = pos.z;
		auto col = st->color;
		temp.colorR = col.x;
		temp.colorG = col.y;
		temp.colorB = col.z;
		temp.type = st->type;
		if (st->visable)
			temp.brightness = st->brightness;
		else temp.brightness = 0.0f;
		st->renderUpdate = false;
		buffer.push_back(temp);
	}
	g->renderUpdate = false;
	logger << "Setting up vbo data with buffer size " << buffer.size() << "...\n";

	data.clear();
	data.reserve(buffer.size() * DATASIZE);
	for (auto& bf : buffer) {
		//std::cout << "[";
		for (uint i = 0; i < DATASIZE; i++) {
			data.push_back(*((float*)(&bf) + i));
			//std::cout << *((float*)(&bf) + i) << ", ";
		}
		//std::cout << "], " << std::endl;
	}

	logger << "Data Buffering completed. Linking it to glBuffer..." << "\n";

	glGenVertexArrays(1, &vaoId);
	glBindVertexArray(vaoId);
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(StarData), (void*)0);
	glEnableVertexAttribArray(0); //Position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(StarData), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); //Color
	glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(StarData), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2); //Type
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(StarData), (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(3); //Brightness

	logger << "\nBuffer setup completed.\n";
}

bool GridGalaxyRenderer::needZoomIn()
{
	return false;
}

bool GridGalaxyRenderer::needZoomOut()
{
	return (anchor->gridType < 1);
}

void GridGalaxyRenderer::prerender()
{
	glEnable(GL_MULTISAMPLE);
	if (data.size() == 0) {
		initData();
	}
	else {
		auto g = (GridGalaxy*)global->worldManager->getGrid(1, gdId);
		if (g == nullptr) return;

		if (g->renderUpdate) {
			for (auto st : g->stars) {
				if (st->renderUpdate) {
					logger << "Updating visual buffer data for object Star " << st->genId << "...";
					auto pos = st->pos;
					auto o = st->vboPos;
					data[o + 0] = pos.x;
					data[o + 1] = pos.y;
					data[o + 2] = pos.z;
					auto col = st->color;
					data[o + 3] = col.x;
					data[o + 4] = col.y;
					data[o + 5] = col.z;
					data[o + 6] = *(float*)(&st->type);
					if (st->visable)
						data[o + 7] = st->brightness;
					else {
						data[o + 7] = 0.0f;
					}

					glBindBuffer(GL_ARRAY_BUFFER, vboId);
					glBufferSubData(GL_ARRAY_BUFFER, o * sizeof(float), sizeof(StarData), data.data() + o);
					logger << "  Done.\n";
					st->renderUpdate = false;
				}
			}
			g->renderUpdate = false;
		}
	}
}

void GridGalaxyRenderer::render()
{
	switch (anchor->gridType)
	{
	case 0: //galaxies view
		//do something???

		break;

	case 1: //galaxy view
		GridGalaxy * g = (GridGalaxy*)global->worldManager->getGrid(1, gdId);

		//render galaxy mode 1

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		//glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		enable();

		setMat4("viewportMatrix", g->gMatView());
		setMat4("projectionMatrix", g->gMatProj());
		setFloat("cutout", cutout);

		glBindVertexArray(vaoId);
		glDrawArrays(GL_POINTS, 0, ulint(data.size()) / DATASIZE);

		//render galaxy mode 2

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);
		//glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		mode2->enable();

		mode2->setMat4("viewportMatrix", g->gMatView());
		mode2->setMat4("projectionMatrix", g->gMatProj());
		mode2->setFloat("cutout", cutout);
		mode2->setFloat("naer", global->nearView);
		mode2->setFloat("far", global->farView);

		glBindVertexArray(vaoId);
		glDrawArrays(GL_POINTS, 0, ulint(data.size()) / DATASIZE);
		break;
	}
	glDisable(GL_MULTISAMPLE);
}