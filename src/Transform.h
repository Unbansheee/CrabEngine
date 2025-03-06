//
// Created by Vinnie on 22/02/2025.
//

#pragma once
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>
#include "CrabTypes.h"

inline constexpr float deg_to_rad(float deg)
{
	return (deg * (PI / 180.0f));
}

inline constexpr float rad_to_deg(float rad)
{
	return (rad * (180.0f / PI));
}

inline constexpr Vector3 vec3_deg_to_rad(Vector3 deg)
{
	return Vector3{deg_to_rad(deg.x), deg_to_rad(deg.y), deg_to_rad(deg.z) };
}

inline constexpr Vector3 vec3_rad_to_deg(Vector3 rad)
{
	return Vector3{ rad_to_deg(rad.x), rad_to_deg(rad.y), rad_to_deg(rad.z) };
}

// A Transform represents a position in space, and contains a world-space model matrix
struct Transform
{
	Transform() = default;
	
	// Local position
	Vector3 Position = { 0.0f, 0.0f, 0.0f };

	// Local Orientation
	Quat Orientation = Quat(1,0,0,0);

	// Local scale
	Vector3 Scale = { 1.0f, 1.0f, 1.0f };

	// Worldspace Model Matrix
	Matrix4 ModelMatrix = Matrix4(1.0);

	Matrix4 GetLocalModelMatrix() const
	{
		return translate(Matrix4(1.0), Position) * Matrix4(Orientation) * scale(Matrix4(1.0), Scale);
	}

	const Matrix4& GetWorldModelMatrix() const
	{
		return ModelMatrix;
	}

	// Convert a position in Local Space to World Space (this might be broken)
	Vector3 LocalToWorldPosition(Vector3 in) const;

	// Convert a direction vector in Local Space to World Space (this might be broken)
	Vector3 LocalToWorldDirection(Vector3 in) const;

	// Convert a position in Local Space to World Space (this might be broken)
	Vector3 LocalToWorldScale(Vector3 in) const;

	// Convert a direction vector in Local Space to World Space (this might be broken)
	Vector3 WorldToLocalScale(Vector3 in) const;

	// Convert a position in World Space to Local Space (this might be broken)
	Vector3 WorldToLocalPosition(Vector3 in) const;

	// Convert a direction vector in World Space to Local Space (this might be broken)
	Vector3 WorldToLocalDirection(Vector3 in) const;

	// Convert a position in World Space to Local Space (this might be broken)
	Quat LocalToWorldRotation(const Quat& in) const;

	// Convert a direction vector in World Space to Local Space (this might be broken)
	Quat WorldToLocalRotation(const Quat& in) const;
	
public:
	static const Transform& identity() {
		static Transform t;
		return t;
	}

};