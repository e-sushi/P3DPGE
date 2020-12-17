#include "RenderSceneSystem.h"
#include "../internal/olcPixelGameEngine.h"
#include "../math/Math.h"
#include "../geometry/Triangle.h"
#include "../EntityAdmin.h"
#include "../utils/Debug.h"

#include "../components/Scene.h"
#include "../components/Mesh.h"
#include "../components/Camera.h"
#include "../components/InputSingleton.h"
#include "../components/Keybinds.h"
#include "../components/Light.h"
#include "../components/ScreenSingleton.h"
#include "../components/Transform.h"

//TODO(or,delle) this took about 50% CPU time (in release), look into optimizing this
//TODO(rc, sushi) change this to take in types and not individual values
//in essence this algorithm scans down a triangle and fills each row it occupies
//with the texture. this is necessary to account for us clipping triangles.
void TexturedTriangle(Scene* scene, ScreenSingleton* screen, olc::PixelGameEngine* p,
										int x1, int y1, float u1, float v1, float w1,
										int x2, int y2, float u2, float v2, float w2,
										int x3, int y3, float u3, float v3, float w3,
										olc::Sprite* tex) {
	if (y2 < y1) { std::swap(y1, y2); std::swap(x1, x2); std::swap(u1, u2); std::swap(v1, v2); std::swap(w1, w2); }
	if (y3 < y1) { std::swap(y1, y3); std::swap(x1, x3); std::swap(u1, u3); std::swap(v1, v3); std::swap(w1, w3); }
	if (y3 < y2) { std::swap(y2, y3); std::swap(x2, x3); std::swap(u2, u3); std::swap(v2, v3); std::swap(w2, w3); }

	int   dy1 = y2 - y1;	int dx1 = x2 - x1;
	float dv1 = v2 - v1;	float du1 = u2 - u1;
	float dw1 = w2 - w1;

	int	  dy2 = y3 - y1;	int dx2 = x3 - x1;
	float dv2 = v3 - v1;	float du2 = u3 - u1;
	float dw2 = w3 - w1;

	float tex_u, tex_v, tex_w;

	float	dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0,
			dw1_step = 0, dw2_step = 0;

	if (dy1) dax_step = dx1 / (float)abs(dy1);
	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	if (dy1) du1_step = du1 / (float)abs(dy1);
	if (dy1) dv1_step = dv1 / (float)abs(dy1);
	if (dy1) dw1_step = dw1 / (float)abs(dy1);

	if (dy2) du2_step = du2 / (float)abs(dy2);
	if (dy2) dv2_step = dv2 / (float)abs(dy2);
	if (dy2) dw2_step = dw2 / (float)abs(dy2);

	if (dy1){
		for (int i = y1; i <= y2; i++){
			int ax = x1 + (float)(i - y1) * dax_step;
			int bx = x1 + (float)(i - y1) * dbx_step;

			float tex_su = u1 + (float)(i - y1) * du1_step;
			float tex_sv = v1 + (float)(i - y1) * dv1_step;
			float tex_sw = w1 + (float)(i - y1) * dw1_step;

			float tex_eu = u1 + (float)(i - y1) * du2_step;
			float tex_ev = v1 + (float)(i - y1) * dv2_step;
			float tex_ew = w1 + (float)(i - y1) * dw2_step;

			if (ax > bx){
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}

			tex_u = tex_su;
			tex_v = tex_sv;
			tex_w = tex_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			for (int j = ax; j < bx; j++){
				tex_w = (1.0f - t) * tex_sw + t * tex_ew;

				if (tex_w > scene->pixelDepthBuffer[i * screen->width + j]) {
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					p->Draw(j, i, tex->Sample(tex_u / tex_w, tex_v / tex_w));
					scene->pixelDepthBuffer[i * screen->width + j] = tex_w;
				}
				t += tstep;
			}

		}
	}

	dy1 = y3 - y2; dx1 = x3 - x2;
	dv1 = v3 - v2; du1 = u3 - u2;
	dw1 = w3 - w2;

	if (dy1) dax_step = dx1 / (float)abs(dy1);
	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	du1_step = 0, dv1_step = 0;
	if (dy1) du1_step = du1 / (float)abs(dy1);
	if (dy1) dv1_step = dv1 / (float)abs(dy1);
	if (dy1) dw1_step = dw1 / (float)abs(dy1);

	if (dy1){
		for (int i = y2; i <= y3; i++){
			int ax = x2 + (float)(i - y2) * dax_step;
			int bx = x1 + (float)(i - y1) * dbx_step;

			float tex_su = u2 + (float)(i - y2) * du1_step;
			float tex_sv = v2 + (float)(i - y2) * dv1_step;
			float tex_sw = w2 + (float)(i - y2) * dw1_step;

			float tex_eu = u1 + (float)(i - y1) * du2_step;
			float tex_ev = v1 + (float)(i - y1) * dv2_step;
			float tex_ew = w1 + (float)(i - y1) * dw2_step;

			if (ax > bx){
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sw, tex_ew);
			}

			tex_u = tex_su;
			tex_v = tex_sv;
			tex_w = tex_sw;

			float tstep = 1.0f / ((float)(bx - ax));
			float t = 0.0f;

			for (int j = ax; j < bx; j++){
				tex_u = (1.0f - t) * tex_su + t * tex_eu;
				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
				tex_w = (1.0f - t) * tex_sw + t * tex_ew;

				if (tex_w > scene->pixelDepthBuffer[i * (size_t)screen->width + j]) {
					p->Draw(j, i, tex->Sample(tex_u / tex_w, tex_v / tex_w));
					scene->pixelDepthBuffer[i * (size_t)screen->width + j] = tex_w;
				}
				t += tstep;
			}
		}
	}
} //TexturedTriangle


