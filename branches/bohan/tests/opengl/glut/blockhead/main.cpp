/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* File for "Putting It All Together" lesson of the OpenGL tutorial on
 * www.videotutorialsrock.com
 */



#include <cstdlib>
#include <ctime>
#include <iostream>
#include <math.h>
#include <set>
#include <sstream>
#include <stdlib.h>
#include <vector>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "imageloader.h"
#include "md2model.h"
#include "text3d.h"

using namespace std;

const float PI = 3.1415926535f;
const int NUM_GUYS = 100;
//The width of the terrain in units, after scaling
const float TERRAIN_WIDTH = 50.0f;
//The amount of time between each time that we handle collisions
const float TIME_BETWEEN_HANDLE_COLLISIONS = 0.01f;

//Returns a random float from 0 to < 1
float randomFloat() {
	return (float)rand() / ((float)RAND_MAX + 1);
}

//Represents a terrain, by storing a set of heights and normals at 2D locations
class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date
	public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;
			
			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}
			
			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}
			
			computedNormals = false;
		}
		
		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;
			
			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}
		
		int width() {
			return w;
		}
		
		int length() {
			return l;
		}
		
		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}
		
		//Returns the height at (x, z)
		float getHeight(int x, int z) {
			return hs[z][x];
		}
		
		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			
			//Compute the rough version of the normals
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}
			
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);
					
					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}
					
					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					
					normals2[z][x] = sum;
				}
			}
			
			//Smooth out the normals
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];
					
					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}
			
			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;
			
			computedNormals = true;
		}
		
		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}
	
	delete image;
	t->computeNormals();
	return t;
}

//Returns the approximate height of the terrain at the specified (x, z) position
float heightAt(Terrain* terrain, float x, float z) {
	//Make (x, z) lie within the bounds of the terrain
	if (x < 0) {
		x = 0;
	}
	else if (x > terrain->width() - 1) {
		x = terrain->width() - 1;
	}
	if (z < 0) {
		z = 0;
	}
	else if (z > terrain->length() - 1) {
		z = terrain->length() - 1;
	}
	
	//Compute the grid cell in which (x, z) lies and how close we are to the
	//left and outward edges
	int leftX = (int)x;
	if (leftX == terrain->width() - 1) {
		leftX--;
	}
	float fracX = x - leftX;
	
	int outZ = (int)z;
	if (outZ == terrain->width() - 1) {
		outZ--;
	}
	float fracZ = z - outZ;
	
	//Compute the four heights for the grid cell
	float h11 = terrain->getHeight(leftX, outZ);
	float h12 = terrain->getHeight(leftX, outZ + 1);
	float h21 = terrain->getHeight(leftX + 1, outZ);
	float h22 = terrain->getHeight(leftX + 1, outZ + 1);
	
	//Take a weighted average of the four heights
	return (1 - fracX) * ((1 - fracZ) * h11 + fracZ * h12) +
		fracX * ((1 - fracZ) * h21 + fracZ * h22);
}

//The amount by which the Guy class's step function advances the state of a guy
const float GUY_STEP_TIME = 0.01f;

//Represents a guy
class Guy {
	private:
		MD2Model* model;
		Terrain* terrain;
		float terrainScale; //The scaling factor for the terrain
		float x0;
		float z0;
		float animTime; //The current position in the animation of the model
		float radius0; //The approximate radius of the guy
		float speed;
		//The angle at which the guy is currently walking, in radians.  An angle
		//of 0 indicates the positive x direction, while an angle of PI / 2
		//indicates the positive z direction.  The angle always lies between 0
		//and 2 * PI.
		float angle;
		//The amount of time until step() should next be called
		float timeUntilNextStep;
		bool isTurningLeft; //Whether the guy is currently turning left
		float timeUntilSwitchDir; //The amount of time until switching direction
		
