#include <string.h>
#include <stdint.h>
#include <SDL.h>
#include "button.h"
#include "keyboard.h"


#define BUTTON_COUNT			10


#define BUTTON_KEY_LEFT			0
#define BUTTON_KEY_RIGHT		1
#define BUTTON_KEY_UP			2
#define BUTTON_KEY_DOWN			3
#define BUTTON_KEY_CLICK		4

#define BUTTON_BUT_LEFT			5
#define BUTTON_BUT_RIGHT		6
#define BUTTON_BUT_UP			7
#define BUTTON_BUT_DOWN			8
#define BUTTON_BUT_CLICK		9


typedef struct {
	int button, state, delay;
} Button_Button;


static Button_Button button[BUTTON_COUNT];
static int auto_repeat_delay;

void Button_Init(void)
{
	auto_repeat_delay = 50;
	Button_Reset();
}

void Button_Quit(void)
{
	// Nothing.
}

void Button_Reset(void)
{
	memset(&button, 0, sizeof(Button_Button) * BUTTON_COUNT);

	button[BUTTON_KEY_LEFT].button = 	SDL_USEREVENT_BUTTON_LEFT;
	button[BUTTON_KEY_RIGHT].button = 	SDL_USEREVENT_BUTTON_RIGHT;
	button[BUTTON_KEY_UP].button = 		SDL_USEREVENT_BUTTON_UP;
	button[BUTTON_KEY_DOWN].button = 	SDL_USEREVENT_BUTTON_DOWN;
	button[BUTTON_KEY_CLICK].button = 	SDL_USEREVENT_BUTTON_CLICK;
	
	button[BUTTON_BUT_LEFT].button = 	SDL_USEREVENT_BUTTON_LEFT;
	button[BUTTON_BUT_RIGHT].button = 	SDL_USEREVENT_BUTTON_RIGHT;
	button[BUTTON_BUT_UP].button = 		SDL_USEREVENT_BUTTON_UP;
	button[BUTTON_BUT_DOWN].button = 	SDL_USEREVENT_BUTTON_DOWN;
	button[BUTTON_BUT_CLICK].button = 	SDL_USEREVENT_BUTTON_CLICK;
}


static void RaiseButtonEvent(int button)
{
	SDL_Event event;

//	printf("RAISING EVENT %d\n", button);

        event.type = SDL_USEREVENT;
        event.user.code = SDL_USEREVENT_BUTTON;

        event.user.data1 = (void*) ((intptr_t) button);

	SDL_PushEvent(&event);
}

static void ButtonDown(int index)
{
//	printf("button down index=%d button=%d\n", index, button[index].button);
	button[index].state = 1;
        button[index].delay = auto_repeat_delay;
        RaiseButtonEvent(button[index].button);
}

static void ButtonDown_NoRepeat(int index)
{
//	printf("button down norepeat index=%d button=%d\n", index, button[index].button);
	button[index].state = 1;
	button[index].delay = 0;
	RaiseButtonEvent(button[index].button);
}

static void ButtonUp(int index)
{
//	printf("button up index=%d button=%d\n", index, button[index].button);
	button[index].state = 0;
}


void Button_SetAutoRepeatDelay(int new_delay)
{
	auto_repeat_delay = new_delay;
}

int Button_GetAutoRepeatDelay(void)
{
	return auto_repeat_delay;
}

void Button_RaiseAutoRepeatEvents(void)
{
	int i;

	for (i=0; i<BUTTON_COUNT; i++)
		if (button[i].state == 1 && button[i].delay > 0)
			if (--button[i].delay == 0) {
				button[i].delay = auto_repeat_delay;

				RaiseButtonEvent(button[i].button);
			}
}

