#pragma once

#include "Math.hpp"

#include <vector>

class Spline
{
public:
    size_t count = 0;
    float total_length = 0.0f;

    std::vector<vec3> points;
    std::vector<vec3> controls;
    std::vector<vec3> normals;
    std::vector<float> lengths;

    void RecalculateControls(size_t i);
    void InsertPoint(vec3 position);
    void MovePoint(size_t index, vec3 position);
    void MoveControl(size_t index, vec3 position);
    void MoveNormal(size_t index, vec3 position);
    size_t GetIndex(size_t i);
    void Update();
    vec3 GetPoint(float f);
    vec3 GetGradient(float f);
    vec3 GetNormal(float f);
    float CalculateSegmentLength(int node);
    float GetNormalisedOffset(float p);
};
