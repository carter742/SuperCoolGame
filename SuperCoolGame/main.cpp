#include "./game/game.h"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"

#include <random>

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

	player.acceleration += gm::normalize(movementAcceleration) * conf::PLAYER_MOVEMENT_SPEED;
}


static void initGame(gm::GameData& gameData)
{
	gameData.player = gm::Entity{ { 0.f, 370.f }, { 20.f, 14.f }, sf::Color::Green };
	gameData.player.sprite.setTexture(gameData.rocketshipTexture);
	gameData.player.sprite.setTextureRect(gameData.defaultTextureRect);
	gameData.player.sprite.setScale({ 2.f, 2.f });
	gameData.player.textureOffset = { 2.f, 4.f };
	gameData.entities.push_back(&gameData.player);
}

static void checkWindowInputs(sf::RenderWindow& window)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
	}
}

static void shootPlayerProjectile(gm::GameData& gameData)
{
	if (gameData.frame % 15 == 0)
	{
		gm::Projectile* projectile = new gm::Projectile{
			{gameData.player.position.x + gameData.player.size.x * 0.5f - 5.f, gameData.player.position.y },
			{10.f, 10.f},
			sf::Color::Magenta
		};

		projectile->sprite.setTexture(gameData.playerBulletTexture);
		projectile->sprite.setTextureRect(gameData.defaultTextureRect);
		projectile->textureOffset = { 1.5f, 1.5f };
		projectile->sprite.setScale({ 0.8f, 0.8f });

		std::size_t i = gm::addToVector(gameData.projectiles, projectile);

		gameData.projectiles[i]->velocity = sf::Vector2f{ 0.f, -1.f } * conf::PLAYER_BULLET_SPEED;
		gameData.projectiles[i]->collisionLayerToCheck = 1;
	}
}

static void spawnAsteroids(gm::GameData& gameData)
{
	if (gameData.frame % 10 == 0)
	{
		std::random_device rd;
		std::mt19937 generator{ rd() };

		std::uniform_real_distribution<float> sizeDistribution{ 10.f, 50.f };
		std::uniform_real_distribution<float> speedDistribution{ 1.f, 2.f };
		std::uniform_int_distribution<int> positionDistribution{ 0, 720 };

		const float size = sizeDistribution(generator);
		const int x = positionDistribution(generator);

		gm::Projectile* asteroid = new gm::Projectile{
			{static_cast<float>(x), -49.f},
			{size, size},
			sf::Color::Cyan
		};

		asteroid->sprite.setTexture(gameData.asteroidsTexture);
		asteroid->sprite.setTextureRect(gameData.defaultTextureRect);
		asteroid->sprite.setScale({ (size + 6.f) / 16.f, (size + 6.f) / 16.f });
		asteroid->textureOffset = { 3.f, 3.f };

		std::size_t i = gm::addToVector(gameData.projectiles, asteroid);

		gameData.projectiles[i]->velocity = sf::Vector2f{ 0.f, 1.f } * conf::ENEMY_MOVEMENT_SPEED * speedDistribution(generator);
		gameData.projectiles[i]->friction = { 1.f, 1.f };
		gameData.projectiles[i]->hp = 10;
		gameData.projectiles[i]->collisionLayer = 1;
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

	//create ui
	tgui::Gui gui{ window };

	auto score = tgui::Label::create();
	score->setText("Score: 0");
	score->setTextSize(12);
	score->getRenderer()->setTextColor(sf::Color::White);
	gui.add(score);

	//init game
	gm::GameData gameData;
	initGame(gameData);

	float deltaTime = 0.f;

	//make texture for rendering
	sf::RenderTexture renderTexture;
	if (!renderTexture.create(window.getSize().x, window.getSize().y))
	{
		printf("Failed to make texture!\n");
	}

	//game loop
	while (window.isOpen())
	{
		deltaTime = gameData.clock.restart().asSeconds();

		checkWindowInputs(window);
		playerMovement(gameData.player);
		
		gm::entityMovementCalculations(deltaTime, gameData.entities);
		gm::projectileMovementCalculations(deltaTime, gameData.projectiles);
		
		gm::staticCollisionCheck(gameData.staticBodies, gameData.entities);
		gm::entityCollisionCheck(gameData.entities);
		gm::projectileCollisionCheck(window, gameData.projectiles, gameData.entities);

		renderTexture.clear();
		gm::drawSpriteList(gameData.frame, renderTexture, gameData.projectiles);
		gm::drawSpriteList(gameData.frame, renderTexture, gameData.entities);
		renderTexture.display();


		window.clear();

		sf::Sprite renderTextureSprite{ renderTexture.getTexture() };
		window.draw(renderTextureSprite);

		gui.draw();
		window.display();

		shootPlayerProjectile(gameData);
		spawnAsteroids(gameData);

		gameData.frame += 1;

		//update score every ten frames
		score->setText("Score: " + std::to_string(static_cast<int>(gameData.frame / 20)));
	}
}