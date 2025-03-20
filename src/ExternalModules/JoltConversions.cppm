//
// Created by Vinnie on 21/03/2025.
//

export module jolt_conversions;
import crab_types;
import jolt;

export Vector3 jolt_to_glm(const JPH::Vec3& v)
{
    return {v.GetX(), v.GetY(), v.GetZ()};
};

export Quat jolt_to_glm(const JPH::Quat& v)
{
    return {v.GetW(), v.GetX(), v.GetY(), v.GetZ()};
};

export JPH::Vec3 glm_to_jolt(const Vector3& v)
{
    return {v.x, v.y, v.z};
};

export JPH::Quat glm_to_jolt(const Quat& v)
{
    return {v.x, v.y, v.z, v.w};
};