# running_game_pad
Game input from the ttgo watch 2020

This turns the pedometer into a games controller. Run to move forward, hit the left-hand side of the screen to turn left, 
hit the right hand side to turn right and click the power button to perform an action.

You can tweak what these do with the keyboard settings:

//keyboard settings
uint8_t go_key = KEY_UP_ARROW;
uint8_t left_key = KEY_LEFT_ARROW;
uint8_t right_key = KEY_RIGHT_ARROW;
char button_key = 'z';

The watch acts as a bluetooth keyboard that you'll need to pair with your computer or phone.
