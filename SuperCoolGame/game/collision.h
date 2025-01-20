#pragma once

#include <vector>
#include <cmath>

#include "SFML/Graphics/Rect.hpp"
#include "gameParts.h"
#include "vectorMath.h"

namespace gm
{
	sf::Vector2f getOverlap(const sf::FloatRect& rectA, const sf::FloatRect& rectB);
	void entityCollisionCheck(std::vector<Entity*>& entities);
	void staticCollisionCheck(std::vector<StaticBody*>& staticBodies, std::vector<Entity*>& entities);
	void projectileCollisionCheck(const sf::RenderWindow& window, std::vector<Projectile*>& projectiles, std::vector<Entity*> entities);
}