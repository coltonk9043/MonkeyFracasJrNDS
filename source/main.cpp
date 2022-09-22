/*
// An NDS port of Monkey Fracas Jr. from Futurama
// Port created by Colton K - ckenne87 - 251 089 374
*/

#include <nds.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <../include/Entities.h>

// Include our texture files (GRIT auto-generated)
#include "barrel.h"
#include "fracas.h"
#include "ship.h"

// Defines constants.
#define NUM_STARS 30
#define NUM_BARRELS 4
#define NUM_BULLETS 20

// Static Variables storing GFX information
u16* shipGFX;
u16* barrelGFX;
u16* fracasGFX;

// Stucture for a basic star.
typedef struct 
{
	int x;
	int y;
	int speed;
}Star;

// Static variables storing entity information.
int totalEntities = 0;
Star stars[NUM_STARS];
Player* player;
Fracas* fracas;
Barrel* barrels[NUM_BARRELS];

// Static variables for game information.
bool gameRunning = true;

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
	this->alive = true;
	this->rotatable = false;
	this->entityID = entID;
}

Entity::~Entity(){
	return;
}

int Entity::getX() {return this->x;}
int Entity::getY() {return this->y;}
int Entity::getBoundingBoxX() {return this->boundingBoxX;}
int Entity::getBoundingBoxY() {return this->boundingBoxY;}
int Entity::getBoundingBoxWidth() {return this->boundingBoxWidth;}
int Entity::getBoundingBoxHeight() {return this->boundingBoxHeight;}

void Entity::setX(int x){
    this->x = x;
}

void Entity::setY(int y){
    this->y = y;
}

void Entity::setXVelocity(int xVelocity){
    this->xVelocity = xVelocity;
}

void Entity::setYVelocity(int yVelocity){
    this->yVelocity = yVelocity;
}

void Entity::setGFX(u16* gfx){
    this->gfx = gfx;
}

void Entity::Update(u16 KeysHeld){
    this->UpdateEntity(KeysHeld);
	this->x += this->xVelocity;
	this->y += this->yVelocity;
	if(this->y < 0){
		this->y = 0;
	}else if (this->y > SCREEN_HEIGHT - 32 ){
		this->y = SCREEN_HEIGHT - 32;
	}
	this->rotation += degreesToAngle(this->rotationSpeed);

	this->Draw();
}

void Entity::Draw(void){
	if(rotatable){
		oamRotateScale(&oamMain, entityID, rotation, intToFixed(1, 8), intToFixed(1, 8));
		oamSet(&oamMain, entityID, x, y, 0, palleteID, SpriteSize_32x32, SpriteColorFormat_16Color , gfx, entityID, false, false, false, false, false); 
	}else oamSet(&oamMain, entityID, x, y, 0, palleteID, SpriteSize_32x32, SpriteColorFormat_16Color , gfx, -1, false, false, false, false, false);  
}

void Entity::OnHit(){

}

/*
 *	Player Class Section
 * 	Contains all of the methods of the player class defined in Entities.h
 */ 
Player::Player(int x, int y, int xVelocity, int yVelocity, int entID) : Entity(x, y, xVelocity, yVelocity, entID){
	this->gfx = shipGFX;
	this->palleteID = 0;
	this->boundingBoxX = 0;
	this->boundingBoxY = 2;
	this->boundingBoxWidth = 32;
	this->boundingBoxHeight = 24;
	this->rotation = -1;
}

Player::~Player(){
	//delete(this);
}

void Player::UpdateEntity(u16 KeysHeld)  {
	if(KeysHeld & KEY_UP) this->yVelocity = -2;
	else if(KeysHeld & KEY_DOWN) this->yVelocity = 2;
	else this->yVelocity = 0;

	// Checks the collision of each barrel in the scene (as that is the only thing that will be affecting the player.)
	for(int i = 0; i < NUM_BARRELS; i++){
		Barrel* barrelToCheck = barrels[i];
		float x1 = x + boundingBoxX + boundingBoxWidth;
		float x2 = barrelToCheck->getX() + barrelToCheck->getBoundingBoxX() + barrelToCheck->getBoundingBoxWidth();
		float y1 = y + boundingBoxY + boundingBoxHeight;
		float y2 = barrelToCheck->getY() + barrelToCheck->getBoundingBoxY() + barrelToCheck->getBoundingBoxHeight();
		
		// Checks to see if there is a collision on the X-axis (such that two objects bounding boxes align on the X axis)
		float maxX;
		float minX;
		if(x + boundingBoxX < barrelToCheck->getX() + barrelToCheck->getBoundingBoxX()) minX = barrelToCheck->getX() +barrelToCheck->getBoundingBoxX();
		else minX = x + boundingBoxX;
		if(x1 < x2) maxX = x1;
		else maxX = x2;
		if(maxX <= minX) continue;

		// Checks to see if there is a collision on the Y-axis (such that two objects bounding boxes align on the Y axis)
		float maxY;
		float minY;
		if(y + boundingBoxY < barrelToCheck->getY() + barrelToCheck->getBoundingBoxY()) minY = barrelToCheck->getY() + barrelToCheck->getBoundingBoxY();
		else minY = y + boundingBoxY;
		if(y1 < y2) maxY = y1;
		else maxY = y2;
		if(maxY <= minY) continue;

		// If there is a collision on both axis, assume that the bounding boxes are touching.
		else OnHit();
	}
}

