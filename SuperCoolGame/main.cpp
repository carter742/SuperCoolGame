#include "SFML/Graphics.hpp"
#include <fstream>
#include <iostream>
#include <random>

constexpr float PLAYER_MOVEMENT_SPEED = 15.f;
constexpr float PLAYER_DASH_SPEED = 500.f;

struct Entity
{
	bool collisionEnabled = true;
	bool gravityEnabled = false;

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

template<typename T>
std::string vectorToString(T vector)
{
	return "x: " + std::to_string(vector.x) + ", y: " + std::to_string(vector.y);
}

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
		player.acceleration.y -= PLAYER_MOVEMENT_SPEED;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		player.acceleration.y += PLAYER_MOVEMENT_SPEED;
	}
}

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
void drawRect(sf::RenderWindow& window, const std::vector<T*>& entities)
{
	static sf::RectangleShape rect;
	for (const auto& entity : entities)
	{
		rect.setPosition(entity->position);
		rect.setSize(entity->size);
		rect.setFillColor(entity->color);
		window.draw(rect);
	}
}


void loadLevel(const std::string levelFilePath)
{
	std::ifstream f{ levelFilePath.c_str() };


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

	sf::View camera{ sf::FloatRect{ player.position, static_cast<sf::Vector2f>(window.getSize()) }};
	window.setView(camera);

	while (window.isOpen())
	{
		deltaTime = clock.restart().asSeconds();

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Right)
				{
					const sf::Vector2f mousePosition = sf::Vector2f{ (float)event.mouseButton.x, (float)event.mouseButton.y } - static_cast<sf::Vector2f>(window.getSize()) * 0.5f;
					player.acceleration = mousePosition / std::sqrt(mousePosition.x * mousePosition.x + mousePosition.y * mousePosition.y) * PLAYER_DASH_SPEED;
				}

				if (event.mouseButton.button == sf::Mouse::Left)
				{
					const sf::Vector2 mousePosition{ (float)event.mouseButton.x, (float)event.mouseButton.y };
					printf("%s\n", vectorToString<sf::Vector2f>(mousePosition).c_str());
				}
			}
		}

		playerMovement(player);

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

		staticCollisionCheck(staticBodies, entities);
		entityCollisionCheck(entities);

		camera.setCenter(player.position + player.size * 0.5f);
		window.setView(camera);

		window.clear();
		drawRect<Entity>(window, entities);
		drawRect<StaticBody>(window, staticBodies);
		window.display();
	}
}