#include "Physic.h"
#include "ModelBase.h"
#include "Global.h"


void PhysxErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file,
    int line)
{
    std::string errorCode = std::bitset<8>(code).to_string();
    logger << "-------Physx Error-------\n";
    logger << "Info|Warning|Param|Oper|Mem|Intern|Abort|PerfWarn\n";
    logger << errorCode << "\n";
    logger << std::string(message) << "\n";
    logger << "At Line " << std::to_string(line) << " of File:\n";
    logger << std::string(file) << "\n";
}

static physx::PxFilterFlags PhysxFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
    pairFlags = (physx::PxPairFlag::eCONTACT_DEFAULT | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND);
    return physx::PxFilterFlag::eNOTIFY;
}

void PhysxEventCallback::deleteActor(Actor* remove)
{
    //TODO: Delete callback
}

void PhysxEventCallback::addToGroup(physx::PxShape* source, uint groupID)
{
    source->setQueryFilterData(physx::PxFilterData(groupID, 0, 0, 0));
}

void PhysxEventCallback::addOnContactAA(Actor* source, Actor* target, ContactCallback callback)
{
}
void PhysxEventCallback::addOnContactAGa(Actor* source, uint type, ContactCallback callback)
{
}
void PhysxEventCallback::addOnContactAa(Actor* source, ContactCallback callback)
{
}
void PhysxEventCallback::addOnContactAT(Actor* source, Actor* trigger, TriggerCallback callback)
{
}
void PhysxEventCallback::addOnContactAGt(Actor* source, uint type, TriggerCallback callback)
{
}
void PhysxEventCallback::addOnContactAt(Actor* source, TriggerCallback callback)
{
}
void PhysxEventCallback::addOnContactTA(Actor* source, Actor* target, TriggerCallback callback)
{
}
void PhysxEventCallback::addOnContactTGa(Actor* source, uint type, TriggerCallback callback)
{
}
void PhysxEventCallback::addOnContactTa(Actor* source, TriggerCallback callback)
{
}

void PhysxEventCallback::addOnWake(Actor* source, EventCallback callback)
{
    source->a->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
    wake.push_back({ source, callback });
}
void PhysxEventCallback::addOnSleep(Actor* source, EventCallback callback)
{
    source->a->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
    sleep.push_back({ source, callback });
}

