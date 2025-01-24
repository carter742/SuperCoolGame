#pragma once

#include "vectorMath.h"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"

#include <array>
#include <vector>
#include <cmath>
#include <string>
#include <random>

//game configuration
namespace conf
{
	constexpr float WINDOW_WIDTH = 400.f;
	constexpr float WINDOW_HEIGHT = 200.f;

	constexpr float PLAYER_MOVEMENT_SPEED = 100.f;
	constexpr float PLAYER_DASH_SPEED = 500.f;
	constexpr float PLAYER_BULLET_SPEED = 5.f;
	constexpr float ENEMY_ROCKET_SHIP_SPEED = 2.f;
	constexpr float ENEMY_MOVEMENT_SPEED = 0.5f;
	constexpr float NEBULA_MOVEMENT_SPEED = 0.1f;
}

namespace gm
{
	class GameData; //information on this class further down in file ;)
}

//Game Objects
namespace gm
{
	//This is the base object all other objects build off of.
	class Base
	{
	public:
		//This callback is called whenever a collision happens to this object
		using CollisionCallback = void (*)(const std::string& group, Base* self);
		CollisionCallback collisionCallback = nullptr;

		//This callback is called every frame to do whatever you want.
		using ProcessCallback = void (*)(GameData& gameData, Base* self);
		ProcessCallback processCallback = nullptr;

		//this is the sprite for the object
		sf::Sprite sprite;

		//Offsets the texture of the sprite
		sf::Vector2f textureOffset;

		//controls the number of frames between each animation frame
		unsigned int timeBetweenAnimationFrames = 10;

		/*
		* controls the length of the animation(This is in pixels not frames).
		* ie. 3 16x16 images will have a length of 48.
		*/
		unsigned int animationLength = 32;

		//size and position of collision rect
		sf::Vector2f position, size;

		//color of collision rect
		sf::Color color;

		//can be used for identifying objects in the collision callback
		std::string group;

		//the object will only collide with other objects that check the set layer. Zero means it is a part of all layers
		unsigned int collisionLayer = 0;

		//the object will only check for collisions with other objects that are on the same layer. Zero means it is a part of all layers
		unsigned int collisionLayerToCheck = 0;

		Base() = default;
		Base(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : position(position), size(size), color(color) {};
	};

	/*
	* This class is used for the player. It is called Entity because I was planning on
	* using it for more then just the player.
	*/
	class Entity : public Base
	{
	public:
		//checks if the player is in a nebula to slow down its speed
		bool inNebula = false;

		//controls if collisons happen or not
		bool collisionEnabled = true;

		//disables and enables gravity
		bool gravityEnabled = false;

		//used for movement calculations
		sf::Vector2f velocity;
		sf::Vector2f acceleration;
		sf::Vector2f friction = { 0.5f, 0.5f };

		//used for comparisons of entities
		std::string id;

		//player health
		int hp = 3;

		Entity();
		Entity(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color);

		bool operator==(const Entity& b);
	};

	//Static bodies don't move, only use if you're making something that doesn't move.
	class StaticBody : public Base
	{
	public:
		StaticBody() = default;
		StaticBody(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : Base(position, size, color) {}
	};

	//Projectiles are made to hurt the player
	class Projectile : public Base
	{
	public:
		//this can disable the damage the projectile gives
		bool enableDamage = true;

		//this can disable the damage the projectile takes
		bool takeDamage = true;

		/*
		* this controls if the projectile will die in one it.
		* Don't know why I didn't just set the hp to 1, but I don't
		* want to remove it because I can't remember why I used it. 
		*/
		bool dissapearOnHit = true;

		//used for movement calculations
		sf::Vector2f velocity;
		sf::Vector2f acceleration;
		sf::Vector2f friction = { 1.f, 1.f };
		sf::Vector2f lastSize;

		//health
		int maxHp = 1;
		int hp = 1;

		Projectile() : Base({ 0.f, 0.f }, { 0.f, 0.f }, sf::Color::Red) {}
		Projectile(const sf::Vector2f position, const sf::Vector2f size, const sf::Color color) : Base(position, size, color) {}
	};
}

//The gameData stores any important data that needs to be accessed. 
namespace gm
{
	class GameData
	{
	public:
		//keeps track of the current frame for time.
		unsigned long long frame = 0;

		//keeps track of when the player can shoot again
		unsigned long long nextShootingFrame = 0;

		//keeps track of the score
		unsigned long long score = 0;
		unsigned long long highscore = 0;

		//player shooting controls
		bool playerSplitShot = true;
		bool playerShooting = false;

		//enable or disable drawing collisions
		bool debugMode = false;

		//used for delta time
		sf::Clock clock;

		Entity player;

		//used to tell if the player has healed or lost health
		int lastPlayerHp = 5;

		//keeps track of game objects
		std::vector<Entity*> entities;
		std::vector<StaticBody*> staticBodies;
		std::vector<Projectile*> projectiles;
		
