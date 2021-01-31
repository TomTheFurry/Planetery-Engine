
#include "ModelBase.h"
#include "Global.h"
#include "RenderProgram.h"
#include "Geo.h"

void ModelDataStatic::init() {
    if (inited) return;
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, vertLength * sizeof(float), vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    if (indLength != 0) {
        glGenBuffers(1, &veoId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veoId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indLength * sizeof(float), ind, GL_STATIC_DRAW);
    }
}

const float static cubeVertices[] = {
    //3, 2
    //0, 1
    //     XYZ                BaseColor 
    //Front (-y)
    -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
    //Back (+y)
     1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 0.0f, 0.0f,
     //Left (-x)
     -1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
     -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
     -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
     -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
     //Right (+x)
      1.0f, -1.0f, -1.0f,  0.0f, 1.0f, 1.0f,
      1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
      1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
      1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
      //bottom (-z)
      -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
       1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
       1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
      -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
      //top (+z)
      -1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 0.0f,
       1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
       1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
      -1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f
};
const uint static cubeIndices[] = {
     0, 2, 3,   0, 1, 2,  //front
     4, 6, 7,   4, 5, 6,  //back
     8,10,11,   8, 9,10,  //left
    12,14,15,  12,13,14,  //right
    16,18,19,  16,17,18,  //bottom
    20,22,23,  20,21,22   //top
};
const float static convexVertices[] = {
    //3, 2
    //0, 1
    //XYZ                BaseColor 
    //X: -1    |    0    |    1              
    //    0    |  7   4  |    8        
    //  3   1  |         |  11  9      
    //    2    |  6   5  |    10             

    -1.0f,1.0f,0.0f,  0.0f,1.0f,0.5f,
    -1.0f,0.0f,1.0f,  0.0f,0.5f,1.0f,
    -1.0f,-1.0f,0.0f, 0.0f,0.0f,0.5f,
    -1.0f,0.0f,-1.0f, 0.0f,0.5f,0.0f,

    0.0f,1.0f,1.0f,   0.5f,1.0f,1.0f,
    0.0f,-1.0f,1.0f,  0.5f,0.0f,1.0f,
    0.0f,-1.0f,-1.0f, 0.5f,0.0f,0.0f,
    0.0f,1.0f,-1.0f,  0.5f,1.0f,0.0f,

    1.0f,1.0f,0.0f,   1.0f,1.0f,0.5f,
    1.0f,0.0f,1.0f,   1.0f,0.5f,1.0f,
    1.0f,-1.0f,0.0f,  1.0f,0.0f,0.5f,
    1.0f,0.0f,-1.0f,  1.0f,0.5f,0.0f
};
const uint static convexIndices[] = {
     //flat
     0, 1, 2,   0, 2, 3,  //x:-1
     8, 9,10,   8,10,11,  //x:1

     2, 6,10,   2,10, 5,  //y:-1
     0, 7, 8,   0, 8, 4,  //y:1

     3, 7,11,   3,11, 6,  //z:-1
     1, 4, 9,   1, 9, 5,   //z:1
     //corner
     2, 3, 6, // -,-,-
     2, 1, 5, // -,-,+
     0, 3, 7, // -,+,-
     0, 1, 4, // -,+,+
     6,10,11, // +,-,-
     5, 9,10, // +,-,+
     7, 8,11, // +,+,-
     4, 8, 9  // +,+,+
};
const float static convexVerticesOnly[] = {
    //3, 2
    //0, 1
    //     XYZ                BaseColor 

    -1.0f,1.0f,0.0f,
    -1.0f,0.0f,1.0f,
    -1.0f,-1.0f,0.0f,
    -1.0f,0.0f,-1.0f,

    0.0f,1.0f,1.0f,
    0.0f,-1.0f,1.0f,
    0.0f,-1.0f,-1.0f,
    0.0f,1.0f,-1.0f,

    1.0f,1.0f,0.0f,
    1.0f,0.0f,1.0f,
    1.0f,-1.0f,0.0f,
    1.0f,0.0f,-1.0f
};

static const ModelData dataCube = { cubeVertices, cubeIndices };
static const ModelData dataConvex = { convexVertices, convexIndices };
static ModelDataStatic dataCubeStatic = { cubeVertices, cubeIndices };
static ModelDataStatic dataConvexStatic = { convexVertices, convexIndices };




