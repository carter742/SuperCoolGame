#include "gameParts.h"

namespace gm
{
	Entity::Entity(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) 
		: Base(position, size, color), velocity(0.f, 0.f), acceleration(0.f, 0.f)
	{
		id = std::to_string(std::rand()) + std::to_string(std::rand()) + std::to_string(std::rand());
	}

	Entity::Entity()
		: Base({ 0.f, 0.f }, { 0.f, 0.f }, sf::Color::Red)
	{
		id = std::to_string(std::rand()) + std::to_string(std::rand()) + std::to_string(std::rand());
	}

	bool Entity::operator==(const Entity& b)
	{
		return id == b.id;
	}


	void entityMovementCalculations(const float& deltaTime, std::vector<Entity*>& entities)
	{
		for (auto& entity : entities)
		{
			if (!entity)
				continue;

			if (entity->gravityEnabled)
				entity->acceleration.y += 10.f;

			entity->velocity += entity->acceleration * deltaTime;
			entity->position += entity->velocity;

			entity->acceleration = { 0.f, 0.f };

			entity->velocity.x *= entity->friction.x;
			entity->velocity.y *= entity->friction.y;
		}
	}

	void projectileMovementCalculations(const float& deltaTime, std::vector<Projectile*>& projectiles)
	{
		for (auto& projectile : projectiles)
		{
			if (!projectile)
				continue;

			projectile->velocity += projectile->acceleration * deltaTime;
			projectile->position += projectile->velocity;

			projectile->acceleration = { 0.f, 0.f };

			projectile->velocity.x *= projectile->friction.x;
			projectile->velocity.y *= projectile->friction.y;
		}
	}
}