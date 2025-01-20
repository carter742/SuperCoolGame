#include "collision.h"

namespace gm
{
	sf::Vector2f getOverlap(const sf::FloatRect& rectA, const sf::FloatRect& rectB)
	{
		return {
			std::min(rectA.left + rectA.width, rectB.left + rectB.width) - std::max(rectA.left, rectB.left),
			std::min(rectA.top + rectA.height, rectB.top + rectB.height) - std::max(rectA.top, rectB.top)
		};
	}

	void entityCollisionCheck(std::vector<Entity*>& entities)
	{
		for (auto& entityA : entities)
		{
			if (!entityA || !entityA->collisionEnabled)
				continue;

			sf::FloatRect rectA{ entityA->position + entityA->velocity, entityA->size };

			for (auto& entityB : entities)
			{
				if (!entityB || entityA == entityB)
					continue;

				if (entityA->collisionLayerToCheck != entityB->collisionLayer)
					continue;

				sf::FloatRect rectB{ entityB->position + entityB->velocity, entityB->size };

				if (!rectA.intersects(rectB))
					continue;

				const sf::Vector2f overlap = getOverlap(rectA, rectB);

				if (overlap.x < overlap.y && rectA.top + rectA.height - overlap.y > rectB.top)
				{
					const char direction = (entityA->velocity.x > 0) - (entityA->velocity.x < 0);
					entityA->position.x -= overlap.x * direction * 0.5f;
					entityB->position.x += overlap.x * direction * 0.5f;
				}
				else
				{
					const char direction = (entityA->velocity.y > 0) - (entityA->velocity.y < 0);
					entityA->position.y -= overlap.y * direction * 0.5f;
					entityB->position.y += overlap.y * direction * 0.5f;
				}
			}
		}
	}

	void staticCollisionCheck(std::vector<StaticBody*>& staticBodies, std::vector<Entity*>& entities)
	{
		for (auto& entity : entities)
		{
			if (!entity)
				continue;

			sf::FloatRect rectA{ entity->position + entity->velocity, entity->size };

			for (auto& staticBody : staticBodies)
			{
				if (!entity)
					continue;

				if (entity->collisionLayerToCheck != staticBody->collisionLayer)
					continue;
				
				sf::FloatRect rectB{ staticBody->position, staticBody->size };

				sf::Vector2f movementDirection{
					std::signbit(entity->velocity.x) ? -1.f : 1.f,
					std::signbit(entity->velocity.y) ? -1.f : 1.f
				};

				if (!rectA.intersects(rectB))
					continue;

				const sf::Vector2f overlap = getOverlap(rectA, rectB);

				if (overlap.x < overlap.y)
				{
					const char direction = (entity->velocity.x > 0) - (entity->velocity.x < 0);
					entity->position.x -= overlap.x * direction;
				}
				else
				{
					const char direction = (entity->velocity.y > 0) - (entity->velocity.y < 0);
					entity->position.y -= overlap.y * direction;
				}
			}
		}
	}

	void projectileCollisionCheck(const sf::RenderWindow& window, std::vector<Projectile*>& projectiles, std::vector<Entity*>& entities)
	{
		sf::FloatRect windowRect{ {0.f, 0.f}, static_cast<sf::Vector2f>(window.getSize()) };

		for (auto& projectile : projectiles)
		{
			if (!projectile)
				continue;

			sf::FloatRect projectileRect{ projectile->position + projectile->velocity, projectile->size };

			if (!windowRect.intersects(projectileRect))
			{
				projectile = nullptr;
				continue;
			}

			for (auto& entity : entities)
			{
				if (!entity)
					continue;

				if (projectile->collisionLayerToCheck != entity->collisionLayer)
					continue;

				sf::FloatRect entityRect{ entity->position + entity->velocity, entity->size };

				if (!projectileRect.intersects(entityRect))
					continue;

				projectile = nullptr;

				entity->hp -= 1;

				if (entity->hp <= 0)
					entity = nullptr;
			}
		}
	}
}