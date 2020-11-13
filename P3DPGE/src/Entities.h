#pragma once
#include "olcPixelGameEngine.h"
#include "Math.h"
#include <fstream>
#include <strstream>
#include <algorithm>

namespace qvm = boost::qvm;

//collection of 3 points forming the basis of meshes
struct Triangle {
	Vector3 points[3];
	Vector3 projectedPoints[3];

	Triangle(Vector3 p1, Vector3 p2, Vector3 p3) {
		points[0] = p1;
		points[1] = p2;
		points[2] = p3;
		copy_points();
	}

	void copy_points() {
		for (int p = 0; p < 3; p++) { projectedPoints[p] = points[p]; }
	}

	Vector3 get_normal() {
		Vector3 l1 = points[1] - points[0];
		Vector3 l2 = points[2] - points[0];
		return l2.cross(l1).normalized();
	}

	Vector3 get_proj_normal() {
		Vector3 l1 = projectedPoints[1] - projectedPoints[0];
		Vector3 l2 = projectedPoints[2] - projectedPoints[0];
		return l2.cross(l1).normalized();
	}
};

//a collection of triangles that make up the geomery of objects in space
struct Mesh {
	std::vector<Triangle> triangles;

	void Draw(olc::PixelGameEngine* p, olc::Pixel color, Vector3 pos) {

		std::vector<Triangle> visibleTriangles;

		//camera is currently permenantly at zero
		Vector3 camera(0, 0, 0);
		//temp lighting set up
		Vector3 light_direction(0, 0, 1);
		light_direction = light_direction.normalized();
		//std::cout << t.get_normal().z << std::endl;

		for (auto& t : triangles) {

			if (t.get_proj_normal().dot(t.points[0] - camera) > 0) {
				//store triangles we want to draw for sorting
				visibleTriangles.push_back(t);
			}
		}

		//sort triangles back to front
		//javid uses a lambda function here so that's why its so odd
		//it basically just tells sort if it needs to swap t1 and t2
		//and this is determined by if t1's midpoint is greater than
		//t2's midpoint.
		//this method of culling hidden triangles is known as the Painter's Algorithm
		//and all it does is sort them back to front and then draws them in that order
		std::sort(visibleTriangles.begin(), visibleTriangles.end(), [](Triangle& t1, Triangle& t2) {
			float mp1 = (t1.points[0].z + t1.points[1].z + t1.points[2].z) / 3;
			float mp2 = (t2.points[0].z + t2.points[1].z + t2.points[2].z) / 3;
			return mp1 > mp2;
			});

		for (Triangle t : visibleTriangles) {
			float dp = light_direction.dot(t.get_normal());
			p->FillTriangle(
				t.projectedPoints[0].x, t.projectedPoints[0].y,
				t.projectedPoints[1].x, t.projectedPoints[1].y,
				t.projectedPoints[2].x, t.projectedPoints[2].y,
				olc::Pixel(255 * abs(dp), 255 * abs(dp), 255 * abs(dp)));

			//put this bool somewhere better later.
			bool wireframe = true;
			if (wireframe) {
				p->DrawTriangle(
					t.projectedPoints[0].x, t.projectedPoints[0].y,
					t.projectedPoints[1].x, t.projectedPoints[1].y,
					t.projectedPoints[2].x, t.projectedPoints[2].y,
					olc::BLACK);
			}
		}
	}
};

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
	//DO NOT make a pointer unless you're prepared to solve the read access violation that comes with it
	Mesh mesh;
	 
	Entity() {}
	Entity(int id, EntityParams) {
		this->id = id;
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
	}
	virtual ~Entity() {}

	// User must override these functions as required. I have not made
	// them abstract because I do need a default behaviour to occur if
	// they are not overwritten
	//TODO: this ^
	virtual void Draw(olc::PixelGameEngine* p) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual bool ContainsPoint(Vector3 point) = 0;

	//these functions are virtual but aren't implemented
	//in any child yet as I see no use for differenciating
	//between them yet
	virtual void RotateX(Vector3 offset = V3ZERO);
	virtual void RotateY(Vector3 offset = V3ZERO);
	virtual void RotateZ(Vector3 offset = V3ZERO);
	//TODO: this function does not work well with rotating as if they are called
	//on the same frame consistently it will begin oscillating around the axis
	//that's being rotated over. most likely have to define an order
	virtual void Translate(Vector3 translation);
	virtual void ProjectToScreen(mat<float, 4, 4> ProjMat, olc::PixelGameEngine* p, mat<float, 4, 4> view);

	void SetTag(std::string newTag);
	void SetColor(olc::Pixel newColor);
};

