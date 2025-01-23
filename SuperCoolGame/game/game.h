#pragma once

#include "vectorMath.h"
#include "SFML/Graphics.hpp"

#include <array>
#include <vector>
#include <cmath>
#include <string>

namespace conf
{
	constexpr float WINDOW_WIDTH = 400.f;
	constexpr float WINDOW_HEIGHT = 200.f;

	constexpr float PLAYER_MOVEMENT_SPEED = 100.f;
	constexpr float PLAYER_DASH_SPEED = 500.f;
	constexpr float PLAYER_BULLET_SPEED = 5.f;
	constexpr float ENEMY_ROCKET_SHIP_SPEED = 2.f;
	constexpr float ENEMY_MOVEMENT_SPEED = 0.5f;
	constexpr float NEBULA_MOVEMENT_SPEED = 0.1f;
}

namespace gm
{
	class GameData;
}

namespace gm
{
	class Base
	{
	public:
		using CollisionCallback = void (*)(const std::string& group, Base* self);
		CollisionCallback collisionCallback = nullptr;

		using ProcessCallback = void (*)(GameData& gameData, Base* self);
		ProcessCallback processCallback = nullptr;

		sf::Sprite sprite;
		sf::Vector2f textureOffset;

		unsigned int timeBetweenAnimationFrames = 10;
		unsigned int animationLength = 32;

		sf::Vector2f position, size;
		sf::Color color;

		std::string group;

		unsigned int collisionLayer = 0;
		unsigned int collisionLayerToCheck = 0;

		Base() = default;
		Base(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : position(position), size(size), color(color) {};
	};

	class Entity : public Base
	{
	public:
		bool inNebula = false;

		bool collisionEnabled = true;
		bool gravityEnabled = false;

		sf::Vector2f velocity;
		sf::Vector2f acceleration;
		std::string id;

		sf::Vector2f friction = { 0.5f, 0.5f };

		int hp = 3;

		Entity();
		Entity(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color);

		bool operator==(const Entity& b);
	};

	class StaticBody : public Base
	{
	public:
		StaticBody() = default;
		StaticBody(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : Base(position, size, color) {}
	};

	class Projectile : public Base
	{
	public:
		bool enableDamage = true;
		bool takeDamage = true;
		bool dissapearOnHit = true;

		sf::Vector2f velocity;
		sf::Vector2f acceleration;
		sf::Vector2f friction = { 1.f, 1.f };

		int maxHp = 1;
		int hp = 1;

		Projectile() : Base({ 0.f, 0.f }, { 0.f, 0.f }, sf::Color::Red) {}
		Projectile(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : Base(position, size, color) {}
	};
}


namespace gm
{
	class GameData
	{
	public:
		unsigned long long frame = 0;
		unsigned long long nextShootingFrame = 0;
		unsigned long long score = 0;

		bool playerSplitShot = true;
		bool playerShooting = false;
		bool playerInNebula = false;

		bool debugMode = false;

		sf::Clock clock;

		Entity player;
		unsigned int playerAmmo = 20;

		std::vector<Entity*> entities;
		std::vector<StaticBody*> staticBodies;
		std::vector<Projectile*> projectiles;

		const sf::IntRect defaultTextureRect{ {0, 0}, {16, 16} };
		sf::Texture rocketshipTexture;
		sf::Texture asteroidsTexture;
		sf::Texture playerBulletTexture;
		sf::Texture nebulaTexture;
		sf::Texture enemyRocketshipTexture;
		sf::Texture heartTexture;

		GameData();
	};
}

namespace gm
{
	template<typename R, typename T>
	void drawRect(R& texture, const T& entity)
	{
		static sf::RectangleShape rect;
		rect.setPosition(entity.position);
		rect.setSize(entity.size);
		rect.setFillColor(entity.color);
		texture.draw(rect);
	}

	template<typename R, typename T>
	void drawRectList(R& texture, const std::vector<T*>& entities)
	{
		static sf::RectangleShape rect;
		for (const auto& entity : entities)
		{
			if (!entity)
				continue;

			rect.setPosition(entity->position);
			rect.setSize(entity->size);
			rect.setFillColor(entity->color);
			texture.draw(rect);
		}
	}


	template<typename R, typename T>
	void drawSprite(const unsigned long long& frame, R& texture, T& entity)
	{
		entity.sprite.setPosition(entity.position - entity.textureOffset);

		sf::IntRect entityRect = entity.sprite.getTextureRect();

		if (frame % entity.timeBetweenAnimationFrames == 0)
		{
			entityRect.left += 16;
			if (entityRect.left > entity.animationLength)
				entityRect.left = 0;
		}

		entity.sprite.setTextureRect(entityRect);
		texture.draw(entity.sprite);
	}

	template<typename R, typename T>
	void drawSpriteList(const unsigned long long& frame, R& texture, std::vector<T*>& entities)
	{
		for (auto& entity : entities)
		{
			if (!entity)
				continue;

			entity->sprite.setPosition(entity->position - entity->textureOffset);

			sf::IntRect entityRect = entity->sprite.getTextureRect();

			if (frame % entity->timeBetweenAnimationFrames == 0)
			{
				entityRect.left += 16;
				if (entityRect.left > static_cast<int>(entity->animationLength))
					entityRect.left = 0;
			}

			entity->sprite.setTextureRect(entityRect);
			texture.draw(entity->sprite);
		}
	}

	template<typename T>
	std::size_t addToVector(std::vector<T*>& vector, T* element)
	{
		for (std::size_t i = 0; i < vector.size(); i++)
		{
			if (!vector[i])
			{
				vector[i] = element;
				return i;
			}
		}

		vector.push_back(element);
		return static_cast<std::size_t>(vector.size() - 1);
	}
}

namespace gm
{
	void entityMovementCalculations(const float& deltaTime, std::vector<Entity*>& entities);
	void projectileMovementCalculations(const float& deltaTime, std::vector<Projectile*>& projectiles);
	
	template<typename T>
	void executeProcesses(GameData& gameData, std::vector<T*>& objects)
	{
		for (auto& object : objects)
		{
			if (!object || !object->processCallback)
				continue;

			object->processCallback(gameData, static_cast<Base*>(object));
		}
	}
}

namespace gm
{
	sf::Vector2f getOverlap(const sf::FloatRect& rectA, const sf::FloatRect& rectB);
	void entityCollisionCheck(std::vector<Entity*>& entities);
	void staticCollisionCheck(std::vector<StaticBody*>& staticBodies, std::vector<Entity*>& entities);
	void projectileCollisionCheck(const sf::RenderWindow& window, std::vector<Projectile*>& projectiles, std::vector<Entity*>& entities);
}