void Button_TranslateEvent(SDL_Event *event_p, int is_flipped)
{
	static int gp2x_button_down = 0;
	static int gp2x_button_up = 0;

	static int previous_is_flipped = 0;

	if (previous_is_flipped != is_flipped) {
		gp2x_button_down = gp2x_button_up = 0;
		previous_is_flipped = is_flipped;
	}
	

	/* If we have an SDL event then process it:
	 */
	if (event_p == NULL) return;

	/* Term:
	 */
	if (event_p->type == SDL_QUIT) {
//		printf("is quit\n");
		RaiseButtonEvent(SDL_USEREVENT_BUTTON_QUIT);
		return;

	/* Keyboard:
	 */
	} else if (event_p->type == SDL_KEYDOWN) {
		// QUIT:
		if ( event_p->key.keysym.sym == SDLK_ESCAPE) {
//			printf("is quit\n");
			RaiseButtonEvent(SDL_USEREVENT_BUTTON_QUIT);
		} else

		// LEFT:
		if ( (! is_flipped && event_p->key.keysym.sym == SDLK_LEFT)
			|| (is_flipped && event_p->key.keysym.sym == SDLK_RIGHT)
		   ) {
//			printf("calling left %d\n", BUTTON_KEY_LEFT);
			ButtonDown(BUTTON_KEY_LEFT);
		} else
	
		// RIGHT:
		if ( (! is_flipped && event_p->key.keysym.sym == SDLK_RIGHT)
			|| (is_flipped && event_p->key.keysym.sym == SDLK_LEFT)
		   ) {
//			printf("calling right %d\n", BUTTON_KEY_RIGHT);
			ButtonDown(BUTTON_KEY_RIGHT);
		} else
	
		// UP:
		if ( (! is_flipped && event_p->key.keysym.sym == SDLK_UP)
			|| (is_flipped && event_p->key.keysym.sym == SDLK_DOWN)
		   ) {
			ButtonDown(BUTTON_KEY_UP);
		} else

		// DOWN:
		if ( (! is_flipped && event_p->key.keysym.sym == SDLK_DOWN)
			|| (is_flipped && event_p->key.keysym.sym == SDLK_UP)
		   ) {
			ButtonDown(BUTTON_KEY_DOWN);
		} else

		// CLICK:
		if ( event_p->key.keysym.sym == SDLK_LALT) {
			ButtonDown_NoRepeat(BUTTON_KEY_CLICK);
			RaiseButtonEvent(SDL_USEREVENT_BUTTON_CLICK_DOWN);
		}
	
	} else if (event_p->type == SDL_KEYUP) {

		// LEFT:
		if ( (! is_flipped && event_p->key.keysym.sym == SDLK_LEFT)
			|| (is_flipped && event_p->key.keysym.sym == SDLK_RIGHT)
		   ) {
			ButtonUp(BUTTON_KEY_LEFT);
		} else
	
		// RIGHT:
		if ( (! is_flipped && event_p->key.keysym.sym == SDLK_RIGHT)
			|| (is_flipped && event_p->key.keysym.sym == SDLK_LEFT)
		   ) {
			ButtonUp(BUTTON_KEY_RIGHT);
		} else
	
		// UP:
		if ( (! is_flipped && event_p->key.keysym.sym == SDLK_UP)
			|| (is_flipped && event_p->key.keysym.sym == SDLK_DOWN)
		   ) {
			ButtonUp(BUTTON_KEY_UP);
		} else
	
		// DOWN:
		if ( (! is_flipped && event_p->key.keysym.sym == SDLK_DOWN)
			|| (is_flipped && event_p->key.keysym.sym == SDLK_UP)
		   ) {
			ButtonUp(BUTTON_KEY_DOWN);
		} else

		// CLICK:
		if ( event_p->key.keysym.sym == SDLK_LALT) {
//			printf("***** CLICK\n");
			ButtonUp(BUTTON_KEY_CLICK);
			RaiseButtonEvent(SDL_USEREVENT_BUTTON_CLICK_UP);
		}

	// GP2X Buttons:
	} else if (event_p->type == SDL_JOYBUTTONDOWN) {

		// LEFT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_LEFT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_RIGHT) ) {
			ButtonDown(BUTTON_BUT_LEFT);
		} else

		// RIGHT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_RIGHT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_LEFT) ) {
			ButtonDown(BUTTON_BUT_RIGHT);
		} else

		// UP:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_UP) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_DOWN) ) {
			if (gp2x_button_down == 0) {ButtonDown(BUTTON_BUT_UP); gp2x_button_up=1;}
		} else

		// UPLEFT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_UPLEFT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_DOWNRIGHT) ) {
			if (gp2x_button_down == 0) {ButtonDown(BUTTON_BUT_UP); gp2x_button_up=1;}
		} else

		// UPRIGHT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_UPRIGHT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_DOWNLEFT) ) {
			if (gp2x_button_down == 0) {ButtonDown(BUTTON_BUT_UP); gp2x_button_up=1;}
		} else

		// DOWN:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_DOWN) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_UP) ) {
			if (gp2x_button_down == 0) {ButtonDown(BUTTON_BUT_DOWN); gp2x_button_down=1;}
		} else

		// DOWNLEFT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_DOWNLEFT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_UPRIGHT) ) {
			if (gp2x_button_down == 0) {ButtonDown(BUTTON_BUT_DOWN); gp2x_button_down=1;}
		} else

		// DOWNRIGHT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_DOWNRIGHT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_UPLEFT) ) {
			if (gp2x_button_down == 0) {ButtonDown(BUTTON_BUT_DOWN); gp2x_button_down=1;}
		} else

		// CLICK:
		if (event_p->jbutton.button == GP2X_BUTTON_CLICK) {
                        ButtonDown_NoRepeat(BUTTON_KEY_CLICK);
                        RaiseButtonEvent(SDL_USEREVENT_BUTTON_CLICK_DOWN);
		} else

		// In normal mode a button presses a key:
		if ( (! is_flipped) && (event_p->jbutton.button
		 == GP2X_BUTTON_A || event_p->jbutton.button
		 == GP2X_BUTTON_B || event_p->jbutton.button
		 == GP2X_BUTTON_X || event_p->jbutton.button
		 == GP2X_BUTTON_Y) ) {
                        ButtonDown_NoRepeat(BUTTON_KEY_CLICK);
                        RaiseButtonEvent(SDL_USEREVENT_BUTTON_CLICK_DOWN);
		} else
		// In flipped mode the volume buttons press a key:
		if ( is_flipped && (event_p->jbutton.button
		 == GP2X_BUTTON_VOLUP || event_p->jbutton.button
		 == GP2X_BUTTON_VOLDOWN) ) {
			ButtonDown(BUTTON_BUT_CLICK);
                        RaiseButtonEvent(SDL_USEREVENT_BUTTON_CLICK_DOWN);
		} else

		// In flipped mode the buttons are a d-pad:
		if ( is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_A) {
			ButtonDown(BUTTON_BUT_RIGHT);
		} else if ( is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_B) {
			ButtonDown(BUTTON_BUT_LEFT);
		} else if ( is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_Y) {
			ButtonDown(BUTTON_BUT_DOWN);
		} else if ( is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_X) {
			ButtonDown(BUTTON_BUT_UP);
		}



	} else if (event_p->type == SDL_JOYBUTTONUP) {

		// LEFT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_LEFT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_RIGHT) ) {
			ButtonUp(BUTTON_BUT_LEFT);
		} else

		// RIGHT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_RIGHT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_LEFT) ) {
			ButtonUp(BUTTON_BUT_RIGHT);
		} else

		// UPLEFT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_UPLEFT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_DOWNRIGHT) ) {
			ButtonUp(BUTTON_BUT_UP); gp2x_button_up = 0;
		} else

		// UPRIGHT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_UPRIGHT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_DOWNLEFT) ) {
			ButtonUp(BUTTON_BUT_UP); gp2x_button_up = 0;
		} else

		// UP:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_UP) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_DOWN) ) {
			ButtonUp(BUTTON_BUT_UP); gp2x_button_up = 0;
		} else

		// DOWN:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_DOWN) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_UP) ) {
			ButtonUp(BUTTON_BUT_DOWN); gp2x_button_down = 0;
		} else

		// DOWNLEFT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_DOWNLEFT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_UPRIGHT) ) {
			ButtonUp(BUTTON_BUT_DOWN); gp2x_button_down = 0;
		} else

		// DOWNRIGHT:
		if ( (! is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_DOWNRIGHT) || (is_flipped 
		 && event_p->jbutton.button == GP2X_BUTTON_UPLEFT) ) {
			ButtonUp(BUTTON_BUT_DOWN); gp2x_button_down = 0;
		} else

		// CLICK:
		if (event_p->jbutton.button == GP2X_BUTTON_CLICK) {
			ButtonUp(BUTTON_BUT_CLICK);
                        RaiseButtonEvent(SDL_USEREVENT_BUTTON_CLICK_UP);
		} else

		// QUIT:
		if (event_p->jbutton.button == GP2X_BUTTON_SELECT) {
			RaiseButtonEvent(SDL_USEREVENT_BUTTON_QUIT);
		} else

		// In normal mode a button presses a key:
		if ( (! is_flipped) && (event_p->jbutton.button
		 == GP2X_BUTTON_A || event_p->jbutton.button
		 == GP2X_BUTTON_B || event_p->jbutton.button
		 == GP2X_BUTTON_X || event_p->jbutton.button
		 == GP2X_BUTTON_Y) ) {
			ButtonUp(BUTTON_BUT_CLICK);
			RaiseButtonEvent(SDL_USEREVENT_BUTTON_CLICK_UP);
		} else

		// In flipped mode the volume buttons press a key:
		if ( is_flipped && (event_p->jbutton.button
		 == GP2X_BUTTON_VOLUP || event_p->jbutton.button
		 == GP2X_BUTTON_VOLDOWN) ) {
			ButtonUp(BUTTON_BUT_CLICK);
			RaiseButtonEvent(SDL_USEREVENT_BUTTON_CLICK_UP);
		} else

		// In flipped mode the buttons are a d-pad:
		if ( is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_A) {
			ButtonUp(BUTTON_BUT_RIGHT);
		} else if ( is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_B) {
			ButtonUp(BUTTON_BUT_LEFT);
		} else if ( is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_Y) {
			ButtonUp(BUTTON_BUT_DOWN);
		} else if ( is_flipped && event_p->jbutton.button
		 == GP2X_BUTTON_X) {
			ButtonUp(BUTTON_BUT_UP);
		}

	}
}

