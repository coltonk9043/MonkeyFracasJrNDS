/*
// An NDS port of Monkey Fracas Jr. from Futurama
// Port created by Colton K - ckenne87 - 251 089 374
*/

#include <nds.h>
#include <stdlib.h>
#include <stdio.h>
#include "Entities.h"

// Include our texture files (GRIT auto-generated)
#include "barrel.h"
#include "fracas.h"
#include "ship.h"

// Defines constants.
#define NUM_STARS 30
#define NUM_BARRELS 4
#define NUM_BULLETS 10
#define BARRELS_TO_BOSS 25
#define FRACAS_MAX_HP 100

// Static Variables storing GFX information
u16* shipGFX;
u16* barrelGFX;
u16* fracasTLGFX;
u16* fracasTRGFX;
u16* fracasBLGFX;
u16* fracasBRGFX;
u16* bulletGFX;

// Stucture for a basic star.
typedef struct {
	int x, y, speed;
}Star;

// Static variables storing entity information.
int totalEntities = 0;
Star stars[NUM_STARS];
Player* player;
Fracas* fracas;
Barrel* barrels[NUM_BARRELS];
Bullet* bullets[NUM_BULLETS];

// Static variables for game information.
bool gameRunning = true;
bool paused = false;
bool won = false;
bool shotBullet = false;
bool clickedPauseButton = false;
bool fracasSpawned = false;
int barrelsAvoided = 0;
int barrelsShot = 0;

/*
 *	Entity Class Section
 * 	Contains all of the methods of the entity class defined in Entities.h
 */ 
Entity::Entity(int x, int y, int xVelocity, int yVelocity, int entID){
	this->x = x;
	this->y = y;
	this->xVelocity = xVelocity;
	this->yVelocity = yVelocity;
	this->rotation = 0.0f;
	this->active = true;
	this->rotatable = false;
	this->entityID = entID;
}

Entity::~Entity(){}

// Getters and Setters for our Entity class
int Entity::getX() { return this->x; }
int Entity::getY() { return this->y; }
int Entity::getBoundingBoxX() { return this->boundingBoxX; }
int Entity::getBoundingBoxY() { return this->boundingBoxY; }
int Entity::getBoundingBoxWidth() { return this->boundingBoxWidth; }
int Entity::getBoundingBoxHeight() { return this->boundingBoxHeight; }
bool Entity::isActive() { return this->active; }
void Entity::setX(int x) { this->x = x; }
void Entity::setY(int y) { this->y = y; }
void Entity::setXVelocity(int xVelocity) { this->xVelocity = xVelocity; }
void Entity::setYVelocity(int yVelocity) { this->yVelocity = yVelocity; }
void Entity::setGFX(u16* gfx){ this->gfx = gfx; }
void Entity::setActive(bool active) 
{ 
	this->active = active; 
	oamSetHidden(&oamMain, entityID, !active);
}

/**
 * @brief Updates any specific entity. Handles movement, rotation, and drawing.
 * @param KeysHeld 
 */
void Entity::Update(u16 KeysHeld){
	if(this->active){
 		this->UpdateEntity(KeysHeld);
		this->x += this->xVelocity;
		this->y += this->yVelocity;
		if(this->y < 0){
			this->y = 0;
		}else if (this->y > SCREEN_HEIGHT - 32 ){
			this->y = SCREEN_HEIGHT - 32;
		}
		this->rotation = (this->rotation + this->rotationSpeed) % 360;
		this->Draw();
	}
}

/**
 * @brief Given a list of entities, find the first occurence of a collision between two entities.
 * @param entities 
 * @param numEntities 
 * @return Entity* 
 */
Entity* Entity::GetCollision(Entity* entities[], int numEntities){
	// Checks the collision of each barrel in the scene (as that is the only thing that will be affecting the player.)
	for(int i = 0; i < numEntities; i++){
		Entity* entityToCheck = entities[i];
		if(!entityToCheck->active) continue;
		float x1 = x + boundingBoxX + boundingBoxWidth;
		float x2 = entityToCheck->getX() + entityToCheck->getBoundingBoxX() + entityToCheck->getBoundingBoxWidth();
		float y1 = y + boundingBoxY + boundingBoxHeight;
		float y2 = entityToCheck->getY() + entityToCheck->getBoundingBoxY() + entityToCheck->getBoundingBoxHeight();
		
		// Checks to see if there is a collision on the X-axis (such that two objects bounding boxes align on the X axis)
		float maxX;
		float minX;
		if(x + boundingBoxX < entityToCheck->getX() + entityToCheck->getBoundingBoxX()) minX = entityToCheck->getX() + entityToCheck->getBoundingBoxX();
		else minX = x + boundingBoxX;
		if(x1 < x2) maxX = x1;
		else maxX = x2;
		if(maxX <= minX) continue;

		// Checks to see if there is a collision on the Y-axis (such that two objects bounding boxes align on the Y axis)
		float maxY;
		float minY;
		if(y + boundingBoxY < entityToCheck->getY() + entityToCheck->getBoundingBoxY()) minY = entityToCheck->getY() + entityToCheck->getBoundingBoxY();
		else minY = y + boundingBoxY;
		if(y1 < y2) maxY = y1;
		else maxY = y2;
		if(maxY <= minY) continue;

		// If there is a collision on both axis, assume that the bounding boxes are touching.
		return entityToCheck;
	}
	return nullptr;
}

