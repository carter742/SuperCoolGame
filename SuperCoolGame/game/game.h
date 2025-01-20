#pragma once

#include <array>
#include "gameParts.h"
#include "vectorMath.h"
#include "collision.h"


namespace gm
{
	constexpr unsigned int MAX_PROJECTILES = 10000;
	
	struct GameData
	{
		unsigned long long frame = 0;
		sf::Clock clock;

		Entity player;

		std::vector<Entity*> entities;
		std::vector<StaticBody*> staticBodies;
		std::vector<Projectile*> projectiles;
	};
}