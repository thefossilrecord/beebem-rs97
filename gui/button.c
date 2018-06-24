/* START OF button.c ---------------------------------------------------------------
 *
 *	A simple clickable button.
 *
 *	---
 *	THIS GUI IS TOTALLY *BROKEN*! PLEASE DO NOT USE IT!
 *	---
 */

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include <gui/log.h>

#include <gui/functions.h>

#include <gui/button.h>
#include <gui/button_private.h>

#include <gui/window.h>
#include <gui/window_private.h>

#include <gui/widget.h>
#include <gui/widget_private.h>

#include <gui/widget_shared.h>

#include <gui/graphics.h>

#include <gui/skin.h>

#include <SDL.h>



/* The default skin to use, if enabled:
 */
static EG_Skin_Button *default_skin_ptr = NULL;



/* Callbacks:
 */

static EG_BOOL Callback_Paint(EG_Widget *widget_ptr, SDL_Rect area)
{
	EG_Button *button_ptr;
	EG_Window *window_ptr;

	SDL_Rect loc;
	SDL_Color color;
	SDL_Surface *surface_ptr;

	EG_BOOL bold;

	int old_pattern = EG_Graphics_GetPattern();
	Uint8 old_forecolor = EG_Graphics_GetForeColor();
	Uint8 old_backcolor = EG_Graphics_GetBackColor();

	if ( EG_Shared_GetRenderingDetails(widget_ptr, area,
	 (void*) &button_ptr, &window_ptr, &surface_ptr, &color, &loc) != EG_TRUE )
		return(EG_TRUE);

#ifdef EG_DEBUG
	printf("SOMEONE CALLED PAINT FOR '%s' [repaint area (window)="
	 "{%d, %d, %d, %d}:widget area (SDL_Surface)={%d, %d, %d, %d}]\n"
	 , EG_Widget_GetName(widget_ptr), area.x, area.y, area.w, area.h
	 , loc.x, loc.y, loc.w, loc.h);
#endif


        /* Render the widget:
         */

	/* Dull down the button a bit when disabled.
	 */
	if (EG_Widget_IsEnabled(widget_ptr) != EG_TRUE){
		color.r = (int) ( color.r * 0.7);
		color.g = (int) ( color.g * 0.7);	
		color.b = (int) ( color.b * 0.7);
	}
	EG_Graphics_SetPattern(0);
	EG_Graphics_SetForeColor(SDL_MapRGB(surface_ptr->format, color.r, color.g, color.b));

		/* Draw the caption.
		 */
		if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE)
			bold = EG_TRUE;
		else
			bold = EG_FALSE;	