		//Advances the state of the guy by GUY_STEP_TIME seconds (without
		//altering animTime)
		void step() {
			//Update the turning direction information
			timeUntilSwitchDir -= GUY_STEP_TIME;
			while (timeUntilSwitchDir <= 0) {
				timeUntilSwitchDir += 20 * randomFloat() + 15;
				isTurningLeft = !isTurningLeft;
			}
			
			//Update the position and angle
			float maxX = terrainScale * (terrain->width() - 1) - radius0;
			float maxZ = terrainScale * (terrain->length() - 1) - radius0;
			
			x0 += velocityX() * GUY_STEP_TIME;
			z0 += velocityZ() * GUY_STEP_TIME;
			bool hitEdge = false;
			if (x0 < radius0) {
				x0 = radius0;
				hitEdge = true;
			}
			else if (x0 > maxX) {
				x0 = maxX;
				hitEdge = true;
			}
			
			if (z0 < radius0) {
				z0 = radius0;
				hitEdge = true;
			}
			else if (z0 > maxZ) {
				z0 = maxZ;
				hitEdge = true;
			}
			
			if (hitEdge) {
				//Turn more quickly if we've hit the edge
				if (isTurningLeft) {
					angle -= 0.5f * speed * GUY_STEP_TIME;
				}
				else {
					angle += 0.5f * speed * GUY_STEP_TIME;
				}
			}
			else if (isTurningLeft) {
				angle -= 0.05f * speed * GUY_STEP_TIME;
			}
			else {
				angle += 0.05f * speed * GUY_STEP_TIME;
			}
			
			while (angle > 2 * PI) {
				angle -= 2 * PI;
			}
			while (angle < 0) {
				angle += 2 * PI;
			}
		}
	public:
		Guy(MD2Model* model1,
			Terrain* terrain1,
			float terrainScale1) {
			model = model1;
			terrain = terrain1;
			terrainScale = terrainScale1;
			
			animTime = 0;
			timeUntilNextStep = 0;
			
			//Initialize certain fields to random values
			radius0 = 0.4f * randomFloat() + 0.25f;
			x0 = randomFloat() *
				(terrainScale * (terrain->width() - 1) - radius0) + radius0;
			z0 = randomFloat() *
				(terrainScale * (terrain->length() - 1) - radius0) + radius0;
			speed = 1.5f * randomFloat() + 2.0f;
			isTurningLeft = randomFloat() < 0.5f;
			angle = 2 * PI * randomFloat();
			timeUntilSwitchDir = randomFloat() * (20 * randomFloat() + 15);
		}
		
		//Advances the state of the guy by the specified amount of time, by
		//calling step() the appropriate number of times and adjusting animTime
		void advance(float dt) {
			//Adjust animTime
			animTime += 0.45f * dt * speed / radius0;
			if (animTime > -100000000 && animTime < 1000000000) {
				animTime -= (int)animTime;
				if (animTime < 0) {
					animTime += 1;
				}
			}
			else {
				animTime = 0;
			}
			
			//Call step() the appropriate number of times
			while (dt > 0) {
				if (timeUntilNextStep < dt) {
					dt -= timeUntilNextStep;
					step();
					timeUntilNextStep = GUY_STEP_TIME;
				}
				else {
					timeUntilNextStep -= dt;
					dt = 0;
				}
			}
		}
		
		void draw() {
			if (model == NULL) {
				return;
			}
			
			float scale = radius0 / 2.5f;
			
			glPushMatrix();
			glTranslatef(x0, scale * 10.0f + y(), z0);
			glRotatef(90 - angle * 180 / PI, 0, 1, 0);
			glColor3f(1, 1, 1);
			glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
			glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
			glScalef(scale, scale, scale);
			model->draw(animTime);
			glPopMatrix();
		}
		
		float x() {
			return x0;
		}
		
		float z() {
			return z0;
		}
		
		//Returns the current height of the guy on the terrain
		float y() {
			return terrainScale *
				heightAt(terrain, x0 / terrainScale, z0 / terrainScale);
		}
		
		float velocityX() {
			return speed * cos(angle);
		}
		
		float velocityZ() {
			return speed * sin(angle);
		}
		
		//Returns the approximate radius of the guy
		float radius() {
			return radius0;
		}
		
		//Returns the angle at which the guy is currently walking, in radians.
		//An angle of 0 indicates the positive x direction, while an angle of
		//PI / 2 indicates the positive z direction.
		float walkAngle() {
			return angle;
		}	
		
		//Adjusts the angle at which this guy is walking in response to a
		//collision with the specified guy
		void bounceOff(Guy* otherGuy) {
			float vx = velocityX();
			float vz = velocityZ();
			
			float dx = otherGuy->x0 - x0;
			float dz = otherGuy->z0 - z0;
			float m = sqrt(dx * dx + dz * dz);
			dx /= m;
			dz /= m;
			
			float dotProduct = vx * dx + vz * dz;
			vx -= 2 * dotProduct * dx;
			vz -= 2 * dotProduct * dz;
			
			if (vx != 0 || vz != 0) {
				angle = atan2(vz, vx);
			}
		}
};

