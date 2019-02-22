#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

#include <vector>
using std::vector;


/*
Name: Pavanjot Gill
*/


float sceneRotX; //Contains the rotation angle for the scene about the x axis
float sceneRotY; //Contains the rotation angle for the scene about the y axis

float  a, b, c; //variables used for fault algorithm
int indexX1, indexX2, indexZ1, indexZ2; //random points on the terrain to be used for creating the line for the fault algorithm
float maxHeight, minHeight; //max and min height of the terrain generated
int wireToggle; //toggle for wireframe mode (0 - solid, 1 - wire, 2 - wire + solid)
int lightToggle; //toggle for lighting(1 - lights, 0 - lights off)
int shadeMode; //togggle for shading mode (0 - flat, 1 - smooth)

int lookatPos[3]; //camera position

float position[4], position1[4]; //light positions
int terrainSize; //size of terrain
int lightMove; //which light is currently selected for moving by user

int customTerrain; //whether custom terrain mode is on or off
int mouseClick; //whether when in custom terrain mode, user has selected first point or second (both used to create a line)
int click1X, click1Z, click2X, click2Z; //store the points that the user clicked on to create a line for custom fault algorithm

/*
structure to store the normals for each face after they are calculated
*/
struct faceNorm{
	float normal[3];
};

/*
structure to store the normals for each vertex after they are calculated
*/
struct vertNorm{
	float normal[3];
};


vector<vector <int> > verts; //nested vector which stores the heightvalue for x and z coordinates (heightmap)

vector<vector <faceNorm> > faceNormals; //nested vector which stores the normals for each face of type faceNorm structure
vector<vector <vertNorm> > vertNormals; //nested vector which stores the normals for each vertex of type vertNorm structure

/*
Function used to draw the quads
when the vertices are drawn, the coordinates (x and z) are subtracted by (terrainSize/2) so the entire terrain is centered at 0,0
*/
void drawQuads(int x, int z, bool wire){
	float color;
	/*
	this conditional is used when both solid and wire frame are rendered
	*/
	if (wire == false){

		/*
		* If shadeMode = 0, shading is GL_Flat so need face normals
		if shadeMode = 1, shading is GL_Smooth so need vertex normals (occurs below)
		*/
		if (shadeMode == 0){
			
			glNormal3fv(faceNormals[x][z].normal);
		}

		glBegin(GL_QUADS);

		color = (verts[x][z] + abs(minHeight)) / (maxHeight + abs(minHeight)); //formula to get greyscale values, minHeight is black and maxHeight is white, the rest is relative to this
		glColor3f(color, color, color);

		if (shadeMode == 1){
			glNormal3fv(vertNormals[x][z].normal);
		}
		glVertex3f(x - (terrainSize / 2), verts[x][z], z - (terrainSize / 2));

		color = (verts[x + 1][z] + abs(minHeight)) / (maxHeight + abs(minHeight));
		glColor3f(color, color, color); 
		
		if (shadeMode == 1){
			glNormal3fv(vertNormals[x + 1][z].normal);
		}
		glVertex3f(x - (terrainSize / 2) + 1, verts[x + 1][z], z - (terrainSize / 2));

		color = (verts[x + 1][z + 1] + abs(minHeight)) / (maxHeight + abs(minHeight));
		glColor3f(color, color, color);

		if (shadeMode == 1){
			glNormal3fv(vertNormals[x + 1][z + 1].normal);
		}
		glVertex3f(x - (terrainSize / 2) + 1, verts[x + 1][z + 1], z - (terrainSize / 2) + 1);

		color = (verts[x][z + 1] + abs(minHeight)) / (maxHeight + abs(minHeight));
		glColor3f(color, color, color);

		if (shadeMode == 1){
			glNormal3fv(vertNormals[x][z + 1].normal);
		}
		glVertex3f(x - (terrainSize / 2), verts[x][z + 1], z - (terrainSize / 2) + 1);

		glEnd();
	}
	/*
	Draw without greyscale coloring when both solid and wire frame are selected
	*/
	else{
		glBegin(GL_QUADS);

		glVertex3f(x - (terrainSize / 2), verts[x][z], z - (terrainSize / 2));
		glVertex3f(x - (terrainSize / 2) + 1, verts[x + 1][z], z - (terrainSize / 2));
		glVertex3f(x - (terrainSize / 2) + 1, verts[x + 1][z + 1], z - (terrainSize / 2) + 1);
		glVertex3f(x - (terrainSize / 2), verts[x][z + 1], z - (terrainSize / 2) + 1);

		glEnd();
	}
}

