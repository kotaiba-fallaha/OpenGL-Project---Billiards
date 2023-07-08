#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
using namespace std;

/////////////////////////////////////// initialize parameters ///////////////////////////////////////
#define PI 3.1415926535898
// stick parameters 
int stickBallX, stickBallY, stickEndX, stickEndY, stickTall=100;
float stickTransform=0, stickMoveFactor = 0.1;
// mouse parameters 
int mouseX, mouseY;
bool leftButtonPressed = FALSE;
// balls parameters 
void move_balls();
#define r 5
#define ballsCount 16
bool ballsMove = false;
float ballsMoveFactor = 0.1, ballsSlowdownFactor = 0.99;
struct ball{
	float R, G, B;
	float x, y;
	double vx, vy;
};
ball balls[ballsCount] ={
	{ 1.0f, 1.0f, 1.0f, 100.0f, 50.0f, 0.0, 0.0 },  // white ball
	//5
    { 0.0f, 0.0f, 0.0f, 79.8f, 168.8f, 0.0, 0.0 },   // black ball
    { 0.5f, 0.5f, 0.5f, 89.9f, 168.8f, 0.0, 0.0 }, // solid gray ball
    { 1.0f, 1.0f, 0.5f, 100.0f, 168.8f, 0.0, 0.0 }, // striped light yellow ball
    { 1.0f, 0.0f, 0.0f, 110.1f, 168.8f, 0.0, 0.0 }, // solid red ball
    { 0.0f, 0.0f, 1.0f, 120.2f, 168.8f, 0.0, 0.0 },   // striped blue ball
	//4
    { 0.0f, 1.0f, 1.0f, 84.8f, 159.6f, 0.0, 0.0 },  // striped cyan ball
    { 0.5f, 1.0f, 1.0f, 94.9f, 159.6f, 0.0, 0.0 },  // striped turquoise ball
    { 0.0f, 1.0f, 0.0f, 105.1f, 159.6f, 0.0, 0.0 }, // solid green ball
    { 1.0f, 0.5f, 0.0f, 115.2f, 159.6f, 0.0, 0.0 },  // solid orange ball
	//3
    { 1.0f, 0.0f, 1.0f, 89.9f, 150.4f, 0.0, 0.0 }, // striped magenta ball
    { 0.0f, 1.0f, 0.5f, 100.0f, 150.4f, 0.0, 0.0 }, // striped lime ball
    { 0.0f, 0.5f, 1.0f, 110.1f, 150.4f, 0.0, 0.0 },  // striped sky blue ball
	//2
    { 1.0f, 1.0f, 0.0f, 94.9f, 141.2f, 0.0, 0.0 },  // solid yellow ball
    { 1.0f, 0.5f, 1.0f, 105.1f, 141.2f, 0.0, 0.0 }, // striped pink ball
	//1
    { 1.0f, 0.0f, 0.0f, 100.0f, 132.0f, 0.0, 0.0 }, // solid red ball
};
// pockets parameters 
#define pocketsCount 6
struct pocket{
	int x, y;
};
pocket pockets[pocketsCount] ={
	{45,5},
	{45,195},
	{45,100},
	{155,195},
	{155,100},
	{155,5},
};

/////////////////////////////////////// helper functions ///////////////////////////////////////
double angle(double v1x, double v1y, double v2x, double v2y){ // get angle between 2 vectors 
	double dotProduct = v1x * v2x + v1y * v2y;
    double magnitude1 = sqrt(v1x * v1x + v1y * v1y);
    double magnitude2 = sqrt(v2x * v2x + v2y * v2y);
    double cosTheta = dotProduct / (magnitude1 * magnitude2);
    double radians = acos(cosTheta);
    double degrees = radians * 180 / PI;
	// Check if the angle is negative (i.e. v2 is to the left of v1)
    double crossProduct = v1x * v2y - v1y * v2x;
    if (crossProduct > 0) {
        degrees = 360 - degrees;
    }
    //cout << "The degree between vectors v1 and v2 is " << degrees << " degrees." << endl;
	return degrees;
}

double reflection_angle(double vx , double vy, double surfaceAngle) {
	// calculate ball angle from its velocity
    double ballAngleRadians = atan2(vy, vx);
    double ballAngleDegrees = ballAngleRadians * 180.0 / PI;
    ballAngleDegrees = fmod(fmod(ballAngleDegrees, 360.0) + 360.0, 360.0);
	ballAngleRadians = ballAngleDegrees * PI / 180;
	// transfer surface angle to radians
	surfaceAngle *= PI / 180;
	// calculate reflection angle
	double reflectionAngle = 2 * surfaceAngle - ballAngleRadians;
    return reflectionAngle;
}

