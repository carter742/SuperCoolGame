﻿#include "SFML/Graphics.hpp"
#include <iostream>
#include <random>

constexpr float PLAYER_MOVEMENT_SPEED = 10.f;
constexpr float PLAYER_JUMP_HEIGHT = 30.f;

struct Entity
{
	bool collisionEnabled = true;
	bool gravityEnabled = true;

	sf::Vector2f position, size, velocity, acceleration;
	sf::Color color;
	std::string id;

	sf::Vector2f friction = { 0.9f, 0.9f };

	Entity(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color): position(position), size(size), color(color), velocity(0.f, 0.f), acceleration(0.f, 0.f)
	{
		id = std::to_string(std::rand()) + std::to_string(std::rand()) + std::to_string(std::rand());
	}

	bool operator==(const Entity& b)
	{
		return id == b.id;
	}
};

struct StaticBody
{
	sf::Vector2f position, size;
	sf::Color color;
};


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
		if (!entityA->collisionEnabled)
			continue;

		sf::FloatRect rectA{ entityA->position + entityA->velocity, entityA->size };

		for (auto& entityB : entities)
		{
			if (entityA == entityB)
				continue;

			sf::FloatRect rectB{ entityB->position + entityB->velocity, entityB->size };

			if (!rectA.intersects(rectB))
				continue;

			const sf::Vector2f overlap = getOverlap(rectA, rectB);

			if (overlap.x < overlap.y)
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
		sf::FloatRect rectA{ entity->position + entity->velocity, entity->size };

		for (auto& staticBody : staticBodies)
		{
			sf::FloatRect rectB{ staticBody->position, staticBody->size };

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

void playerMovement(Entity& player)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		player.acceleration.x -= PLAYER_MOVEMENT_SPEED;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		player.acceleration.x += PLAYER_MOVEMENT_SPEED;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		player.acceleration.y -= PLAYER_JUMP_HEIGHT;
	}
}

void drawRect(sf::RenderWindow& window, const Entity& entity)
{
	static sf::RectangleShape rect;
	rect.setPosition(entity.position);
	rect.setSize(entity.size);
	rect.setFillColor(entity.color);
	window.draw(rect);
}

void drawRect(sf::RenderWindow& window, const StaticBody& staticBody)
{
	static sf::RectangleShape rect;
	rect.setPosition(staticBody.position);
	rect.setSize(staticBody.size);
	rect.setFillColor(staticBody.color);
	window.draw(rect);
}

int main()
{
	sf::Clock clock;
	float deltaTime = 0.f;

	sf::RenderWindow window{ sf::VideoMode{ 800, 400 }, "Game" };
	window.setFramerateLimit(60);

	Entity player{ { 100.f, 0.f }, { 20.f, 20.f }, sf::Color::Green };
	Entity enemy{ {50.f, 0.f}, { 20.f, 20.f }, sf::Color::Cyan };

	StaticBody floor{ {0.f, 200.f}, {800.f, 200.f}, sf::Color::Red};

	std::vector<Entity*> entities{ &player, &enemy };
	std::vector<StaticBody*> staticBodies{ &floor };

	while (window.isOpen())
	{
		deltaTime = clock.restart().asSeconds();

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		playerMovement(player);
		staticCollisionCheck(staticBodies, entities);
		entityCollisionCheck(entities);

		for (auto& entity : entities)
		{
			if (entity->gravityEnabled)
				entity->acceleration.y += 10.f;

			entity->velocity += entity->acceleration * deltaTime;
			entity->position += entity->velocity;

			entity->acceleration = { 0.f, 0.f };

			entity->velocity.x *= entity->friction.x;
			entity->velocity.y *= entity->friction.y;
		}

		window.clear();
		drawRect(window, player);
		drawRect(window, enemy);
		drawRect(window, floor);
		window.display();
	}
}