/*
display function which calls drawQuads function
materials and lighting are all created in this function
*/
void display(){

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/*
	Material values for terrain
	*/
	float terrainAmb[] = { 1, 1, 1, 1.0 };
	float terrainDif[] = { 1, 1, 1, 1.0 };
	float terrainSpec[] = { 1, 1, 1, 1.0 };
	float terrainShiny = 32;

	/*float position[4] = { -1*(terrainSize/2), 42, -1*(terrainSize/2), 0};*/

	/*
	Lighting values for terrain light0
	*/
	float amb[4] = { .1, .1, .1, 1 };
	float diff[4] = { 0, 1, 1, 1 };
	float spec[4] = { 0, 0, 0, 1 };

	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

	/*
	light1 has same values as light0 except for diffuse and position
	*/
	diff[0] = 0.3;
	diff[1] = 1;
	diff[2] = 0.25;

	glLightfv(GL_LIGHT1, GL_POSITION, position1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diff);
	glLightfv(GL_LIGHT1, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spec);

	gluLookAt(lookatPos[0], lookatPos[1], lookatPos[2], 0, 0, 0, 0, 1, 0); //camera position
	

	glPushMatrix();
	
	glRotatef(sceneRotY, 0, 1, 0); //used to rotate everything in the scene
	glRotatef(sceneRotX, 1, 0, 0);

	glPushMatrix();

	/*
	set material properties
	*/
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, terrainAmb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, terrainDif);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, terrainSpec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, terrainShiny);

	/*
	if wireToggle = 2, that means that terrain has to be rendered twice, once as a solid, and another as a wireframe
	*/
	if (wireToggle == 2){
		glPolygonMode(GL_FRONT, GL_LINE);
		for (int x = 0; x < (terrainSize - 1); x++){
			for (int z = 0; z < (terrainSize - 1); z++){
				glColor3f(0, 0, 0);
				drawQuads(x, z, true);
			}
		}

		glPolygonMode(GL_FRONT, GL_FILL);
		for (int x = 0; x < (terrainSize - 1); x++){
			for (int z = 0; z < (terrainSize - 1); z++){
				drawQuads(x, z, false);
			}
		}
	}
	else{ //else just draw it like normal
		for (int x = 0; x < (terrainSize - 1); x++){
			for (int z = 0; z < (terrainSize - 1); z++){
				drawQuads(x, z, false);
			}
		}
	}

	glPopMatrix();

	glPopMatrix(); 

	glutSwapBuffers();

}

/*
display function used to create the 2D overview
*/
void display2D(){

	float color;

	glClear(GL_COLOR_BUFFER_BIT);

	for (int x = 0; x < (terrainSize - 1); x++){
	for (int z = 0; z < (terrainSize - 1); z++){
		color = (verts[x][z] + abs(minHeight)) / (maxHeight + abs(minHeight)); //use same greyscale formula as above
		glColor3f(color, color, color);
		glPointSize(1); //point size of 1 pixel
		glBegin(GL_POINTS); //create all points
		glVertex2f(x - (terrainSize/2), z - (terrainSize/2));
		glEnd();
}
}
	glFlush();
}

/*
Function to calculate the normals for each quad face
normals calculated stored in faceNormals vector
source: Fernandes, António. "Terrain Tutorial." OpenGL @ Lighthouse 3D -. António Ramires Fernandes. Web. 26 Nov. 2014. <http://www.lighthouse3d.com/opengl/terrain/index.php3?normals>
*/
void calcFaceNorms(){
	float vector1[3];
	float vector2[3];
	float normal[3];
	float magnitude;
	
	for (int x = 0; x < (terrainSize - 1); x++){
		faceNormals.push_back(vector <faceNorm>());
		for (int z = 0; z < (terrainSize - 1); z++){
			faceNormals[x].push_back(faceNorm());

			vector1[0] = (x + 1) - x;
			vector1[1] = verts[x + 1][z] - verts[x][z];
			vector1[2] = z - z;

			vector2[0] = (x - x);
			vector2[1] = verts[x][z + 1] - verts[x][z];
			vector2[2] = (z + 1) - z;

			normal[0] = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normal[1] = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normal[2] = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);

			magnitude = sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));

			faceNormals[x][z].normal[0] = (normal[0] / magnitude);
			faceNormals[x][z].normal[1] = (normal[1] / magnitude);
			faceNormals[x][z].normal[2] = (normal[2] / magnitude);
		}
	}
}


