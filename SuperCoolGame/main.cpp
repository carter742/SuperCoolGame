#include "./game/game.h"
#include "TGUI/TGUI.hpp"
#include "TGUI/Backend/SFML-Graphics.hpp"
#include "SFML/Audio.hpp"

#include <random>

// Moves the player and handles player shooting. 
static void playerMovement(gm::GameData& gameData)
{
	sf::Vector2f movementAcceleration{ 0.f, 0.f};

	//move left
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		movementAcceleration.x -= 1;
	}
	//move right
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		movementAcceleration.x += 1;
	}
	//move up
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		movementAcceleration.y -= 1;
	}
	//move down
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		movementAcceleration.y += 1;
	}

	//shoot
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

	//debug mode
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

	//assign player acceleration for movement
	gameData.player.acceleration += gm::normalize(movementAcceleration) * conf::PLAYER_MOVEMENT_SPEED;
}

// @brief Called when the player collides with an object
static void playerCollisonReaction(const std::string& group, gm::Base* self)
{
	//typecast to Entity to access extra varaibles
	gm::Entity* player = static_cast<gm::Entity*>(self);

	//check if player has picked up health
	if (group == "healthPickUp")
	{
		player->hp += 1;
	}

	//used to slow down player if they are in a nebula
	player->inNebula = group == "nebula";
}

