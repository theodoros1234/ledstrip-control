#define red   11
#define green 10
#define blue  9
#define cycle_rate 1536
bool invert=false;
byte r_v=255,g_v=255,b_v=255,br=255,read_val,str_pos=255;
char mode='S';

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
}
