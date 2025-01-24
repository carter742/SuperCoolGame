#include "game.h"

namespace gm
{
	//load external textures and sounds
	//potenntial performance could be gained if a tilemap is used instead, so only one texture is loaded in.
	GameData::GameData()
	{
		//load sounds
		shootingSoundBuffer.loadFromFile("./assets/soundEffects/Laser_Shoot.wav");
		shootingSound.setBuffer(shootingSoundBuffer);
		healthSoundBuffer.loadFromFile("./assets/soundEffects/Pickup_Coin.wav");
		healthSound.setBuffer(healthSoundBuffer);
		hurtSoundBuffer.loadFromFile("./assets/soundEffects/Hit_Hurt.wav");
		hurtSound.setBuffer(hurtSoundBuffer);
		hurtTwoSoundBuffer.loadFromFile("./assets/soundEffects/Hit_Hurt2.wav");
		hurtTwoSound.setBuffer(hurtSoundBuffer);

		//load textures
		rocketshipTexture.loadFromFile("./assets/sprites/rocketship.png");
		asteroidsTexture.loadFromFile("./assets/sprites/asteroids.png");
		playerBulletTexture.loadFromFile("./assets/sprites/playerBullet.png");
		nebulaTexture.loadFromFile("./assets/sprites/nebula.png");
		enemyRocketshipTexture.loadFromFile("./assets/sprites/enemyRocket.png");
		heartTexture.loadFromFile("./assets/sprites/heart.png");
	}
}

//Entity creation
namespace gm
{
	Entity::Entity(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color)
		: Base(position, size, color), velocity(0.f, 0.f), acceleration(0.f, 0.f)
	{
		//create a new id. The random number is so large that you have a better chance of getting struck by lighting then for
		//a collision to happend.
		id = std::to_string(std::rand()) + std::to_string(std::rand()) + std::to_string(std::rand());
	}

	Entity::Entity()
		: Base({ 0.f, 0.f }, { 0.f, 0.f }, sf::Color::Red)
	{
		//create a new id. The random number is so large that you have a better chance of getting struck by lighting then for
		//a collision to happend.
		id = std::to_string(std::rand()) + std::to_string(std::rand()) + std::to_string(std::rand());
	}

	bool Entity::operator==(const Entity & b)
	{
		return id == b.id;
	}
}

//movement calculations
namespace gm {
	void entityMovementCalculations(const float& deltaTime, std::vector<Entity*>&entities)
	{
		//perform calculations on all entities
		for (auto& entity : entities)
		{
			//check if the entity exists
			if (!entity)
				continue;

			//check if gravity should be applied
			if (entity->gravityEnabled)
				entity->acceleration.y += 10.f;

			//check if in nebula
			if (entity->inNebula)
			{
				//slow movements when in nebula
				entity->acceleration *= 0.2f;
				entity->inNebula = false;
			}

			//apply acceleration in relation to delta time
			entity->velocity += entity->acceleration * deltaTime;
			
			//check if the entity is the player
			//done due to time constraints. A better solution then hard coding this would have been found otherwise.
			if (entity->group == "player")
			{
				//get the next players position
				const sf::Vector2f futurePosition = entity->position + entity->velocity;
				
				//stop the player from going out of bounds

				//left wall
				if (futurePosition.x < 0)
				{
					entity->velocity.x = 0.f;
					entity->position.x = 0.f;
				}

				//top wall
				if (futurePosition.y < 0)
				{
					entity->velocity.y = 0.f;
					entity->position.y = 0.f;
				}

				//right wall
				if (futurePosition.x > conf::WINDOW_WIDTH - entity->size.x)
				{
					entity->velocity.x = 0.f;
					entity->position.x = conf::WINDOW_WIDTH - entity->size.x;
				}

				//bottom wall
				if (futurePosition.y > conf::WINDOW_HEIGHT - entity->size.y)
				{
					entity->velocity.y = 0.f;
					entity->position.y = conf::WINDOW_HEIGHT - entity->size.y;
				}
			}

			//move the entity
			entity->position += entity->velocity;

			//reset acceleration
			entity->acceleration = { 0.f, 0.f };

			//apply friction
			entity->velocity.x *= entity->friction.x;
			entity->velocity.y *= entity->friction.y;
		}
	}