struct GuyPair {
	Guy* guy1;
	Guy* guy2;
};

const int MAX_QUADTREE_DEPTH = 6;
const int MIN_GUYS_PER_QUADTREE = 2;
const int MAX_GUYS_PER_QUADTREE = 5;

//Our data structure for making collision detection faster
class Quadtree {
	private:
		float minX;
		float minZ;
		float maxX;
		float maxZ;
		float centerX; //(minX + maxX) / 2
		float centerZ; //(minZ + maxZ) / 2
		
		/* The children of this, if this has any.  children[0][*] are the
		 * children with x coordinates ranging from minX to centerX.
		 * children[1][*] are the children with x coordinates ranging from
		 * centerX to maxX.  Similarly for the other dimension of the children
		 * array.
		 */
		Quadtree *children[2][2];
		//Whether this has children
		bool hasChildren;
		//The guys in this, if this doesn't have any children
		set<Guy*> guys;
		//The depth of this in the tree
		int depth;
		//The number of guys in this, including those stored in its children
		int numGuys;
		
		//Adds a guy to or removes one from the children of this
		void fileGuy(Guy* guy, float x, float z, bool addGuy) {
			//Figure out in which child(ren) the guy belongs
			for(int xi = 0; xi < 2; xi++) {
				if (xi == 0) {
					if (x - guy->radius() > centerX) {
						continue;
					}
				}
				else if (x + guy->radius() < centerX) {
					continue;
				}
				
				for(int zi = 0; zi < 2; zi++) {
					if (zi == 0) {
						if (z - guy->radius() > centerZ) {
							continue;
						}
					}
					else if (z + guy->radius() < centerZ) {
						continue;
					}
					
					//Add or remove the guy
					if (addGuy) {
						children[xi][zi]->add(guy);
					}
					else {
						children[xi][zi]->remove(guy, x, z);
					}
				}
			}
		}
		
		//Creates children of this, and moves the guys in this to the children
		void haveChildren() {
			for(int x = 0; x < 2; x++) {
				float minX2;
				float maxX2;
				if (x == 0) {
					minX2 = minX;
					maxX2 = centerX;
				}
				else {
					minX2 = centerX;
					maxX2 = maxX;
				}
				
				for(int z = 0; z < 2; z++) {
					float minZ2;
					float maxZ2;
					if (z == 0) {
						minZ2 = minZ;
						maxZ2 = centerZ;
					}
					else {
						minZ2 = centerZ;
						maxZ2 = maxZ;
					}
					
					children[x][z] =
						new Quadtree(minX2, maxX2, minZ2, maxZ2, depth + 1);
				}
			}
			
			//Remove all guys from "guys" and add them to the new children
			for(set<Guy*>::iterator it = guys.begin(); it != guys.end();
					it++) {
				Guy* guy = *it;
				fileGuy(guy, guy->x(), guy->z(), true);
			}
			guys.clear();
			
			hasChildren = true;
		}
		
		//Adds all guys in this or one of its descendants to the specified set
		void collectGuys(set<Guy*> &gs) {
			if (hasChildren) {
				for(int x = 0; x < 2; x++) {
					for(int z = 0; z < 2; z++) {
						children[x][z]->collectGuys(gs);
					}
				}
			}
			else {
				for(set<Guy*>::iterator it = guys.begin(); it != guys.end();
						it++) {
					Guy* guy = *it;
					gs.insert(guy);
				}
			}
		}
		
		//Destroys the children of this, and moves all guys in its descendants
		//to the "guys" set
		void destroyChildren() {
			//Move all guys in descendants of this to the "guys" set
			collectGuys(guys);
			
			for(int x = 0; x < 2; x++) {
				for(int z = 0; z < 2; z++) {
					delete children[x][z];
				}
			}
			
			hasChildren = false;
		}
		
		//Removes the specified guy at the indicated position
		void remove(Guy* guy, float x, float z) {
			numGuys--;
			
			if (hasChildren && numGuys < MIN_GUYS_PER_QUADTREE) {
				destroyChildren();
			}
			
			if (hasChildren) {
				fileGuy(guy, x, z, false);
			}
			else {
				guys.erase(guy);
			}
		}
	public:
		//Constructs a new Quadtree.  d is the depth, which starts at 1.
		Quadtree(float minX1, float minZ1, float maxX1, float maxZ1, int d) {
			minX = minX1;
			minZ = minZ1;
			maxX = maxX1;
			maxZ = maxZ1;
			centerX = (minX + maxX) / 2;
			centerZ = (minZ + maxZ) / 2;
			
			depth = d;
			numGuys = 0;
			hasChildren = false;
		}
		