/*
Function used when calculating vertex normals, could be done in the calcVertNorms function but this saves lines
source: Fernandes, António. "Terrain Tutorial." OpenGL @ Lighthouse 3D -. António Ramires Fernandes. Web. 26 Nov. 2014. <http://www.lighthouse3d.com/opengl/terrain/index.php3?normals>
*/
float* calcNorm(float* point1, float* point2, float* point3){

	float vector1[3];
	float vector2[3];
	float magnitude;
	float normal[3];

	vector1[0] = point2[0] - point1[0];
	vector1[1] = point2[1] - point1[1];
	vector1[2] = point2[2] - point1[2];

	vector2[0] = point3[0] - point1[0];
	vector2[1] = point3[1] - point1[1];
	vector2[2] = point3[2] - point1[2];
	normal[0] = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
	normal[1] = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
	normal[2] = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);

	magnitude = sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));


	normal[0] = (normal[0] / magnitude);
	normal[1] = (normal[1] / magnitude);
	normal[2] = (normal[2] / magnitude);

	return normal;

}

/*
Function used for calculating vertex normals
calls the function calcNorm()
normals calculated stored in vertNormals vector
source: Fernandes, António. "Terrain Tutorial." OpenGL @ Lighthouse 3D -. António Ramires Fernandes. Web. 26 Nov. 2014. <http://www.lighthouse3d.com/opengl/terrain/index.php3?normals>
*/
void calcVertNorms(){
	float origin[3];
	float normal1[3];
	float normal2[3];
	float normal3[3];
	float normal4[3];
	float normal[3];
	float magnitude;

	for (int x = 0; x < terrainSize; x++){

		vertNormals.push_back(vector <vertNorm>());

		for (int z = 0; z < terrainSize; z++){

			vertNormals[x].push_back(vertNorm());

			origin[0] = x;
			origin[1] = verts[x][z];
			origin[2] = z;

			if (x == 0 && z == 0){ //calculate normal on a corner
				float point1[] = { x + 1, verts[x + 1][z], z };
				float point2[] = { x, verts[x][z + 1], z + 1 };

				vertNormals[x][z].normal[0] = (calcNorm(origin, point1, point2))[0];
				vertNormals[x][z].normal[1] = (calcNorm(origin, point1, point2))[1];
				vertNormals[x][z].normal[2] = (calcNorm(origin, point1, point2))[2];
			}
			else if (x == 0 && z == (terrainSize - 1)){ //calculate normal on a different corner
				float point1[] = { x + 1, verts[x+1][z], z};
				float point2[] = { x, verts[x][z - 1], z - 1};


				vertNormals[x][z].normal[0] = (calcNorm(origin, point1, point2))[0];
				vertNormals[x][z].normal[1] = (calcNorm(origin, point1, point2))[1];
				vertNormals[x][z].normal[2] = (calcNorm(origin, point1, point2))[2];

			}
			else if (x == (terrainSize - 1) && z == 0){ //calculate normal on a third corner
				float point1[] = { x - 1, verts[x-1][z], z };
				float point2[] = { x, verts[x][z+1], z+1};


				vertNormals[x][z].normal[0] = (calcNorm(origin, point1, point2))[0];
				vertNormals[x][z].normal[1] = (calcNorm(origin, point1, point2))[1];
				vertNormals[x][z].normal[2] = (calcNorm(origin, point1, point2))[2];

			}
			else if (x == (terrainSize - 1) && z == (terrainSize - 1)){ //calculate normal on the last corner
				float point1[] = { x, verts[x][z - 1], z - 1 };
				float point2[] = { x - 1, verts[x - 1][z], z };

				vertNormals[x][z].normal[0] = (calcNorm(origin, point1, point2))[0];
				vertNormals[x][z].normal[1] = (calcNorm(origin, point1, point2))[1];
				vertNormals[x][z].normal[2] = (calcNorm(origin, point1, point2))[2];

			}
			else if (x == 0){

				float point1[] = { x, verts[x][z + 1], z + 1 };
				float point2[] = { x + 1, verts[x + 1][z], z };
				float point3[] = { x, verts[x][z - 1], z - 1 };

				/*
				Following code is used to caluclate the normal for a vertex on an edge
				the code is repeated for each edge (x = 0, x = terrainSize - 1, z = 0, and z = terrainSize - 1)
				*/
				normal1[0] = calcNorm(origin, point1, point2)[0];
				normal1[1] = calcNorm(origin, point1, point2)[1];
				normal1[2] = calcNorm(origin, point1, point2)[2];

				normal2[0] = calcNorm(origin, point2, point3)[0];
				normal2[1] = calcNorm(origin, point2, point3)[1];
				normal2[2] = calcNorm(origin, point2, point3)[2];

				normal3[0] = calcNorm(origin, point3, point1)[0];
				normal3[1] = calcNorm(origin, point3, point1)[1];
				normal3[2] = calcNorm(origin, point3, point1)[2];

				normal[0] = normal1[0] + normal2[0] + normal3[0];
				normal[1] = normal1[1] + normal2[1] + normal3[1];
				normal[2] = normal1[2] + normal2[2] + normal3[2];

				magnitude = sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));

				vertNormals[x][z].normal[0] = (normal[0] / magnitude);
				vertNormals[x][z].normal[1] = (normal[1] / magnitude);
				vertNormals[x][z].normal[2] = (normal[2] / magnitude);

			}
			else if (z == 0){
				
				float point1[] = { x - 1, verts[x + 1][z], z };
				float point2[] = { x, verts[x][z + 1], z + 1 };
				float point3[] = { x + 1, verts[x - 1][z], z };

				normal1[0] = calcNorm(origin, point1, point2)[0];
				normal1[1] = calcNorm(origin, point1, point2)[1];
				normal1[2] = calcNorm(origin, point1, point2)[2];

				normal2[0] = calcNorm(origin, point2, point3)[0];
				normal2[1] = calcNorm(origin, point2, point3)[1];
				normal2[2] = calcNorm(origin, point2, point3)[2];
				
				normal3[0] = calcNorm(origin, point3, point1)[0];
				normal3[1] = calcNorm(origin, point3, point1)[1];
				normal3[2] = calcNorm(origin, point3, point1)[2];

				normal[0] = normal1[0] + normal2[0] + normal3[0];
				normal[1] = normal1[1] + normal2[1] + normal3[1];
				normal[2] = normal1[2] + normal2[2] + normal3[2];

				magnitude = sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));

				vertNormals[x][z].normal[0] = (normal[0] / magnitude);
				vertNormals[x][z].normal[1] = (normal[1] / magnitude);
				vertNormals[x][z].normal[2] = (normal[2] / magnitude);
			}
			else if (x == (terrainSize - 1)){
				float point1[] = { x, verts[x][z - 1], z - 1 };
				float point2[] = { x - 1, verts[x - 1][z], z };
				float point3[] = { x, verts[x][z + 1], z + 1 };

				normal1[0] = calcNorm(origin, point1, point2)[0];
				normal1[1] = calcNorm(origin, point1, point2)[1];
				normal1[2] = calcNorm(origin, point1, point2)[2];

				normal2[0] = calcNorm(origin, point2, point3)[0];
				normal2[1] = calcNorm(origin, point2, point3)[1];
				normal2[2] = calcNorm(origin, point2, point3)[2];

				normal3[0] = calcNorm(origin, point3, point1)[0];
				normal3[1] = calcNorm(origin, point3, point1)[1];
				normal3[2] = calcNorm(origin, point3, point1)[2];

				normal[0] = normal1[0] + normal2[0] + normal3[0];
				normal[1] = normal1[1] + normal2[1] + normal3[1];
				normal[2] = normal1[2] + normal2[2] + normal3[2];

				magnitude = sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));

				vertNormals[x][z].normal[0] = (normal[0] / magnitude);
				vertNormals[x][z].normal[1] = (normal[1] / magnitude);
				vertNormals[x][z].normal[2] = (normal[2] / magnitude);
			}
			else if (z == (terrainSize - 1)){
				float point1[] = { x + 1, verts[x + 1][z], z };
				float point2[] = { x, verts[x][z - 1], z - 1 };
				float point3[] = { x - 1, verts[x - 1][z], z };

				normal1[0] = calcNorm(origin, point1, point2)[0];
				normal1[1] = calcNorm(origin, point1, point2)[1];
				normal1[2] = calcNorm(origin, point1, point2)[2];

				normal2[0] = calcNorm(origin, point2, point3)[0];
				normal2[1] = calcNorm(origin, point2, point3)[1];
				normal2[2] = calcNorm(origin, point2, point3)[2];

				normal3[0] = calcNorm(origin, point3, point1)[0];
				normal3[1] = calcNorm(origin, point3, point1)[1];
				normal3[2] = calcNorm(origin, point3, point1)[2];

				normal[0] = normal1[0] + normal2[0] + normal3[0];
				normal[1] = normal1[1] + normal2[1] + normal3[1];
				normal[2] = normal1[2] + normal2[2] + normal3[2];

				magnitude = sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));

				vertNormals[x][z].normal[0] = (normal[0] / magnitude);
				vertNormals[x][z].normal[1] = (normal[1] / magnitude);
				vertNormals[x][z].normal[2] = (normal[2] / magnitude);

			}
			else{
				/*
				Following code is used to calculate the vertex normals for standard vertices that aren't on edges or corners
				*/
				float point1[] = { x - 1, verts[x - 1][z], z };
				float point2[] = { x, verts[x][z + 1], z + 1 };
				float point3[] = { x + 1, verts[x + 1][z], z };
				float point4[] = { x, verts[x][z - 1], z - 1 };

				normal1[0] = calcNorm(origin, point1, point2)[0];
				normal1[1] = calcNorm(origin, point1, point2)[1];
				normal1[2] = calcNorm(origin, point1, point2)[2];

				normal2[0] = calcNorm(origin, point2, point3)[0];
				normal2[1] = calcNorm(origin, point2, point3)[1];
				normal2[2] = calcNorm(origin, point2, point3)[2];

				normal3[0] = calcNorm(origin, point3, point4)[0];
				normal3[1] = calcNorm(origin, point3, point4)[1];
				normal3[2] = calcNorm(origin, point3, point4)[2];

				normal4[0] = calcNorm(origin, point4, point1)[0];
				normal4[1] = calcNorm(origin, point4, point1)[1];
				normal4[2] = calcNorm(origin, point4, point1)[2];

				normal[0] = normal1[0] + normal2[0] + normal3[0] + normal4[0];
				normal[1] = normal1[1] + normal2[1] + normal3[1] + normal4[1];
				normal[2] = normal1[2] + normal2[2] + normal3[2] + normal4[2];

				magnitude = sqrt((normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]));

				vertNormals[x][z].normal[0] = (normal[0] / magnitude);
				vertNormals[x][z].normal[1] = (normal[1] / magnitude);
				vertNormals[x][z].normal[2] = (normal[2] / magnitude);


			}
		}
		}
	
}

