/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  Branislav Mateas <xmatea00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    13.12.2020
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{

}

bool TreeMeshBuilder::blockEmpty(const float gridSize,  const Vec3_t<float> &offset, const ParametricScalarField &field)
{
    const float mEdgeResolution = gridSize * mGridResolution;
    const Vec3_t<float> midpoint(offset.x * mGridResolution + (mEdgeResolution/2.0),
                                 offset.y * mGridResolution + (mEdgeResolution/2.0),
                                 offset.z * mGridResolution + (mEdgeResolution/2.0));
    return (evaluateFieldAt(midpoint, field) > mIsoLevel + (sqrtf(3.0) / 2.0) * mEdgeResolution);
}

unsigned TreeMeshBuilder::divide(const unsigned gridSize, const Vec3_t<float> &offset, const ParametricScalarField &field)
{
    if (blockEmpty(float(gridSize), offset, field))
        return 0;
    if (gridSize <= 1)
        return buildCube(offset, field);
    unsigned totalTriangles = 0;
    for(unsigned i = 0; i < 8; ++i)
    {
        Vec3_t<float> vertex = sc_vertexNormPos[i];
        #pragma omp task firstprivate(vertex) shared(gridSize, offset, field, totalTriangles) final((gridSize/2.0) <= 1)
        {
        const Vec3_t<float> newOffset(offset.x + vertex.x * (gridSize / 2.0),
                                      offset.y + vertex.y * (gridSize / 2.0),
                                      offset.z + vertex.z * (gridSize / 2.0));
        #pragma omp atomic
        totalTriangles += divide((gridSize / 2.0), newOffset, field);
        }
    }
    #pragma omp taskwait
    return totalTriangles;   
}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    unsigned result = 0;
    #pragma omp parallel shared(result, field)
    #pragma omp master
    result = divide(mGridSize, Vec3_t<float>(0.0,0.0,0.0), field);
    return result;
}

float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();

    for(unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        value = std::min(value, distanceSquared);
    }

    return sqrt(value);
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    #pragma omp critical
	mTriangles.push_back(triangle);
}
