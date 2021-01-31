#pragma once

#include <string>
#include <bitset>
#include <Vector>

#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <PxSimulationEventCallback.h>
#include <physx/PxPhysicsAPI.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Definer.h"
#include "ModelBase.h"

static const ulint SIM_TICK_SPEED = (1. / 60. * NS_PER_S_F);
static const uint TICK_TRY_WARNING = 10;
static const uint TICK_TRY_EXCAPE = 20;
static const ulint TICK_MISS_WARNING = 1000000000;
static const ulint TICK_MISS_EXCAPE = 5000000000;

class Actor;

class PhysxErrorCallback : public physx::PxErrorCallback {
public:
	virtual void reportError(physx::PxErrorCode::Enum c, const char* m, const char* f, int l);
};

static physx::PxFilterFlags PhysxFilterShader(physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags,
	const void* constantBlock,
	physx::PxU32 constantBlockSize
);

class PhysxEventCallback : public physx::PxSimulationEventCallback {
public:
	using ContactCallback = void (*)(Actor*, Actor*, physx::PxContactPair);
	using TriggerCallback = void (*)(Actor*, Actor*, physx::PxTriggerPair);
	using EventCallback = void (*)(Actor*);

	void deleteActor(Actor* remove);

	static void addToGroup(physx::PxShape* source, uint groupID);

	/*------Contact Callback------*/
	//AA
	void addOnContactAA(Actor* source, Actor* target, ContactCallback callback);
	void addOnContactAGa(Actor* source, uint type, ContactCallback callback);
	void addOnContactAa(Actor* source, ContactCallback callback);
	//AT
	void addOnContactAT(Actor* source, Actor* trigger, TriggerCallback callback);
	void addOnContactAGt(Actor* source, uint type, TriggerCallback callback);
	void addOnContactAt(Actor* source, TriggerCallback callback);
	//TA
	void addOnContactTA(Actor* source, Actor* target, TriggerCallback callback);
	void addOnContactTGa(Actor* source, uint type, TriggerCallback callback);
	void addOnContactTa(Actor* source, TriggerCallback callback);

	/*------Other Callback------*/
	void addOnWake(Actor* source, EventCallback callback);
	void addOnSleep(Actor* source, EventCallback callback);

	//TODO: addOnAdvance(), addOnBreak(constraint)

	/*------Physx Callback Linker------*/
	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count);
	virtual void onWake(physx::PxActor** actors, physx::PxU32 count);
	virtual void onSleep(physx::PxActor** actors, physx::PxU32 count);
	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);
	virtual void onAdvance(physx::PxRigidBody const* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count);
	
	~PhysxEventCallback();


private:
	
	std::vector <std::tuple<Actor*, Actor*, ContactCallback>> AA;
	std::vector <std::tuple<Actor*, uint, ContactCallback>> AGa;
	std::vector <std::tuple<Actor*, ContactCallback>> Aa;
	std::vector <std::tuple<Actor*, Actor*, TriggerCallback>> AT;
	std::vector <std::tuple<Actor*, uint, TriggerCallback>> AGt;
	std::vector <std::tuple<Actor*, TriggerCallback>> At;
	std::vector <std::tuple<Actor*, Actor*, TriggerCallback>> TA;
	std::vector <std::tuple<Actor*, uint, TriggerCallback>> TGa;
	std::vector <std::tuple<Actor*, TriggerCallback>> Ta;

	std::vector <std::tuple<Actor*, EventCallback>> wake;
	std::vector <std::tuple<Actor*, EventCallback>> sleep;
};

class PhysxFilterCallback : public physx::PxSimulationFilterCallback {
public:
	virtual physx::PxFilterFlags pairFound(physx::PxU32 pairID,
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		const physx::PxActor* a0, const physx::PxShape* s0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		const physx::PxActor* a1, const physx::PxShape* s1,
		physx::PxPairFlags& pairFlags
	);
	virtual void pairLost(physx::PxU32 pairID,
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1, bool objectRemoved
	);
	virtual bool statusChange(physx::PxU32& pairID,
		physx::PxPairFlags& pairFlags, physx::PxFilterFlags& filterFlags
	);
	virtual ~PhysxFilterCallback();
};


class ModelPhysic;
class ModelStatic;
class ModelDynamic;

class Actor
{
public:
	bool isStatic;
	ModelPhysic* model;
	physx::PxActor* a;
	virtual void update();
	virtual ~Actor();
};

class ActorStatic : public Actor
{
public:
	ActorStatic(ModelStatic* m, physx::PxShape* s);
	virtual void update();
	virtual ~ActorStatic();
};

class ActorDynamic : public Actor
{
public:
	ActorDynamic(ModelDynamic* m, physx::PxShape* s, float density);
	virtual void update();
	virtual ~ActorDynamic();
};



class Scene
{
public:
	bool enabled = true;
	float simSpeed = 1.0f;
	ulint simStep = SIM_TICK_SPEED;
	ulint simStepLeftover = 0;
	float scale;

	Scene(float scale, vec3 gravity);
	void addActor(Actor* actor);
	physx::PxScene* s;
	std::vector<Actor*> actors;
};



class PhysicAPI
{
public:

	static void init();
	static void cleanup();

	static physx::PxPhysics* getPhysics();

	static void tickAll(ulint tickDelta);

	static physx::PxScene* newScene(vec3 gravity, Scene* sceneObj);

	static physx::PxPhysics* physics;
	static physx::PxCooking* cooking;
	static physx::PxFoundation* globalSetting;
	static physx::PxCpuDispatcher* cpuDispatcher;
	static physx::PxCudaContextManager* cudaManager;

	static std::vector<Scene*> scenes;

	static physx::PxDefaultAllocator defaultAllocator;
	static PhysxErrorCallback errorCallback;
	static PhysxEventCallback eventCallback;
	static PhysxFilterCallback filterCallback;

};