/*
method used to generate terrain using fault algorithm 
source: Fernandes, António. "Terrain Tutorial." OpenGL @ Lighthouse 3D -. António Ramires Fernandes. Web. 23 Nov. 2014. <http://www.lighthouse3d.com/opengl/terrain/index.php?impdetails>
*/
void faultAlg(){
	for (int counter = 0; counter < 300; counter++){
		indexX1 = (rand() % terrainSize) - (terrainSize/2);
		indexX2 = (rand() % terrainSize) - (terrainSize/2);
		indexZ1 = (rand() % terrainSize) - (terrainSize/2);
		indexZ2 = (rand() % terrainSize) - (terrainSize/2);
		a = (indexZ2 - indexZ1);
		b = (-1 * (indexX2 - indexX1));
		c = ((-1 * (indexX1*(a))) + (indexZ1 * (-1 * b)));

		for (int x = 0; x < terrainSize; x++){
			for (int z = 0; z < terrainSize; z++){
				if ((a*(x - terrainSize/2) + b*(z - terrainSize/2) - c) > 0){
					verts[x][z] += 1;
				}
				else{
					verts[x][z] -= 1;
				}
			}
		}

		maxHeight = minHeight = 1; //to avoid divide by zero when calculating greyscale values

		for (int x = 0; x < terrainSize; x++){
			for (int z = 0; z < terrainSize; z++){
				if (verts[x][z] > maxHeight){
					maxHeight = verts[x][z];
				}
				else if (verts[x][z] < minHeight){
					minHeight = verts[x][z];
				}
			}
		}
	}
	faceNormals.clear();
	vertNormals.clear();
	calcFaceNorms();
	calcVertNorms();
}