void simulate_collision(ball& b1, ball& b2, double distance) {
    // calculate the normal and tangent vectors
    double nx = (b2.x - b1.x) / distance;
    double ny = (b2.y - b1.y) / distance;
    double tx = -ny;
    double ty = nx;

    double v1n, v2n, v1t, v2t, v1n_new, v2n_new;
	if(b2.vx == 0 && b2.vy == 0){
		// calculate the velocity components along the normal and tangent vectors
		v1n = b1.vx * nx + b1.vy * ny;
		v1t = b1.vx * tx + b1.vy * ty;
		v2t = v1t;
		// calculate the new normal velocity component after the collision
		v1n_new = -v1n;
		v2n_new = v1n;
	}
	else if(b1.vx == 0 && b1.vy == 0){
		// calculate the velocity components along the normal and tangent vectors
		v2n = b2.vx * nx + b2.vy * ny;
		v2t = b2.vx * tx + b2.vy * ty;
		v1t = v2t;
		// calculate the new normal velocity component after the collision
		v1n_new = v2n;
		v2n_new = -v2n;
	}
	else{
		// calculate the velocity components along the normal and tangent vectors
		v1n = b1.vx * nx + b1.vy * ny;
		v1t = b1.vx * tx + b1.vy * ty;
		v2n = b2.vx * nx + b2.vy * ny;
		v2t = b2.vx * tx + b2.vy * ty;
		// calculate the new normal velocity components after the collision
		v1n_new = v2n;
		v2n_new = v1n;
	}

    // calculate the new total velocity vectors after the collision
    b1.vx = v1n_new * nx + v1t * tx;
    b1.vy = v1n_new * ny + v1t * ty;
    b2.vx = v2n_new * nx + v2t * tx;
    b2.vy = v2n_new * ny + v2t * ty;
}

void detect_balls_collision(ball& b1){
	for (int i = 0; i <= ballsCount; i++){  
		ball& b2 = balls[i];
		// calculate the distance between the centers of the two balls
		double distance = sqrt(pow(b1.x - b2.x, 2) + pow(b1.y - b2.y, 2));
		// check collide between balls
		if (distance <= 2 * r && distance != 0){
			// calculate the velocity components vx and vy of balls
			simulate_collision(b1, b2, distance);
		}
	}
}

void detect_walls_collision(ball& b){
	// reflection the ball from the walls
	if(b.x > 150 || b.x < 50){ // right left wall
		double reflectionAngle = reflection_angle(b.vx , b.vy, 90.0);
		// update ball velocity
		b.vx = abs(b.vx) * cos(reflectionAngle);
		b.vy = abs(b.vy) * sin(reflectionAngle);
	}
	if(b.y > 190 || b.y < 10){ // top bottom wall
		double reflectionAngle = reflection_angle(b.vx , b.vy, 0);
		// update ball velocity
		b.vx = abs(b.vx) * cos(reflectionAngle);
		b.vy = abs(b.vy) * sin(reflectionAngle);
	}
}

void detect_pockets_enterence(ball& b){
	for (int i = 0; i <= pocketsCount; i++){  
		// calculate the distance between the centers of the two balls
		double distance = sqrt(pow(b.x - pockets[i].x, 2) + pow(b.y - pockets[i].y, 2));
		// check collide between balls
		if (distance <= 2 * r){
			b.vx = 0;
			b.vy = 0;
			b.x = -10;
			b.y = -10;
			if(b.R==1&&b.G==1&&b.B==1){
				b.x = 100;
				b.y = 50;
			}
		}
	}
}

/////////////////////////////////////// animation functions ///////////////////////////////////////
void stick_refresh(int x,int y){
	// calculate angle between white ball and mouse 
	double radians = angle(x-balls[0].x, y-balls[0].y, balls[0].x+50, 0) * PI / 180;
	// calculate stik end point from transforming white ball center (stickTall+stickTransform) to mouse direction
	stickEndX = (stickTall+stickTransform) * cos(radians) + balls[0].x;
	stickEndY = (stickTall+stickTransform) * sin(radians) + balls[0].y;
	// calculate stik ball point from transforming white ball center (r+stickTransform) to mouse direction
	stickBallX = (r+stickTransform) * cos(radians) + balls[0].x;
	stickBallY = (r+stickTransform) * sin(radians) + balls[0].y;
}