ModelBase::ModelBase() {}
void ModelBase::destruct() {}
void ModelBase::glInit() {}
void ModelBase::glRender() {}
ModelBase::~ModelBase() {}
uint ModelBase::gVao() {return uint(-1);}
uint ModelBase::gVeo() {return uint(-1);}
uint ModelBase::gLength() {return uint(0);}
mat4 ModelBase::gMat() {return mat4(1.0f);}
GLenum ModelBase::gType() {return GLenum(GL_POINTS);}
void ModelBase::renderUpdate(ulint delta) {}

ModelPhysic::ModelPhysic() { actor = nullptr; }
ModelPhysic::~ModelPhysic() {}
void ModelPhysic::pxUpdate() {}

ModelStatic::ModelStatic() {}
ModelStatic::~ModelStatic() {}
void ModelStatic::pxUpdate() {}

ModelDynamic::ModelDynamic() {}
ModelDynamic::~ModelDynamic() {}
void ModelDynamic::pxUpdate() {}
void ModelDynamic::pxSleep() {}
void ModelDynamic::pxWake() {}



ModelDataStatic& ModelCube::modelData = dataCubeStatic;
ModelCube::ModelCube(Scene* scene) : ModelCube(vec3(0.0f), scene) {}
ModelCube::ModelCube(vec3 position, Scene* scene)
{
    pos = position;

    auto px = PhysicAPI::getPhysics();
    physx::PxShape* ps = px->createShape(physx::PxBoxGeometry(physx::PxVec3(1.0f)),
        *px->createMaterial(0.5, 0.5, 0.5));
    ps->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, true);
    ps->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, true);
    actor = new ActorDynamic(this, ps, 50);
    actor->a->setActorFlag(physx::PxActorFlag::Enum::eDISABLE_GRAVITY, false);
    scene->addActor(actor);
}
void ModelCube::destruct()
{
	delete actor;
	actor = nullptr;
}

void ModelCube::glInit() {
    modelData.init();
}
void ModelCube::glRender()
{
    global->r->render(this);
}
ModelCube::~ModelCube()
{
}

uint ModelCube::gVao() { return modelData.vaoId; }
uint ModelCube::gVeo() { return modelData.veoId; }
uint ModelCube::gLength() { return modelData.indLength; }
mat4 ModelCube::gMat() { return glm::translate(mat4(1.0f), gPos())*_EulerMat(gRot()); }
GLenum ModelCube::gType() { return GL_TRIANGLES; }

void ModelCube::pxUpdate()
{
    physx::PxTransform loc = ((physx::PxRigidDynamic*)(actor->a))->getGlobalPose();
    auto vo = ((physx::PxRigidDynamic*)(actor->a))->getLinearVelocity();
    auto avo = ((physx::PxRigidDynamic*)(actor->a))->getAngularVelocity();
    sync(_CastGLPX(loc.p), (_CastGLPXrot(loc.q)), _CastGLPX(vo), _CastGLPXrotVol(avo)*1.f );
}
void ModelCube::pxSleep()
{
    physx::PxTransform loc = ((physx::PxRigidDynamic*)(actor->a))->getGlobalPose();
    sync(_CastGLPX(loc.p), (_CastGLPXrot(loc.q)), vec3(0.f), vec3(0.f));
}
void ModelCube::pxWake() {}

void ModelCube::renderUpdate(ulint delta)
{
    update(delta);
}

bool ModelCube::isInside(vec3 p)
{
    vec3 lp = pos - 1.0f;
    vec3 up = pos + 1.0f;
    return glm::all(glm::greaterThanEqual(p, lp)) && glm::all(glm::lessThanEqual(p, up));
}
bool ModelCube::isInside(vec3 p, vec3 w)
{
	vec3 lp = (pos - w) - 1.0f;
	vec3 up = (pos + w) + 1.0f;
	return glm::all(glm::greaterThanEqual(p, lp)) && glm::all(glm::lessThanEqual(p, up));
}



