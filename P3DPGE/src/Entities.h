#pragma once
#include <fstream>
#include <sstream>
#include "math/Math.h"
#include "internal/olcPixelGameEngine.h"

struct Mesh;
struct BoxMesh;
struct Triangle;
struct Collider;

//global campos variable until I figured out a better way to do that
extern Vector3 g_campos;

#define EntityArgs position, rotation, scale
#define EntityParams Vector3 position, Vector3 rotation, Vector3 scale
#define EntityDefaultParams Vector3 position = V3ZERO, Vector3 rotation = V3ZERO, Vector3 scale = V3ONE
//the basis of all objects drawn on screen or otherwise, includes basic information such
//as position, rotation, scale, tags, etc.
class Entity {
public:
	int id = -1;
	std::string tag;
	Vector3 position;
	Vector3 prev_position;
	Vector3 rotation;
	Vector3 prev_rotation;
	Vector3 scale;
	Mesh* mesh = 0;
	olc::Sprite* sprite = 0;
	//TODO(reo, sushi) implement this eventually
	olc::Decal* decal = 0;

	Timer* timer;

	bool ENTITY_DEBUG = 1;

#define DEBUGE DEBUG if(ENTITY_DEBUG)

	Entity (EntityDefaultParams);
	virtual ~Entity();
	virtual bool LineIntersect(Edge3* e);
	virtual void Draw(olc::PixelGameEngine* p, mat<float, 4, 4> ProjMat, mat<float, 4, 4> view);
	virtual bool SpecialDraw();
	//these functions are virtual but aren't implemented
	//in any child yet as I see no use for differenciating
	//between them yet
	virtual void RotateX(Vector3 offset = V3ZERO);
	virtual void RotateY(Vector3 offset = V3ZERO);
	virtual void RotateZ(Vector3 offset = V3ZERO);
	virtual void Rotate(Vector3 offset = V3ZERO);
	virtual void Translate();
	void SetTag(std::string newTag);
	virtual void SetColor(olc::Pixel newColor);

	// User must override these functions as required. I have not made
	// them abstract because I do need a default behaviour to occur if
	// they are not overwritten
	//TODO: this ^
	virtual void Update(float deltaTime);
	virtual bool ContainsPoint(Vector3 point) = 0;
	virtual bool ContainsScreenPoint(Vector3 point) = 0;

	virtual void DrawPosition(olc::PixelGameEngine* p, mat<float, 4, 4> ProjMat, mat<float, 4, 4> view);
	virtual void DrawVertices(olc::PixelGameEngine* p, mat<float, 4, 4> ProjMat, mat<float, 4, 4> view);

	virtual std::string str();
};

#define PhysEntityArgs velocity, acceleration, rotVelocity, rotAcceleration, mass, bStatic
#define PhysEntityParams Vector3 velocity, Vector3 acceleration, Vector3 rotVelocity, Vector3 rotAcceleration, float mass, float elasticity, bool bStatic
#define PhysEntityDefaultParams Vector3 velocity = V3ZERO, Vector3 acceleration = V3ZERO, Vector3 rotVelocity = V3ZERO, Vector3 rotAcceleration = V3ZERO, float mass = 1, float elasticity = 1, bool bStatic = false
//the physics based implentation of Entity, anything that moves in time is this
class PhysEntity : public Entity {
public:
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 rotVelocity;
	Vector3 rotAcceleration;
	float mass;
	float elasticity;
	bool bStatic;
	Collider* collider;
	std::vector<Vector3> forces;
	Vector3 inputs;

	//there may be a more elegent way to perform this
	Vector3 pos_lerp_from;
	Vector3 pos_lerp_to;

	//use these when rotation is done in physics
	Vector3 rot_lerp_from;
	Vector3 rot_lerp_to;

	PhysEntity (EntityDefaultParams, PhysEntityDefaultParams);
	void Update(float deltaTime) override;
	virtual void PhysUpdate(float deltaTime);
	void Interpolate(float t);
	void AddForce(PhysEntity* creator, Vector3 force, bool bIgnoreMass = false);
	void AddFrictionForce(PhysEntity* creator, float frictionCoef, float deltaTime, bool bIngoreMass = false);
	void AddImpulse(PhysEntity* creator, Vector3 impulse, bool bIgnoreMass = false);
	void GenerateRadialForce(Vector3 position, float radius, float strength, float falloff, bool bIgnoreMass);

