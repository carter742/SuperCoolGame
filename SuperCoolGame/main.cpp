#include "./game/game.h"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"
#include "SFML/Audio.hpp"

#include <random>

static void playerMovement(gm::GameData& gameData)
{
	sf::Vector2f movementAcceleration{ 0.f, 0.f};

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		movementAcceleration.x -= 1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		movementAcceleration.x += 1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		movementAcceleration.y -= 1;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		movementAcceleration.y += 1;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		if (gameData.playerShooting == false)
			gameData.nextShootingFrame = gameData.frame;
		gameData.playerShooting = true;
	}
	else
	{
		gameData.playerShooting = false;
	}

	static bool pkeyUp = true;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::P) && pkeyUp)
	{
		gameData.debugMode = !gameData.debugMode;
		pkeyUp = false;
	}
	else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::P))
	{
		pkeyUp = true;
	}

	gameData.player.acceleration += gm::normalize(movementAcceleration) * conf::PLAYER_MOVEMENT_SPEED;
}

static void playerCollisonReaction(const std::string& group, gm::Base* self)
{
	gm::Entity* player = static_cast<gm::Entity*>(self);
	if (group == "healthPickUp")
	{
		player->hp += 1;
	}

	player->inNebula = group == "nebula";
}

static void initGame(gm::GameData& gameData)
{
	gameData.player = gm::Entity{ { 0.f, 170.f }, { 12.f, 12.f }, sf::Color::Green };
	gameData.player.group = "player";
	gameData.player.sprite.setTexture(gameData.rocketshipTexture);
	gameData.player.sprite.setTextureRect(gameData.defaultTextureRect);
	gameData.player.sprite.setScale({ 1.2f, 1.2f });
	gameData.player.textureOffset = { 1.f, 1.5f };
	gameData.player.collisionCallback = &playerCollisonReaction;
	gameData.player.hp = 5;
	gameData.entities.push_back(&gameData.player);
}

static void checkWindowInputs(sf::RenderWindow& window)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::C)
			{
				window.close();
			}
		}
	}
}

static void shootPlayerProjectile(gm::GameData& gameData)
{
	if (gameData.playerShooting && gameData.frame >= gameData.nextShootingFrame)
	{
		if (!gameData.playerSplitShot)
		{
			gm::Projectile* projectile = new gm::Projectile{
					{gameData.player.position.x + gameData.player.size.x * 0.5f - 5.f, gameData.player.position.y },
					{10.f, 10.f},
					sf::Color::Magenta
			};

			projectile->group = "projectile";
			projectile->sprite.setTexture(gameData.playerBulletTexture);
			projectile->sprite.setTextureRect(gameData.defaultTextureRect);
			projectile->textureOffset = { 1.5f, 1.5f };
			projectile->sprite.setScale({ 0.8f, 0.8f });

			const std::size_t vectorIndex = gm::addToVector(gameData.projectiles, projectile);

			gameData.projectiles[vectorIndex]->velocity = sf::Vector2f{ 0.f, -1.f } *conf::PLAYER_BULLET_SPEED;
			gameData.projectiles[vectorIndex]->collisionLayerToCheck = 1;
			gameData.projectiles[vectorIndex]->collisionLayer = 2;
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				gm::Projectile* projectile = new gm::Projectile{
					{gameData.player.position.x + gameData.player.size.x * 0.5f - 5.f, gameData.player.position.y },
					{10.f, 10.f},
					sf::Color::Magenta
				};

				projectile->group = "projectile";
				projectile->sprite.setTexture(gameData.playerBulletTexture);
				projectile->sprite.setTextureRect(gameData.defaultTextureRect);
				projectile->textureOffset = { 1.5f, 1.5f };
				projectile->sprite.setScale({ 0.8f, 0.8f });

				const std::size_t vectorIndex = gm::addToVector(gameData.projectiles, projectile);

				gameData.projectiles[vectorIndex]->velocity = sf::Vector2f{ static_cast<float>(i - 1) * 0.2f, -1.f } *conf::PLAYER_BULLET_SPEED;
				gameData.projectiles[vectorIndex]->collisionLayerToCheck = 1;
			}
		}
		gameData.nextShootingFrame += 5;
	}
}