//	EG_Graphics_SetForeColor(56);

	if (button_ptr->skin_button_ptr != NULL) {
		bold=EG_FALSE;

		if (button_ptr->depressed == EG_TRUE)
			EG_Skin_Button_Paint(button_ptr->skin_button_ptr, surface_ptr, 2, loc.x, loc.y, loc.w, loc.h);
		else if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE)
			EG_Skin_Button_Paint(button_ptr->skin_button_ptr, surface_ptr, 1, loc.x, loc.y, loc.w, loc.h);
		else
			EG_Skin_Button_Paint(button_ptr->skin_button_ptr, surface_ptr, 0, loc.x, loc.y, loc.w, loc.h);

		if (button_ptr->depressed == EG_TRUE)
			EG_Draw_String(surface_ptr, NULL, bold, &loc, button_ptr->alignment
        	         , button_ptr->caption );
		else
			EG_Draw_String(surface_ptr, &color, bold, &loc, button_ptr->alignment
			 , button_ptr->caption );


	} else if (button_ptr->skin_button && default_skin_ptr != NULL && button_ptr->without_border != EG_TRUE) {
		bold=EG_FALSE;


		if (button_ptr->depressed == EG_TRUE) {
//			EG_Graphics_SetForeColor(0);
			EG_Skin_Button_Paint(default_skin_ptr, surface_ptr, 2, loc.x, loc.y, loc.w, loc.h);
		} else if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE) {
//			EG_Graphics_SetForeColor(100);
			EG_Skin_Button_Paint(default_skin_ptr, surface_ptr, 1, loc.x, loc.y, loc.w, loc.h);
		} else {
//			EG_Graphics_SetForeColor(100);
			EG_Skin_Button_Paint(default_skin_ptr, surface_ptr, 0, loc.x, loc.y, loc.w, loc.h);
		}

		if (button_ptr->depressed == EG_TRUE)
			EG_Draw_String(surface_ptr, NULL, bold, &loc, button_ptr->alignment
        	         , button_ptr->caption );
		else
			EG_Draw_String(surface_ptr, &color, bold, &loc, button_ptr->alignment
			 , button_ptr->caption );

	} else {
		/* Fill button area with button color.
		 */
//		EG_Draw_Box(surface_ptr, &loc, &color);
		
//		EG_Graphics_SetPattern(0);
//		EG_Graphics_SetBackColor(SDL_MapRGB(surface_ptr->format, 255, 0, 0));
		EG_Graphics_DrawFilledRect(surface_ptr, loc.x, loc.y, loc.w, loc.h);
	
		/* Draw buttons border, if depressed (pressed), sunk, otherwise high.
		 */
		if (button_ptr->without_border != EG_TRUE) {
			if (button_ptr->depressed == EG_TRUE){
				//EG_Draw_Border(surface_ptr,&loc, &color, EG_Draw_Border_BoxLow);
				EG_Graphics_DrawBorder(surface_ptr, loc.x, loc.y, loc.w, loc.h, EG_GRAPHICS_BORDER_SUNK);
				loc.x++; loc.y++;
			}else{
				//EG_Draw_Border(surface_ptr,&loc,&color, EG_Draw_Border_BoxHigh);
				EG_Graphics_DrawBorder(surface_ptr, loc.x, loc.y, loc.w, loc.h, EG_GRAPHICS_BORDER_RAISED);
			}
		}

		EG_Draw_String(surface_ptr, &color, bold, &loc, button_ptr->alignment
		 , button_ptr->caption );

		EG_Graphics_SetPattern(old_pattern);
		EG_Graphics_SetForeColor(old_forecolor);
		EG_Graphics_SetBackColor(old_backcolor);

	}

	/* Paint succeeded so return true.
	 */
	return(EG_TRUE);
}

static EG_BOOL Callback_SDL_Event(EG_Widget *widget_ptr, SDL_Event *event_ptr)
{
	EG_Button *button_ptr;
	EG_Window *window_ptr;
	EG_BOOL mouse_over_widget;

	EG_BOOL return_value = EG_FALSE;

	EG_BOOL call_user_callback = EG_FALSE;

	/* Populates variables needed to process the event.  If the event
	 * shouldn't have been passed to us, then logs the fact (for bug
	 * reporting) and returns false.  Quit the callback if this returns
	 * false.
	 */
	if (EG_Shared_GetEventDetails(widget_ptr, event_ptr
	 , (void*) &button_ptr, &window_ptr, &mouse_over_widget) != EG_TRUE)
		return(return_value);

	/* Process SDL Event:
	 */


	/* Is a mouse button event?
	 */
	if (event_ptr->type == SDL_MOUSEBUTTONDOWN || event_ptr->type
	 == SDL_MOUSEBUTTONUP){

		/* If depressed = false and left mouse button is pressed and
		 * mouse pointer is within area of widget.
		 */
		if (button_ptr->depressed == EG_FALSE && event_ptr->button.state
		 == SDL_PRESSED && event_ptr->button.button == SDL_BUTTON_LEFT
		 && mouse_over_widget == EG_TRUE){

			/* Set depressed = true, repaint widget.
			 */
			button_ptr->depressed=EG_TRUE;
			EG_Window_LockSmartMovement(window_ptr);
			(void) EG_Widget_RepaintLot(widget_ptr);
		}

		/* If depressed = true and left mouse button is released.
		 */
		if (button_ptr->depressed == EG_TRUE && event_ptr->button.state
		 == SDL_RELEASED && event_ptr->button.button ==SDL_BUTTON_LEFT){
	
			/* If released with mouse pointer within area of widget
			 * and --------the minimum click-time has been met. 
			 */
			if (mouse_over_widget == EG_TRUE && 
			 EG_Widget_GetVisibleToggle(widget_ptr) == EG_TRUE
			 && EG_Widget_GetStoppedToggle(widget_ptr) == EG_FALSE){
			 	/* Call users 'OnClick' event.
				 */
				if (EG_Window_SetFocusToThisWidget(widget_ptr)
				 != EG_TRUE)
				 	EG_Log(EG_LOG_WARNING, dL"Could not"
					 " move focus to pressed EG_Button."
					 , dR);

				//EG_Widget_CallUserOnClick(widget_ptr);
				call_user_callback = EG_TRUE;
			}
	
			/* Regardless of mouse pointers location, set
			 * depressed = false; repaint widget.
			 */
			button_ptr->depressed=EG_FALSE;
			EG_Window_UnlockSmartMovement(window_ptr);
			(void) EG_Widget_RepaintLot(widget_ptr);
		}
	}


	/* If widget has focus and 'Enter' key is pressed.
	 */

	if ( /* (event_ptr->type == SDL_KEYDOWN && event_ptr->key.keysym.sym == SDLK_RETURN) 
	 || */ ( event_ptr->type == SDL_USEREVENT && event_ptr->user.code == EG_USER_SDL_TRIGGER_SELECT_DOWN )
	) {

		/* If this widget currently has focus on the window.
		 */
		if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE){
			button_ptr->depressed=EG_TRUE;
			EG_Window_LockSmartMovement(window_ptr);
			(void) EG_Widget_RepaintLot(widget_ptr);
		}
	}

	if ( /* (event_ptr->type == SDL_KEYUP && event_ptr->key.keysym.sym == SDLK_RETURN)
	 || */ ( event_ptr->type == SDL_USEREVENT && event_ptr->user.code == EG_USER_SDL_TRIGGER_SELECT_UP )
	) {
		/* If this widget currently has focus on the window.
		 */
		if (EG_Window_ThisWidgetHasFocus(widget_ptr) == EG_TRUE){
			/* Eat this event, we don't want it passed to 
			 * other widgets.
			 */
			return_value = EG_TRUE;

			//EG_Widget_CallUserOnClick(widget_ptr);
			call_user_callback = EG_TRUE;

			button_ptr->depressed=EG_FALSE;
			EG_Window_UnlockSmartMovement(window_ptr);
			(void) EG_Widget_RepaintLot(widget_ptr);

		}
	}

	/* If mouse over widget, and users event callback is set, call users
	 * callback.
	 */
	if (mouse_over_widget == EG_TRUE)
		EG_Widget_CallUserOnEvent(widget_ptr, event_ptr);

	if (call_user_callback == EG_TRUE)
		EG_Widget_CallUserOnClick(widget_ptr);

	return(return_value);
}