	void projectileMovementCalculations(const float& deltaTime, std::vector<Projectile*>&projectiles)
	{
		//calculate projectile movement
		for (auto& projectile : projectiles)
		{
			//check if the projectile exists
			if (!projectile)
				continue;

			//apply acceleration
			projectile->velocity += projectile->acceleration * deltaTime;

			//move the projectile
			projectile->position += projectile->velocity;

			//reset acceleration
			projectile->acceleration = { 0.f, 0.f };

			//apply friction
			projectile->velocity.x *= projectile->friction.x;
			projectile->velocity.y *= projectile->friction.y;
		}
	}
}

//collision
namespace gm
{
	//finds the overlap of two rectangles
	sf::Vector2f getOverlap(const sf::FloatRect& rectA, const sf::FloatRect& rectB)
	{
		return {
			std::min(rectA.left + rectA.width, rectB.left + rectB.width) - std::max(rectA.left, rectB.left),
			std::min(rectA.top + rectA.height, rectB.top + rectB.height) - std::max(rectA.top, rectB.top)
		};
	}


	void entityCollisionCheck(std::vector<Entity*>& entities)
	{
		//loop over all entites
		for (auto& entityA : entities)
		{
			//check if they exist and if they have collision enabled
			if (!entityA || !entityA->collisionEnabled)
				continue;

			//create their collision rectangle
			sf::FloatRect rectA{ entityA->position + entityA->velocity, entityA->size };

			//loop over all entities
			for (auto& entityB : entities)
			{
				//check if they exist and if the entity has the same id as entity A
				if (!entityB || entityA == entityB)
					continue;

				//check if entity A is checking that collision layer
				if (entityA->collisionLayerToCheck != entityB->collisionLayer)
					continue;

				//create a rectangle for entity B's collision
				sf::FloatRect rectB{ entityB->position + entityB->velocity, entityB->size };

				//check if they entities collide
				if (!rectA.intersects(rectB))
					continue;

				//find the overlap to resolve the collision
				const sf::Vector2f overlap = getOverlap(rectA, rectB);

				//check which overlap is smaller
				if (overlap.x < overlap.y && rectA.top + rectA.height - overlap.y > rectB.top)
				{
					//move the entities by half the overlap in opposite directions on the x axis
					const char direction = (entityA->velocity.x > 0) - (entityA->velocity.x < 0);
					entityA->position.x -= overlap.x * direction * 0.5f;
					entityB->position.x += overlap.x * direction * 0.5f;
				}
				else
				{
					//move the entities by half the overlap in opposite directions on the y axis
					const char direction = (entityA->velocity.y > 0) - (entityA->velocity.y < 0);
					entityA->position.y -= overlap.y * direction * 0.5f;
					entityB->position.y += overlap.y * direction * 0.5f;
				}

				//check if there is a collision callback
				if (entityA->collisionCallback)
				{
					entityA->collisionCallback(entityB->group, entityA);
				}

				//check if there is a collision callback
				if (entityB->collisionCallback)
				{
					entityB->collisionCallback(entityA->group, entityB);
				}
			}
		}
	}

	void staticCollisionCheck(std::vector<StaticBody*>& staticBodies, std::vector<Entity*>& entities)
	{
		//loop over all entites
		for (auto& entity : entities)
		{
			//check if they exist
			if (!entity)
				continue;

			//create their collision rectangle
			sf::FloatRect rectA{ entity->position + entity->velocity, entity->size };

			//loop over static bodies
			for (auto& staticBody : staticBodies)
			{
				//check if the entity exists again. Can't remember why :P 
				if (!entity)
					continue;
					
				//check if the entity is checking that collision layer
				if (entity->collisionLayerToCheck != staticBody->collisionLayer)
					continue;

				//create the collision rect for the static body
				sf::FloatRect rectB{ staticBody->position, staticBody->size };

				//find the movement direction
				sf::Vector2f movementDirection{
					std::signbit(entity->velocity.x) ? -1.f : 1.f,
					std::signbit(entity->velocity.y) ? -1.f : 1.f
				};

				//check if there was a collision
				if (!rectA.intersects(rectB))
					continue;

				//find the overlap to resolve the collision
				const sf::Vector2f overlap = getOverlap(rectA, rectB);

				//check which overlap is smaller
				if (overlap.x < overlap.y)
				{
					//move the entity along the x axis
					const char direction = (entity->velocity.x > 0) - (entity->velocity.x < 0);
					entity->position.x -= overlap.x * direction;
				}
				else
				{
					//move the entity along the y axis
					const char direction = (entity->velocity.y > 0) - (entity->velocity.y < 0);
					entity->position.y -= overlap.y * direction;
				}

				//check if there is a collision callback
				if (entity->collisionCallback)
				{
					entity->collisionCallback(staticBody->group, entity);
				}

				//check if there is a collision callback
				if (staticBody->collisionCallback)
				{
					staticBody->collisionCallback(entity->group, staticBody);
				}
			}
		}
	}


