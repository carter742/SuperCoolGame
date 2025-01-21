#pragma once

#include <array>
#include "gameParts.h"
#include "vectorMath.h"
#include "collision.h"

namespace conf
{
	constexpr float PLAYER_MOVEMENT_SPEED = 100.f;
	constexpr float PLAYER_DASH_SPEED = 500.f;
	constexpr float PLAYER_BULLET_SPEED = 5.f;//10.f;
	constexpr float ENEMY_MOVEMENT_SPEED = 0.5f;
}

namespace gm
{
	class GameData
	{
	public:
		unsigned long long frame = 0;
		sf::Clock clock;

		Entity player;

		std::vector<Entity*> entities;
		std::vector<StaticBody*> staticBodies;
		std::vector<Projectile*> projectiles;

		const sf::IntRect defaultTextureRect{ {0, 0}, {16, 16} };
		sf::Texture rocketshipTexture;
		sf::Texture asteroidsTexture;
		sf::Texture playerBulletTexture;

		GameData();
	};
}