		~Quadtree() {
			if (hasChildren) {
				destroyChildren();
			}
		}
		
		//Adds a guy to this
		void add(Guy* guy) {
			numGuys++;
			if (!hasChildren && depth < MAX_QUADTREE_DEPTH &&
				numGuys > MAX_GUYS_PER_QUADTREE) {
				haveChildren();
			}
			
			if (hasChildren) {
				fileGuy(guy, guy->x(), guy->z(), true);
			}
			else {
				guys.insert(guy);
			}
		}
		
		//Removes a guy from this
		void remove(Guy* guy) {
			remove(guy, guy->x(), guy->z());
		}
		
		//Changes the position of a guy in this from the specified position to
		//its current position
		void guyMoved(Guy* guy, float x, float z) {
			remove(guy, x, z);
			add(guy);
		}
		
		//Adds potential collisions to the specified set
		void potentialCollisions(vector<GuyPair> &collisions) {
			if (hasChildren) {
				for(int x = 0; x < 2; x++) {
					for(int z = 0; z < 2; z++) {
						children[x][z]->potentialCollisions(collisions);
					}
				}
			}
			else {
				//Add all pairs (guy1, guy2) from guys
				for(set<Guy*>::iterator it = guys.begin(); it != guys.end();
						it++) {
					Guy* guy1 = *it;
					for(set<Guy*>::iterator it2 = guys.begin();
							it2 != guys.end(); it2++) {
						Guy* guy2 = *it2;
						//This test makes sure that we only add each pair once
						if (guy1 < guy2) {
							GuyPair gp;
							gp.guy1 = guy1;
							gp.guy2 = guy2;
							collisions.push_back(gp);
						}
					}
				}
			}
		}
};

void potentialCollisions(vector<GuyPair> &cs, Quadtree* quadtree) {
	quadtree->potentialCollisions(cs);
}

//Returns whether guy1 and guy2 are currently colliding
bool testCollision(Guy* guy1, Guy* guy2) {
	float dx = guy1->x() - guy2->x();
	float dz = guy1->z() - guy2->z();
	float r = guy1->radius() + guy2->radius();
	if (dx * dx + dz * dz < r * r) {
		float vx = guy1->velocityX() - guy2->velocityX();
		float vz = guy1->velocityZ() - guy2->velocityZ();
		return vx * dx + vz * dz < 0;
	}
	else {
		return false;
	}
}

void handleCollisions(vector<Guy*> &guys,
					  Quadtree* quadtree,
					  int &numCollisions) {
	vector<GuyPair> gps;
	potentialCollisions(gps, quadtree);
	for(unsigned int i = 0; i < gps.size(); i++) {
		GuyPair gp = gps[i];
		
		Guy* g1 = gp.guy1;
		Guy* g2 = gp.guy2;
		if (testCollision(g1, g2)) {
			g1->bounceOff(g2);
			g2->bounceOff(g1);
			numCollisions++;
		}
	}
}

//Moves the guys over the given interval of time, without handling collisions
void moveGuys(vector<Guy*> &guys, Quadtree* quadtree, float dt) {
	for(unsigned int i = 0; i < guys.size(); i++) {
		Guy* guy = guys[i];
		float oldX = guy->x();
		float oldZ = guy->z();
		guy->advance(dt);
		quadtree->guyMoved(guy, oldX, oldZ);
	}
}

//Advances the state of the guys over the indicated interval of time
void advance(vector<Guy*> &guys,
			 Quadtree* quadtree,
			 float t,
			 float &timeUntilHandleCollisions,
			 int &numCollisions) {
	while (t > 0) {
		if (timeUntilHandleCollisions <= t) {
			moveGuys(guys, quadtree, timeUntilHandleCollisions);
			handleCollisions(guys, quadtree, numCollisions);
			t -= timeUntilHandleCollisions;
			timeUntilHandleCollisions = TIME_BETWEEN_HANDLE_COLLISIONS;
		}
		else {
			moveGuys(guys, quadtree, t);
			timeUntilHandleCollisions -= t;
			t = 0;
		}
	}
}