const ModelData& ModelConvex::modelData = dataConvex;
ModelConvex::ModelConvex(Scene* scene) : ModelConvex(vec3(0.0f), scene) {}
ModelConvex::ModelConvex(vec3 position, Scene* scene)
{
    pos = position;

    auto px = PhysicAPI::getPhysics();

    physx::PxConvexMeshDesc convexDesc;
    convexDesc.points.count = modelData.vertLength/3;
    convexDesc.points.stride = modelData.vertLength*sizeof(float)/convexDesc.points.count;
    convexDesc.points.data = modelData.vert;
    convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
    physx::PxDefaultMemoryOutputStream output;
    physx::PxConvexMeshCookingResult::Enum result;
    if (!PhysicAPI::cooking->cookConvexMesh(convexDesc, output, &result)) throw;

    physx::PxDefaultMemoryInputData input{ output.getData(), output.getSize() };

    auto mesh = px->createConvexMesh(input);
    physx::PxShape* ps = px->createShape(physx::PxConvexMeshGeometry(mesh),
        *px->createMaterial(0.5, 0.5, 0.5));

    ps->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, true);
    ps->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, true);

    actor = new ActorDynamic(this, ps, 50);
    actor->a->setActorFlag(physx::PxActorFlag::Enum::eDISABLE_GRAVITY, false);
    scene->addActor(actor);
}
void ModelConvex::destruct()
{
    delete actor;
    actor = nullptr;
}

void ModelConvex::glInit() {
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glGenBuffers(1, &veoId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veoId);

    glBufferData(GL_ARRAY_BUFFER, modelData.vertLength * sizeof(float), modelData.vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelData.indLength * sizeof(float), modelData.ind, GL_STATIC_DRAW);
}
void ModelConvex::glRender()
{
    global->r->render(this);
}
ModelConvex::~ModelConvex()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &veoId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelConvex::gVao() { return vaoId; }
uint ModelConvex::gVeo() { return veoId; }
uint ModelConvex::gLength() { return modelData.indLength; }
mat4 ModelConvex::gMat() { return glm::translate(mat4(1.0f), gPos()) * _EulerMat(gRot()); }
GLenum ModelConvex::gType() { return GL_TRIANGLES; }

void ModelConvex::pxUpdate()
{
    physx::PxTransform loc = ((physx::PxRigidDynamic*)(actor->a))->getGlobalPose();
    auto vo = ((physx::PxRigidDynamic*)(actor->a))->getLinearVelocity();
    auto avo = ((physx::PxRigidDynamic*)(actor->a))->getAngularVelocity();
    sync(_CastGLPX(loc.p), (_CastGLPXrot(loc.q)), _CastGLPX(vo), _CastGLPXrotVol(avo) * 1.f);
}
void ModelConvex::pxSleep()
{
    physx::PxTransform loc = ((physx::PxRigidDynamic*)(actor->a))->getGlobalPose();
    sync(_CastGLPX(loc.p), (_CastGLPXrot(loc.q)), vec3(0.f), vec3(0.f));
}
void ModelConvex::pxWake() {}

void ModelConvex::renderUpdate(ulint delta)
{
    update(delta);
}

bool ModelConvex::isInside(vec3 p)
{
    vec3 lp = pos - 1.0f;
    vec3 up = pos + 1.0f;
    return glm::all(glm::greaterThanEqual(p, lp)) && glm::all(glm::lessThanEqual(p, up));
}
bool ModelConvex::isInside(vec3 p, vec3 w)
{
    vec3 lp = (pos - w) - 1.0f;
    vec3 up = (pos + w) + 1.0f;
    return glm::all(glm::greaterThanEqual(p, lp)) && glm::all(glm::lessThanEqual(p, up));
}



ModelSphere::ModelSphere(Scene* scene) : ModelSphere(vec3(0.0f), scene) {}
ModelSphere::ModelSphere(vec3 position, Scene* scene)
{
    pos = position;

    auto px = PhysicAPI::getPhysics();

    physx::PxShape* ps = px->createShape(physx::PxSphereGeometry(1.f),
        *px->createMaterial(0.5, 0.5, 0.5));

    ps->setFlag(physx::PxShapeFlag::Enum::eSIMULATION_SHAPE, true);
    ps->setFlag(physx::PxShapeFlag::Enum::eSCENE_QUERY_SHAPE, true);

    actor = new ActorDynamic(this, ps, 50);
    actor->a->setActorFlag(physx::PxActorFlag::Enum::eDISABLE_GRAVITY, false);
    scene->addActor(actor);
}
void ModelSphere::destruct()
{
    delete actor;
    actor = nullptr;
}

void ModelSphere::glInit() {
    auto geo = Geo::genIcoSphere(1.f, vec3(0.f), 3);
    ind = std::move(std::get<1>(geo));
    auto& verts = std::get<0>(geo);
    for (const auto& point : verts) {
        vert.insert(vert.end(), { point.x,point.y,point.z, 0.7f, 0.7f, 0.7f });
    }

    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glGenBuffers(1, &veoId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veoId);

    glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(float), vert.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(float), ind.data(), GL_STATIC_DRAW);
}
void ModelSphere::glRender()
{
    global->r->render(this);
}
ModelSphere::~ModelSphere()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &veoId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelSphere::gVao() { return vaoId; }
uint ModelSphere::gVeo() { return veoId; }
uint ModelSphere::gLength() { return ind.size(); }
mat4 ModelSphere::gMat() { return glm::translate(mat4(1.0f), gPos()) * _EulerMat(gRot()); }
GLenum ModelSphere::gType() { return GL_TRIANGLES; }

