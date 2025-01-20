#pragma once
#include <string>
#include "SFML/Graphics.hpp"

namespace gm
{
	struct Entity
	{
		bool collisionEnabled = true;
		bool gravityEnabled = false;

		sf::Vector2f position, size, velocity, acceleration;
		sf::Color color;
		std::string id;

		sf::Vector2f friction = { 0.5f, 0.5f };

		Entity();
		Entity(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color);
		
		bool operator==(const Entity& b);
	};

	struct StaticBody
	{
		sf::Vector2f position, size;
		sf::Color color;
	};

	struct Projectile
	{
		sf::Vector2f position, size;
		sf::Color color;

		sf::Vector2f velocity, acceleration;
		sf::Vector2f friction = { 1.f, 1.f };
	};

	template<typename T>
	void drawRect(sf::RenderWindow& window, const T& entity)
	{
		static sf::RectangleShape rect;
		rect.setPosition(entity.position);
		rect.setSize(entity.size);
		rect.setFillColor(entity.color);
		window.draw(rect);
	}

	template<typename T>
	void drawRectList(sf::RenderWindow& window, const std::vector<T*>& entities)
	{
		static sf::RectangleShape rect;
		for (const auto& entity : entities)
		{
			if (!entity)
				continue;

			rect.setPosition(entity->position);
			rect.setSize(entity->size);
			rect.setFillColor(entity->color);
			window.draw(rect);
		}
	}

	void entityMovementCalculations(const float& deltaTime, std::vector<Entity*>& entities);
	void projectileMovementCalculations(const float& deltaTime, std::vector<Projectile*>& projectiles);

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
		return vector.size() - 1;
	}
}