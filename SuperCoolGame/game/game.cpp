#include "game.h"

namespace gm
{
	GameData::GameData()
	{
		rocketshipTexture.loadFromFile("./assets/sprites/rocketship.png");
		asteroidsTexture.loadFromFile("./assets/sprites/asteroids.png");
		playerBulletTexture.loadFromFile("./assets/sprites/playerBullet.png");
	}
}