void ModelSphere::pxUpdate()
{
    physx::PxTransform loc = ((physx::PxRigidDynamic*)(actor->a))->getGlobalPose();
    auto vo = ((physx::PxRigidDynamic*)(actor->a))->getLinearVelocity();
    auto avo = ((physx::PxRigidDynamic*)(actor->a))->getAngularVelocity();
    sync(_CastGLPX(loc.p), (_CastGLPXrot(loc.q)), _CastGLPX(vo), _CastGLPXrotVol(avo) * 1.f);
}
void ModelSphere::pxSleep()
{
    physx::PxTransform loc = ((physx::PxRigidDynamic*)(actor->a))->getGlobalPose();
    sync(_CastGLPX(loc.p), (_CastGLPXrot(loc.q)), vec3(0.f), vec3(0.f));
}
void ModelSphere::pxWake() {}

void ModelSphere::renderUpdate(ulint delta)
{
    update(delta);
}



const ModelData& ModelCubeStatic::modelData = dataCube;
ModelCubeStatic::ModelCubeStatic(Scene* scene) : ModelCubeStatic(vec3(0.0f), scene) {}
ModelCubeStatic::ModelCubeStatic(vec3 position, Scene* scene)
{
    pos = position;

    auto px = PhysicAPI::getPhysics();
    physx::PxShape* ps = px->createShape(physx::PxBoxGeometry(physx::PxVec3(1.0f)),
        *px->createMaterial(0.5, 0.5, 0.5));

    actor = new ActorStatic(this, ps);
    scene->addActor(actor);
}
void ModelCubeStatic::destruct()
{
	delete actor;
	actor = nullptr;
}

void ModelCubeStatic::glInit()
{
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glGenBuffers(1, &veoId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veoId);

    glBufferData(GL_ARRAY_BUFFER, modelData.vertLength * sizeof(float), modelData.vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelData.indLength * sizeof(float), modelData.ind, GL_STATIC_DRAW);
}
void ModelCubeStatic::glRender()
{
    global->r->render(this);
}
ModelCubeStatic::~ModelCubeStatic()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &veoId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelCubeStatic::gVao()
{
    return vaoId;
}

uint ModelCubeStatic::gVeo()
{
    return veoId;
}

uint ModelCubeStatic::gLength()
{
    return modelData.indLength;
}

mat4 ModelCubeStatic::gMat()
{
    return glm::translate(mat4(1.0f), gPos()) * _EulerMat(gRot());
}

GLenum ModelCubeStatic::gType()
{
    return GL_TRIANGLES;
}

void ModelCubeStatic::pxUpdate()
{
    physx::PxTransform loc = ((physx::PxRigidStatic*)(actor->a))->getGlobalPose();
    sync(_CastGLPX(loc.p), (_CastGLPXrot(loc.q)));
}

void ModelCubeStatic::renderUpdate(ulint delta)
{
    update(delta);
}



const ModelData& ModelCube2::modelData = dataCube;
ModelCube2::ModelCube2() : ModelCube2(vec3(0.0f)) {}
ModelCube2::ModelCube2(vec3 position)
{
    pos = position;
}
void ModelCube2::destruct()
{
}

void ModelCube2::glInit()
{
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glGenBuffers(1, &veoId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veoId);

    glBufferData(GL_ARRAY_BUFFER, modelData.vertLength * sizeof(float), modelData.vert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelData.indLength * sizeof(float), modelData.ind, GL_STATIC_DRAW);
}
void ModelCube2::glRender()
{
    global->r->render(this);
}
ModelCube2::~ModelCube2()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &veoId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelCube2::gVao() { return vaoId; }
uint ModelCube2::gVeo() { return veoId; }
uint ModelCube2::gLength() { return modelData.indLength; }
mat4 ModelCube2::gMat() { return glm::translate(mat4(1.0f), pos); }
GLenum ModelCube2::gType() { return GL_TRIANGLES; }

void ModelCube2::pxUpdate(physx::PxActor* actor) {}
void ModelCube2::renderUpdate(ulint delta) { global->r->render(this); }

