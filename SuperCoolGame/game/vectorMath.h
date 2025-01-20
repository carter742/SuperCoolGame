#pragma once
#include "SFML/System/Vector2.hpp"

namespace gm
{
	template<typename T>
	std::string vectorToString(T vector)
	{
		return "x: " + std::to_string(vector.x) + ", y: " + std::to_string(vector.y);
	}

	template<typename T>
	float getMagnitude(T vector)
	{
		return std::sqrt(vector.x * vector.x + vector.y * vector.y);
	}

	template<typename T>
	T normalize(T vector)
	{
		float magnitude = getMagnitude(vector);

		if (magnitude == 0)
			return { 0.f, 0.f };

		return vector / magnitude;
	}
}