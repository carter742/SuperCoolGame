#include "SFML/Graphics.hpp"
#include "./game/game.h"

constexpr float PLAYER_MOVEMENT_SPEED = 100.f;
constexpr float PLAYER_DASH_SPEED = 500.f;
constexpr float PLAYER_BULLET_SPEED = 10.f;

static void playerMovement(gm::Entity& player)
{
	sf::Vector2f movementAcceleration{ 0.f, 0.f};

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		movementAcceleration.x -= 1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		movementAcceleration.x += 1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		movementAcceleration.y -= 1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		movementAcceleration.y += 1;
	}

	player.acceleration += gm::normalize(movementAcceleration) * PLAYER_MOVEMENT_SPEED;
}


static void initGame(gm::GameData& gameData)
{
	gameData.player = { { 0.f, 370.f }, { 20.f, 20.f }, sf::Color::Green };
	gameData.entities.push_back(&gameData.player);

	gameData.entities.push_back(new gm::Entity{ {50.f, 0.f}, { 20.f, 20.f }, sf::Color::Cyan });
	gameData.entities.back()->collisionLayer = 1;
}

static void checkWindowInputs(sf::RenderWindow& window, gm::GameData& gameData)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Right)
			{
				const sf::Vector2f mousePosition = sf::Vector2f{ (float)event.mouseButton.x, (float)event.mouseButton.y } - static_cast<sf::Vector2f>(window.getSize()) * 0.5f;
				gameData.player.acceleration = gm::normalize(mousePosition) * PLAYER_DASH_SPEED;
			}

			if (event.mouseButton.button == sf::Mouse::Left)
			{
				gameData.projectiles.push_back(new gm::Projectile{ 
					gameData.player.position, 
					{10.f, 10.f}, 
					sf::Color::Magenta 
				});

				gameData.projectiles.back()->velocity = sf::Vector2f{0.f, -1.f} * PLAYER_BULLET_SPEED;
			}
		}
	}
}

void shootPlayerProjectile(gm::GameData& gameData)
{
	if (gameData.frame % 15 == 0)
	{
		gm::Projectile* projectile = new gm::Projectile{
			{gameData.player.position.x + gameData.player.size.x * 0.5f - 5.f, gameData.player.position.y - 10.f},
			{10.f, 10.f},
			sf::Color::Magenta
		};

		std::size_t i = gm::addToVector(gameData.projectiles, projectile);

		gameData.projectiles[i]->velocity = sf::Vector2f{ 0.f, -1.f } *PLAYER_BULLET_SPEED;
		gameData.projectiles[i]->collisionLayerToCheck = 1;
	}
}

void spawnAsteroids(gm::GameData& gameData)
{
	if (gameData.frame % 30 == 0)
	{
		gm::Entity* asteroid = new gm::Entity{
			{static_cast<float>(std::rand() % 780), 0},
			{10.f, 10.f},
			sf::Color::Cyan
		};

		std::size_t i = gm::addToVector(gameData.entities, asteroid);

		gameData.entities[i]->velocity = sf::Vector2f{ 0.f, 1.f } * PLAYER_MOVEMENT_SPEED;
		gameData.entities[i]->collisionLayer = 1;
	}
}

int main()
{
	//create window
	sf::RenderWindow window{ sf::VideoMode{ 800, 400 }, "Game" };
	window.setFramerateLimit(60);

	//create camera
	sf::View camera{ sf::FloatRect{
		{0.f, 0.f},
		static_cast<sf::Vector2f>(window.getSize())
	} };

	window.setView(camera);

	//init game
	gm::GameData gameData;
	initGame(gameData);

	float deltaTime = 0.f;

	//game loop
	while (window.isOpen())
	{
		deltaTime = gameData.clock.restart().asSeconds();

		checkWindowInputs(window, gameData);
		playerMovement(gameData.player);
		
		gm::entityMovementCalculations(deltaTime, gameData.entities);
		gm::projectileMovementCalculations(deltaTime, gameData.projectiles);
		
		gm::staticCollisionCheck(gameData.staticBodies, gameData.entities);
		gm::entityCollisionCheck(gameData.entities);
		gm::projectileCollisionCheck(window, gameData.projectiles, gameData.entities);

		window.clear();
		gm::drawRectList(window, gameData.entities);
		gm::drawRectList(window, gameData.staticBodies);
		gm::drawRectList(window, gameData.projectiles);
		window.display();

		shootPlayerProjectile(gameData);
		spawnAsteroids(gameData);

		gameData.frame += 1;
	}
}