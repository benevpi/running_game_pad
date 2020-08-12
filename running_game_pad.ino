#include <BleKeyboard.h>
#define LILYGO_WATCH_2020_V1
#include <LilyGoWatch.h>

// Color definitions stolen from Adafruit
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

//keyboard settings
uint8_t go_key = KEY_UP_ARROW;
uint8_t left_key = KEY_LEFT_ARROW;
uint8_t right_key = KEY_RIGHT_ARROW;
char button_key = 'z';

//touch sensitivity
uint8_t thresh = 255;

int last_step_time = 0;
int step_time = 0;
int stop_step_speed = 700; //speed at which we'll put the brakes on

int pulse_width = 100; // we'll PWM the 'go' key 
int pulse_width_percent = 0;



boolean stepping = false;
boolean tapping_left = false;
boolean tapping_right = false;
boolean button_press = false;

boolean was_stepping = false;
boolean was_tapping_left = false;
boolean was_tapping_right = false;

boolean going = false;
boolean stopping = false;
boolean turning_left = false;
boolean turning_right = false;

//watch settings
int brightness = 30;
bool irq = false;
char buf[128];
char battery_chars[128];


BleKeyboard bleKeyboard;
TTGOClass *ttgo;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  bleKeyboard.begin();

  ttgo = TTGOClass::getWatch();
  ttgo->begin();

  ttgo->openBL();
  ttgo->bl->adjust(brightness);
  

  pinMode(BMA423_INT1, INPUT);
  
  attachInterrupt(BMA423_INT1, [] {
      irq = 1;
  }, RISING);

  pinMode(AXP202_INT, INPUT_PULLUP);
  attachInterrupt(AXP202_INT, [] {
      irq = true;
  }, FALLING);

  //!Clear IRQ unprocessed  first
  ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
  ttgo->power->clearIRQ();

  ttgo->bma->begin();
  ttgo->bma->attachInterrupt(); // not sure about this? looks like this does all the config as well.
   
  ttgo->bma->enableStepCountInterrupt();

  last_step_time = millis();
}

void loop() {
  Serial.println(millis());

  was_stepping = stepping;
  was_tapping_left = tapping_left;
  was_tapping_right = tapping_right;
  button_press = false;

  ttgo->tft->setTextColor(GREEN, BLACK);
  ttgo->tft->drawString(battery_chars, 22, 3, 4);
  snprintf(battery_chars, sizeof(battery_chars), "Battery: %u                ", ttgo->power->getBattPercentage());

  ttgo->tft->setTextColor(GREEN, BLACK);
  snprintf(buf, sizeof(buf), "Steps: %u", ttgo->bma->getCounter());
  ttgo->tft->drawString(buf, 22, 90, 4);

  //set the stepping going if there are steps
  if (irq) {
    irq = 0;

    bool  rlst;
    do {
        rlst =  ttgo->bma->readInterrupt();
    } while (!rlst);

    if (ttgo->bma->isStepCounter()) {
    stepping = true;
    ttgo->tft->setTextColor(GREEN, BLACK);
    ttgo->tft->drawString("stepping         ", 22, 60, 4);    
    last_step_time = millis();
    }

  ttgo->power->readIRQ();
  if (ttgo->power->isPEKShortPressIRQ()) {
    button_press = true;
  }
  ttgo->power->clearIRQ();
  }
  

  //stop the steps if there aren't
  if(millis()-last_step_time > stop_step_speed) { 
    stepping = false;
    ttgo->tft->setTextColor(GREEN, BLACK);
    snprintf(buf, sizeof(buf), "not stepping");
    ttgo->tft->drawString(buf, 22, 60, 4);  
  }

  int16_t x;
  int16_t y;
  tapping_left = false;
  tapping_right = false;
  if (ttgo->getTouch(x, y)) {
    if(x<100){
      tapping_left=true;
      ttgo->tft->setTextColor(GREEN, BLACK);
      snprintf(buf, sizeof(buf), "left     ");
      ttgo->tft->drawString(buf, 22, 120, 4);  
    }
    else if(x>140){
      tapping_right=true;
      ttgo->tft->setTextColor(GREEN, BLACK);
      snprintf(buf, sizeof(buf), "right   ");
      ttgo->tft->drawString(buf, 22, 120, 4);  
    }
    else {
      ttgo->tft->setTextColor(GREEN, BLACK);
      snprintf(buf, sizeof(buf), "none  ");
      ttgo->tft->drawString(buf, 22, 120, 4);  
    }
  }

  if(tapping_left == false and tapping_right==false) {
    ttgo->tft->setTextColor(GREEN, BLACK);
    snprintf(buf, sizeof(buf), "none  ");
    ttgo->tft->drawString(buf, 22, 120, 4); 
  }

  //hmm, this is meltingly fast -- let's only do this if there's a change otherwise we could be overloading a bluetooth buffer
  
  if(bleKeyboard.isConnected() and (tapping_right != was_tapping_right or tapping_left != was_tapping_left or stepping !=was_stepping or button_press)) {
    bleKeyboard.releaseAll();
    if(stepping) {bleKeyboard.press(go_key);}
    if(tapping_left) {bleKeyboard.press(left_key);}
    if(tapping_right) {bleKeyboard.press(right_key);}
    if(button_press) {bleKeyboard.press(button_key);}
  }

}