#define PhysEntityArgs velocity, acceleration, rotVelocity, rotAcceleration, mass, bStatic
#define PhysEntityParams Vector3 velocity = V3ZERO, Vector3 acceleration = V3ZERO, Vector3 rotVelocity = V3ZERO, Vector3 rotAcceleration = V3ZERO, float mass = 1, float elasticity = 1, bool bStatic = false
//the physics based implentation of Entity, anything that moves in time is this
class PhysEntity : public Entity{
public:
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 rotVelocity;
	Vector3 rotAcceleration;
	float mass;
	float elasticity;
	bool bStatic;

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
	void AddImpulse(PhysEntity* creator, Vector3 impulse, bool bIgnoreMass = false);
	void GenerateRadialForce(Vector3 position, float radius, float strength, float falloff, bool bIgnoreMass);
	virtual bool CheckCollision(Entity* entity) = 0;
	virtual void ResolveCollision(Entity* entity) = 0;
};

struct Sphere : public PhysEntity {
	float radius;

	Sphere() : PhysEntity(){}
	Sphere(float r, int id, EntityParams, PhysEntityParams) : PhysEntity(EntityArgs, PhysEntityArgs) {
		this->radius = r;
	}

	void Draw(olc::PixelGameEngine* p) override;
	bool ContainsPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(Entity* entity) override;
};

//formed by a single dimention vector
struct Box : public PhysEntity {
	Vector3 dimensions; //full dimensions

	Box() : PhysEntity(){}
	Box(Vector3 dimensions, int id, EntityParams, PhysEntityParams) : PhysEntity(EntityArgs, PhysEntityArgs){
		this->dimensions = dimensions;

		//TODO talk about using half dimensions
		//vertices making up the box
		Vector3 c = position; //center point
		Vector3 p1 = c + dimensions.xInvert().yInvert().zInvert();
		Vector3 p2 = c + dimensions.yInvert().zInvert();
		Vector3 p3 = c + dimensions.xInvert().zInvert();
		Vector3 p4 = c + dimensions.xInvert().yInvert();
		Vector3 p5 = c + dimensions.xInvert();
		Vector3 p6 = c + dimensions.yInvert();
		Vector3 p7 = c + dimensions.zInvert();
		Vector3 p8 = c + dimensions;

		//west
		mesh.triangles.push_back(Triangle(p3, p1, p4));
		mesh.triangles.push_back(Triangle(p3, p4, p5));
		//top
		mesh.triangles.push_back(Triangle(p4, p1, p2));
		mesh.triangles.push_back(Triangle(p4, p2, p6));
		//east
		mesh.triangles.push_back(Triangle(p8, p6, p2));
		mesh.triangles.push_back(Triangle(p8, p2, p7));
		//bottom
		mesh.triangles.push_back(Triangle(p3, p5, p8));
		mesh.triangles.push_back(Triangle(p3, p8, p7));
		//south
		mesh.triangles.push_back(Triangle(p5, p4, p6));
		mesh.triangles.push_back(Triangle(p5, p6, p8));
		//north
		mesh.triangles.push_back(Triangle(p7, p2, p1));
		mesh.triangles.push_back(Triangle(p7, p1, p3));
	}
	
	void Draw(olc::PixelGameEngine* p) override;
	bool ContainsPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(Entity* entity) override;
};

//struct convexPoly

//name subjuct to change
//meant to represent a complex object eg. not a sphere or box, etc.
//currently meshes will be imported from Blender, see NOTES
//if we do choose to keep using Blender, it will probably be beneficial
//to reorganize the model importing pipeline to be more general
struct Complex : public PhysEntity {

	std::string model_name;

	Complex(std::string file_name, int id, EntityParams, PhysEntityParams) : PhysEntity(EntityArgs, PhysEntityArgs){

		if (!LoadFromObjectFile(file_name)) {
			std::cout << "OBJ LOAD ERROR" << std::endl;
		}

		model_name = Math::append_decimal(file_name);

	}

	//this function is done exactly how Javid does it in
	//his video and should probably be redone later
	//for ex he uses a lot of weird ways to get strings
	//from the obj file that may not be necessary
	//TODO: generate complex object relative to input position
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
				mesh.triangles.push_back(Triangle(vertices[f[0] - 1], vertices[f[1] - 1], vertices[f[2] - 1]));
			}

		}

		return true;
	}

	void Draw(olc::PixelGameEngine* p) override;
	bool ContainsPoint(Vector3 point) override;
	bool CheckCollision(Entity* entity) override;
	void ResolveCollision(Entity* entity) override;

};

//archaic camera class for now
//in fact its nothing
struct Camera : public Entity {

	Vector3 lookDir;

	Camera() {  }

	mat<float, 4, 4> MakeViewMatrix(float yaw);

	void Update(float deltaTime) override;
	void Draw(olc::PixelGameEngine* p) override;
	bool ContainsPoint(Vector3 point) override;


};

//archaic light class
struct Light : public Entity {
	
};