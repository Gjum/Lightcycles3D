/* Controller.h
 * Base class to define the interface for bike-controlling classes.
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller {
	public:
		Controller();
		virtual ~Controller();

		virtual void onNewGame();
		virtual void updateControls();
		virtual void updateView(float frameSec);
};

#endif // CONTROLLER_H