void move_balls(){
	ballsMove = false;
	for(int i = 0; i < ballsCount; i++){
		// check if ball moving
		if(abs(balls[i].vx) > 1 || abs(balls[i].vy) > 1){
			ballsMove = true;
			// update the positions of the balls based on it's velocity
			balls[i].x += balls[i].vx * ballsMoveFactor;
			balls[i].y += balls[i].vy * ballsMoveFactor;
			// slowdown the ball
			balls[i].vx *= ballsSlowdownFactor;
			balls[i].vy *= ballsSlowdownFactor;
			// 
			detect_balls_collision(balls[i]);
			//
			detect_walls_collision(balls[i]);
			//
			detect_pockets_enterence(balls[i]);
		}
	}
}

/////////////////////////////////////// OpenGL functions///////////////////////////////////////
void display()
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw the table
	glColor3f(0.0, 0.3, 0.0); // Dark green color for the table
	glBegin(GL_QUADS);
	glVertex2f(40, 0);
	glVertex2f(40, 200);
	glVertex2f(160, 200);
	glVertex2f(160, 0);
	glEnd();

	// Draw the table borders
	glColor3f(0.4, 0.2, 0.0); // Brown color for the wooden borders
	glLineWidth(20.0);
	glBegin(GL_LINE_LOOP);
	glVertex2f(40, 0);
	glVertex2f(40, 200);
	glVertex2f(160, 200);
	glVertex2f(160, 0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex2f(43, 3);
	glVertex2f(43, 197);
	glVertex2f(157, 197);
	glVertex2f(157, 3);
	glEnd();

	// Draw the table pockets
	glLineWidth(1.0f);
	for (int j = 0; j < pocketsCount; j++){
		glColor3f(0, 0, 0);
		glBegin(GL_LINES);
		for (float i = 0; i <= 2 * PI * r; i+=0.1){
			glVertex2f(pockets[j].x, pockets[j].y);
			glVertex2f(pockets[j].x + r*sin(i), pockets[j].y + cos(i)*r);
		}
		glEnd();
	}

	// Draw the balls
	glLineWidth(1.0f);
	for (int j = 0; j < ballsCount; j++){
		glColor3f(balls[j].R, balls[j].G, balls[j].B);
		glBegin(GL_LINES);
		for (float i = 0; i <= 2 * PI * r; i+=0.1){
			glVertex2f(balls[j].x, balls[j].y);
			glVertex2f(balls[j].x + r*sin(i), balls[j].y + cos(i)*r);
		}
		glEnd();
	}

	// Draw the cue stick
	if(!ballsMove){
		glColor3f(0.8, 0.4, 0.1);
		glLineWidth(5.0);
		glBegin(GL_LINES);
		glVertex2f(stickBallX, stickBallY);
		glVertex2f(stickEndX, stickEndY);
		glEnd();
	}

	glutSwapBuffers();
}

void motion(int x,int y){
	if(!ballsMove){
		// calculate mouse position according to window coordinates 
		mouseX = x/3;
		mouseY = (600-y)/3;
		stick_refresh(mouseX, mouseY);
		// redraw 
		glutPostRedisplay();
	}
}

void mouse(int button,int state,int x,int y){
	if(!ballsMove){
		if (button == GLUT_LEFT_BUTTON) {
			// calculate mouse position according to window coordinates 
			mouseX = x/3;
			mouseY = (600-y)/3;
			if (state == GLUT_DOWN) {
				leftButtonPressed = true;
			} 
			else if (state == GLUT_UP) {
				leftButtonPressed = false;
				ballsMove = true;
				// make ball angle equal to stick opposite angle when mouse released
				double ball_angle = angle(mouseX-balls[0].x, mouseY-balls[0].y, balls[0].x+50, 0) + 180;
				if(ball_angle>360)
					ball_angle -= 360;
				ball_angle *= PI / 180;
				// update ball velocity
				balls[0].vx = stickTransform * cos(ball_angle);
				balls[0].vy = stickTransform * sin(ball_angle);
				// 
				stickTransform=0;
			}
		}
	}
}

void idle() {
	if(ballsMove){
		move_balls();
		// redraw 
		glutPostRedisplay();
	}
	else if(leftButtonPressed){
		if(stickTransform<20){
			stick_refresh(mouseX, mouseY);
			stickTransform+=stickMoveFactor;
			// redraw 
			glutPostRedisplay();
		}
	}
}


/////////////////////////////////////// main functions///////////////////////////////////////
void main(int argc, char* argv[]) {
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(600, 600);
	glutCreateWindow("PIONNTS");

	glClearColor(0, 0.722, 0.659, 0);
	gluOrtho2D(0, 200, 0, 200);
	
	// Set the callback function
	glutDisplayFunc(display);
	glutPassiveMotionFunc(motion);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
    glutIdleFunc(idle);

	// Start the main loop
	glutMainLoop();
}