bool ModelCube2::isInside(vec3 p)
{
    vec3 lp = pos - 1.0f;
    vec3 up = pos + 1.0f;
    return glm::all(glm::greaterThanEqual(p, lp)) && glm::all(glm::lessThanEqual(p, up));
}
bool ModelCube2::isInside(vec3 p, vec3 w)
{
    vec3 lp = (pos - w) - 1.0f;
    vec3 up = (pos + w) + 1.0f;
    return glm::all(glm::greaterThanEqual(p, lp)) && glm::all(glm::lessThanEqual(p, up));
}
bool ModelCube2::isRayCollide(vec3 sr, vec3 er)
{
    sr -= pos;
    er -= pos;

    return false;
}



ModelGrid::ModelGrid(uint si, float sp, vec3 color)
{
    size = si+1;
    space = sp;
    float offset = -((float)(size-1) / 2.0f * space);
    for (uint gx = 0; gx < size; gx++) {
        for (uint gy = 0; gy < size; gy++) {
            for (uint gz = 0; gz < size; gz++) {
                vertices.push_back(float(gx) * space + offset);
                vertices.push_back(float(gy) * space + offset);
                vertices.push_back(float(gz) * space + offset);
                //vertices.push_back(color.x);
                //vertices.push_back(color.y);
                //vertices.push_back(color.z);
                vertices.push_back(1.0 * gx / (size - 1));
                vertices.push_back(1.0 * gy / (size - 1));
                vertices.push_back(1.0 * gz / (size - 1));
            }
        }
    }
    
    // i = gx * size^2 + gy * size + gz

    //-x to +x
    for (uint gy = 0; gy < size; gy++) {
        for (uint gz = 0; gz < size; gz++) {
            indices.push_back(gy * size + gz);
            indices.push_back((size - 1) * size * size + gy * size + gz);
        }
    }
    //-y to +y
    for (uint gx = 0; gx < size; gx++) {
        for (uint gz = 0; gz < size; gz++) {
            indices.push_back(gx * size * size + gz);
            indices.push_back(gx * size * size + (size - 1) * size + gz);
        }
    }
    //-z to +z
    for (uint gx = 0; gx < size; gx++) {
        for (uint gy = 0; gy < size; gy++) {
            indices.push_back(gx * size * size + gy * size);
            indices.push_back(gx * size * size + gy * size + (size - 1));
        }
    }
}
void ModelGrid::destruct()
{
}

void ModelGrid::glInit()
{
	glGenVertexArrays(1, &vaoId);
	glGenBuffers(1, &vboId);
	glBindVertexArray(vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glGenBuffers(1, &veoId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veoId);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), indices.data(), GL_STATIC_DRAW);
	length = indices.size();
}
void ModelGrid::glRender()
{
    global->r->outlineMode();
    global->r->render(this);
    global->r->objectMode();
}
ModelGrid::~ModelGrid()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &veoId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelGrid::gVao() { return vaoId; }
uint ModelGrid::gVeo() { return veoId; }
uint ModelGrid::gLength() { return length; }
mat4 ModelGrid::gMat() { return mat4(1.0f); }
GLenum ModelGrid::gType() { return GL_LINES; }

void ModelGrid::pxUpdate(physx::PxActor* actor) {}
void ModelGrid::renderUpdate(ulint delta) {}



ModelPoints::ModelPoints(uint count, vec3 centre, vec3 size, vec3 color)
{
    SeededRng r = SeededRng(1);

    vec3 l = centre - size / 2.0f;
    vec3 u = centre + size / 2.0f;

    for (uint i = 0; i < count; i++) {
        vertices.push_back(r.next(l.x, u.x));
        vertices.push_back(r.next(l.y, u.y));
        vertices.push_back(r.next(l.z, u.z));
        vertices.push_back(color.x);
        vertices.push_back(color.y);
        vertices.push_back(color.z);
    }
}
void ModelPoints::destruct() {}

