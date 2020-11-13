#pragma once
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Input.h"
#include "Render.h"
using namespace olc;


//// Requires boost library 1.74.0 ////


/* General TODOs and NOTEs board
   TODOs should be ordered about NOTEs and TODOs should be listed in order of
   severity.

TODO sushi: create a WorldMatrix that takes in several matrix operations and
	  gets there product in the order specified then apply that final matrix
	  to the object. this would probably eliminate the need to constantly
	  throw the object between local and world space everytime we do an 
	  operation on it

TODO sushi: put mesh and triangle somewhere better than entities
	  since these are the only two things that aren't entities
	  and often I find myself wanting to call functions outside
	  of entity, it would be best to put these somewhere else
	  maybe in Render itself 

TODO sushi: find a reason to either keep or remove Render::Init() 

NOTE sushi: it may be benefitial to have an objects triangles always be defined in
	  local space so we don't have to keep translating between world and local
	  everytime we do something to it. but this may cause other issues

NOTE sushi: should we use Blender to import objects or make our own tool?
	  maybe use Blender then make a custom one later? dunno, it'd 
	  be a fun project to build our own tool

NOTE sushi: currently, generating an object relative to mouse position
	  does not work in 3D

*/

class P3DPGE : public PixelGameEngine {
private:
	float time;

public:
	P3DPGE() { sAppName = "P3DPGE"; }

	bool OnUserCreate() override {

		time = 0;
		Physics::Init();
		Render::Init();
		
		return true;
	}
	
	bool OnUserUpdate(float deltaTime) {
		Clear(olc::BLACK);

		//time
		time += deltaTime;

		//input
		Input::Update(this, deltaTime);

		//physics
		Physics::Update(deltaTime);

		//rendering
		Render::Update(this);

		return true;
	}

	bool OnUserDestroy() {
		Input::Cleanup();
		Physics::Cleanup();
		Render::Cleanup();

		return true;
	}
	
};

int main() {
	P3DPGE game;
	if (game.Construct(600, 600, 2, 2)) { game.Start(); }
}