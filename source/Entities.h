/*
// Header file containing class and function prototypes.
*/

#ifndef ENTITIES_H
#define ENTITIES_H

#include <nds.h>
#include <stdlib.h>
#include <stdio.h>

class Entity{
	public:
		Entity(int x, int y, int xVelocity, int yVelocity, int entID);
		virtual ~Entity();
		int getX();
		int getY();
		int getBoundingBoxX();
		int getBoundingBoxY();
		int getBoundingBoxWidth();
		int getBoundingBoxHeight();
		void setX(int x);
		void setY(int y);
		void setXVelocity(int xVelocity);
		void setYVelocity(int yVelocity);
		void setGFX(u16* gfx);
		void Update(u16 KeysHeld);
		void setActive(bool active);
		bool isActive();
		Entity* GetCollision(Entity* entities[], int numEntities);
		virtual void UpdateEntity(u16 KeysHeld) = 0;
		virtual void Draw(void);
		virtual void OnHit(Entity* entity);
	protected:
		int x,y, xVelocity, yVelocity, rotation, boundingBoxX, boundingBoxY, boundingBoxWidth, boundingBoxHeight, rotationSpeed, entityID, palleteID;
		bool active, rotatable;
		u16* gfx;
		SpriteSize spriteSize;
};

class Player : public Entity{
	public:
		Player(int x, int y, int xVelocity, int yVelocity, int entID);
		~Player();
	private:
		void UpdateEntity(u16 KeysHeld) override;
		void OnHit(Entity* entity) override;
};

class Barrel : public Entity{
	public:
		Barrel(int x, int y, int xVelocity, int yVelocity, int entID);
		~Barrel();
	private:
		void UpdateEntity(u16 KeysHeld) override;
		void OnHit(Entity* entity) override;
};

class Fracas : public Entity{
	public:
		Fracas(int x, int y, int xVelocity, int yVelocity, int entID);
		~Fracas();
		void Draw(void) override;
		int getHP();
	private:
		int hp;
		u16* gfxTR;
		u16* gfxBL;
		u16* gfxBR;
		void UpdateEntity(u16 KeysHeld) override;
		void OnHit(Entity* entity) override;
};

class Bullet : public Entity{
	public:
		Bullet(int x, int y, int xVelocity, int yVelocity, int entID);
		~Bullet();
	private:
		void UpdateEntity(u16 KeysHeld) override;
};

#endif
