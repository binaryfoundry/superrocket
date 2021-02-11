#include "Spline.hpp"

void Spline::RecalculateControls(size_t i)
{
    size_t prev = GetIndex(i - 1);
    size_t curr = GetIndex(i);
    size_t next = GetIndex(i + 1);

    vec3 p_prev = points[prev];
    vec3 p_curr = points[curr];
    vec3 p_next = points[next];

    vec3 d0 = p_curr - p_prev;
    vec3 d1 = p_next - p_curr;

    vec3 d = glm::normalize((d0 + d1) / 2.0f);

    controls[curr] = d;
}

void Spline::InsertPoint(vec3 position)
{
    points.push_back(position);
    controls.push_back(vec3());
    normals.push_back(vec3(0, 1, 0));

    size_t i = count = points.size();

    if (i > 1)
    {
        RecalculateControls(i - 2);
        RecalculateControls(i - 1);
        RecalculateControls(i);
    }

    Update();
}

void Spline::MovePoint(size_t index, vec3 position)
{
    vec3 offset = position - points[index];
    points[index] += offset;

    Update();
}

void Spline::MoveControl(size_t index, vec3 position)
{
    vec3 point = points[index];

    controls[index] = position - point;

    Update();
}

void Spline::MoveNormal(size_t index, vec3 position)
{
    vec3 point = points[index];

    normals[index] = glm::normalize(position - point);

    Update();
}

size_t Spline::GetIndex(size_t i)
{
    return ((i % count) + count) % count;
}

void Spline::Update()
{
    count = points.size();
    lengths.resize(count);

    total_length = 0.0f;

    for (int i = 0; i < lengths.size(); i++)
    {
        total_length += (lengths[i] = CalculateSegmentLength(i));
    }
}

vec3 Spline::GetPoint(float f)
{
    size_t i = static_cast<size_t>(f);
    size_t i0 = i;
    size_t i1 = (i + 1) % count;

    float t = f - i;
    float c = 1.0f - t;

    float bb0 = c * c * c;
    float bb1 = 3 * t * c * c;
    float bb2 = 3 * t * t * c;
    float bb3 = t * t * t;

    i0 %= points.size();
    i1 %= points.size();

    vec3 p0 = points[i0];
    vec3 p1 = points[i0] + controls[i0];
    vec3 p2 = points[i1] - controls[i1];
    vec3 p3 = points[i1];

    vec3 point =
        p0 * bb0 +
        p1 * bb1 +
        p2 * bb2 +
        p3 * bb3;

    return point;
}

vec3 Spline::GetGradient(float f)
{
    size_t i = static_cast<size_t>(f);
    size_t i0 = i;
    size_t i1 = (i + 1) % count;

    float t = f - i;

    vec3 p0 = points[i0];
    vec3 p1 = points[i0] + controls[i0];
    vec3 p2 = points[i1] - controls[i1];
    vec3 p3 = points[i1];

    vec3 q0 = p0 + ((p1 - p0) * t);
    vec3 q1 = p1 + ((p2 - p1) * t);
    vec3 q2 = p2 + ((p3 - p2) * t);

    vec3 r0 = q0 + ((q1 - q0) * t);
    vec3 r1 = q1 + ((q2 - q1) * t);
    vec3 tangent = r1 - r0;

    return tangent;
}

vec3 Spline::GetNormal(float f)
{
    size_t i = static_cast<size_t>(f);
    size_t i0 = i;
    size_t i1 = (i + 1) % count;

    float t = f - i;

    vec3 p0 = glm::normalize(controls[i0]);
    vec3 p1 = glm::normalize(controls[i1]);

    float d0 = glm::dot(p0, normals[i0]);
    float d1 = glm::dot(p1, normals[i1]);

    vec3 n0 = glm::normalize(normals[i0] - (p0 * d0));
    vec3 n1 = glm::normalize(normals[i1] - (p1 * d1));

    return glm::normalize(n0 * (1 - t) + n1 * t);
}

float Spline::CalculateSegmentLength(int node)
{
    float length = 0.0f;
    float step_size = 0.005f;

    vec3 old_point, new_point;
    old_point = GetPoint(static_cast<float>(node));

    for (float t = 0; t < 1.0f; t += step_size)
    {
        new_point = GetPoint(static_cast<float>(node) + t);
        length += sqrtf(
            (new_point.x - old_point.x) * (new_point.x - old_point.x) +
            (new_point.y - old_point.y) * (new_point.y - old_point.y) +
            (new_point.z - old_point.z) * (new_point.z - old_point.z));
        old_point = new_point;
    }

    return length;
}

float Spline::GetNormalisedOffset(float p)
{
    int i = 0;
    while (p > lengths[i])
    {
        p -= lengths[i];
        i++;
    }
    return static_cast<float>(i) + (p / lengths[i]);
}
