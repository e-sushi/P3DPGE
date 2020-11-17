#pragma once
#include <fstream>
#include <strstream>
#include "Mesh.h"
#include "olcPixelGameEngine.h"


namespace qvm = boost::qvm;



#define EntityArgs id, position, rotation, scale
#define EntityParams Vector3 position = V3ZERO, Vector3 rotation = V3ZERO, Vector3 scale = V3ONE
//the basis of all objects drawn on screen or otherwise, includes basic information such
//as position, rotation, scale, tags, etc.
class Entity {
public:

	//transform
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;

	//info
	int id;
	std::string tag;

	//mesh
	olc::Pixel color = olc::WHITE;
	Mesh* mesh;

	Entity() {}
	Entity(int id, EntityParams) {
		this->id = id;
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
	}
	virtual ~Entity() {
		delete mesh;
	}

	// User must override these functions as required. I have not made
	// them abstract because I do need a default behaviour to occur if
	// they are not overwritten
	//TODO: this ^
	virtual void Update(float deltaTime) = 0;
	virtual bool ContainsPoint(Vector3 point) = 0;

	//these functions are virtual but aren't implemented
	//in any child yet as I see no use for differenciating
	//between them yet
	virtual void RotateX(Vector3 offset = V3ZERO);
	virtual void RotateY(Vector3 offset = V3ZERO);
	virtual void RotateZ(Vector3 offset = V3ZERO);
	virtual void Translate(Vector3 translation);

	void SetTag(std::string newTag);
	void SetColor(olc::Pixel newColor);
};

#define PhysEntityArgs velocity, acceleration, rotVelocity, rotAcceleration, mass, bStatic
#define PhysEntityParams Vector3 velocity = V3ZERO, Vector3 acceleration = V3ZERO, Vector3 rotVelocity = V3ZERO, Vector3 rotAcceleration = V3ZERO, float mass = 1, float elasticity = 1, bool bStatic = false
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

	std::vector<Vector3> forces;

	PhysEntity() : Entity() {}
	PhysEntity(int id, EntityParams, PhysEntityParams) : Entity(EntityArgs) {
		this->velocity = velocity;
		this->acceleration = acceleration;
		this->rotVelocity = rotVelocity;
		this->rotAcceleration = rotAcceleration;
		this->mass = mass;
		this->elasticity = elasticity;
		this->bStatic = bStatic;
	};

	void Update(float deltaTime) override;

	void AddForce(PhysEntity* creator, Vector3 force, bool bIgnoreMass = false);
	void AddFrictionForce(PhysEntity* creator, float frictionCoef, float deltaTime, bool bIngoreMass = false);
	void AddImpulse(PhysEntity* creator, Vector3 impulse, bool bIgnoreMass = false);
	void GenerateRadialForce(Vector3 position, float radius, float strength, float falloff, bool bIgnoreMass);
	virtual bool CheckCollision(Entity* entity) = 0;
	virtual void ResolveCollision(PhysEntity* entity) = 0;
};

struct Sphere : public PhysEntity {
	float radius;

	Sphere() : PhysEntity() {}
	Sphere(float r, int id, EntityParams, PhysEntityParams) : PhysEntity(EntityArgs, PhysEntityArgs) {
		this->radius = r;
	}

	bool ContainsPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(PhysEntity* entity) override;
};

//formed by a single dimension vector
struct Box : public PhysEntity {
	Vector3 dimensions; //full dimensions

	Box() : PhysEntity() {}
	Box(Vector3 dimensions, int id, EntityParams, PhysEntityParams) : PhysEntity(EntityArgs, PhysEntityArgs) {
		this->dimensions = dimensions;
		mesh = new BoxMesh(dimensions, position);
	}

	bool ContainsPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(PhysEntity* entity) override;
};

//struct convexPoly

//name subjuct to change
//meant to represent a complex object eg. not a sphere or box, etc.
//currently meshes will be imported from Blender, see NOTES
//if we do choose to keep using Blender, it will probably be beneficial
//to reorganize the model importing pipeline to be more general
struct Complex : public PhysEntity {
	std::string model_name;

	Complex(std::string file_name, int id, EntityParams, PhysEntityParams) : PhysEntity(EntityArgs, PhysEntityArgs) {
		if (!LoadFromObjectFile(file_name)) {
			std::cout << "OBJ LOAD ERROR" << std::endl;
		}
		model_name = Math::append_decimal(file_name);
	}

	//this function is done exactly how Javid does it in
	//his video and should probably be redone later
	//for ex he uses a lot of weird ways to get strings
	//from the obj file that may not be necessary
	//TODO(+e, sushi) generate complex object relative to input position
	bool LoadFromObjectFile(std::string file_name) {
		std::ifstream f(file_name);
		if (!f.is_open()) { return false; }

		std::vector<Vector3> vertices;

		while (!f.eof()) {
			char line[128];
			f.getline(line, 128);

			std::strstream s;

			s << line;

			char junk;

			if (line[0] == 'v') {
				Vector3 v;

				s >> junk >> v.x >> v.y >> v.z;
				vertices.push_back(v);
			}

			if (line[0] == 'f') {
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				mesh->triangles.push_back(Triangle(vertices[f[0] - 1], vertices[f[1] - 1], vertices[f[2] - 1]));
			}
		}

		return true;
	}

	bool ContainsPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(PhysEntity* entity) override;
};

//archaic camera class for now
//in fact its nothing
struct Camera : public Entity {
	Vector3 lookDir;

	Camera() {  }

	mat<float, 4, 4> MakeViewMatrix(float yaw);

	void Update(float deltaTime) override;
	bool ContainsPoint(Vector3 point) override;
};

//archaic light class
struct Light : public Entity {
};