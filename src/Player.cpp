#include "Player.h"

#include <SFML/OpenGL.hpp>
#include "Settings.h"

Player::Player(Game *game, int bikeIDArg) {
	this->game = game;
	sf::ContextSettings cs;
	cs.depthBits = 24;
	window = new sf::Window(sf::VideoMode(650, 500), "Lightcycles", sf::Style::Default, cs);

	if (!window) exit(EXIT_FAILURE);
	window->setFramerateLimit(60);
	window->setActive(true);
	glEnable(GL_DEPTH_TEST);

	bikeID = bikeIDArg;
	viewedBikeID = bikeID;
	controlKeyLeft = sf::Keyboard::Unknown;
	controlKeyRight = sf::Keyboard::Unknown;
	yRot = 0;
}

Player::~Player() {
	window->close();
	delete window;
}

void Player::setControls(sf::Keyboard::Key left, sf::Keyboard::Key right) {
	controlKeyLeft = left;
	controlKeyRight = right;
}

void Player::onNewGame() {
	viewedBikeID = bikeID;
}

void Player::turnBike(bool right) {
	Bike *bike = game->getBike(bikeID);
	if (!bike->isDying()) bike->turn(right);
	else viewedBikeID = game->nextLivingBike(viewedBikeID, right);
}

void Player::drawBikeAndWalls(Bike *bike) {
	glColor3fv(bike->color);
	// TODO fancy bike
	if (!bike->isDying()) {
		glPushMatrix();
		glTranslatef(bike->pos.x, 0, bike->pos.z);

		glBegin(GL_QUADS);

		glVertex3f( 0.5, 1,  0.5);
		glVertex3f(-0.5, 1,  0.5);
		glVertex3f(-0.5, 1, -0.5);
		glVertex3f( 0.5, 1, -0.5);

		glVertex3f( 0.5, 0.001,  0.5);
		glVertex3f(-0.5, 0.001,  0.5);
		glVertex3f(-0.5, 0.001, -0.5);
		glVertex3f( 0.5, 0.001, -0.5);

		glEnd();

		glPopMatrix();
	}
	// walls
	if (!bike->isDead()) {
		glBegin(GL_QUADS);

		Point wa = bike->pos, wb;
		for (int i = bike->walls.size() - 1; i >= 0; i--) {
			wb = bike->walls.at(i);
			float ww = ((wa.x < wb.x) ? wa.x : wb.x) - wallRadius; // west
			float we = ((wa.x > wb.x) ? wa.x : wb.x) + wallRadius; // east
			float wn = ((wa.z < wb.z) ? wa.z : wb.z) - wallRadius; // north
			float ws = ((wa.z > wb.z) ? wa.z : wb.z) + wallRadius; // south

			float bh = bike->wallHeight;

			// western wall
			glVertex3f(ww,  0, wn);
			glVertex3f(ww, bh, wn);
			glVertex3f(ww, bh, ws);
			glVertex3f(ww,  0, ws);

			// eastern wall
			glVertex3f(we,  0, wn);
			glVertex3f(we, bh, wn);
			glVertex3f(we, bh, ws);
			glVertex3f(we,  0, ws);

			// northern wall
			glVertex3f(ww,  0, wn);
			glVertex3f(ww, bh, wn);
			glVertex3f(we, bh, wn);
			glVertex3f(we,  0, wn);

			// southern wall
			glVertex3f(ww,  0, ws);
			glVertex3f(ww, bh, ws);
			glVertex3f(we, bh, ws);
			glVertex3f(we,  0, ws);

			// top of the wall
			glVertex3f(ww, bh, wn);
			glVertex3f(ww, bh, ws);
			glVertex3f(we, bh, ws);
			glVertex3f(we, bh, wn);

			wa = wb;
		}

		glEnd();
	}
}

void Player::drawFloorAndBorders() {
	glPushMatrix();
	glBegin(GL_QUADS);

	// floor
	glColor3f(0.2, 0.2, 0.2);

	glVertex3f(       0, 0, 0);
	glVertex3f(       0, 0, mapSizeZ);
	glVertex3f(mapSizeX, 0, mapSizeZ);
	glVertex3f(mapSizeX, 0, 0);

	// borders
	glColor3f(0.3, 0.3, 0.3);

	glVertex3f(       0, 0, 0);
	glVertex3f(       0, 1, 0);
	glVertex3f(mapSizeX, 1, 0);
	glVertex3f(mapSizeX, 0, 0);

	glVertex3f(       0, 0, mapSizeZ);
	glVertex3f(       0, 1, mapSizeZ);
	glVertex3f(mapSizeX, 1, mapSizeZ);
	glVertex3f(mapSizeX, 0, mapSizeZ);

	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glVertex3f(0, 1, mapSizeZ);
	glVertex3f(0, 0, mapSizeZ);

	glVertex3f(mapSizeX, 0, 0);
	glVertex3f(mapSizeX, 1, 0);
	glVertex3f(mapSizeX, 1, mapSizeZ);
	glVertex3f(mapSizeX, 0, mapSizeZ);

	glEnd();
	glPopMatrix();
}

void Player::drawScene() {
	for (int i = 0; i < game->bikesInGame(); i++)
		drawBikeAndWalls(game->getBike(i));
	drawFloorAndBorders();
}

void Player::drawWindow() {
	window->setActive(true);

	int windowWidth = window->getSize().x;
	int windowHeight = window->getSize().y;
	glViewport(0, 0, windowWidth, windowHeight);

	glClearColor(0.5, 0.5, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	///// draw first person view /////

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.7, 100);

	// smooth curves
	// TODO do not turn smoothly when game restarts
	const float targetRot = 90*game->getBike(viewedBikeID)->direction;
	float deltaRot = targetRot - yRot;
	while (deltaRot >  180) deltaRot -= 360;
	while (deltaRot < -180) deltaRot += 360;
	yRot += deltaRot/3.0;

	glTranslatef(0, -viewHeight, -7); // a bit from the top
	glRotatef(yRot, 0, 1, 0);
	glTranslatef(-(game->getBike(viewedBikeID)->pos.x),
			-0,
			-(game->getBike(viewedBikeID)->pos.z)); // from the position of the viewed bike

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	drawScene();

	///// draw map /////

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,            // left
			windowWidth,  // right
			0,            // bottom
			windowHeight, // top
			0.1,          // near
			100);         // far
	glRotatef(90, 1, 0, 0);
	glTranslatef(-0, -2, -windowHeight);
	glScalef(windowWidth / 4 / mapSizeX,
			1,
			windowWidth / 4 / mapSizeX);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	drawScene();

	///// finish rendering /////

	window->display();
}

