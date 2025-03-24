module Engine.Math;
import Engine.Types;
//#include "glm/gtc/quaternion.hpp"

using namespace glm;

bool MathUtils::DecomposeTransform(const Matrix4& transform, Vector3& outTranslation, Quat& outRotation,
    Vector3& outScale)
{
    outTranslation = transform[3];
    for(int i = 0; i < 3; i++)
        outScale[i] = glm::length(vec3(transform[i]));
    const glm::mat3 rotMtx(
        glm::vec3(transform[0]) / outScale[0],
        glm::vec3(transform[1]) / outScale[1],
        glm::vec3(transform[2]) / outScale[2]);
    outRotation = glm::quat_cast(rotMtx);

    return true;
}