/*
function allowing user to create the lines for the fault algorithm themselves when in custom terrain mode
source: Fernandes, António. "Terrain Tutorial." OpenGL @ Lighthouse 3D -. António Ramires Fernandes. Web. 27 Nov. 2014. <http://www.lighthouse3d.com/opengl/terrain/index.php?impdetails>
*/
void customFaultAlg(int x1, int z1, int x2, int z2){
	a = (z2 - z1);
	b = (-1 * (x2 - x1));
	c = ((-1 * (x1*(a))) + (z1 * (-1 * b)));

	for (int x = 0; x < terrainSize; x++){
		for (int z = 0; z < terrainSize; z++){
			if ((a*(x - terrainSize/2) + b*(z - terrainSize/2) - c) > 0){
				verts[x][z] += 1;
			}
			else{
				verts[x][z] -= 1;
			}
		}
	}

	maxHeight = minHeight = 1; //to avoid divide by zero when calculating greyscale values

	for (int x = 0; x < terrainSize; x++){
		for (int z = 0; z < terrainSize; z++){
			if (verts[x][z] > maxHeight){
				maxHeight = verts[x][z];
			}
			else if (verts[x][z] < minHeight){
				minHeight = verts[x][z];
			}
		}
	}
	faceNormals.clear();
	vertNormals.clear();
	calcFaceNorms();
	calcVertNorms();
}