void ModelPoints::glInit()
{
	glGenVertexArrays(1, &vaoId);
	glGenBuffers(1, &vboId);
	glBindVertexArray(vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	length = vertices.size() / 6;
}
void ModelPoints::glRender()
{
    global->r->outlineMode();
    global->r->render(this);
    global->r->objectMode();
}
ModelPoints::~ModelPoints()
{
    glDeleteBuffers(1, &vboId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelPoints::gVao() { return vaoId; }
uint ModelPoints::gVeo() { return uint(-1); }
uint ModelPoints::gLength() { return length; }
mat4 ModelPoints::gMat() { return mat4(1.0f); }
GLenum ModelPoints::gType() { return GL_POINTS; }

void ModelPoints::pxUpdate(physx::PxActor* actor) {}
void ModelPoints::renderUpdate(ulint delta) {}

ModelGui::ModelGui()
{
    //Gen cross point

    lines.push_back(0.0f - CROSS_SIZE);
    lines.push_back(0.0f);
    lines.push_back(0.0f);
    lines.push_back(1.0f);
    lines.push_back(1.0f);
    lines.push_back(1.0f);

    lines.push_back(0.0f + CROSS_SIZE);
    lines.push_back(0.0f);
    lines.push_back(0.0f);
    lines.push_back(1.0f);
    lines.push_back(1.0f);
    lines.push_back(1.0f);

    lines.push_back(0.0f);
    lines.push_back(0.0f - CROSS_SIZE * global->windowRatio());
    lines.push_back(0.0f);
    lines.push_back(1.0f);
    lines.push_back(1.0f);
    lines.push_back(1.0f);

    lines.push_back(0.0f);
    lines.push_back(0.0f + CROSS_SIZE * global->windowRatio());
    lines.push_back(0.0f);
    lines.push_back(1.0f);
    lines.push_back(1.0f);
    lines.push_back(1.0f);
}

void ModelGui::destruct()
{
}

void ModelGui::glInit()
{
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);

    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(float), lines.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    length = lines.size() / 6;
}

void ModelGui::glRender()
{
    global->r->render(this);
}

ModelGui::~ModelGui()
{
    glDeleteBuffers(1, &vboId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelGui::gVao()
{
    return vaoId;
}

uint ModelGui::gVeo()
{
    return uint(-1);
}

uint ModelGui::gLength()
{
    return length;
}

mat4 ModelGui::gMat()
{
    return mat4(1.0f);
}

GLenum ModelGui::gType()
{
    return GL_LINES;
}

void ModelGui::pxUpdate(physx::PxActor* actor)
{
}

void ModelGui::renderUpdate(ulint delta)
{
}



const float ModelFloor::floorVertices[] = {
	-1.f,-1.f, 0.f,  0.2f, 0.2f, 0.2f,
	-1.f, 1.f, 0.f,  0.2f, 0.2f, 0.2f,
	 1.f,-1.f, 0.f,  0.2f, 0.2f, 0.2f,
	 1.f, 1.f, 0.f,  0.2f, 0.2f, 0.2f };

ModelFloor::ModelFloor(Scene* scene) : ModelFloor(vec3(0.f), vec3(0.f, 0.f, 1.f), scene) {}
ModelFloor::ModelFloor(vec3 position, vec3 normal, Scene* scene)
{
	pos = position;

	auto px = PhysicAPI::getPhysics();
	auto planeTransform = physx::PxTransformFromPlaneEquation(
		physx::PxPlane(_CastGLPX(position), _CastGLPX(normal)));
	physx::PxShape* ps = px->createShape(physx::PxPlaneGeometry(), *px->createMaterial(0.5, 0.5, 0.5), true);
	actor = new ActorStatic(this, ps);
	((physx::PxRigidStatic*)actor->a)->setGlobalPose(planeTransform);
	scene->addActor(actor);
}

void ModelFloor::destruct()
{
	delete actor;
}

void ModelFloor::glInit()
{
	glGenVertexArrays(1, &vaoId);
	glGenBuffers(1, &vboId);
	glBindVertexArray(vaoId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void ModelFloor::glRender()
{
    global->r->render(this);
}

ModelFloor::~ModelFloor()
{
	glDeleteBuffers(1, &vboId);
	glDeleteVertexArrays(1, &vaoId);
}

uint ModelFloor::gVao()
{
	return vaoId;
}

uint ModelFloor::gLength()
{
	return 4;
}

mat4 ModelFloor::gMat()
{
	return glm::scale(glm::translate(mat4(1.0f), gPos()) * _EulerMat(gRot()), vec3(100.f));
}

GLenum ModelFloor::gType()
{
	return GL_TRIANGLE_STRIP;
}

void ModelFloor::pxUpdate()
{
	physx::PxTransform loc = ((physx::PxRigidStatic*)(actor->a))->getGlobalPose();
	sync(_CastGLPX(loc.p), (_CastGLPXrot(loc.q)));
}

void ModelFloor::renderUpdate(ulint delta)
{
	update(delta);
}