/**
 * @brief Draws a specific entity on the screen.
 */
void Entity::Draw(void){
	// 2 conditions. If an object is rotatable or not. If so, it will determine the angle to rotate by and render it differently.
	if(rotatable){
		oamRotateScale(&oamMain, entityID, degreesToAngle(rotation), intToFixed(1, 8), intToFixed(1, 8));
		oamSet(&oamMain, entityID, x, y, 0, palleteID, spriteSize, SpriteColorFormat_16Color , gfx, entityID, false, !active, false, false, false); 
	}else oamSet(&oamMain, entityID, x, y, 0, palleteID, spriteSize, SpriteColorFormat_16Color , gfx, -1, false, !active, false, false, false);  
}

/**
 * @brief Virtual function that represents when an entity is hit by another.
 * @param entity 
 */
void Entity::OnHit(Entity* entity){
	return;
}

/*
 *	Player Class Section
 * 	Contains all of the methods of the player class defined in Entities.h
 */ 
Player::Player(int x, int y, int xVelocity, int yVelocity, int entID) : Entity(x, y, xVelocity, yVelocity, entID){
	this->gfx = shipGFX;
	this->spriteSize = SpriteSize_32x32;
	this->palleteID = 0;
	this->boundingBoxX = 0;
	this->boundingBoxY = 2;
	this->boundingBoxWidth = 32;
	this->boundingBoxHeight = 24;
	this->rotation = -1;
}

Player::~Player(){}

/**
 * @brief Updates the player entity.
 * @param KeysHeld 
 */
void Player::UpdateEntity(u16 KeysHeld)  {
	if(KeysHeld & KEY_UP) this->yVelocity = -2;
	else if(KeysHeld & KEY_DOWN) this->yVelocity = 2;
	else this->yVelocity = 0;

	Entity* entityHit = this->GetCollision((Entity**)barrels, NUM_BARRELS);
	if(entityHit != nullptr) this->OnHit(entityHit);
}

/**
 * @brief When the player is hit by any entity, perform this action.
 * @param entity
 */
void Player::OnHit(Entity* entity){
	if(won) return;
	gameRunning = false;
}

/*
 *	Barrel Class Section
 * 	Contains all of the methods of the Barrel class defined in Entities.h
 */ 
Barrel::Barrel(int x, int y, int xVelocity, int yVelocity, int entID) : Entity(x, y, xVelocity, yVelocity, entID){
	this->gfx = barrelGFX;
	this->spriteSize = SpriteSize_32x32;
	this->rotatable = true;
	this->palleteID = 1;
	this->boundingBoxX = 8;
	this->boundingBoxY = 8;
	this->boundingBoxWidth = 16;
	this->boundingBoxHeight = 16;
	this->rotationSpeed = 2.0f;
	this->rotation = rand() % 359;
}

Barrel::~Barrel(){}

/**
 * @brief Update method for any barrel entity.
 * @param KeysHeld 
 */
void Barrel::UpdateEntity(u16 KeysHeld)  {
	if(this->x < -32){
		if(fracasSpawned){
			this->active = false;
		}else{
			this->x = 356;
			this->y = rand() % 192;
			barrelsAvoided++;
		}
	}
	Entity* entityHit = this->GetCollision((Entity**)bullets, NUM_BULLETS);
	if(entityHit != nullptr) this->OnHit(entityHit);
}

/**
 * @brief When a barrel is hit by any entity, perform this action.
 * @param entity 
 */
void Barrel::OnHit(Entity* entity){
	entity->setActive(false);
	if(fracasSpawned){
		this->active = false;
	}else{
		this->x = 356;
		this->y = rand() % 192;
		barrelsAvoided++;
	}
	barrelsShot++;
}

