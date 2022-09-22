/*
// Header file containing class and function prototypes.
// Port created by Colton K - ckenne87 - 251 089 374
*/

#ifndef ENTITIES_H
#define ENTITIES_H

#include <nds.h>
#include <stdlib.h>
#include <stdio.h>

class Entity{
	public:
		Entity(int x, int y, int xVelocity, int yVelocity, int entID);
		~Entity(); 
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
		virtual void UpdateEntity(u16 KeysHeld) = 0;
		void Draw(void);
		virtual void OnHit();
	protected:
		int x,y, xVelocity, yVelocity, rotation, boundingBoxX, boundingBoxY, boundingBoxWidth, boundingBoxHeight, rotationSpeed, entityID, palleteID;
		bool alive, rotatable;
		u16* gfx;
};

class Player : public Entity{
	public:
		Player(int x, int y, int xVelocity, int yVelocity, int entID);
	 	~Player();
	private:
		void UpdateEntity(u16 KeysHeld) override;
		void OnHit() override;
};

class Barrel : public Entity{
	public:
		Barrel(int x, int y, int xVelocity, int yVelocity, int entID);
	 	~Barrel();
	private:
		void UpdateEntity(u16 KeysHeld) override;
		void OnHit() override;
};

class Fracas : public Entity{
	public:
		Fracas(int x, int y, int xVelocity, int yVelocity, int entID);
	 	~Fracas();
	private:
		void UpdateEntity(u16 KeysHeld) override;
};

#endif