#include "ModelBase.h"

ModelBase::ModelBase()
{
}

ModelBase::~ModelBase()
{
}

uint ModelBase::gVao()
{
	return uint(-1);
}

uint ModelBase::gVeo()
{
	return uint(-1);
}

uint ModelBase::gLength()
{
	return uint(0);
}

GLenum ModelBase::gType()
{
	return GLenum(GL_POINTS);
}



const float ModelCube::cubeVertices[] = {
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
const uint ModelCube::cubeIndices[] = {
     0, 2, 3,   0, 1, 2,  //front
     4, 6, 7,   4, 5, 6,  //back
     8,10,11,   8, 9,10,  //left
    12,14,15,  12,13,14,  //right
    16,18,19,  16,17,18,  //bottom
    20,22,23,  20,21,22   //top
};
const int ModelCube::cubeVertLength = sizeof(cubeVertices) / sizeof(float);
const int ModelCube::cubeIndLength = sizeof(cubeIndices) / sizeof(unsigned int);

ModelCube::ModelCube()
{
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glBindVertexArray(vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glGenBuffers(1, &veoId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veoId);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    length = sizeof(cubeIndices) / sizeof(float);
}

ModelCube::~ModelCube()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &veoId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelCube::gVao()
{
    return vaoId;
}

uint ModelCube::gVeo()
{
	return veoId;
}

uint ModelCube::gLength()
{
    return length;
}

GLenum ModelCube::gType()
{
	return GL_TRIANGLES;
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

ModelGrid::~ModelGrid()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &veoId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelGrid::gVao()
{
    return vaoId;
}

uint ModelGrid::gVeo()
{
    return veoId;
}

uint ModelGrid::gLength()
{
    return length;
}

GLenum ModelGrid::gType()
{
    return GL_LINES;
}



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

ModelPoints::~ModelPoints()
{
    glDeleteBuffers(1, &vboId);
    glDeleteVertexArrays(1, &vaoId);
}

uint ModelPoints::gVao()
{
    return vaoId;
}

uint ModelPoints::gVeo()
{
    return uint(-1);
}

uint ModelPoints::gLength()
{
    return length;
}

GLenum ModelPoints::gType()
{
    return GL_POINTS;
}