/*
 *	Fracas Class Section
 * 	Contains all of the methods of the Fracas class defined in Entities.h
 */ 
Fracas::Fracas(int x, int y, int xVelocity, int yVelocity, int entID) : Entity(x, y, xVelocity, yVelocity, entID){
	this->gfx = fracasTLGFX;
	this->gfxTR = fracasTRGFX;
	this->gfxBL = fracasBLGFX;
	this->gfxBR = fracasBRGFX;
	this->spriteSize = SpriteSize_64x64;
	this->rotatable = false;
	this->palleteID = 2;
	this->boundingBoxWidth = 128;
	this->boundingBoxHeight = 128;
	this->hp = FRACAS_MAX_HP;
}

/**
 * @brief Destroy the Fracas:: Fracas object
 */
Fracas::~Fracas(){
	oamSetHidden(&oamMain, entityID, true);
	oamSetHidden(&oamMain, entityID+1, true);
	oamSetHidden(&oamMain, entityID+2, true);
	oamSetHidden(&oamMain, entityID+3, true);
}

/**
 * @brief Update method for any Fracas entity.
 * @param KeysHeld 
 */
void Fracas::UpdateEntity(u16 KeysHeld)  {
	// Whenever a barrel has reached the left side of the screen, respawn it and let Fracas 'throw' it.
	for(int i = 0; i < NUM_BARRELS; i++){
		Barrel* b = barrels[i];
		if(b->isActive()) continue;
		b->setX(x+64);
		b->setY(y+64);
		b->setXVelocity(-2);
		b->setYVelocity(-2 + (rand() % 4));
		b->setActive(true);
		break;
	}			

	// Checks if any of the bullets in the scene collided with Fracas.
	Entity* entityHit = this->GetCollision((Entity**)bullets, NUM_BULLETS);
	if(entityHit != nullptr) this->OnHit(entityHit);

	// If Fracas's hp is below or equal to 0, move Fracas back and end the game.
	if(this->hp <= 0){
		won = true;
		if(x < SCREEN_WIDTH) x++;
		else this->active = false;
	}else{
		// Move him out from the right side of the screen when the game starts.
		if(x > SCREEN_WIDTH - 128) x--;
	}
}

/**
 * @brief Draws all 4 64x64 sections of Fracas.
 */
void Fracas::Draw(void){
	oamSet(&oamMain, entityID, x, y, 0, palleteID, spriteSize, SpriteColorFormat_16Color , gfx, -1, false, !active, false, false, false);  
	oamSet(&oamMain, entityID+1, x+64, y, 0, palleteID, spriteSize, SpriteColorFormat_16Color , gfxTR, -1, false, !active, false, false, false);  
	oamSet(&oamMain, entityID+2, x, y+64, 0, palleteID, spriteSize, SpriteColorFormat_16Color , gfxBL, -1, false, !active, false, false, false);  
	oamSet(&oamMain, entityID+3, x+64, y+64, 0, palleteID, spriteSize, SpriteColorFormat_16Color , gfxBR, -1, false, !active, false, false, false);  
}

/**
 * @brief When Fracas is hit by any entity, perform this action.
 * @param entity 
 */
void Fracas::OnHit(Entity* entity){
	entity->setActive(false);
	this->hp--;
}

/**
 * @brief Returns Fracas's HP.
 * @return int 
 */
int Fracas::getHP(){
	return this->hp;
}

/*
 *	Bullet Class Section
 * 	Contains all of the methods of the Fracas class defined in Entities.h
 */ 
Bullet::Bullet(int x, int y, int xVelocity, int yVelocity, int entID) : Entity(x, y, xVelocity, yVelocity, entID){
	this->gfx = bulletGFX;
	this->spriteSize = SpriteSize_16x16;
	this->rotatable = false;
	this->active = false;
	this->palleteID = 3;
	this->boundingBoxX = 0;
	this->boundingBoxY = 0;
	this->boundingBoxWidth = 16;
	this->boundingBoxHeight = 4;
}

Bullet::~Bullet(){
	oamSetHidden(&oamMain, entityID, true);
}

/**
 * @brief Updates a bullet entity.
 * @param KeysHeld 
 */
void Bullet::UpdateEntity(u16 KeysHeld)  {
	// If the bullet is off the screen, reset it's position and set it to inactive.
	if(this->x >= SCREEN_WIDTH){
		this->x = -100;
		this->y = -100;
		this->xVelocity = 0;
		this->active = false;
	}
}

