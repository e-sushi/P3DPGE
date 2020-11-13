#include "Entities.h"

//// Entity	////

void Entity::SetTag(std::string newTag){
	tag = newTag;
}

void Entity::SetColor(olc::Pixel newColor) {
	color = newColor;
}

void Entity::RotateX(Vector3 offset) {
	for (auto& m : mesh.triangles) {
		for (auto& n : m.points) {
			n.rotateV3_X(rotation.x, position, offset);
		}
	}
}

void Entity::RotateY(Vector3 offset) {
	for (auto& m : mesh.triangles) {
		for (auto& n : m.points) {
			n.rotateV3_Y(rotation.y, position, offset);
		}
	}
}

void Entity::RotateZ(Vector3 offset) {
	for (auto& m : mesh.triangles) {
		for (auto& n : m.points) {
			n.rotateV3_Z(rotation.z, position, offset);
		}
	}
}

void Entity::Translate(Vector3 translation) {
	for (auto& m : mesh.triangles) {
		for (auto& n : m.points) {
			n.translateV3(translation);
		}
	}
	position += translation;
}

void Entity::ProjectToScreen(mat<float, 4, 4> ProjMat, olc::PixelGameEngine* p, mat<float,4,4> view) {
	for (auto& t : mesh.triangles) {
		t.copy_points();
	}
	for (auto& m : mesh.triangles) {
		for (auto& n : m.projectedPoints) {
			n = n * view;
			n.ProjToScreen(ProjMat, p, position);
		}
	}
}

//// Physics Entity ////

//TODO handle oscilating
void PhysEntity::Update(float deltaTime){
	if(!bStatic) {
		velocity += acceleration * deltaTime;
		//if (velocity.x < .1f && velocity.x > -.01f) { velocity.x = 0; }
		//if (velocity.y < .1f && velocity.y > -.01f) { velocity.y = 0; }
		//if (velocity.z < .1f && velocity.z > -.01f) { velocity.z = 0; }
		position += velocity * deltaTime;

		rotVelocity += rotAcceleration * deltaTime;
		rotation += rotVelocity * deltaTime;
	}
}

//adds a force to this entity, and this entity applies that force back on the sending object
//simply, changes acceleration by force
void PhysEntity::AddForce(PhysEntity* creator, Vector3 force, bool bIgnoreMass){
	this->acceleration += bIgnoreMass ? force : force / mass;
	//if (acceleration.x < .1f && acceleration.x > -.01f) { acceleration.x = 0; }
	//if (acceleration.y < .1f && acceleration.y > -.01f) { acceleration.y = 0; }
	//if (acceleration.z < .1f && acceleration.z > -.01f) { acceleration.z = 0; }
	if (creator) { creator->acceleration -= bIgnoreMass ? force : force / creator->mass; }
}

//adds an impulse to this entity, and this entity applies that impulse back on the sending object
//simply, changes velocity by impulse force
void PhysEntity::AddImpulse(PhysEntity* creator, Vector3 impulse, bool bIgnoreMass) {
	this->velocity += bIgnoreMass ? impulse : impulse / mass;
	if (creator) { creator->acceleration -= bIgnoreMass ? impulse : impulse / creator->mass; }
}

//TODO this
void PhysEntity::GenerateRadialForce(Vector3 position, float radius, float strength, float falloff, bool bIgnoreMass){

}

//// Sphere	////

void Sphere::Draw(olc::PixelGameEngine* p) {
	p->FillCircle(position.x, position.y, radius, color);
}

bool Sphere::ContainsPoint(Vector3 point) {
	return point.distanceTo(position) <= radius;
}

//TODO: expand this to a general entity check, but right now it just checks circles
//TODO if other object is sphere, can optimize the equation to not use sqrt
bool Sphere::CheckCollision(Entity* entity) {
	if (Sphere* sphere = dynamic_cast<Sphere*>(entity)) {
		Vector3 vectorBetween = position - sphere->position;
		float distanceBetween = vectorBetween.mag();
		if (distanceBetween <= (radius + sphere->radius)) {
			//TEMP manual static resolution in here
			float overlap = .5f * (distanceBetween - radius - sphere->radius);
			vectorBetween = vectorBetween.normalized() * overlap;
			position -= vectorBetween;
			sphere->position += vectorBetween;
			return true;
		}
	}
	return false;
}

//TODO: expand this to a general entity check, but right now it just checks circles
void Sphere::ResolveCollision(Entity* entity) {

}

//// Box ////

void Box::Draw(olc::PixelGameEngine* p) { mesh.Draw(p, color, position); }

//not sure if this still works or not, when I was trying to select boxes
//it wouldn't do anything but i feel it should still work
bool Box::ContainsPoint(Vector3 point) {
	bool checkX = point.x >= position.x - dimensions.x / 2 && point.x <= position.x + dimensions.x / 2;
	bool checkY = point.y >= position.y - dimensions.y / 2 && point.y <= position.y + dimensions.y / 2;
	//bool checkZ = point.z >= position.z - dimensions.z / 2 && point.z <= position.z + dimensions.z / 2;
	return  checkX && checkY;//&& checkZ;
}

//TODO: expand this to a general entity check
bool Box::CheckCollision(Entity* entity) {
	return false;
}

//TODO: expand this to a general entity check
void Box::ResolveCollision(Entity* entity) {

}

//// Complex ////

void Complex::Draw(olc::PixelGameEngine* p) { mesh.Draw(p, color, position); }

bool Complex::ContainsPoint(Vector3 point) {
	//this will probably be hard too implement good luck
	return false;
}

bool Complex::CheckCollision(Entity* entity) {
	return false;
}

void Complex::ResolveCollision(Entity* entity) {

}

//// Camera ////

mat<float, 4, 4> Camera::MakeViewMatrix(float yaw) {

	Vector3 target(0, 0, 1);
	Vector3 up(0, 1, 0);

	lookDir = target * Math::GetRotateV3_Y(yaw);
	target = position + lookDir;

	mat<float, 4, 4> view = inverse(Math::PointAt(position, target, up));

	return view;
}

//i wish i could not write these
void Camera::Draw(olc::PixelGameEngine* p) { 
	//camera is not drawn
}

bool Camera::ContainsPoint(Vector3 point) {
	return false;
}

void Camera::Update(float deltaTime) {

}