//@brief inizializes the player
static void initGame(gm::GameData& gameData)
{
	//player attributes
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

//check for any window inputs
static void checkWindowInputs(sf::RenderWindow& window, tgui::Gui& gui)
{
	//check window events
	sf::Event event;
	while (window.pollEvent(event))
	{
		gui.handleEvent(event);

		//handle window close
		if (event.type == sf::Event::Closed)
			window.close();
	}
}

//controls the players bullets
static void shootPlayerProjectile(gm::GameData& gameData)
{
	//checks if the player is currently shooting and if enough time has passed to shoot again.
	if (gameData.playerShooting && gameData.frame >= gameData.nextShootingFrame)
	{
		//shoot one bullet at a time
		if (!gameData.playerSplitShot)
		{
			//create a projectile
			gm::Projectile* projectile = new gm::Projectile{
					{gameData.player.position.x + gameData.player.size.x * 0.5f - 5.f, gameData.player.position.y },
					{10.f, 10.f},
					sf::Color::Magenta
			};

			//add to the projectile list
			const std::size_t vectorIndex = gm::addToVector(gameData.projectiles, projectile);

			//assign attributes
			gameData.projectiles[vectorIndex]->group = "projectile";
			gameData.projectiles[vectorIndex]->sprite.setTexture(gameData.playerBulletTexture);
			gameData.projectiles[vectorIndex]->sprite.setTextureRect(gameData.defaultTextureRect);
			gameData.projectiles[vectorIndex]->textureOffset = { 1.5f, 1.5f };
			gameData.projectiles[vectorIndex]->sprite.setScale({ 0.8f, 0.8f });
			gameData.projectiles[vectorIndex]->velocity = sf::Vector2f{ 0.f, -1.f } *conf::PLAYER_BULLET_SPEED;
			gameData.projectiles[vectorIndex]->collisionLayerToCheck = 1;
			gameData.projectiles[vectorIndex]->collisionLayer = 2;
		}
		else
		{
			//shoot multiple projectiles
			for (int i = 0; i < 3; i++)
			{
				//create projectile
				gm::Projectile* projectile = new gm::Projectile{
					{gameData.player.position.x + gameData.player.size.x * 0.5f - 5.f, gameData.player.position.y },
					{10.f, 10.f},
					sf::Color::Magenta
				};

				//add to the projectile list
				const std::size_t vectorIndex = gm::addToVector(gameData.projectiles, projectile);

				//assign attributes
				gameData.projectiles[vectorIndex]->group = "projectile";
				gameData.projectiles[vectorIndex]->sprite.setTexture(gameData.playerBulletTexture);
				gameData.projectiles[vectorIndex]->sprite.setTextureRect(gameData.defaultTextureRect);
				gameData.projectiles[vectorIndex]->textureOffset = { 1.5f, 1.5f };
				gameData.projectiles[vectorIndex]->sprite.setScale({ 0.8f, 0.8f });
				gameData.projectiles[vectorIndex]->velocity = sf::Vector2f{ static_cast<float>(i - 1) * 0.2f, -1.f } *conf::PLAYER_BULLET_SPEED;
				gameData.projectiles[vectorIndex]->collisionLayerToCheck = 1;
			}
		}
		
		//generate a random pitch
		std::random_device rd;
		std::mt19937 generator{ rd() };
		std::uniform_real_distribution<float> pitchDistribution(0.8f, 1.2f);

		//play shooting sound and set pitch
		gameData.shootingSound.setPitch(pitchDistribution(generator));
		gameData.shootingSound.play();

		//set the next frame that the player will shoot on
		gameData.nextShootingFrame += 5;
	}
}

//spawn rocket ships
static void spawnEnemyRocketShips(gm::GameData& gameData, const bool fromRight, int spawnRate = 2)
{
	//has a 50% chance to spawn a rocket every 20 frame 
	if (gameData.frame % 20 == 0 && rand() % spawnRate == 1)
	{
		//used to generate random numbers
		std::random_device rd;
		std::mt19937 generator{ rd() };

		//random number generator for speed and position
		std::uniform_real_distribution<float> speedDistribution{ 1.f, 2.f };
		std::uniform_int_distribution<int> positionDistribution{ 0, 280 };

		//get random y coordinate
		const int y = positionDistribution(generator);

		//create the rocket
		gm::Projectile*  rocket = new gm::Projectile{
			{(fromRight) ? conf::WINDOW_WIDTH + 20.f : -20.f, static_cast<float>(y)},
			{10.f, 10.f},
			sf::Color::Cyan
		};
		
		//change the movement direction if the player is moving from the left
		if (!fromRight)
		{
			rocket->sprite.setRotation(180.f);
			rocket->textureOffset = { -11.f, -11.f };
		}

		//add the rocket to the projectile vector
		std::size_t i = gm::addToVector(gameData.projectiles, rocket);

		//assign rocket attributes
		gameData.projectiles[i]->group = "rocketship";
		gameData.projectiles[i]->sprite.setTexture(gameData.enemyRocketshipTexture);
		gameData.projectiles[i]->sprite.setTextureRect(gameData.defaultTextureRect);
		gameData.projectiles[i]->velocity = sf::Vector2f{ (fromRight) ? - 1.f : 1.f, 0} *conf::ENEMY_ROCKET_SHIP_SPEED * speedDistribution(generator);
		gameData.projectiles[i]->friction = { 1.f, 1.f };
		gameData.projectiles[i]->hp = 1;
		gameData.projectiles[i]->collisionLayer = 1;
	}
}

//the asteroids process is called every frame. Controls the size of the asteroid relative to health. 
static void asteroidCallback(gm::GameData& gameData, gm::Base* self)
{
	//typeast asteroid for additional varaibles
	gm::Projectile* asteroid = static_cast<gm::Projectile*>(self);
	
	//set size based on health
	asteroid->size = static_cast<sf::Vector2f>(sf::Vector2i{ asteroid->hp, asteroid->hp });

	//center asteroid if size changes
	if (asteroid->lastSize != asteroid->size && asteroid->lastSize.x != 0)
	{
		asteroid->position += static_cast<sf::Vector2f>(sf::Vector2i{ asteroid->hp, asteroid->hp }) * 0.025f;
	}

	//rescale based on health
	asteroid->sprite.setScale({ (asteroid->size.x + 6.f) / 16.f, (asteroid->size.y + 6.f) / 16.f });

	//used to adjust the size and position
	asteroid->lastSize = asteroid->size;
}

//spawns an asteroid 
static void spawnAsteroids(gm::GameData& gameData, int spawnRates = 10)
{
	if (gameData.frame % 10 == 0 && rand() % spawnRates == 1)
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
		nebula->position -= sf::Vector2f{ 0.1f, 0.1f };
		nebula->sprite.setScale({ (nebula->size.x + 6.f) / 16, (nebula->size.y + 6.f) / 16.f });
	}
}

static void spawnNebula(gm::GameData& gameData, int spawnRates = 10)
{
	if (gameData.frame % 50 == 0 && rand() % spawnRates == 1)
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
	sf::Sprite healthPoint;
	healthPoint.setTexture(gameData.heartTexture);
	healthPoint.setTextureRect(gameData.defaultTextureRect);

	for (int i = 0; i < gameData.player.hp; i++)
	{
		healthPoint.setPosition(12.f * i + 3.f, 20.f);
		window.draw(healthPoint);
	}
}