//Returns a vector of numGuys new guys
vector<Guy*> makeGuys(int numGuys, MD2Model* model, Terrain* terrain) {
	vector<Guy*> guys;
	for(int i = 0; i < numGuys; i++) {
		guys.push_back(new Guy(model,
							   terrain,
							   TERRAIN_WIDTH / (terrain->width() - 1)));
	}
	return guys;
}

//Draws the terrain
void drawTerrain(Terrain* terrain) {
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.3f, 0.9f, 0.0f);
	for(int z = 0; z < terrain->length() - 1; z++) {
		glBegin(GL_TRIANGLE_STRIP);
		for(int x = 0; x < terrain->width(); x++) {
			Vec3f normal = terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z), z);
			normal = terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}
}

//Draws a string at the top of the screen indicating that the specified number
//of collisions have occurred
void drawNumCollisions(int numCollisions) {
	ostringstream oss;
	oss << "Collisions: " << numCollisions;
	string str = oss.str();
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor3f(1.0f, 1.0f, 0.0f);
	glPushMatrix();
	glTranslatef(0.0f, 1.7f, -5.0f);
	glScalef(0.2f, 0.2f, 0.2f);
	t3dDraw2D(str, 0, 0);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}





MD2Model* _model;
vector<Guy*> _guys;
Terrain* _terrain;
float _angle = 0;
Quadtree* _quadtree;
//The amount of time until we next check for and handle all collisions
float _timeUntilHandleCollisions = 0;
int _numCollisions; //The total number of collisions that have occurred

void cleanup() {
	delete _model;
	
	for(unsigned int i = 0; i < _guys.size(); i++) {
		delete _guys[i];
	}
	
	t3dCleanup();
}

void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			cleanup();
			exit(0);
	}
}

//Makes the image into a texture, and returns the id of the texture
GLuint loadTexture(Image *image) {
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_RGB,
				 image->width, image->height,
				 0,
				 GL_RGB,
				 GL_UNSIGNED_BYTE,
				 image->pixels);
	return textureId;
}

void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
	
	t3dInit(); //Initialize text drawing functionality
	
	//Load the model
	_model = MD2Model::load("blockybalboa.md2");
	if (_model != NULL) {
		_model->setAnimation("run");
	}
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 1.0, 200.0);
}

void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//Draw the number of collisions that have occurred
	drawNumCollisions(_numCollisions);
	
	//The scaling factor for the terrain
	float scale = TERRAIN_WIDTH / (_terrain->width() - 1);
	
	glTranslatef(0, 0, -1.0f * scale * (_terrain->length() - 1));
	glRotatef(30, 1, 0, 0);
	glRotatef(_angle, 0, 1, 0);
	glTranslatef(-TERRAIN_WIDTH / 2, 0, -scale * (_terrain->length() - 1) / 2);
	
	GLfloat ambientLight[] = {0.5f, 0.5f, 0.5f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	
	GLfloat lightColor[] = {0.5f, 0.5f, 0.5f, 1.0f};
	GLfloat lightPos[] = {-0.2f, 0.3f, -1, 0.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
	//Draw the guys
	for(unsigned int i = 0; i < _guys.size(); i++) {
		_guys[i]->draw();
	}
	
	//Draw the terrain
	glScalef(scale, scale, scale);
	drawTerrain(_terrain);
	
	glutSwapBuffers();
}

void update(int value) {
	_angle += 0.3f;
	if (_angle > 360) {
		_angle -= 360;
	}
	
	advance(_guys,
			_quadtree,
			0.025f,
			_timeUntilHandleCollisions,
			_numCollisions);
	
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}

int main(int argc, char** argv) {
	srand((unsigned int)time(0)); //Seed the random number generator
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	
	glutCreateWindow("Putting It All Together - videotutorialsrock.com");
	initRendering();
	
	_terrain = loadTerrain("heightmap.bmp", 30.0f); //Load the terrain
	_guys = makeGuys(NUM_GUYS, _model, _terrain); //Create the guys
	//Compute the scaling factor for the terrain
	float scaledTerrainLength =
		TERRAIN_WIDTH / (_terrain->width() - 1) * (_terrain->length() - 1);
	//Construct and initialize the quadtree
	_quadtree = new Quadtree(0, 0, TERRAIN_WIDTH, scaledTerrainLength, 1);
	for(unsigned int i = 0; i < _guys.size(); i++) {
		_quadtree->add(_guys[i]);
	}
	
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutReshapeFunc(handleResize);
	glutTimerFunc(25, update, 0);
	
	glutMainLoop();
	return 0;
}