void Player::OnHit(){
	gameRunning = false;
}

/*
 *	Barrel Class Section
 * 	Contains all of the methods of the Barrel class defined in Entities.h
 */ 
Barrel::Barrel(int x, int y, int xVelocity, int yVelocity, int entID) : Entity(x, y, xVelocity, yVelocity, entID){
	this->gfx = barrelGFX;
	this->rotatable = true;
	this->palleteID = 1;
	this->boundingBoxX = 8;
	this->boundingBoxY = 8;
	this->boundingBoxWidth = 16;
	this->boundingBoxHeight = 16;
	this->rotationSpeed = 2.0f;
}

Barrel::~Barrel(){
	//delete(this);
}

void Barrel::UpdateEntity(u16 KeysHeld)  {
	if(this->x < -32){
		this->x = 356;
		this-> y = rand() % 192;
	}
}

void Barrel::OnHit(){

}

/*
 *	Fracas Class Section
 * 	Contains all of the methods of the Fracas class defined in Entities.h
 */ 
Fracas::Fracas(int x, int y, int xVelocity, int yVelocity, int entID) : Entity(x, y, xVelocity, yVelocity, entID){
	this->gfx = fracasGFX;
	this->rotatable = true;
	this->palleteID = 2;
	this->boundingBoxWidth = 64;
	this->boundingBoxHeight = 64;
}

Fracas::~Fracas(){
	//delete(this);
}

void Fracas::UpdateEntity(u16 KeysHeld)  {
	
}

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
 

void InitStars(void)
{
	int i;
	for(i = 0; i < NUM_STARS; i++)
	{
		stars[i].x = rand() % 256;
		stars[i].y = rand() % 192;
		stars[i].speed = rand() % 4 + 1;
	}
}

void SpawnPlayer(){
	player = new Player(20,20, 0, 0, totalEntities);
	totalEntities++;
}

void SpawnBarrels(){
	for(int i = 0; i < NUM_BARRELS; i++){
		int randomX = rand() % 256;
		int randomY = rand() % 192;
		Barrel* b = new Barrel(SCREEN_WIDTH + randomX, randomY, -2 - (rand() % 2), 0, totalEntities);
		barrels[i] = b;
		totalEntities++;
	}
}

void SpawnFracas(){
	fracas = new Fracas(SCREEN_WIDTH, SCREEN_HEIGHT / 2, 0, 0, totalEntities);
	totalEntities++;
}

int main(void) 
{
	touchPosition touch;
 	
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
	fracasGFX = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_16Color );
	dmaCopy(fracasTiles, fracasGFX, fracasTilesLen);
	dmaCopy(fracasPal, &(SPRITE_PALETTE[32]), fracasPalLen);

	// Intiailizes the background and gets the pointer in VRAM memory.
	int bgMain = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
	u16* videoMemoryStars = bgGetGfxPtr(bgMain);

	// Draws simple controls in the console window.
	printf("          Controls:\n\n");
	printf("D-PAD Up   - Move Spaceship up\n");
	printf("D-PAD Down - Move Spaceship down\n");
	printf("A          - Shoot Lasers\n");
	printf("Start      - Respawn\n");
	printf("\n\n\n\n\n\n\n\n\n\n\n\n");
	printf("Big thanks to: \nAlex Brandt,\nMathias Babin,\nand Caro Strickland!");
	
	// Initializes all of the stars that are on the screen.
	InitStars();

	// Spawns the player.
	SpawnPlayer();
	SpawnBarrels();

	// Infinitely loop the game logic.
	while(1)
	{
		// Scans for keys.
		scanKeys();
		u16 held = keysHeld();
		//if(held & KEY_START) break;
		if(held & KEY_TOUCH) touchRead(&touch);

		if(gameRunning){
			// Update the player.
			if(player != nullptr){
				player->Update(held);
			}
		
			// Update the barrels on screen.
			for(int i = 0; i < NUM_BARRELS; i++){
				Barrel* b = barrels[i];
				if(b != nullptr){
					b->Update(held);
				}
			}

			// Update the fracas, if he is spawned.
			if(fracas != nullptr){
				fracas->Update(held);
			}

			//Renders all of the stars in the scene.
			for(int i = 0; i < NUM_STARS; i++)
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
			if(held & KEY_START){
				totalEntities = 0;
				gameRunning = true;
				delete(player);
				SpawnPlayer();
				for(int i = 0; i < NUM_BARRELS; i++){
					delete(barrels[i]);
				}
				SpawnBarrels();
				InitStars();
			}
		}
		

		swiWaitForVBlank();

		oamUpdate(&oamMain);
	}

	// Once the game ends, return 0.
	return 0;
}