/**
 * @brief Moves any particular star on the screen over by it's velocity.
 * @param star 
 * @param velocityX 
 * @param velocityY 
 * @param x 
 * @param y 
 */
void setStar(Star* star, int velocityX, int velocityY, int x, int y)
{
	star->x += velocityX;
	star->y += velocityY;
	if(star->y < 0 || star->y > SCREEN_HEIGHT)
	{
		star->x = x;
		star->y = DISP_YTRIGGERED;
		star->speed = rand() % 4 + 1;	
	}
	else if(star->x < 0 || star->x > SCREEN_WIDTH)
	{
		star->x = x;
		star->y = y;
		star->speed = rand() % 4 + 1;	
	}
}
 
/**
 * @brief Creates and displays all of the stars on screen.
 */
void InitStars(void)
{
	for(int i = 0; i < NUM_STARS; i++)
	{
		stars[i].x = rand() % 256;
		stars[i].y = rand() % 192;
		stars[i].speed = rand() % 4 + 1;
	}
}

/**
 * @brief Spawns the entities that must be present at the beginning of the game.
 */
void SpawnEntities(){
	// Spawns the player.
	player = new Player(20,20, 0, 0, totalEntities);
	totalEntities++;

	// Spawns barrels as 'placeholders'
	for(int i = 0; i < NUM_BARRELS; i++){
		int randomX = rand() % 256;
		int randomY = rand() % 192;
		Barrel* b = new Barrel(SCREEN_WIDTH + randomX, randomY, -2 - (rand() % 2), 0, totalEntities);
		barrels[i] = b;
		totalEntities++;
	}

	// Spawns bullets as 'placeholders'
	for(int i = 0; i < NUM_BULLETS; i++){
		Bullet* b = new Bullet(-100, -100, 0, 0, totalEntities);
		bullets[i] = b;
		totalEntities++;
	}
}

/**
 * @brief Shoots a bullet from the base of a player.
 * @param bullet 
 */
void ShootBullet(){
	for(int i = 0; i < NUM_BULLETS; i++){
		Bullet* b = bullets[i];
		if(b->isActive()) continue;
		b->setX(player->getX()+16);
		b->setY(player->getY()+12);
		b->setXVelocity(5);
		b->setActive(true);
		shotBullet = true;
		break;
	}
	
}

/**
 * @brief Spawns Fracas in the scene.
 */
void SpawnFracas(){
	fracas = new Fracas(SCREEN_WIDTH, (SCREEN_HEIGHT / 2) - 64, 0, 0, totalEntities);
	fracasSpawned = true;
	totalEntities++;
}

/**
 * @brief Basically restarts the game from the beginning. Deletes all objects and reinitializes them.
 */
void RestartGame(){
	// Resets all game stats.
	int i;
	totalEntities = 0;
	barrelsAvoided = 0;
	fracasSpawned = false;
	gameRunning = true;
	won = false;
	paused = false;

	// Deletes all Entities.
	delete(player);
	if(fracas != nullptr) delete(fracas);
	for(i = 0; i < NUM_BARRELS; i++){
		delete(barrels[i]);
	}
	for(i = 0; i < NUM_BULLETS; i++){
		delete(bullets[i]);
	}

	// Spawns Entities back in.
	SpawnEntities();
	InitStars();
}