static void spawnEnemyRocketShips(gm::GameData& gameData, const bool fromRight)
{
	if (gameData.frame % 20 == 0 && rand() % 2 == 1)
	{
		std::random_device rd;
		std::mt19937 generator{ rd() };

		std::uniform_real_distribution<float> speedDistribution{ 1.f, 2.f };
		std::uniform_int_distribution<int> positionDistribution{ 0, 280 };

		const int y = positionDistribution(generator);

		gm::Projectile*  rocket = new gm::Projectile{
			{(fromRight) ? conf::WINDOW_WIDTH + 20.f : -20.f, static_cast<float>(y)},
			{10.f, 10.f},
			sf::Color::Cyan
		};

		rocket->group = "rocketship";
		rocket->sprite.setTexture(gameData.enemyRocketshipTexture);
		rocket->sprite.setTextureRect(gameData.defaultTextureRect);

		if (!fromRight)
		{
			rocket->sprite.setRotation(180.f);
			rocket->textureOffset = { -11.f, -11.f };
		}

		std::size_t i = gm::addToVector(gameData.projectiles, rocket);

		gameData.projectiles[i]->velocity = sf::Vector2f{ (fromRight) ? - 1.f : 1.f, 0} *conf::ENEMY_ROCKET_SHIP_SPEED * speedDistribution(generator);
		gameData.projectiles[i]->friction = { 1.f, 1.f };
		gameData.projectiles[i]->hp = 1;
		gameData.projectiles[i]->collisionLayer = 1;
	}
}

static void asteroidCallback(gm::GameData& gameData, gm::Base* self)
{
	gm::Projectile* asteroid = static_cast<gm::Projectile*>(self);
	asteroid->size = static_cast<sf::Vector2f>(sf::Vector2i{ asteroid->hp, asteroid->hp });
	asteroid->sprite.setScale({ (asteroid->size.x + 6.f) / 16.f, (asteroid->size.y + 6.f) / 16.f });
}

static void spawnAsteroids(gm::GameData& gameData)
{
	if (gameData.frame % 10 == 0 && rand() % 10 == 1)
	{
		std::random_device rd;
		std::mt19937 generator{ rd() };

		std::uniform_real_distribution<float> sizeDistribution{ 30.f, 50.f };
		std::uniform_real_distribution<float> speedDistribution{ 1.f, 2.f };
		std::uniform_int_distribution<int> positionDistribution{ 0, 380 };

		const float size = sizeDistribution(generator);
		const int x = positionDistribution(generator);

		gm::Projectile* asteroid = new gm::Projectile{
			{static_cast<float>(x), -49.f},
			{size, size},
			sf::Color::Cyan
		};

		asteroid->group = "asteroid";
		asteroid->sprite.setTexture(gameData.asteroidsTexture);
		asteroid->sprite.setTextureRect(gameData.defaultTextureRect);
		asteroid->sprite.setScale({ (size + 6.f) / 16.f, (size + 6.f) / 16.f });
		asteroid->textureOffset = { 3.f, 3.f };

		std::size_t i = gm::addToVector(gameData.projectiles, asteroid);

		gameData.projectiles[i]->velocity = sf::Vector2f{ 0.f, 1.f } *conf::ENEMY_MOVEMENT_SPEED * speedDistribution(generator);
		gameData.projectiles[i]->friction = { 1.f, 1.f };
		gameData.projectiles[i]->hp = static_cast<int>(size);
		gameData.projectiles[i]->maxHp = static_cast<int>(size);
		gameData.projectiles[i]->collisionLayer = 1;

		asteroid->processCallback = &asteroidCallback;
	}
}

static void nebulaCallback(gm::GameData& gameData, gm::Base* self)
{
	gm::Projectile* nebula = static_cast<gm::Projectile*>(self);
	
	if (gameData.frame % 2 == 0)
	{
		nebula->size += sf::Vector2f{0.2f, 0.2f};
		nebula->sprite.setScale({ (nebula->size.x + 6.f) / 16, (nebula->size.y + 6.f) / 16.f });
	}
}

static void spawnNebula(gm::GameData& gameData)
{
	if (gameData.frame % 50 == 0 && rand() % 10 == 1)
	{
		std::random_device rd;
		std::mt19937 generator{ rd() };

		std::uniform_real_distribution<float> speedDistribution{ 1.f, 2.f };
		std::uniform_int_distribution<int> positionDistribution{ 0, 420 };

		const int x = positionDistribution(generator);

		gm::Projectile* nebula = new gm::Projectile{
			{static_cast<float>(x), -49.f},
			{10.f, 10.f},
			sf::Color::Cyan
		};
		
		nebula->sprite.setColor(sf::Color{ 255, 255, 255, 100 });

		std::size_t i = gm::addToVector(gameData.projectiles, nebula);

		gameData.projectiles[i]->processCallback = &nebulaCallback;
		gameData.projectiles[i]->group = "nebula";
		gameData.projectiles[i]->sprite.setTexture(gameData.nebulaTexture);
		gameData.projectiles[i]->sprite.setTextureRect(gameData.defaultTextureRect);
		gameData.projectiles[i]->enableDamage = false;
		gameData.projectiles[i]->takeDamage = false;
		gameData.projectiles[i]->animationLength = 48;
		gameData.projectiles[i]->textureOffset = { 3.f, 3.f };
		gameData.projectiles[i]->velocity = sf::Vector2f{ 0.f, 1.f } * conf::NEBULA_MOVEMENT_SPEED * speedDistribution(generator);
		gameData.projectiles[i]->friction = { 1.f, 1.f };
		gameData.projectiles[i]->hp = 10;
		gameData.projectiles[i]->collisionLayer = 1;
	}
}