		//stores loaded sound effects and also stores music
		sf::SoundBuffer shootingSoundBuffer;
		sf::Sound shootingSound;
		sf::SoundBuffer healthSoundBuffer;
		sf::Sound healthSound;
		sf::SoundBuffer hurtSoundBuffer;
		sf::Sound hurtSound;
		sf::SoundBuffer hurtTwoSoundBuffer;
		sf::Sound hurtTwoSound;

		//stores loaded textures
		const sf::IntRect defaultTextureRect{ {0, 0}, {16, 16} };
		sf::Texture rocketshipTexture;
		sf::Texture asteroidsTexture;
		sf::Texture playerBulletTexture;
		sf::Texture nebulaTexture;
		sf::Texture enemyRocketshipTexture;
		sf::Texture heartTexture;

		GameData();
	};
}

//drawing functions
namespace gm
{
	//Draw the rect of type T to type R.
	template<typename R, typename T>
	void drawRect(R& texture, const T& entity)
	{
		static sf::RectangleShape rect;
		rect.setPosition(entity.position);
		rect.setSize(entity.size);
		rect.setFillColor(entity.color);
		texture.draw(rect);
	}

	//Draw the list of rects of type T to type R.
	template<typename R, typename T>
	void drawRectList(R& texture, const std::vector<T*>& entities)
	{
		static sf::RectangleShape rect;
		for (const auto& entity : entities)
		{
			if (!entity)
				continue;

			rect.setPosition(entity->position);
			rect.setSize(entity->size);
			rect.setFillColor(entity->color);
			texture.draw(rect);
		}
	}

	//Draw the sprite of type T to type R.
	template<typename R, typename T>
	void drawSprite(const unsigned long long& frame, R& texture, T& entity)
	{
		//apply sprite offset
		entity.sprite.setPosition(entity.position - entity.textureOffset);

		//get sprite rect for animating
		sf::IntRect entityRect = entity.sprite.getTextureRect();

		//check if the current frame should update.
		if (frame % entity.timeBetweenAnimationFrames == 0)
		{
			//update the current frame and loop to 0 if it is outside of the animation length
			entityRect.left += 16;
			if (entityRect.left > entity.animationLength)
				entityRect.left = 0;
		}

		//draw the sprite
		entity.sprite.setTextureRect(entityRect);
		texture.draw(entity.sprite);
	}

	//Draw the sprite list of type T to type R.
	template<typename R, typename T>
	void drawSpriteList(const unsigned long long& frame, R& texture, std::vector<T*>& entities)
	{
		for (auto& entity : entities)
		{
			if (!entity)
				continue;

			//apply sprite offset
			entity->sprite.setPosition(entity->position - entity->textureOffset);

			//get sprite rect for animating
			sf::IntRect entityRect = entity->sprite.getTextureRect();

			//check if the current frame should update.
			if (frame % entity->timeBetweenAnimationFrames == 0)
			{
				//update the current frame and loop to 0 if it is outside of the animation length
				entityRect.left += 16;
				if (entityRect.left > static_cast<int>(entity->animationLength))
					entityRect.left = 0;
			}

			//draw the sprite
			entity->sprite.setTextureRect(entityRect);
			texture.draw(entity->sprite);
		}
	}
}

namespace gm
{
	/*
	* adds type T to a vector of type T.If there is a empty location the element will be added there instead of the back.
	* returns the size so that the location of the element can be found.
	*/
	template<typename T>
	std::size_t addToVector(std::vector<T*>& vector, T* element)
	{
		//check for empty location to add the element
		for (std::size_t i = 0; i < vector.size(); i++)
		{
			//if there is an empty location add it
			if (!vector[i])
			{
				vector[i] = element;
				return i;
			}
		}

		//if there is no empty location add to the back of the vector
		vector.push_back(element);
		return static_cast<std::size_t>(vector.size() - 1);
	}
}

//object movements and processes
namespace gm
{
	//calculates the entities movement
	void entityMovementCalculations(const float& deltaTime, std::vector<Entity*>& entities);
	//calculates the projectiles movement
	void projectileMovementCalculations(const float& deltaTime, std::vector<Projectile*>& projectiles);
	
	//execute the processes on the objects of type T.
	template<typename T>
	void executeProcesses(GameData& gameData, std::vector<T*>& objects)
	{
		//loop over the objects. 
		for (auto& object : objects)
		{
			//make sure the objects exist and have a process
			if (!object || !object->processCallback)
				continue;

			//execute process
			object->processCallback(gameData, static_cast<Base*>(object));
		}
	}
}

//collision
namespace gm
{
	//finds the overlap of two rectangles
	sf::Vector2f getOverlap(const sf::FloatRect& rectA, const sf::FloatRect& rectB);

	/*
	* collison for the three different objects.I would have found a more elagant approach, where
	* I only need one function, but I ran out of time.
	*/
	void entityCollisionCheck(std::vector<Entity*>& entities);
	void staticCollisionCheck(std::vector<StaticBody*>& staticBodies, std::vector<Entity*>& entities);
	void projectileCollisionCheck(gm::GameData& gameData, std::vector<Projectile*>& projectiles, std::vector<Entity*>& entities);
}