	void AddInput(Vector3 input);

	void Draw(olc::PixelGameEngine* p, mat<float, 4, 4> ProjMat, mat<float, 4, 4> view) override;

	virtual bool CheckCollision(Entity* entity) = 0;
	virtual void ResolveCollision(PhysEntity* entity) = 0;
};

struct Sphere : public PhysEntity {
	float radius;

	Sphere() : PhysEntity() {}
	Sphere(float r, EntityDefaultParams, PhysEntityDefaultParams);
	bool ContainsPoint(Vector3 point) override;
	bool ContainsScreenPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(PhysEntity* entity) override;

	virtual std::string str() override;
};

//formed by a single dimension vector
struct Box : public PhysEntity {
	Vector3 dimensions; //full dimensions

	Box() : PhysEntity() {}
	Box(Vector3 dimensions, EntityDefaultParams, PhysEntityDefaultParams);
	bool ContainsPoint(Vector3 point) override;
	bool ContainsScreenPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(PhysEntity* entity) override;

	virtual std::string str() override;
};

//struct convexPoly

//represents a complex object eg. not a sphere or box, etc.
//currently generated by wavefront (.obj) files
struct Complex : public PhysEntity {
	std::string model_name;

	Complex(std::string file_name, EntityDefaultParams, PhysEntityDefaultParams);
	//this function is done exactly how Javid does it in
	//his video and should probably be redone later
	//for ex he uses a lot of weird ways to get strings
	//from the obj file that may not be necessary
	//TODO(e, sushi) generate complex object relative to input position
	bool LoadFromObjectFile(std::string file_name);
	bool ContainsPoint(Vector3 point) override;
	bool ContainsScreenPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(PhysEntity* entity) override;

	virtual std::string str() override;
};

//should there be 2 vf2d's instead of a Vector3 pos and Vector3 endPos?
struct Line2 : public Entity {
	Vector3 endPosition;
	olc::Pixel color = olc::WHITE;

	Edge edge;

	Line2(Vector3 endPosition, EntityDefaultParams);
	void Update(float deltaTime) override;
	bool ContainsPoint(Vector3 point) override;
	bool ContainsScreenPoint(Vector3 point) override;
	void Draw(olc::PixelGameEngine* p, mat<float, 4, 4> ProjMat, mat<float, 4, 4> view) override;
	bool SpecialDraw() override;
	void SetColor(olc::Pixel newColor) override;

	virtual std::string str() override;
};

//edge is not yet implemented here yet
struct Line3 : public Entity {
	Vector3 endPosition;
	olc::Pixel color = olc::WHITE;
	Edge3 edge;

	Line3(Vector3 endPosition, EntityDefaultParams);
	void Update(float deltaTime) override;
	bool ContainsPoint(Vector3 point) override;
	bool ContainsScreenPoint(Vector3 point) override;
	void Draw(olc::PixelGameEngine* p, mat<float, 4, 4> ProjMat, mat<float, 4, 4> view) override;
	bool SpecialDraw() override;
	void SetColor(olc::Pixel newColor) override;

	virtual std::string str() override;
};

//for spawning single triangles
struct DebugTriangle : public Entity {
	DebugTriangle(Triangle triangle, EntityDefaultParams);

	void Update(float deltaTime) override;
	bool ContainsPoint(Vector3 point) override;
	bool ContainsScreenPoint(Vector3 point) override;

	virtual std::string str() override;
};

struct Camera : public Entity {
	Vector3 lookDir;
	Vector3 target = V3FORWARD;
	Vector3 up = V3UP;
	float nearZ = .1f; //the distance from the camera's position to screen plane
	float farZ = 1000.1f; //the maximum render distance
	float fieldOfView = 90.f;

	Camera(EntityDefaultParams) : Entity(EntityArgs) {
		position = V3ZERO;
	}

	mat<float, 4, 4> MakeViewMatrix(float yaw, bool force_target = false);
	mat<float, 4, 4> ProjectionMatrix(olc::PixelGameEngine* p);

	void Update(float deltaTime) override;
	bool ContainsPoint(Vector3 point) override;
	bool ContainsScreenPoint(Vector3 point) override;

	void copy(Camera c) {
		this->lookDir = c.lookDir;
		this->position = c.position;
	}

	virtual std::string str() override;
};

//archaic light class
struct Light : public Entity {
	
	Vector3 direction;

	Light(){}


};