/*
create vector of size (terrainSize)
*/
void fillVerts(){
	for (int x = 0; x < terrainSize; x++){
		verts.push_back(vector <int> ());
		for (int z = 0; z < terrainSize; z++){
			verts[x].push_back(0);
		}
	}
}

/*
Keyboard method that does different things depending on which key is pressed
*/
void keyBoard(unsigned char keyPressed, int x, int y){

	switch (keyPressed)
	{

	case 'q': //exit program
		exit(0);
		break;
		
	case 'w': //wire frame toggle
		if (wireToggle == 0){
			wireToggle = 1;
			glPolygonMode(GL_FRONT, GL_LINE);
			glutPostRedisplay();
		}
		else if (wireToggle == 1){
			wireToggle = 2;
			glutPostRedisplay();
		}
		else{
			wireToggle = 0;
			glPolygonMode(GL_FRONT, GL_FILL);
			glutPostRedisplay();
		}
		break;

	case 'c': //custom terrain mode toggle
		if (customTerrain == 0){
			customTerrain = 1;
			verts.clear();
			fillVerts();
		}
		else{
			customTerrain = 0;
			verts.clear();
			fillVerts();
			faultAlg();
		}
		glutPostRedisplay();
		glutPostWindowRedisplay(2);
		break;
	case 'r': //reset (creates new random terrain)
		verts.clear();

		sceneRotX = 0; //scene is at start position
		sceneRotY = 0;
		wireToggle = 0;
		customTerrain = 0;

		position[0] = (-1 * (terrainSize / 2));
		position[1] = 50;
		position[2] = (-1 * (terrainSize / 2));
		position[3] = 0;

		position1[0] = -1 * position[0];
		position1[1] = position[1];
		position1[2] = -1 * position[2];
		position[3] = 0;

		fillVerts();
		faultAlg();

		glutPostRedisplay();
		glutPostWindowRedisplay(2);
		break;

	case '+': //increase size of terrain
		if (terrainSize < 300){
			terrainSize += 50;
			verts.clear();
			fillVerts();
			customTerrain = 0;
			faultAlg();
			lookatPos[0] += 50;
			lookatPos[1] += 50;
			lookatPos[2] += 50;
			glutPostRedisplay();
			glutPostWindowRedisplay(2);
		}
		break;
	case '-': //decrease size of terrain
		if (terrainSize > 50){
			terrainSize -= 50;
			verts.clear();
			fillVerts();
			customTerrain = 0;
			faultAlg();
			lookatPos[0] -= 50;
			lookatPos[1] -= 50;
			lookatPos[2] -= 50;
			glutPostRedisplay();
			glutPostWindowRedisplay(2);
		}
		break;

	case 's': //switching shade mode
		if (shadeMode == 0){
			glShadeModel(GL_SMOOTH);
			shadeMode = 1;
		}
		else{
			glShadeModel(GL_FLAT);
			shadeMode = 0;
		}
		glutPostRedisplay();
		break;

	case 'l': //light mode toggle
		if (lightToggle == 0){
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			glEnable(GL_LIGHT1);
			
			lightToggle = 1;
		}
		else{
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
			glDisable(GL_LIGHT1);
			lightToggle = 0;
			lightMove = 0;
		}
		glutPostRedisplay();
		break;

	case '0': //select light source 0 for moving, only if light toggle is on
		if (lightToggle == 1){
			lightMove = 1;
		}
		else{
			printf("Please turn on lighting before selecting light source to move!");
		}
		break;

	case '1': //select light source 1 for moving, only if light toggle is on
		if (lightToggle == 1){
			lightMove = 2;
		}
		else{
			printf("Please turn on lighting before selecting light source to move!");
		}
		break;

	case 't': //move selected light source (increase x direction)
		if (lightMove == 1){
			position[0]++;
		}
		else if (lightMove == 2){
			position1[0]++;
		}
		glutPostRedisplay();
		break;

	case 'y': //move selected light source (decrease x direction)
		if (lightMove == 1){
			position[0]--;
		}
		else if (lightMove == 2){
			position1[0]--;
		}
		glutPostRedisplay();
		break;

	case 'u': //move selected light source (increase y direction)
		if (lightMove == 1){
			position[1]++;
		}
		else if (lightMove == 2){
			position1[1]++;
		}
		glutPostRedisplay();
		break;

	case 'i': //move selected light source (decrease y direction)
		if (lightMove == 1){
			position[1]--;
		}
		else if (lightMove == 2){
			position1[1]--;
		}
		glutPostRedisplay();
		break;

	case 'o': //move selected light source (increase z direction)
		if (lightMove == 1){
			position[2]++;
		}
		else if (lightMove == 2){
			position1[2]++;
		}
		glutPostRedisplay();
		break;

	case 'p': //move selected light source (decrease x direction)
		if (lightMove == 1){
			position[2]--;
		}
		else if (lightMove == 2){
			position1[2]--;
		}
		glutPostRedisplay();
		break;
		}
}

