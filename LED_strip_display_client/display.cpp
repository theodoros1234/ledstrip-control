#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <iostream>
#include <ctime>
using namespace std;
unsigned long long int red,green,blue;
const char hx[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

void ImageFromDisplay(std::vector<uint8_t>& Pixels, int& Width, int& Height, int& BitsPerPixel) {

}

int main() {
    int Width;
    int Height;
    int size;
    FILE * arduino = fopen("/dev/ttyUSB0","w");
    fputs("$S",arduino);

    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);

    XWindowAttributes attributes = {0};
    XGetWindowAttributes(display, root, &attributes);

    Width = 1920;
    Height = 1080;
    size=Width*Height*4;
    XImage* img;// = XGetImage(display, root, 0, 0 , Width, Height, AllPlanes, ZPixmap);;
    clock_t prev_time=clock(),time;
    while (true) {
      img = XGetImage(display, root, 0, 0 , Width, Height, AllPlanes, ZPixmap);
      //BitsPerPixel = img->bits_per_pixel;
      //Pixels.resize(Width * Height * 4);
    
      //memcpy(&Pixels[0], img->data, Pixels.size());
      //cout << (int)((uint8_t)img->data[1314800]) << ' ' << (int)((uint8_t)img->data[1314801]) << ' ' << (int)((uint8_t)img->data[1314802]) << ' ' << (int)((uint8_t)img->data[1314803]) << "         " << '\r';
      /*
      for (auto i:Pixels)
        cout << (unsigned int)i << endl;
      */
      
      for (int i=0;i<size;i+=4)
        blue+=(uint8_t)img->data[i];
      for (int i=1;i<size;i+=4)
        green+=(uint8_t)img->data[i];
      for (int i=2;i<size;i+=4)
        red+=(uint8_t)img->data[i];
      green*=0.6235294117647059;blue*=0.6235294117647059;
      red/=Width*Height;green/=Width*Height;blue/=Width*Height;
      char output[] = {'#',hx[red/16],hx[red%16],hx[green/16],hx[green%16],hx[blue/16],hx[blue%16]};
      fputs(output,arduino);
      fflush(arduino);
      //cout << output << '\r';
      XDestroyImage(img);
      time=clock();
      cout << CLOCKS_PER_SEC/double(time-prev_time) << " fps        \r" << flush;
      prev_time=time;
    }
    fclose(arduino);
    XCloseDisplay(display);
   return 0;
}