int ClipTriangles(const Vector3& plane_p, Vector3 plane_n, Triangle* in_tri, std::array<Triangle*, 2>& out_tris) {
	plane_n.normalize();

	//temp storage to classify points on either side of plane
	Vector3* inside_points[3];  int nInsidePointCount = 0;
	Vector3* outside_points[3]; int nOutsidePointCount = 0;
	Vector3* inside_tex[3];		int nInsideTexCount = 0;
	Vector3* outside_tex[3];	int nOutsideTexCount = 0;

	auto dist = [&](const Vector3& p)
	{
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - plane_n.dot(plane_p));
	};

	//signed distance of each point in triangle to plane
	float d0 = dist(in_tri->proj_points[0]);
	float d1 = dist(in_tri->proj_points[1]);
	float d2 = dist(in_tri->proj_points[2]);

	if (d0 >= 0) { inside_points[nInsidePointCount++]   = &in_tri->proj_points[0]; inside_tex[nInsideTexCount++]   = &in_tri->proj_tex_points[0]; }
	else         { outside_points[nOutsidePointCount++] = &in_tri->proj_points[0]; outside_tex[nOutsideTexCount++] = &in_tri->proj_tex_points[0]; }
	if (d1 >= 0) { inside_points[nInsidePointCount++]   = &in_tri->proj_points[1]; inside_tex[nInsideTexCount++]   = &in_tri->proj_tex_points[1]; }
	else         { outside_points[nOutsidePointCount++] = &in_tri->proj_points[1]; outside_tex[nOutsideTexCount++] = &in_tri->proj_tex_points[1]; }
	if (d2 >= 0) { inside_points[nInsidePointCount++]   = &in_tri->proj_points[2]; inside_tex[nInsideTexCount++]   = &in_tri->proj_tex_points[2]; }
	else         { outside_points[nOutsidePointCount++] = &in_tri->proj_points[2]; outside_tex[nOutsideTexCount++] = &in_tri->proj_tex_points[2]; }

	//classify points and break input triangle into smaller trangles if
	//required. there are four possible outcomes

	float t;
	if (nInsidePointCount == 0) { 
	//all points lie outside the plane
		return 0; 
	} else if (nInsidePointCount == 3) { 
	//all points lie inside the plane so do nothing and allow triangle to pass
		out_tris = {in_tri, 0};
		return 1; 
	} else if (nInsidePointCount == 1 && nOutsidePointCount == 2) {
		Triangle* newTri = new Triangle();
		newTri->color = in_tri->color;
		newTri->e = 0; //set pointer to zero, so we can mark it for deletion

		//the inside point is valid so we keep it
		newTri->proj_points[0] = *inside_points[0];
		newTri->proj_tex_points[0] = *inside_tex[0];

		//but the two new points are not where the original triangle intersects with the plane
		newTri->proj_points[1] = Math::VectorPlaneIntersect(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		newTri->proj_tex_points[1].x = t * (outside_tex[0]->x - inside_tex[0]->x) + inside_tex[0]->x;
		newTri->proj_tex_points[1].y = t * (outside_tex[0]->y - inside_tex[0]->y) + inside_tex[0]->y;
		newTri->proj_tex_points[1].z = t * (outside_tex[0]->z - inside_tex[0]->z) + inside_tex[0]->z;

		newTri->proj_points[2] = Math::VectorPlaneIntersect(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
		newTri->proj_tex_points[2].x = t * (outside_tex[1]->x - inside_tex[0]->x) + inside_tex[0]->x;
		newTri->proj_tex_points[2].y = t * (outside_tex[1]->y - inside_tex[0]->y) + inside_tex[0]->y;
		newTri->proj_tex_points[2].z = t * (outside_tex[1]->z - inside_tex[0]->z) + inside_tex[0]->z;

		out_tris = {newTri, 0};
		return 1; //return new triangle
	} else if (nInsidePointCount == 2 && nOutsidePointCount == 1) {
	//triangle will be clipped and becomes a quad which is cut into two more triangles
		Triangle* newTri = new Triangle();
		newTri->color = in_tri->color;
		newTri->e = 0; //set pointer to zero, so we can mark it for deletion
		newTri->proj_points[0] = *inside_points[0];
		newTri->proj_points[1] = *inside_points[1];
		newTri->proj_tex_points[0] = *inside_tex[0];
		newTri->proj_tex_points[1] = *inside_tex[1];
		newTri->proj_points[2] = Math::VectorPlaneIntersect(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		newTri->proj_tex_points[2].x = t * (outside_tex[0]->x - inside_tex[0]->x) + inside_tex[0]->x;
		newTri->proj_tex_points[2].y = t * (outside_tex[0]->y - inside_tex[0]->y) + inside_tex[0]->y;
		newTri->proj_tex_points[2].z = t * (outside_tex[0]->z - inside_tex[0]->z) + inside_tex[0]->z;

		Triangle* newTri2 = new Triangle();
		newTri2->color = in_tri->color;
		newTri2->e = 0; //set pointer to zero, so we can mark it for deletion
		newTri2->proj_points[0] = *inside_points[1];
		newTri2->proj_tex_points[0] = *inside_tex[1];
		newTri2->proj_points[1] = newTri->proj_points[2];
		newTri2->proj_tex_points[1] = newTri->proj_tex_points[2];
		newTri2->proj_points[2] = Math::VectorPlaneIntersect(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
		newTri2->proj_tex_points[2].x = t * (outside_tex[0]->x - inside_tex[1]->x) + inside_tex[1]->x;
		newTri2->proj_tex_points[2].y = t * (outside_tex[0]->y - inside_tex[1]->y) + inside_tex[1]->y;
		newTri2->proj_tex_points[2].z = t * (outside_tex[0]->z - inside_tex[1]->z) + inside_tex[1]->z;

		out_tris = {newTri, newTri2};
		return 2;
	} else {
		ASSERT(false, "This shouldnt happen");
		return -1;
	}
} //ClipTriangles

int RenderTriangles(Scene* scene, Camera* camera, ScreenSingleton* screen, olc::PixelGameEngine* p) {
	int drawnTriCount = 0;
	for(Triangle* t : scene->triangles) {
		t->copy_points(); //copy worldspace points to proj_points
		Vector3 triNormal = t->get_normal();
		float light_ray1 = (scene->lights[0]->position - t->points[0]).dot(triNormal);

		//if the angle between the middle of the triangle and the camera is greater less 90 degrees, it should show
		if(triNormal.dot(t->midpoint() - camera->position) < 0) {  //TODO(or,delle) see if zClipIndex can remove the .midpoint()
																   //project points to view/camera space
			for(Vector3& pp : t->proj_points) {
				pp = Math::WorldToCamera(pp, camera->viewMatrix).ToVector3();
			}

	//darken pixels based on light in camera space
			for(int spriteX = 0; spriteX < 25; ++spriteX) {
				for(int spriteY = 0; spriteY < 25; ++spriteY) {
					float dist = (t->sprite_pixel_location(spriteX, spriteY) - scene->lights[0]->position).mag();
					//float dp = (scene->lights[0]->position - zClipped[zClipIndex]->points[0]).dot(zClipped[zClipIndex]->get_normal());
					float rgb = floor(std::clamp(255 * (1 /  (2 * dist)), 0.f, 255.f));
					t->sprite->SetPixel(Vector2(spriteX, spriteY), olc::Pixel(rgb,rgb,rgb));
				}
			}

	//clip to the nearZ plane
			std::array<Triangle*, 2> zClipped = {};
			int numZClipped = ClipTriangles(Vector3(0, 0, camera->nearZ), Vector3::FORWARD, t, zClipped);

			for(int zClipIndex = 0; zClipIndex < numZClipped; ++zClipIndex) {
				//project to screen
				for(int pIndex = 0; pIndex < 3; ++pIndex) {
					float w;
					zClipped[zClipIndex]->proj_points[pIndex] = Math::CameraToScreen(zClipped[zClipIndex]->proj_points[pIndex], camera->projectionMatrix, screen->dimensions, w);
					zClipped[zClipIndex]->proj_tex_points[pIndex].x /= w;
					zClipped[zClipIndex]->proj_tex_points[pIndex].y /= w;
					zClipped[zClipIndex]->proj_tex_points[pIndex].z = 1.f / w;
				}
				zClipped[zClipIndex]->sprite = t->sprite;

	//clip to screen borders
				std::list<std::pair<bool, Triangle*>> borderClippedTris;
				borderClippedTris.push_back(std::make_pair((bool)zClipped[zClipIndex]->e, zClipped[zClipIndex]));
				int newBClippedTris = 1;
				for(int bClipSide = 0; bClipSide < 4; ++bClipSide) {
					while(newBClippedTris > 0) {
						std::array<Triangle*, 2> bClipped = {};
						Triangle* tri = borderClippedTris.front().second;
						borderClippedTris.pop_front();
						newBClippedTris--;

						int numBClipped = 0;
						switch (bClipSide) {
							case 0: { numBClipped = ClipTriangles(Vector3::ZERO,					Vector3::UP,	tri, bClipped); } break;
							case 1: { numBClipped = ClipTriangles(Vector3(0, screen->height-1, 0),	Vector3::DOWN,	tri, bClipped); } break;
							case 2: { numBClipped = ClipTriangles(Vector3::ZERO,					Vector3::RIGHT,	tri, bClipped); } break;
							case 3: { numBClipped = ClipTriangles(Vector3(screen->width-1, 0, 0),	Vector3::LEFT,	tri, bClipped); } break;
						}

						for(int bClipIndex = 0; bClipIndex < numBClipped; ++bClipIndex) {
							bClipped[bClipIndex]->sprite = zClipped[zClipIndex]->sprite;

							//if its a new triangle, mark it for deletion since its not owned by a mesh
							if(bClipped[bClipIndex]->e) {
								borderClippedTris.push_back(std::make_pair(false, bClipped[bClipIndex]));
							} else {
								borderClippedTris.push_back(std::make_pair(true, bClipped[bClipIndex]));
							}
						}
					}
					newBClippedTris = borderClippedTris.size();
				}

	//draw triangles
				for(auto& pair : borderClippedTris) {
					Triangle* tr = pair.second;
					if(scene->WIRE_FRAME_NO_TEXTURE) {				//draw wireframed with no textures
						p->DrawTriangle(tr->proj_points[0].x, tr->proj_points[0].y,
							tr->proj_points[1].x, tr->proj_points[1].y,
							tr->proj_points[2].x, tr->proj_points[2].y,
							olc::WHITE);
					} else {										//draw textured
						TexturedTriangle(scene, screen, p,
							tr->proj_points[0].x, tr->proj_points[0].y, tr->proj_tex_points[0].x, tr->proj_tex_points[0].y, tr->proj_tex_points[0].z,
							tr->proj_points[1].x, tr->proj_points[1].y, tr->proj_tex_points[1].x, tr->proj_tex_points[1].y, tr->proj_tex_points[1].z,
							tr->proj_points[2].x, tr->proj_points[2].y, tr->proj_tex_points[2].x, tr->proj_tex_points[2].y, tr->proj_tex_points[2].z,
							tr->sprite);
					}

					//display wireframe
					if(scene->WIRE_FRAME && !scene->WIRE_FRAME_NO_TEXTURE) {
						p->DrawTriangle(tr->proj_points[0].x, tr->proj_points[0].y,
							tr->proj_points[1].x, tr->proj_points[1].y,
							tr->proj_points[2].x, tr->proj_points[2].y,
							olc::WHITE);
					}

					//display edges numbers
					if(scene->DISPLAY_EDGES) {
						tr->display_edges(p);
					}

					//delete new clipping triangles
					if(pair.first && pair.second) {
						delete pair.second;
					}
					drawnTriCount++;
				}
			}
		}
	}
	return drawnTriCount;
} //RenderTriangles

//the input vectors should be in viewMatrix/camera space
//returns true if the line can be rendered after clipping, false otherwise
bool ClipLineToZPlanes(Vector3& start, Vector3& end, Camera* camera) {
	//clip to the near plane
	Vector3 planePoint = Vector3(0, 0, camera->nearZ);
	Vector3 planeNormal = Vector3::FORWARD;
	float d = planeNormal.dot(planePoint);
	bool startBeyondPlane = planeNormal.dot(start) - d < 0;
	bool endBeyondPlane = planeNormal.dot(end) - d < 0;
	float t;
	if (startBeyondPlane && !endBeyondPlane) {
		start = Math::VectorPlaneIntersect(planePoint, planeNormal, start, end, t);
	} else if (!startBeyondPlane && endBeyondPlane) {
		end = Math::VectorPlaneIntersect(planePoint, planeNormal, start, end, t);
	} else if (startBeyondPlane && endBeyondPlane) {
		return false;
	}

	//clip to the far plane
	planePoint = Vector3(0, 0, camera->farZ);
	planeNormal = Vector3::BACK;
	d = planeNormal.dot(planePoint);
	startBeyondPlane = planeNormal.dot(start) - d < 0;
	endBeyondPlane = planeNormal.dot(end) - d < 0;
	if (startBeyondPlane && !endBeyondPlane) {
		start = Math::VectorPlaneIntersect(planePoint, planeNormal, start, end, t);
	} else if (!startBeyondPlane && endBeyondPlane) {
		end = Math::VectorPlaneIntersect(planePoint, planeNormal, start, end, t);
	} else if (startBeyondPlane && endBeyondPlane) {
		return false;
	}
	return true;
} //ClipLineToZPlanes

//cohen-sutherland algorithm https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
//the input vectors should be in screen space
//returns true if the line can be rendered after clipping, false otherwise
bool ClipLineToBorderPlanes(Vector3& start, Vector3& end, ScreenSingleton* screen) {
	//clip to the vertical and horizontal planes
	const int CLIP_INSIDE = 0;
	const int CLIP_LEFT = 1;
	const int CLIP_RIGHT = 2;
	const int CLIP_BOTTOM = 4;
	const int CLIP_TOP = 8;
	auto ComputeOutCode = [&](Vector3& vertex) {
		int code = CLIP_INSIDE;
		if (vertex.x < 0) {
			code |= CLIP_LEFT;
		}
		else if (vertex.x > screen->width) {
			code |= CLIP_RIGHT;
		}
		if (vertex.y < 0) { //these are inverted because we are in screen space
			code |= CLIP_TOP;
		}
		else if (vertex.y > screen->height) {
			code |= CLIP_BOTTOM;
		}
		return code;
	};

	int lineStartCode = ComputeOutCode(start);
	int lineEndCode = ComputeOutCode(end);

	//loop until all points are within or outside the screen zone
	while (true) {
		if (!(lineStartCode | lineEndCode)) {
			//both points are inside the screen zone
			return true;
		}
		else if (lineStartCode & lineEndCode) {
			//both points are in the same outside zone
			return false;
		}
		else {
			float x, y;
			//select one of the points outside
			int code = lineEndCode > lineStartCode ? lineEndCode : lineStartCode;

			//clip the points the the screen bounds by finding the intersection point
			if			(code & CLIP_TOP) {			//point is above screen
				x = start.x + (end.x - start.x) * (-start.y) / (end.y - start.y);
				y = 0;
			} else if	(code & CLIP_BOTTOM) {		//point is below screen
				x = start.x + (end.x - start.x) * (screen->height - start.y) / (end.y - start.y);
				y = screen->height;
			} else if	(code & CLIP_RIGHT) {		//point is right of screen
				y = start.y + (end.y - start.y) * (screen->width - start.x) / (end.x - start.x);
				x = screen->width;
			} else if	(code & CLIP_LEFT) {		//point is left of screen
				y = start.y + (end.y - start.y) * (-start.x) / (end.x - start.x);
				x = 0;
			}

			//update the vector's points and restart loop
			if (code == lineStartCode) {
				start.x = x;
				start.y = y;
				lineStartCode = ComputeOutCode(start);
			}else {
				end.x = x;
				end.y = y;
				lineEndCode = ComputeOutCode(end);
			}
		}
	}
} //ClipLineToBorderPlanes

int RenderLines(Scene* scene, Camera* camera, ScreenSingleton* screen, olc::PixelGameEngine* p) {
	for(Edge3D* l : scene->lines) {
	//convert vertexes from world to camera/viewMatrix space
		Vector3 startVertex = Math::WorldToCamera(l->p[0], camera->viewMatrix).ToVector3();
		Vector3 endVertex = Math::WorldToCamera(l->p[1], camera->viewMatrix).ToVector3();

	//clip vertexes to the near and far z planes in camera/viewMatrix space
		if (!ClipLineToZPlanes(startVertex, endVertex, camera)) { return 0; }

	//convert vertexes from camera/viewMatrix space to clip space
		startVertex = Math::CameraToScreen(startVertex, camera->projectionMatrix, screen->dimensions);
		endVertex = Math::CameraToScreen(endVertex, camera->projectionMatrix, screen->dimensions);

	//clip vertexes to border planes in clip space
		if (!ClipLineToBorderPlanes(startVertex, endVertex, screen)) { return 0; }

	//draw the lines after all clipping and space conversion
		p->DrawLine(startVertex.toVector2(), endVertex.toVector2(), ((RenderedEdge3D*)l)->color);
	}
} //RenderLines

void RenderSceneSystem::Update(float deltaTime, olc::PixelGameEngine* p) {
	Scene* scene = admin->tempScene;
	Camera* camera = admin->tempCamera;
	InputSingleton* input = admin->singletonInput;
	Keybinds* binds = admin->tempKeybinds;
	ScreenSingleton* screen = admin->singletonScreen;

//// Keybinds ////

	//toggle wireframe
	if(input->KeyPressed(binds->debugRenderWireframe) && !input->KeyHeld(olc::CTRL) && !input->KeyHeld(olc::SHIFT)) {
		scene->WIRE_FRAME = scene->WIRE_FRAME ? false : true;
	}

	//toggle wireframe with no textures
	if(input->KeyPressed(binds->debugRenderWireframe) && input->KeyHeld(olc::SHIFT) && !input->KeyHeld(olc::CTRL)) {
		scene->WIRE_FRAME_NO_TEXTURE = scene->WIRE_FRAME_NO_TEXTURE ? false : true;
		scene->TRANSFORM_LOCAL_AXES = scene->TRANSFORM_LOCAL_AXES ? false : true;
	}

	//toggle edge display
	if(input->KeyPressed(binds->debugRenderDisplayEdges) && !input->KeyHeld(olc::CTRL) && !input->KeyHeld(olc::SHIFT)) {
		scene->DISPLAY_EDGES = scene->DISPLAY_EDGES ? false : true;
	}

//// Scene Manangement ////

	//reset the scene
	scene->pixelDepthBuffer = std::vector<float>((size_t)screen->width * (size_t)screen->height);
	scene->triangles.clear();

	//collect all triangles and transform lines
	for(auto pair : admin->entities) {
		for(Component* comp : pair.second->components) {
			if(Mesh* mesh = dynamic_cast<Mesh*>(comp)) {
				for(Triangle& t : mesh->triangles) {
					scene->triangles.push_back(&t);
				}
			}
			/*if(SpriteRenderer* sr = dynamic_cast<SpriteRenderer*>(comp)) { //idea for 2d drawing
			
			}*/
			if(scene->TRANSFORM_LOCAL_AXES) {
				if(Transform* t = dynamic_cast<Transform*>(comp)) {
					scene->lines.push_back(new RenderedEdge3D(t->position, t->position + t->Right() * 5,	olc::RED));
					scene->lines.push_back(new RenderedEdge3D(t->position, t->position + t->Up() * 5,		olc::GREEN));
					scene->lines.push_back(new RenderedEdge3D(t->position, t->position + t->Forward() * 5,	olc::BLUE));
				}
			}
		}
	}

	scene->lights.push_back(new Light(Vector3(0, 0, 1), Vector3(0, 0, 0))); //TODO replace this with light components on entities
	scene->lights.push_back(new Light(Vector3(1, 0, 0), Vector3(0, 0, 0)));

//// Scene Rendering ////

	//render triangles
	int drawnTriCount = RenderTriangles(scene, camera, screen, p);

	//render lines
	int drawnLineCount = RenderLines(scene, camera, screen, p);

//// Rendering Cleanup ////

	//cleanup after drawing
	for(auto l : scene->lights) { if(l->entity == 0) delete l; } //temporary
	scene->lights.clear();
	if(scene->TRANSFORM_LOCAL_AXES) { for(Edge3D* l : scene->lines) { if(l->e == 0) delete l; } }
	scene->lines.clear();

	p->DrawCircle(Math::WorldToScreen2D(scene->lights[0]->position, camera->projectionMatrix, camera->viewMatrix, screen->dimensions), 10);
	p->DrawStringDecal(olc::vf2d(screen->width-300, screen->height - 10), "Tri Total: " + std::to_string(scene->triangles.size()) + "  Tri Drawn: " + std::to_string(drawnTriCount));

} //Update