/*
keyboard method for special keys such as the arrow keys
*/
void specialKeys(int keyPressed, int x, int y)
{

	switch (keyPressed)
	{
		/*
		left and right arrow keys rotate scene about y axis
		up and down arrow keys rotate scene about x axis
		*/
	case GLUT_KEY_LEFT:
		sceneRotY--;
		break;

	case GLUT_KEY_RIGHT:
		sceneRotY++;
		break;

	case GLUT_KEY_DOWN:
		sceneRotX--;
		break;

	case GLUT_KEY_UP:
		sceneRotX++;
		break;
	}
	glutPostRedisplay();
}

/*
mouse function for use when user creating own terrain
stores the mouse position when clicked (regardless of which button)
*/
void mouse2D(int btn, int state, int xcoord, int ycoord){
	/*
	if mouse click is outside of terrain, the coordinate outside of terrain range becomes the edge of the terrain
	*/
		if (customTerrain == 1){
		if ((mouseClick == 0) && (state == GLUT_DOWN)){
			if ((155 - xcoord) < (-terrainSize / 2)){
				click1X = -terrainSize / 2;
			}
			else if ((155 - xcoord) > (terrainSize / 2)){
				click1X = terrainSize / 2;
			}
			else{
				click1X = (155 - xcoord);
			}
			if ((155 - ycoord) < (-terrainSize / 2)){
				click1Z = terrainSize / 2;
			}
			else if ((155 - ycoord) > (terrainSize / 2)){
				click1Z = -terrainSize / 2;
			}
			else{
				click1Z = -1*(155 - ycoord);
			}

			mouseClick = 1;
		}
		else if ((mouseClick == 1) && (state == GLUT_DOWN)){
			if ((155 - xcoord) < (-terrainSize / 2)){
				click2X = -terrainSize / 2;
			}
			else if ((155 - xcoord) > (terrainSize / 2)){
				click2X = terrainSize / 2;
			}
			else{
				click2X = (155 - xcoord);
			}
			if ((155 - ycoord) < (-terrainSize / 2)){
				click2Z = terrainSize / 2;
			}
			else if ((155 - ycoord) > (terrainSize / 2)){
				click2Z = -terrainSize / 2;
			}
			else{
				click2Z = -1 *(155 - ycoord);
			}
			customFaultAlg(click1X, click1Z, click2X, click2Z);
			mouseClick = 0;
			glutPostRedisplay();
			glutPostWindowRedisplay(2);
		}
	}
}

