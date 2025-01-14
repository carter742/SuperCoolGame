#include "SFML/Graphics.hpp"
#include <iostream>
#include <random>

struct Entity
{
	sf::Vector2f position, size;
	sf::Color color;
	std::string id;

	Entity(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color): position(position), size(size), color(color)
	{
		id = std::to_string(std::rand()) + std::to_string(std::rand()) + std::to_string(std::rand());
		printf("%s\n", id.c_str());
	}

	bool operator==(const Entity& b)
	{
		return id == b.id;
	}
};


void collisionCheck(std::vector<Entity>& entities)
{
	for (auto& entityA : entities)
	{
		sf::FloatRect rectA{ entityA.position, entityA.size };

		for (auto& entityB : entities)
		{
			if (entityA == entityB)
				continue;

			sf::FloatRect rectB{ entityB.position, entityB.size };

			if (!rectA.intersects(rectB))
				continue;

			
		}
	}
}

void playerMovement(Entity& player)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		player.position.x -= 1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		player.position.x += 1;
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

int main()
{
	sf::RenderWindow window{ sf::VideoMode{ 800, 400 }, "Game" };
	window.setFramerateLimit(60);

	Entity player{{ 0.f, 0.f }, { 20.f, 20.f }, sf::Color::Red};
	Entity enemy{ {50.f, 0.f}, { 20.f, 20.f }, sf::Color::Cyan };

	std::vector<Entity> collisionRect{ player, enemy };

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		playerMovement(player);
		collisionCheck(collisionRect);

		window.clear();
		drawRect(window, player);
		drawRect(window, enemy);
		window.display();
	}
}