/* Private function:
 */

static void InitializePayload(EG_Button *button_ptr)
{
	button_ptr->depressed = EG_FALSE;
}

static void InitializeWidget(EG_Widget *widget_ptr, SDL_Color color
 , SDL_Rect dimension, int alignment, const char *caption_ptr)
{
	/* Initialize callbacks:
	 */
	(void) EG_Widget_SetCallback_Destroy(widget_ptr, EG_Callback_Generic_Destroy);

	(void) EG_Widget_SetCallback_Paint(widget_ptr, Callback_Paint);
	(void) EG_Widget_SetCallback_SDL_Event(widget_ptr, Callback_SDL_Event);

	(void) EG_Widget_SetCallback_Visible(widget_ptr
	 , EG_Callback_Generic_Visible);

	(void) EG_Widget_SetCallback_Stopped(widget_ptr
	 , EG_Callback_Generic_Stopped);

	(void) EG_Widget_SetCallback_Enabled(widget_ptr
	 , EG_Callback_Generic_Enabled);

	(void) EG_Widget_SetCallback_GotFocus(widget_ptr
	 , EG_Callback_Generic_GotFocus);

	(void) EG_Widget_SetCallback_LostFocus(widget_ptr
	 , EG_Callback_Generic_LostFocus);

	(void) EG_Widget_SetCallback_Attach(widget_ptr
	 , EG_Callback_Generic_Attach);

	/* Initialize state:
	 */
	(void) EG_Widget_SetDimension(widget_ptr, dimension);
	(void) EG_Widget_SetBackgroundColor(widget_ptr, color);

	(void) EG_Button_SetCaption(widget_ptr, caption_ptr);
	(void) EG_Button_SetAlignment(widget_ptr, alignment);

	(void) EG_Button_DrawWithBorder(widget_ptr);

	(void) EG_Button_SetSkin(widget_ptr, NULL);
	(void) EG_Button_UseDefaultSkin(widget_ptr, EG_TRUE);
}


/* Public functions:
 */