/*
initial values when program is started for the main window ("Terrain System")
*/
void init(void)
{
	terrainSize = 50; //initial size of terrain

	/*
	initial position for light source 0
	*/
	position[0] = (-1 * (terrainSize / 2));
	position[1] = 50;
	position[2] = (-1 * (terrainSize / 2));
	position[3] = 0;

	/*
	initial position for light source 1
	*/
	position1[0] = -1 * position[0];
	position1[1] = position[1];
	position1[2] = -1 * position[2];
	position[3] = 0;

	/*
	initial position of camera
	*/
	lookatPos[0] = 0;
	lookatPos[1] = 30;
	lookatPos[2] = 90;

	sceneRotX = 0; //scene is at start position
	sceneRotY = 0;

	/*
	default toggle for all modes
	*/
	wireToggle = 0;
	lightToggle = 0;
	shadeMode = 0;
	lightMove = 0;

	fillVerts(); //create vector with coordinates with height of zero
	
	faultAlg(); //generate terrain
	
	glClearColor(0, 0, 0, 0);
	glColor3f(1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 2500);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //enable culling and have the terrain show up as solid polygons initially
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}

/*
inital values of second window ("2D Overview")
*/
void init2D(){
	glClearColor(0, 0, 0, 0);
	glColor3f(0, 0, 0);

	customTerrain = 0; //custom terrain mode is off default
	mouseClick = 0; //mouse click is 0 initially

	gluOrtho2D(-155, 155, -155, 155);

	glutPostWindowRedisplay(2);

}

/*
Main Method
*/
int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	printf("q - quit\n");
	printf("w - wireframe toggle\n");
	printf("r - reset\n");
	printf("+ - increase terrain size\n");
	printf("- - decrease terrain size\n");
	printf("s - shade model change\n");
	printf("l - light toggle\n");
	printf("0 - select light0 to move\n");
	printf("1 - select light1 to move\n");
	printf("t - increase x position of selected light\n");
	printf("y - decrease x position of selected light\n");
	printf("u - increase y position of selected light\n");
	printf("i - decrease y position of selected light\n");
	printf("o - increase z position of selected light\n");
	printf("p - decrease z position of selected light\n");
	printf("direction keys - rotate scene\n");
	printf("c - custom terrain mode toggle");


	glutInitWindowSize(800, 800);
	glutInitWindowPosition(0, 10);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	
	glutCreateWindow("Terrain System"); //create the window called Terrain System

	glutDisplayFunc(display);
	glutKeyboardFunc(keyBoard);
	glutSpecialFunc(specialKeys);
	
	glEnable(GL_DEPTH_TEST);
	init();

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(310, 310);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("2D Overview"); //create the second window for the 2D overview
	glutSetWindow(2); //set the window's id as 2 (for redraw)
	glutDisplayFunc(display2D);
	glutMouseFunc(mouse2D);
	init2D();

	glutMainLoop();

	return(0);
}