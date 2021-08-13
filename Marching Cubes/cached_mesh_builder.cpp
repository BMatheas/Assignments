/**
 * @file    cached_mesh_builder.cpp
 *
 * @author  Branislav Mateas <xmatea00@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using pre-computed field
 *
 * @date    13.12.2020
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "cached_mesh_builder.h"

CachedMeshBuilder::CachedMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Cached")
{

}

float CachedMeshBuilder::preloadEvaluate(const Vec3_t<float> &pos, const ParametricScalarField &field)
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

unsigned CachedMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    // 1. Compute total number of cubes in the grid.
    size_t totalCubesCount = mGridSize*mGridSize*mGridSize;

    size_t border = (mGridSize+1)*(mGridSize+1)*(mGridSize+1);
    const int sz = (mGridSize+1); 
    cache = new float [border];
    /*
    for(int i = 0; i < border; i++)
    {
        cache[i] = -1;
    }*/
    /* -------------------------- 3D array init and loading with predefined values ------------------------------*/
    /*cache = new float **[sz];
    for(int i = 0; i < sz;++i)
    {
        cache[i] = new float *[sz];
        for(int j = 0; j < sz; ++j)
        {
            cache[i][j] = new float [sz];
            for(int k = 0; k < sz; ++k)
            {
                cache[i][j][k] = -1;
            }
        }
    }
    */
    #pragma omp parallel for shared(border, field) schedule(guided)
    for(size_t i = 0; i < border; ++i)
    {
        const int tx = (i % (mGridSize+1));
        const int ty = ((i / (mGridSize+1)) % (mGridSize+1));
        const int tz = (i / ((mGridSize+1)*(mGridSize+1)));

    	Vec3_t<float> point((i % (mGridSize+1)) * mGridResolution, 
    						   ((i / (mGridSize+1)) % (mGridSize+1)) * mGridResolution,
    						   (i / ((mGridSize+1)*(mGridSize+1))) * mGridResolution
    	);
       //Flat[x + HEIGHT* (y + WIDTH* z)
        const int a = tx + (mGridSize+1)*ty + ((mGridSize+1)*(mGridSize+1)*tz);
        cache[a] = preloadEvaluate(point,field);
    }
    unsigned totalTriangles = 0;

    #pragma omp parallel for shared(totalCubesCount, field) reduction(+:totalTriangles) schedule(guided)
    for(size_t i = 0; i < totalCubesCount; ++i)
    {
        Vec3_t<float> cubeOffset( i % mGridSize,
                                 (i / mGridSize) % mGridSize,
                                  i / (mGridSize*mGridSize));

        totalTriangles += buildCube(cubeOffset, field);
    }
    free(cache);
    return totalTriangles;
}

float CachedMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    int tx = ((pos.x/mGridResolution) + 0.5f);
    int ty = ((pos.y/mGridResolution) + 0.5f);
    int tz = ((pos.z/mGridResolution) + 0.5f);
    unsigned int a = tx + (mGridSize+1)*ty + ((mGridSize+1)*(mGridSize+1)*tz);
    return cache[a];
}

void CachedMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
	#pragma omp critical
	mTriangles.push_back(triangle);
}