	void projectileCollisionCheck(gm::GameData& gameData, std::vector<Projectile*>& projectiles, std::vector<Entity*>& entities)
	{
		//get the window size and make it slightly bigger so the projectiles can spawn of screen
		sf::FloatRect windowRect{ {-50.f, -50.f}, sf::Vector2f{conf::WINDOW_WIDTH, conf::WINDOW_HEIGHT} + sf::Vector2f{100.f, 100.f}};

		//loop over the projectiles
		for (auto& projectile : projectiles)
		{
			//check if the projectile exist
			if (!projectile)
				continue;

			//get the projectile collision rect
			sf::FloatRect projectileRect{ projectile->position + projectile->velocity, projectile->size };

			//if there was a collision with the window remove it
			if (!windowRect.intersects(projectileRect))
			{
				projectile = nullptr;
				continue;
			}

			/*
			* Check if the projectile is not a health pick up. This is done so the player bullets don't destroy the 
			* health. A better solution would have been found if it weren't for time constraints.
			*/
			if (projectile->group != "healthPickUp")
				for (auto& projectileB : projectiles)
				{
					//check if the projectile exists
					if (!projectile)
						break;

					//check if projectile B exists and if the projectiles are the same
					if (!projectileB || projectile == projectileB)
						continue;

					//check if the projectile is checking the layer projectile B is on
					if (projectile->collisionLayerToCheck != projectileB->collisionLayer)
						continue;

					//create the rectangle for projectile B's collision rect
					sf::FloatRect projectileBRect{ projectileB->position + projectileB->velocity, projectileB->size };

					//check if there was a collision
					if (!projectileRect.intersects(projectileBRect))
						continue;

					//check if the projectile should take damage
					if (projectile->takeDamage && projectileB->enableDamage)
						projectile->hp -= 1;

					//check if projectile B should take damage
					if (projectileB->takeDamage && projectile->enableDamage)
						projectileB->hp -= 1;

					//check if there is a collision callback
					if (projectile->collisionCallback)
					{
						projectile->collisionCallback(projectileB->group, projectile);
					}

					//if the projectile is a player bullet, play the hit sounnd. This is a work around for some technical difficulties
					//that could not be solved due to time contraints
					if ((projectile->group == "projectile" || projectileB->group == "projectile") && !(projectile->group == "nebula" || projectileB->group == "nebula"))
					{
						std::random_device rd;
						std::mt19937 generator{ rd() };
						std::uniform_real_distribution<float> pitchDistribution(0.8f, 1.2f);

						gameData.hurtTwoSound.setPitch(pitchDistribution(generator));
						gameData.hurtTwoSound.play();
					}

					//check if the projectile should be destroyed
					if (projectile->hp <= 0)
						projectile = nullptr;

					//check if there is a collision callback
					if (projectileB->collisionCallback)
					{
						projectileB->collisionCallback(projectile->group, projectileB);
					}

					//check if the projectile should be destroyed
					if (projectileB->hp <= 0)
						projectileB = nullptr;

					break;
				}

			//loop over entites
			for (auto& entity : entities)
			{
				//check if the projectile still exists
				if (!projectile)
					break;

				//check if the entity still exists
				if (!entity)
					continue;

				//check if the projectile should check the entity layer
				if (projectile->collisionLayerToCheck != entity->collisionLayer)
					continue;

				//create the entity collision rect
				sf::FloatRect entityRect{ entity->position + entity->velocity, entity->size };

				//check if there was a collision
				if (!projectileRect.intersects(entityRect))
					continue;

				//check if there is a collision callback
				if (projectile->collisionCallback)
				{
					projectile->collisionCallback(entity->group, projectile);
				}

				//check if there is a collision callback
				if (entity->collisionCallback)
				{
					entity->collisionCallback(projectile->group, entity);
				}

				//take damge if enabled
				if (projectile->enableDamage)
					entity->hp -= 1;

				//destroy the projectile if enabled
				if (projectile->dissapearOnHit && projectile->takeDamage)
					projectile = nullptr;

				//destroy the entity on zero hp
				if (entity->hp <= 0)
					entity = nullptr;

				break;
			}
		}
	}
}