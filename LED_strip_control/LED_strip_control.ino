#define red   9
#define green 10
#define blue  6
#define cycle_rate 1536
#define cap_pin_next 15
#define cap_pin_prev 4
#define next_touch_timeout 200
bool invert=false;
byte r_v=255,g_v=255,b_v=255,br=255,read_val,str_pos=255,mode_sel;
char mode='S';
unsigned long next_touch;

void change_mode(bool next) {
  next_touch = millis()+next_touch_timeout;
  if (next)
    mode_sel++;
  else
    mode_sel--;
  if (mode_sel==21)
    mode_sel=0;
  if (mode_sel==-1)
    mode_sel=20;
  
  switch (mode_sel) {
    case 0:
      mode='S';r_v=0xff;g_v=0;b_v=0x00;
      break;
    case 1:
      mode='S';r_v=0xff;g_v=0x2f;b_v=0x00;
      break;
    case 2:
      mode='S';r_v=0xff;g_v=0x4f;b_v=0;
      break;
    case 3:
      mode='S';r_v=0xff;g_v=0x7f;b_v=0;
      break;
    case 4:
      mode='S';r_v=0xff;g_v=0xff;b_v=0;
      break;
    case 5:
      mode='S';r_v=0;g_v=0xff;b_v=0;
      break;
    case 6:
      mode='S';r_v=0;g_v=0xff;b_v=0x7f;
      break;
    case 7:
      mode='S';r_v=0;g_v=0xff;b_v=0xff;
      break;
    case 8:
      mode='S';r_v=0;g_v=0x7f;b_v=0xff;
      break;
    case 9:
      mode='S';r_v=0;g_v=0;b_v=0xff;
      break;
    case 10:
      mode='S';r_v=0x3f;g_v=0;b_v=0xff;
      break;
    case 11:
      mode='S';r_v=0xff;g_v=0;b_v=0xff;
      break;
    case 12:
      mode='S';r_v=0xff;g_v=0;b_v=0x7f;
      break;
    case 13:
      mode='S';r_v=0xff;g_v=0;b_v=0x3f;
      break;
    case 14:
      mode='S';r_v=0xff;g_v=0xff;b_v=0xff;
      break;
    case 15:
      mode='S';r_v=0xff;g_v=0xaf;b_v=0x7f;
      break;
    case 16:
      mode='S';r_v=0xff;g_v=0x7f;b_v=0x3f;
      break;
    case 17:
      mode='S';r_v=0;g_v=0;b_v=0;
      break;
    case 18:
      mode='S';r_v=0xff;g_v=0x4f;b_v=13;
      break;
    case 19:
      mode='S';r_v=0xff;g_v=0x38;b_v=0x03;
      break;
    case 20:
      mode='H';br=0xff;
      break;
  }
  apply_values();
}

uint8_t readCapacitivePin(int pinToMeasure) {
  // Variables used to translate from Arduino to AVR pin naming
  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;
  // Here we translate the input pin number from
  //  Arduino pin number to the AVR PORT, PIN, DDR,
  //  and which bit of those registers we care about.
  byte bitmask;
  port = portOutputRegister(digitalPinToPort(pinToMeasure));
  ddr = portModeRegister(digitalPinToPort(pinToMeasure));
  bitmask = digitalPinToBitMask(pinToMeasure);
  pin = portInputRegister(digitalPinToPort(pinToMeasure));
  // Discharge the pin first by setting it low and output
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  delay(1);
  uint8_t SREG_old = SREG; //back up the AVR Status Register
  // Prevent the timer IRQ from disturbing our measurement
  noInterrupts();
  // Make the pin an input with the internal pull-up on
  *ddr &= ~(bitmask);
  *port |= bitmask;

  // Now see how long the pin to get pulled up. This manual unrolling of the loop
  // decreases the number of hardware cycles between each read of the pin,
  // thus increasing sensitivity.
  uint8_t cycles = 17;
  if (*pin & bitmask) { cycles =  0;}
  else if (*pin & bitmask) { cycles =  1;}
  else if (*pin & bitmask) { cycles =  2;}
  else if (*pin & bitmask) { cycles =  3;}
  else if (*pin & bitmask) { cycles =  4;}
  else if (*pin & bitmask) { cycles =  5;}
  else if (*pin & bitmask) { cycles =  6;}
  else if (*pin & bitmask) { cycles =  7;}
  else if (*pin & bitmask) { cycles =  8;}
  else if (*pin & bitmask) { cycles =  9;}
  else if (*pin & bitmask) { cycles = 10;}
  else if (*pin & bitmask) { cycles = 11;}
  else if (*pin & bitmask) { cycles = 12;}
  else if (*pin & bitmask) { cycles = 13;}
  else if (*pin & bitmask) { cycles = 14;}
  else if (*pin & bitmask) { cycles = 15;}
  else if (*pin & bitmask) { cycles = 16;}

  // End of timing-critical section; turn interrupts back on if they were on before, or leave them off if they were off before
  SREG = SREG_old;

  // Discharge the pin again by setting it low and output
  //  It's important to leave the pins low if you want to 
  //  be able to touch more than 1 sensor at a time - if
  //  the sensor is left pulled high, when you touch
  //  two sensors, your body will transfer the charge between
  //  sensors.
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  return cycles;
}