int main(void) 
{
 	int i;
	// Sets the video mode and sub mod to Mode 5 (4 2D Backgrounds)
	videoSetMode(MODE_5_2D | DISPLAY_SPR_ACTIVE | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_SPRITE);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000 );
	
	consoleDemoInit();

	// Initializes the OAM
	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);

	// Loads the Ship Texture
	shipGFX = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color );
	dmaCopy(shipTiles, shipGFX, shipTilesLen);
	dmaCopy(shipPal, &(SPRITE_PALETTE[0]), shipPalLen);

	// Loads Barrel Texture
	barrelGFX = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color );
	dmaCopy(barrelTiles, barrelGFX, barrelTilesLen);
	dmaCopy(barrelPal, &(SPRITE_PALETTE[16]), barrelPalLen);

	// Loads Fracas Texture
	fracasTLGFX = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color );
	fracasTRGFX = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color );
	fracasBLGFX = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color );
	fracasBRGFX = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color );
	dmaCopy(fracasTiles, fracasTLGFX, fracasTilesLen / 4);
	dmaCopy(fracasTiles + 512, fracasTRGFX, fracasTilesLen / 4);
	dmaCopy(fracasTiles + 1024, fracasBLGFX, fracasTilesLen / 4);
	dmaCopy(fracasTiles + 1536, fracasBRGFX, fracasTilesLen / 4);
	dmaCopy(fracasPal, &(SPRITE_PALETTE[32]), fracasPalLen);

	// Creates bullet Texture at runtime.
	bulletGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_16Color);
	for(i = 0; i < 32; i++)
	{
		bulletGFX[i] = 1;
	}
	SPRITE_PALETTE[49] = RGB15(255,0,0);

	// Intiailizes the background and gets the pointer in VRAM memory.
	int bgMain = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
	u16* videoMemoryStars = bgGetGfxPtr(bgMain);

	// Initializes all of the stars that are on the screen.
	InitStars();

	// Spawns the entities in the game.
	SpawnEntities();

	// Infinitely loop the game logic.
	while(1)
	{
		// Scans for keys.
		scanKeys();
		u16 held = keysHeld();
		if(gameRunning){
			// Handles the pause menu.
			if(held & KEY_START){
				if(!clickedPauseButton){
					gameRunning = false;
					paused = true;
					clickedPauseButton = true;
				}
			}	else{
				clickedPauseButton = false;
			}

			// If the player has won, allow them to reset.
			if(won && (held & KEY_SELECT)){
				RestartGame();
			}

			// Handles spawning bullets when the player pressed A.
			if(held & KEY_A){
				if(!shotBullet){
					ShootBullet();
				}
			}else{
				shotBullet = false;
			}

			// Updates all of the bullets first.
			for(i = 0; i < NUM_BULLETS; i++){
				Bullet* b = bullets[i];
				if(b != nullptr){
					b->Update(held);
				}
			}

			// Update the player.
			if(player != nullptr){
				player->Update(held);
			}
		
			// Update the barrels on screen.
			for(i = 0; i < NUM_BARRELS; i++){
				Barrel* b = barrels[i];
				if(b != nullptr){
					b->Update(held);
				}
			}

			// If Fracas is not spawned and the number of barrels avoided is correct, spawn Fracas.
			if(!fracasSpawned){
				if(barrelsAvoided >= BARRELS_TO_BOSS){
					SpawnFracas();
				}
			}else{
				// Update the fracas, if he is spawned.
				if(fracas != nullptr){
					fracas->Update(held);
				}
			}
			

			//Renders all of the stars in the scene.
			for(i = 0; i < NUM_STARS; i++)
			{
				Star* star = &stars[i];
				// Clears the star from the screen.
				videoMemoryStars[star->x + star->y * SCREEN_WIDTH] = 0x0000;
				// Updates the stars position.
				setStar(star, -star->speed, 0, SCREEN_WIDTH, rand() % SCREEN_HEIGHT);
				// Redraws the star in its new position.
				videoMemoryStars[star->x + star->y * SCREEN_WIDTH] = 0xffff;
			}
		}else{
			// If the game is paused, allow you to input to unpause or reset.
			if(paused){
				if(held & KEY_START){
					if(!clickedPauseButton){
						gameRunning = true;
						paused = false;
						clickedPauseButton = true;
					}
				}else{
					clickedPauseButton = false;
				}
			}
			if(held & KEY_SELECT) RestartGame();
		}

		// Draws simple controls in the console window.
		consoleClear();
		if(gameRunning){
			if(won){
				printf("\n\n\n\n\n\n\n            YOU WON!\n\n");
				printf("     Press SELECT to restart!");
			}else{
				printf("          Controls:\n\n");
				printf("D-PAD Up   - Move Spaceship up\n");
				printf("D-PAD Down - Move Spaceship down\n");
				printf("A          - Shoot Lasers\n\n");
				printf("            Stats:\n");
				printf("================================\n");
				if(fracasSpawned){
					printf("Fracas Health: %i/%i\n", fracas->getHP(), FRACAS_MAX_HP);
				}else{
					printf("Barrels Avoided: %i/%i\n", barrelsAvoided, BARRELS_TO_BOSS);
				}
				printf("Barrels Shot: %i\n", barrelsShot);
				printf("\n\n\n\n\n\n\n\n");
				printf("Big thanks to: \nAlex Brandt,\nMathias Babin,\nand Caro Strickland!");
			}
		}else{
			if(paused){
				printf("\n\n\n\n\n\n\n             PAUSED\n\n");
				printf("   Press SELECT to RESET!");
			}else{
				printf(" \n\n\n\n\n\n\n           YOU DIED!\n\n");
				printf("    Press SELECT to respawn!");
			}
		}
		
		swiWaitForVBlank();
		oamUpdate(&oamMain);
	}

	// Once the game ends, return 0.
	return 0;
}