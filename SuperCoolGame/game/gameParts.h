#pragma once
#include <string>
#include "SFML/Graphics.hpp"

namespace gm
{
	class Base
	{
	public:
		sf::Vector2f position, size;
		sf::Color color;

		unsigned int collisionLayer = 0;
		unsigned int collisionLayerToCheck = 0;

		Base() = default;
		Base(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : position(position), size(size), color(color) {};
	};

	class Entity : public Base
	{
	public:
		bool collisionEnabled = true;
		bool gravityEnabled = false;

		sf::Vector2f velocity, acceleration;
		std::string id;

		sf::Vector2f friction = { 0.5f, 0.5f };

		int hp = 10;

		Entity();
		Entity(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color);
		
		bool operator==(const Entity& b);
	};

	class StaticBody: public Base
	{
	public:
		StaticBody() = default;
		StaticBody(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : Base(position, size, color) {}
	};

	class Projectile : public Base
	{
	public:
		sf::Vector2f velocity, acceleration;
		sf::Vector2f friction = { 1.f, 1.f };

		Projectile() : Base({ 0.f, 0.f }, { 0.f, 0.f }, sf::Color::Red) {}
		Projectile(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : Base(position, size, color) {}
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
		return static_cast<std::size_t>(vector.size() - 1);
	}
}