byte hex(byte in) {
  if ('0'<=in&&in<='9')
    return in-'0';
  if ('a'<=in&&in<='f')
    return in-'a'+10;
  if ('A'<=in&&in<='F')
    return in-'A'+10;
  return 255;
}

void apply_values() {
  if (invert) {
    analogWrite(red,255-r_v);
    analogWrite(green,255-g_v);
    analogWrite(blue,255-b_v);
  } else {
    analogWrite(red,r_v);
    analogWrite(green,g_v);
    analogWrite(blue,b_v);
  }
  digitalWrite(LED_BUILTIN,(r_v>212||g_v>212||b_v>212));
}

void setup() {
  pinMode(red,OUTPUT);
  pinMode(green,OUTPUT);
  pinMode(blue,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  Serial.begin(115200);
  apply_values();
}

void loop() {
  if (Serial.available()) {
    char in=Serial.read();
    if (in=='$') {
      mode='$';
      str_pos=255;
    } else if (in=='?')
      Serial.print(mode);
    else switch (mode) {
      case '$':
        mode=in;
      case 'S':
        if (in=='#') {
          str_pos=0;
          r_v=0;g_v=0;b_v=0;
        } else switch (str_pos) {
          byte val;
          case 0:
            val=hex(in);
            if (val==255)
              break;
            r_v+=val*16;
            str_pos++;
            break;
          case 1:
            val=hex(in);
            if (val==255)
              break;
            r_v+=val;
            str_pos++;
            break;
          case 2:
            val=hex(in);
            if (val==255)
              break;
            g_v+=val*16;
            str_pos++;
            break;
          case 3:
            val=hex(in);
            if (val==255)
              break;
            g_v+=val;
            str_pos++;
            break;
          case 4:
            val=hex(in);
            if (val==255)
              break;
            b_v+=val*16;
            str_pos++;
            break;
          case 5:
            val=hex(in);
            if (val==255)
              break;
            b_v+=val;
            str_pos++;
            apply_values();
            break;
        }
        break;
      case 'H':
        if (in=='#') {
          str_pos=0;
          read_val=0;
        } else switch (str_pos) {
          byte val;
          case 0:
            val=hex(in);
            if (val==255)
              break;
            read_val+=val*16;
            str_pos++;
            break;
          case 1:
            val=hex(in);
            if (val==255)
              break;
            read_val+=val;
            str_pos++;
            br=read_val;
            break;
        }
        break;
      default:
        mode='$';
    }
  }
  switch (mode) {
    case 'H':
      int hue=(millis()/8%cycle_rate);
      // 0,256,512,768,1024,1280,1536
      
      switch (hue/256) {
        case 0:
          r_v=255;
          g_v=hue;
          b_v=0;
          break;
        case 1:
          r_v=511-hue;
          g_v=255;
          b_v=0;
          break;
        case 2:
          r_v=0;
          g_v=255;
          b_v=hue-512;
          break;
        case 3:
          r_v=0;
          g_v=1023-hue;
          b_v=255;
          break;
        case 4:
          r_v=hue-1024;
          g_v=0;
          b_v=255;
          break;
        case 5:
          r_v=255;
          g_v=0;
          b_v=1535-hue;
          break;
      }
      r_v=((int)r_v*br)/255;
      g_v=((int)g_v*br)/255;
      b_v=((int)b_v*br)/255;
      apply_values();
  }
  if (next_touch<millis()) {
    if (readCapacitivePin(cap_pin_next)>1)
      change_mode(true);
    else if (readCapacitivePin(cap_pin_prev)>1)
      change_mode(false);
  }
}