static void spawnHealthPickups(gm::GameData& gameData)
{
	if (gameData.frame % 10 == 0 && rand() % 10 == 1)
	{
		std::random_device rd;
		std::mt19937 generator{ rd() };

		std::uniform_real_distribution<float> speedDistribution{ 1.f, 2.f };
		std::uniform_int_distribution<int> positionDistribution{ 0, 720 };

		const int x = positionDistribution(generator);

		gm::Projectile* healthPickUp = new gm::Projectile{
			{static_cast<float>(x), -49.f},
			{15.f, 15.f},
			sf::Color::Cyan
		};

		std::size_t i = gm::addToVector(gameData.projectiles, healthPickUp);

		gameData.projectiles[i]->group = "healthPickUp";
		gameData.projectiles[i]->sprite.setTexture(gameData.heartTexture);
		gameData.projectiles[i]->sprite.setTextureRect(gameData.defaultTextureRect);
		gameData.projectiles[i]->sprite.setColor(sf::Color::Red);
		gameData.projectiles[i]->textureOffset = { -3.5f, -3.5f };
		gameData.projectiles[i]->velocity = sf::Vector2f{ 0.f, 1.f } *conf::ENEMY_MOVEMENT_SPEED * speedDistribution(generator);
		gameData.projectiles[i]->friction = { 1.f, 1.f };
		gameData.projectiles[i]->collisionLayer = 2;
		gameData.projectiles[i]->enableDamage = false;
		gameData.projectiles[i]->animationLength = 1;
	}
}

static void segmentedBarDisplay(sf::RenderWindow& window, const gm::GameData& gameData, const sf::Color color)
{
	static sf::RectangleShape healthPoint;
	healthPoint.setFillColor(color);
	healthPoint.setSize({ 10.f, 10.f });
	
	for (int i = 0; i < gameData.player.hp; i++)
	{
		healthPoint.setPosition(12.f * i + 3.f, 20.f);
		window.draw(healthPoint);
	}
}

static void levels(gm::GameData& gameData)
{
	const unsigned long long& score = gameData.score;

	spawnAsteroids(gameData);

	if (gameData.player.hp < 5)
		spawnHealthPickups(gameData);

	if (score > 100)
	{
		spawnEnemyRocketShips(gameData, true);
	}
	

	if (score > 200)
	{
		spawnNebula(gameData);
	}


	if (score > 300)
	{
		spawnEnemyRocketShips(gameData, false);
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
	if (!renderTexture.create(static_cast<unsigned int>(conf::WINDOW_WIDTH), static_cast<unsigned int>(conf::WINDOW_HEIGHT)))
	{
		printf("Failed to make texture!\n");
	}

	sf::Vector2f scaleFactor = {
		static_cast<float>(window.getSize().x) / conf::WINDOW_WIDTH,
		static_cast<float>(window.getSize().y) / conf::WINDOW_HEIGHT
	};

	//music
	sf::Music music;
	music.openFromFile("./assets/music/SpaceSong.oga");
	music.setLoopPoints({ sf::milliseconds(0), sf::milliseconds(27435) });
	music.setLoop(true);
	music.play();


	//game loop
	while (window.isOpen())
	{
		deltaTime = gameData.clock.restart().asSeconds();

		checkWindowInputs(window);
		playerMovement(gameData);
		
		gm::executeProcesses(gameData, gameData.projectiles);

		gm::entityMovementCalculations(deltaTime, gameData.entities);
		gm::projectileMovementCalculations(deltaTime, gameData.projectiles);
		
		gm::staticCollisionCheck(gameData.staticBodies, gameData.entities);
		gm::entityCollisionCheck(gameData.entities);
		gm::projectileCollisionCheck(window, gameData.projectiles, gameData.entities);

		renderTexture.clear();
		if (gameData.debugMode)
			gm::drawRectList(renderTexture, gameData.projectiles);

		gm::drawSpriteList(gameData.frame, renderTexture, gameData.projectiles);
		gm::drawSpriteList(gameData.frame, renderTexture, gameData.entities);

		if (gameData.debugMode)
			gm::drawRectList(renderTexture, gameData.entities);
		renderTexture.display();


		window.clear();

		sf::Sprite renderTextureSprite{ renderTexture.getTexture() };
		renderTextureSprite.setScale(scaleFactor);
		window.draw(renderTextureSprite);

		segmentedBarDisplay(window, gameData, sf::Color{ 0, 150, 0 });

		gui.draw();
		window.display();

		shootPlayerProjectile(gameData);
		
		levels(gameData);

		gameData.frame += 1;

		//update score every ten frames
		gameData.score = static_cast<unsigned long long>(gameData.frame / 20);

		score->setText("Score: " + std::to_string(gameData.score));
	}
}