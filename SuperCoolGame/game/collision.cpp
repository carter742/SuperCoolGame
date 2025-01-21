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

				if (entityA->collisionCallback)
				{
					entityA->collisionCallback(entityB->group, entityA);
				}

				if (entityB->collisionCallback)
				{
					entityB->collisionCallback(entityA->group, entityB);
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

				if (entity->collisionCallback)
				{
					entity->collisionCallback(staticBody->group, entity);
				}

				if (staticBody->collisionCallback)
				{
					staticBody->collisionCallback(entity->group, staticBody);
				}
			}
		}
	}

	void projectileCollisionCheck(const sf::RenderWindow& window, std::vector<Projectile*>& projectiles, std::vector<Entity*>& entities)
	{
		sf::FloatRect windowRect{ {-50.f, -50.f}, static_cast<sf::Vector2f>(window.getSize()) + sf::Vector2f{100.f, 100.f} };
		
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

			for (auto& projectileB : projectiles)
			{
				if (!projectile)
					break;

				if (!projectileB || projectile == projectileB)
					continue;

				if (projectile->collisionLayerToCheck != projectileB->collisionLayer)
					continue;

				sf::FloatRect projectileBRect{ projectileB->position + projectileB->velocity, projectileB->size };

				if (!projectileRect.intersects(projectileBRect))
					continue;

				if (projectile->takeDamage || projectileB->enableDamage)
					projectile->hp -= 1;

				if (projectile->collisionCallback)
				{
					projectile->collisionCallback(projectileB->group, projectile);
				}

				if (projectile->hp <= 0)
					projectile = nullptr;

				if (projectileB->takeDamage || projectile->enableDamage)
					projectileB->hp -= 1;

				if (projectileB->collisionCallback)
				{
					projectileB->collisionCallback(projectile->group, projectileB);
				}

				if (projectileB->hp <= 0)
					projectileB = nullptr;

				break;
			}

			for (auto& entity : entities)
			{
				if (!projectile)
					break;

				if (!entity)
					continue;

				if (projectile->collisionLayerToCheck != entity->collisionLayer)
					continue;

				sf::FloatRect entityRect{ entity->position + entity->velocity, entity->size };

				if (!projectileRect.intersects(entityRect))
					continue;

				if (projectile->collisionCallback)
				{
					projectile->collisionCallback(entity->group, projectile);
				}

				if (entity->collisionCallback)
				{
					entity->collisionCallback(projectile->group, entity);
				}

				if (projectile->enableDamage)
					entity->hp -= 1;

				if (projectile->dissapearOnHit)
					projectile = nullptr;

				if (entity->hp <= 0)
					entity = nullptr;

				break;
			}
		}
	}
}