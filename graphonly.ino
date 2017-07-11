//#########################################################################//

#include <Adafruit_GFX.h>
#include <UTFTGLUE.h>
#include <TouchScreen.h>
#include <math.h>

#if !defined(SmallFont)// Declare which fonts we will be using
extern uint8_t SmallFont[];    //.kbv GLUE defines as GFXFont ref
#endif

UTFTGLUE myGLCD(A2, A1, A3, A4, A0);

// most mcufriend shields use these pins and Portrait mode:
uint8_t YP = A1;  // must be an analog pin, use "An" notation!
uint8_t XM = A2;  // must be an analog pin, use "An" notation!
uint8_t YM = 7;   // can be a digital pin
uint8_t XP = 6;   // can be a digital pin
uint8_t SwapXY = 0;
TouchScreen ts = TouchScreen(XP, YP, XM, YM);
TSPoint tp;

uint16_t identifier;
uint8_t Orientation = 1;    //Landscape
char domain = 'F';
int Width, Height, x = 22, y = 0 ;

#define BLACK   0x000000
#define pinread A10
#define ampvary 500
#define freqvary 10

//##################################################################################//

#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 32; //This value MUST ALWAYS be a power of 2
double signalFrequency = 50;
double samplingFrequency = 100;
uint8_t amplitude = 0;
/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define Theta 6.2831 //2*Pi

//#######################################################################################//


//##############################################################################//

void setup()
{
  Serial.begin(9600);
  myGLCD.begin(9600);
  identifier = myGLCD.readID();
  Height = myGLCD.width();
  Width = myGLCD.height();
  ts = TouchScreen(XP, YP, XM, YM, 300);
  myGLCD.begin(identifier);

  myGLCD.fillScreen(BLACK);

  //randomSeed(analogRead(5));   //.kbv Due does not like A0
  pinMode(A0, OUTPUT);       //.kbv mcufriend have RD on A0
  pinMode(pinread , INPUT);

  myGLCD.setFont(SmallFont);

  disp();
  button();

  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRect(12, 12, 314, 225);

}

//###########################################################################//

void loop()
{
  button();
  freq();

  amplitude = analogRead(pinread);

  double cycles = (((samples - 1) * signalFrequency) / samplingFrequency); //Number of signal cycles that the sampling will read

  for (uint8_t i = 0; i < samples; i++)
  {
    vReal[i] = int8_t((amplitude * (sin((i * (Theta * cycles)) / samples))) / 2.0);/* Build data with positive and negative values*/
    //vReal[i] = uint8_t((amplitude * (sin((i * (6.2831 * cycles)) / samples) + 1.0)) / 2.0);/* Build data displaced on the Y axis to include only positive values*/
  }
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */

  PrintVector(vReal, (samples >> 1));
  //myGLCD.drawPixel(i,119+(sin(((i*1.13)*3.14)/180)*95));
  //}
  x = x + freqvary;
  if (x > 310)
  {
    x = 22;
    disp();
  }
  //delay(200);
  //delayMicroseconds(50);
}

//#########################################################################################//


void disp()
{
  // Clear the screen and draw the frame
  myGLCD.clrScr();
  myGLCD.setRotation(1);

  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRect(12, 12, 314, 225);

  myGLCD.setColor(255, 255, 255);
  myGLCD.print("* Spectrum Analyzer *", CENTER, 1);

  myGLCD.setRotation(0);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("* Amplitude(V) *", CENTER, 0);

  myGLCD.setRotation(1);
  // Draw crosshairs
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.drawLine(19, 17, 19, 221);//y-axis
  myGLCD.drawLine(19, 219, 310, 219);//x-axis
  for (int k = 19; k < 310; k += freqvary)
  {
    myGLCD.drawLine(k, 217, k, 221);//x-axis
    if ((k + 1) % 50 == 0)
      myGLCD.printNumI((freqvary * (k + 1) / 100), k - 2, 220);
  }
  for (int k = 19; k < 220; k += 5000 / ampvary)
    myGLCD.drawLine(17, k, 21, k);  //y-axis

}

//##############################################################################################//

void button(void)
{
  myGLCD.setColor(64, 64, 64);
  myGLCD.fillRect(Width - 40, 20 , Width - 20, 41);
  myGLCD.setTextSize(2);
  myGLCD.setColor(255, 255, 255);

  if (domain == 'T')
  {
    myGLCD.setTextSize(2);
    myGLCD.setBackColor(64, 64, 64);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("T", Width - 34, 24);
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect(0, 226, 319, 239);
    myGLCD.setTextSize(1);
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("* Time(S) *", CENTER, 228);

  }
  else
  {
    myGLCD.setTextSize(2);
    myGLCD.setBackColor(64, 64, 64);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("F", Width - 34, 24);
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect(0, 226, 319, 239);
    myGLCD.setTextSize(1);
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("* Frequency(KHz) *", CENTER, 228);
  }

  tp = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(XP, OUTPUT);
  pinMode(YM, OUTPUT);

  
  {



    if (tp.x > 750 && tp.x < 810  && tp.y > 800 && tp.y < 860)
    {
      if (domain == 'T')
      {
        myGLCD.setTextSize(2);
        myGLCD.setColor(255, 255, 255);
        myGLCD.print("F", Width - 34, 24);
        domain = 'F';
      }
      else
      {
        myGLCD.setTextSize(2);
        myGLCD.setColor(255, 255, 255);
        myGLCD.print("T", Width - 34, 24);
        domain = 'T';
      }
    }

  }

}

//######################################################################################//

void PrintVector(double *vData, uint8_t bufferSize)
{
  myGLCD.setColor(0, 255, 255);
  if (domain=='F')
  {
  
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    y = map(vData[i], 0, ampvary, 0, myGLCD.height() - 42) + 24;
    myGLCD.drawPixel(x, myGLCD.height() - y);
  }
  }
  if(domain=='T')
  {
    y = map(amplitude, 0, ampvary, 0, myGLCD.height() - 42) + 24;
    myGLCD.drawPixel(x, myGLCD.height() - y);
  
  }

}
//###############################################//

void freq()
{
  #define acosf   acos
  int sensorPin = A10;    // select the input pin for the potentiometer
  // select the pin for the LED
  int sensorValue = 0;  // variable to store the value coming from the sensor
  int x = 0;
  int i;
  int y;
  int z;
  long a;
  int b;
  int k = 0;
  long c;
  int arr[250] ;
  unsigned long time;
  time = millis();
  Serial.println(time);

  for (i = 0; i < 251; i = i + 1)
  {
    arr[i] = analogRead(sensorPin);
  }

  time = millis();
  Serial.println(time);

  float f;

  for (i = 0; i < 251; i = i + 1)
  {
    if (arr[i] == 0 && arr[i + 1] != 0)
    {
      while (arr[i + 1] != 0)
      {
        k = k + 1;
        i = i + 1;
      }
      break;
    }

  }
  f = 1000 / (2 * k * 0.112);
  Serial.println(k);
  Serial.println("FREQUENCY MEASURED IS");
  Serial.println(f);
 // signalFrequency = f;
  //samplingFrequency=2.2*f;
}

//########################################################