static void levels(gm::GameData& gameData)
{
	const unsigned long long& score = gameData.score;

	if (score < 200)
		spawnAsteroids(gameData);
	else
	{
		spawnAsteroids(gameData, 5);
	}

	if (gameData.player.hp < 5)
		spawnHealthPickups(gameData);

	if (score > 50)
	{
		spawnEnemyRocketShips(gameData, true);
	}


	if (score > 100)
	{
		spawnNebula(gameData);
	}


	if (score > 150)
	{
		spawnEnemyRocketShips(gameData, false);
	}

	if (score > 250)
	{
		spawnEnemyRocketShips(gameData, true);
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
	tgui::Theme blackTheme{ "../TGUI-1.x-nightly/themes/Black.txt" };

	auto titleText = tgui::Label::create();
	titleText->setText("Launch Off");
	titleText->setPosition(0, "20%");
	titleText->setSize("100%", "30%");
	titleText->setRenderer(blackTheme.getRenderer("Label"));
	titleText->setTextSize(100);
	titleText->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
	gui.add(titleText, "title");

	auto startButton = tgui::Button::create();
	startButton->setText("Start");
	startButton->setPosition("50% - 10%", "title.bottom");
	startButton->setSize("20%", "10%");
	startButton->setRenderer(blackTheme.getRenderer("Button"));
	startButton->setTextSize(50);
	gui.add(startButton, "startButton");

	bool startGame = false;

	startButton->onClick([&startGame, &gui, &titleText, &startButton](){
		startGame = true;
		gui.remove(titleText);
		gui.remove(startButton);
	});

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

	while (window.isOpen()) 
	{
		////main menu
		while (window.isOpen() && !startGame)
		{
			window.clear();
			checkWindowInputs(window, gui);
			gui.draw();
			window.display();
		}

		auto score = tgui::Label::create();
		score->setText("Score: 0");
		score->setTextSize(28);
		score->getRenderer()->setTextColor(sf::Color::White);
		gui.add(score);

		//game loop
		music.setLoopPoints({ sf::milliseconds(0), sf::milliseconds(27435) });
		music.setLoop(true);
		music.play();

		while (window.isOpen() && gameData.player.hp > 0)
		{
			deltaTime = gameData.clock.restart().asSeconds();

			checkWindowInputs(window, gui);
			playerMovement(gameData);
		
			gm::executeProcesses(gameData, gameData.projectiles);

			gm::entityMovementCalculations(deltaTime, gameData.entities);
			gm::projectileMovementCalculations(deltaTime, gameData.projectiles);
		
			gm::staticCollisionCheck(gameData.staticBodies, gameData.entities);
			gm::entityCollisionCheck(gameData.entities);
			gm::projectileCollisionCheck(gameData, gameData.projectiles, gameData.entities);

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

			if (gameData.player.hp > gameData.lastPlayerHp)
			{
				std::random_device rd;
				std::mt19937 generator{ rd() };
				std::uniform_real_distribution<float> pitchDistribution(0.8f, 1.2f);

				gameData.healthSound.setPitch(pitchDistribution(generator));
				gameData.healthSound.play();
			}
			else if (gameData.player.hp < gameData.lastPlayerHp)
			{
				std::random_device rd;
				std::mt19937 generator{ rd() };
				std::uniform_real_distribution<float> pitchDistribution(0.8f, 1.2f);

				gameData.hurtSound.setPitch(pitchDistribution(generator));
				gameData.hurtSound.play();
			}

			gameData.lastPlayerHp = gameData.player.hp;
		}

		if (gameData.highscore < gameData.score)
			gameData.highscore = gameData.score;

		music.stop();
		gui.remove(score);

		auto gameoverTitleText = tgui::Label::create();
		gameoverTitleText->setText("Score: " + std::to_string(gameData.score));
		gameoverTitleText->setPosition(0, "20%");
		gameoverTitleText->setSize("100%", "20%");
		gameoverTitleText->setRenderer(blackTheme.getRenderer("Label"));
		gameoverTitleText->setTextSize(80);
		gameoverTitleText->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
		gui.add(gameoverTitleText, "title");

		auto highscoreText = tgui::Label::create();
		highscoreText->setText("Highscore: " + std::to_string(gameData.highscore));
		highscoreText->setPosition(0, "title.bottom");
		highscoreText->setSize("100%", "30%");
		highscoreText->setRenderer(blackTheme.getRenderer("Label"));
		highscoreText->setTextSize(50);
		highscoreText->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
		gui.add(highscoreText, "highscore");

		auto restartButton = tgui::Button::create();
		restartButton->setText("Start");
		restartButton->setPosition("50% - 10%", "highscore.bottom");
		restartButton->setSize("20%", "10%");
		restartButton->setRenderer(blackTheme.getRenderer("Button"));
		restartButton->setTextSize(50);
		gui.add(restartButton, "restartButton");

		

		bool exitGameOverMenu = false;

		restartButton->onClick([&exitGameOverMenu, &startGame, &gameData](){
			exitGameOverMenu = true;
			gameData.frame = 0;
			gameData.score = 0;
			gameData.projectiles.clear();
			gameData.entities.clear();
			gameData.staticBodies.clear();
			initGame(gameData);
		});

		while (window.isOpen() && !exitGameOverMenu)
		{
			window.clear();
			checkWindowInputs(window, gui);
			gui.draw();
			window.display();
		}

		gui.remove(gameoverTitleText);
		gui.remove(highscoreText);
		gui.remove(restartButton);
	}
}