void PhysxEventCallback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
}
void PhysxEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
{
    logger << "Physx: OnWakeCallback: " << count << "\n";
    for (uint i = 0; i < count; i++) {
        Actor* actor = (Actor*)actors[i]->userData;
        for (auto& callback : wake) {
            if (std::get<0>(callback) == actor) {
                std::get<1>(callback)(actor);
            }
        }
    }
}
void PhysxEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
    logger << "Physx: OnSleepCallback: " << count << "\n";
    for (uint i = 0; i < count; i++) {
        Actor* actor = (Actor*)actors[i]->userData;
        for (auto& callback : sleep) {
            if (std::get<0>(callback) == actor) {
                std::get<1>(callback)(actor);
            }
        }
    }
}
void PhysxEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    logger << "Physx: OnContactCallback: " << nbPairs << "\n";
}
void PhysxEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
}
void PhysxEventCallback::onAdvance(physx::PxRigidBody const* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
{
}

PhysxEventCallback::~PhysxEventCallback() {}



physx::PxFilterFlags PhysxFilterCallback::pairFound(physx::PxU32 pairID, physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, const physx::PxActor* a0, const physx::PxShape* s0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, const physx::PxActor* a1, const physx::PxShape* s1, physx::PxPairFlags& pairFlags)
{
    logger << "Physx: PairFoundCallback: " << pairID << "\n";
    pairFlags = (physx::PxPairFlag::eCONTACT_DEFAULT | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND);
    return physx::PxFilterFlag::eNOTIFY;
}

void PhysxFilterCallback::pairLost(physx::PxU32 pairID, physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, bool objectRemoved)
{
    logger << "Physx: PairLostCallback: " << pairID << "\n";
}

bool PhysxFilterCallback::statusChange(physx::PxU32& pairID, physx::PxPairFlags& pairFlags, physx::PxFilterFlags& filterFlags)
{
    //logger << "Physx: PairChangeCallback: " << pairID << "\n";
    return false;
}

PhysxFilterCallback::~PhysxFilterCallback()
{
}



Actor::~Actor() {}
void Actor::update() {}

ActorStatic::ActorStatic(ModelStatic* m, physx::PxShape* s) {
    isStatic = true;
    model = m;
    a = physx::PxCreateStatic(*PhysicAPI::getPhysics(),
        physx::PxTransform(_CastGLPX(m->gPos()), _CastGLPXrot(m->gRot())),
        *s);
    a->userData = this;
}

void ActorStatic::update() {
    model->pxUpdate();
}

ActorStatic::~ActorStatic()
{
    a->release();
}

static void ActorDynamicOnSleep(Actor* a) {
    ((ModelDynamic*)((ActorDynamic*)a)->model)->pxSleep();
}
static void ActorDynamicOnWake(Actor* a) {
    ((ModelDynamic*)((ActorDynamic*)a)->model)->pxWake();
}

ActorDynamic::ActorDynamic(ModelDynamic* m, physx::PxShape* s, float density) {
    isStatic = false;
    model = m;
    auto rotat = _CastGLPXrot(m->gRot());
    a = physx::PxCreateDynamic(*PhysicAPI::getPhysics(),
        physx::PxTransform(_CastGLPX(m->gPos()), rotat),
        *s, density);
    a->userData = this;
    PhysicAPI::eventCallback.addOnSleep(this, ActorDynamicOnSleep);
    PhysicAPI::eventCallback.addOnWake(this, ActorDynamicOnWake);
}

void ActorDynamic::update() {
    model->pxUpdate();
}

ActorDynamic::~ActorDynamic()
{
    a->release();
}

Scene::Scene(float sc, vec3 gravity)
{
    scale = sc;
    s = PhysicAPI::newScene(gravity, this);
}

void Scene::addActor(Actor* actor)
{
    s->addActor(*actor->a);
    actors.push_back(actor);
}








const float LENGTH = 1;
const float SPEED = 10;

physx::PxPhysics* PhysicAPI::physics = nullptr;
physx::PxCooking* PhysicAPI::cooking = nullptr;
physx::PxFoundation* PhysicAPI::globalSetting = nullptr;
physx::PxCpuDispatcher* PhysicAPI::cpuDispatcher = nullptr;
physx::PxCudaContextManager* PhysicAPI::cudaManager = nullptr;

std::vector<Scene*> PhysicAPI::scenes = std::vector<Scene*>();

physx::PxDefaultAllocator PhysicAPI::defaultAllocator = physx::PxDefaultAllocator();
PhysxErrorCallback PhysicAPI::errorCallback = PhysxErrorCallback();
PhysxEventCallback PhysicAPI::eventCallback = PhysxEventCallback();
PhysxFilterCallback PhysicAPI::filterCallback = PhysxFilterCallback();

void PhysicAPI::init()
{
    globalSetting = PxCreateFoundation(PX_PHYSICS_VERSION, defaultAllocator,
        errorCallback);
    if (!globalSetting) {
        logger << "Physx Default Foundation creation failed.\n";
        throw;
    }
    auto cudaDesc = physx::PxCudaContextManagerDesc();
    cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(16);
    cudaManager = PxCreateCudaContextManager(*globalSetting, cudaDesc);


    physx::PxTolerancesScale scale;
    scale.length = LENGTH;
    scale.speed = SPEED;
    physx::PxCookingParams params{scale};
    params.buildGPUData = true;
    params.buildTriangleAdjacencies = true;

    cooking = PxCreateCooking(PX_PHYSICS_VERSION, *globalSetting, params);
    if (!cooking) {
        logger << "Physx Mesh Cooking Foundation creation failed.\n";
        throw;
    }
    physics = PxCreatePhysics(PX_PHYSICS_VERSION, *globalSetting, scale, true);
    if (!physics) {
        logger << "Physx Default Foundation creation failed.\n";
        throw;
    }

}

void PhysicAPI::cleanup()
{
    logger << "PhysxAPI: Cleanup called. Collecting all scene's simulation...\n";
    for (auto s : scenes) {
        s->s->fetchResults(true);
    }
    logger << "PhysxAPI: All simulations collected. Cleaning up fondation...\n";
    cooking->release();
    physics->release();
    globalSetting->release();
    logger << "PhysxAPI: All cleanup done.\n";
}

physx::PxScene* PhysicAPI::newScene(vec3 gravity, Scene* sceneObj)
{
    auto sd = new physx::PxSceneDesc(getPhysics()->getTolerancesScale());
    sd->simulationEventCallback = &eventCallback;
    //sd->sanityBounds = physx::PxBounds3(_CastGLPX(vec3(-100.f)), _CastGLPX(vec3(100.f));
    sd->cpuDispatcher = cpuDispatcher;
    sd->filterShader = PhysxFilterShader;
    sd->flags = physx::PxSceneFlags() | physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sd->limits = physx::PxSceneLimits();
    sd->staticStructure = physx::PxPruningStructureType::eSTATIC_AABB_TREE;
    sd->gravity = _CastGLPX(gravity);
    sd->cudaContextManager = cudaManager;
    sd->filterCallback = &filterCallback;

    auto s = getPhysics()->createScene(*sd);
    if (s == nullptr) {
        logger << "Physx Scene creation failed.\n";
        throw;
    }
    s->userData = sceneObj;
    scenes.push_back(sceneObj);
    s->simulate(0.00001f);
    return s;
}

physx::PxPhysics* PhysicAPI::getPhysics()
{
    return physics;
}

void PhysicAPI::tickAll(ulint delta)
{
    std::vector<uint> update;
    for (uint i = 0; i < scenes.size(); i++) {
        Scene* scene = scenes[i];
        if (scene->enabled) {
            scene->simStepLeftover += delta;
            if (scene->simStepLeftover >= scene->simStep)
                update.push_back(i);
        }
    }

    //Loop tick all scenes
    uint loopCount = 0;
    bool needNextStep = true;
    bool tickMissWarning = false;
    bool tickTryWarning = false;
    while (needNextStep) {
        loopCount++;
        needNextStep = false;
        for (uint i : update) {
            Scene* scene = scenes[i];
            if (scene->simStepLeftover >= scene->simStep &&
                scene->s->checkResults()) {
                scene->s->fetchResults(true); //for new it's true, a backup
                //logger << "Physic ticks: " << delta <<
                //    " , Scene ticks: " << scene->simStep * scene->simSpeed << "\n";

                //TODO: Bug fix for actors that went to sleep does not update to set its speed to 0

                uint size;
                auto uP = scene->s->getActiveActors(size);
                auto updated = std::vector<physx::PxActor*>(uP, uP + size);
                for (auto a : updated) {
                    ((Actor*)(a->userData))->update();
                }

                auto tester = scene->s->getSimulationEventCallback();


                scene->s->simulate(double(scene->simStep) / NS_PER_S_F * double(scene->simSpeed));
                scene->simStepLeftover -= scene->simStep;

                if (scene->simStepLeftover >= scene->simStep) {
                    needNextStep = true;
                    if (scene->simStepLeftover > TICK_MISS_WARNING) {
                        if (scene->simStepLeftover > TICK_MISS_EXCAPE) {
                            scene->simStepLeftover = 0;
                            tickMissWarning = false;
                            logger << "Physics Error: Scene tick buffer reached Limit. Clearing it!\n";
                        }
                        else {
                            tickMissWarning = true;
                        }
                    }
                }
            }
        }
        if (loopCount > TICK_TRY_WARNING) {
            if (loopCount > TICK_TRY_EXCAPE) {
                logger << "Physics Error: Tick try reached Limit. Excaping this tick event!\n";
                needNextStep = false;
                tickTryWarning = false;
            }
            else {
                tickTryWarning = true;
            }
        }
    }

    //if (tickMissWarning) logger << "Physics Warning: Scene tick buffer reached warning threshold!\n";
    //if (tickTryWarning) logger << "Physics Warning: Tick try reached warning threshold!\n";
}