EG_Widget* EG_Button_Create(const char *name_ptr, SDL_Color color, int alignment
 , const char *caption_ptr, SDL_Rect dimension)
{
	EG_Button *button_ptr;
	EG_Widget *widget_ptr;
	void *ptr;

	SHARED__ALLOC_PAYLOAD_STRUCT(ptr, EG_Button
	 , "Unable to malloc EG_Button struct");
	button_ptr = (EG_Button*) ptr;        

        SHARED__CREATE_NEW_EG_WIDGET(widget_ptr, name_ptr, EG_Widget_Type_Button
         , button_ptr);
        
        SHARED__ATTACH_PAYLOAD_TO_WIDGET(widget_ptr, button_ptr);

	InitializePayload(button_ptr);
	InitializeWidget(widget_ptr, color, dimension, alignment, caption_ptr);

	return(widget_ptr);
}

EG_BOOL EG_Button_SetSkin(EG_Widget *widget_ptr, EG_Skin_Button *skin_button_ptr)
{
	EG_Button *button_ptr;
	EG_BUTTON_GET_STRUCT_PTR(widget_ptr, button_ptr, EG_FALSE);

	button_ptr->skin_button_ptr = skin_button_ptr;

	return EG_TRUE;
}

EG_BOOL EG_Button_UseDefaultSkin(EG_Widget *widget_ptr, EG_BOOL v)
{
	EG_Button *button_ptr;
	EG_BUTTON_GET_STRUCT_PTR(widget_ptr, button_ptr, EG_FALSE);

        button_ptr->skin_button = v;

        return EG_TRUE;
}

void EG_Button_SetDefaultSkin(EG_Skin_Button *button_ptr)
{
        default_skin_ptr = button_ptr;
}

void EG_Button_ClearDefaultSkin()
{
        return EG_Button_SetDefaultSkin(NULL);
}



EG_BOOL EG_Button_DrawWithBorder(EG_Widget *widget_ptr)
{
	EG_Button *button_ptr;
	EG_BUTTON_GET_STRUCT_PTR(widget_ptr, button_ptr, EG_FALSE);

	button_ptr->without_border = EG_FALSE;

	return EG_TRUE;
}

EG_BOOL EG_Button_DrawWithoutBorder(EG_Widget *widget_ptr)
{
	EG_Button *button_ptr;
	EG_BUTTON_GET_STRUCT_PTR(widget_ptr, button_ptr, EG_FALSE);

	button_ptr->without_border = EG_TRUE;

	return EG_TRUE;
}

EG_BOOL EG_Button_SetCaption(EG_Widget *widget_ptr, const char *caption_ptr)
{
	int i;
        EG_Button *button_ptr;

        EG_BUTTON_GET_STRUCT_PTR(widget_ptr, button_ptr, EG_FALSE);

        /* Clear caption (not really needed but nice).
         */
        for(i=0; i<MAX_BUTTON_CAPTIONSIZE+1; i++)
                button_ptr->caption[i]='\0';

        /* Set the new caption. If the supplied caption is too long, then
         * truncate it.
         *
         * (And yes I know that I could use strncpy on it's own here, but I
         * don't want to do that.  I think I'd find it too easy to forget that
         * '\0' is not included at the end, when using strncpy).
         */
        if (caption_ptr != NULL){
                if (strlen(caption_ptr) <= MAX_BUTTON_CAPTIONSIZE)
                        strcpy(button_ptr->caption, caption_ptr);
                else
                        strncpy(button_ptr->caption, caption_ptr
                         , MAX_BUTTON_CAPTIONSIZE);
        }

	/* Repaint the widget with new caption.
	 */
	EG_Widget_RepaintLot(widget_ptr);

        return(EG_TRUE);
}

const char* EG_Button_GetCaption(EG_Widget *widget_ptr)
{
        EG_Button *button_ptr;

        EG_BUTTON_GET_STRUCT_PTR(widget_ptr, button_ptr, NULL);
        return(button_ptr->caption);
}

EG_BOOL EG_Button_SetAlignment(EG_Widget *widget_ptr, int alignment)
{
        EG_Button *button_ptr;

        EG_BUTTON_GET_STRUCT_PTR(widget_ptr, button_ptr, EG_FALSE);
        button_ptr->alignment = alignment;
        return(EG_TRUE);
}

int EG_Button_GetAlignment(EG_Widget *widget_ptr)
{
        EG_Button *button_ptr;

        EG_BUTTON_GET_STRUCT_PTR(widget_ptr, button_ptr, EG_BUTTON_ALIGN_CENTER);
        return(button_ptr->alignment);
}


