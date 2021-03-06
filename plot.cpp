#include <Arduino.h>
#include <glcd.h>

#include "antenna_analyzer.h"
#include "gui.h"

int16_t plot_readings[128];
#define X_OFFSET 18
#define Y_OFFSET 14

unsigned long f, f1, f2, stepSize;


int freq2screen(unsigned long freq){
  unsigned long f1, f2, hz_per_pixel;

  hz_per_pixel = spanFreq / 100;
  f1 = centerFreq - spanFreq/2;
  return (int)((freq - f1)/hz_per_pixel) + X_OFFSET;
}

int pwr2screen(int y){
  return  -y/2 + Y_OFFSET;
}

int vswr2screen(int y){
  if (y >= 100)
    y = 99;
  return y/2 + Y_OFFSET-1;
}

// this builds up the top line of the display with frequency and mode
void updateHeading() {
  int vswr_reading;
  // tks Jack Purdum W8TEE
  // replaced fsprint commmands by str commands for code size reduction

  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));

  ultoa(frequency, b, DEC);

  if (mode == MODE_ANTENNA_ANALYZER)
    strcpy(c, "SWR ");
  else if (mode == MODE_MEASUREMENT_RX)
    strcpy(c, "PWR ");
  else if (mode == MODE_NETWORK_ANALYZER)
    strcpy(c, "SNA ");
  
  //one mhz digit if less than 10 M, two digits if more

   if (frequency >= 100000000l){
    strncat(c, b, 3);
    strcat(c, ".");
    strncat(c, &b[3], 3);
    strcat(c, ".");
    strncat(c, &b[6], 3);
  }
  else if (frequency >= 10000000l){
    strncat(c, b, 2);
    strcat(c, ".");
    strncat(c, &b[2], 3);
    strcat(c, ".");
    strncat(c, &b[5], 3);
  }
  else {
    strncat(c, b, 1);
    strcat(c, ".");
    strncat(c, &b[1], 3);    
    strcat(c, ".");
    strncat(c, &b[4], 3);
  }

  GLCD.DrawString(c, 0, 0);

  itoa(spanFreq/10000, c, 10);
  strcat(c, "K/d");
  GLCD.DrawString(c, 128-(strlen(c)*6), 0);
}

void powerHeading(int current_pos){
  
  GLCD.FillRect(0,0,127,12, WHITE);
  freqtoa(f1 + (stepSize * current_pos), b);
  GLCD.DrawString(b, 0, 0);
  
  if (mode == MODE_ANTENNA_ANALYZER)
    sprintf (b, " %d.%01d", plot_readings[current_pos]/10,plot_readings[current_pos] % 10);
  else
    sprintf(b, "%ddbm", plot_readings[current_pos]);

  GLCD.DrawString(b, 80, 0);
  GLCD.DrawLine(current_pos+ X_OFFSET, 8, current_pos + X_OFFSET,11);
}

void setupPowerGrid(){
  int x, y;
  char p[20];

  GLCD.ClearScreen();  

  while(btnDown())
    delay(100);
    
  updateHeading();


  //sprintf(p, "%ldK, %ldK/div", centerFreq/1000, spanFreq/10000); 
  //GLCD.DrawString(p, 0, 57);

  //draw the horizontal grid
  for (y = -10; y >= -90; y-= 20){
    for (x = X_OFFSET; x <= 100+ X_OFFSET; x += 2)
      GLCD.SetDot(x,pwr2screen(y),BLACK);
  }

  //draw the vertical grid
  f1 = centerFreq - (spanFreq/2);
  f2 = centerFreq + (spanFreq/2);
  for (f = f1; f <= f2; f += spanFreq/10){
    for (y =0; y <= 50; y += 2)
      GLCD.SetDot(freq2screen(f),y+Y_OFFSET-2,BLACK);
  }

  for (y = -10; y >= -90; y-= 20){
    itoa(y, p, 10);
    GLCD.DrawString(p, 0, pwr2screen(y)-4);
  }  

}

void xorDot(uint8_t x, uint8_t y) {
  uint8_t data;

  if((x >= DISPLAY_WIDTH) || (y >= DISPLAY_HEIGHT))
    return;

  GLCD.GotoXY(x, y-y%8);                                 // read data from display memory

  data = GLCD.ReadData();
  data = data ^ (0x01 << (y%8));
  //data = 0xFF;
  GLCD.WriteData(data);                                  // write data back to display
}


void setupVSWRGrid(){
  int x, y;
  char p[20];

  GLCD.ClearScreen();  

  while(btnDown())
    delay(100);
    
  updateHeading();

  //draw the horizontal grid
  for (y = 0; y <= 100; y += 20){
    Serial.print("d");
    Serial.println(vswr2screen(y));
    for (x = X_OFFSET; x <= 100+ X_OFFSET; x += 2)
      GLCD.SetDot(x,vswr2screen(y),BLACK);
  }

  //draw the vertical grid
  f1 = centerFreq - (spanFreq/2);
  if (f1 < 0)
      f1 = 0;
  f2 = f1 + spanFreq;
  for (f = f1; f <= f2; f += spanFreq/10){
    Serial.print(f);
    Serial.print(",");
    Serial.println(freq2screen(f));
    for (y =0; y <= 50; y += 2)
      GLCD.SetDot(freq2screen(f),y+Y_OFFSET,BLACK);
  }

  for (y = Y_OFFSET; y < 100 + Y_OFFSET; y += 20){
    itoa(y/10, p, 10);
    strcat(p, ".");
    GLCD.DrawString(p, 0, vswr2screen(y)-8);
  }  

  f1 = centerFreq - (spanFreq/2);
  f2 = f1 + spanFreq;
  stepSize = (f2 - f1)/100;
  int vswr_reading;

  Serial.print("f1 "); Serial.println(f1);
  Serial.print("f2 "); Serial.println(f2);
  Serial.print("step "); Serial.println(stepSize);

  int current_pos = 50;

  bool first = true;
  while (!btnDown()) {
    int i = 0;
    for (f = f1; f < f2; f += stepSize){
      takeReading(f);
      delay(20);
      //now take the readings
      int return_loss = openReading(f) - analogRead(DBM_READING)/5;
      if (return_loss > 30)
	return_loss = 30;
      if (return_loss < 0)
	return_loss = 0;

      int16_t oldY = vswr2screen(plot_readings[i]);
      vswr_reading = pgm_read_word_near(vswr + return_loss);
      plot_readings[i] = vswr_reading;
      //Serial.print(vswr_reading); Serial.print(' '); Serial.print(f); Serial.print(' ');Serial.print(freq2screen(f));
      //Serial.print("x");Serial.print(vswr_reading);Serial.print(' ');Serial.println(vswr2screen(vswr_reading));

      /*if (!first)
	xorDot(freq2screen(f),vswr2screen(old_reading));
	xorDot(freq2screen(f),vswr2screen(vswr_reading));*/
      if (false) {
	int newY = vswr2screen(vswr_reading);
	Serial.print("x"); Serial.print(i + X_OFFSET);
	Serial.print(" oldY"); Serial.print(oldY);
	Serial.print(" newY"); Serial.print(newY);
	Serial.println();
	if (!first)
	  xorDot(i + X_OFFSET,oldY);
	xorDot(i + X_OFFSET,newY);
      } else {
	if (i == 0)
	  GLCD.SetDot(freq2screen(f),vswr2screen(vswr_reading),BLACK);
	else
	  GLCD.DrawLine(i + X_OFFSET-1, vswr2screen(plot_readings[i-1]), i + X_OFFSET, vswr2screen(plot_readings[i]));
      }

      if (i == current_pos) {
	powerHeading(current_pos);
      }

      i++;

      {
	int dir = enc_read();
    
	if ((dir < 0 && current_pos + dir >= 0) ||
	    (dir > 0 && current_pos + dir <= 100)){
	  current_pos += dir;
	  powerHeading(current_pos);
	}
      }
      if (btnDown())
	break;
    }
    first = false;
  }

  powerHeading(current_pos);
  
  while(btnDown())
    delay(100);
}

void updateCursor(int pos, char*text){
  GLCD.FillRect(0,0,127,10, WHITE);
  GLCD.DrawString(text, 0, 0);
  GLCD.DrawLine(pos+ X_OFFSET, 8, pos + X_OFFSET,9);
}

void plotPower(){
  int x, y, pwr;
  char p[20];

  GLCD.ClearScreen();  

  while(btnDown())
    delay(100);
    
  //draw the horizontal grid
  for (y = 0; y <= 100; y += 20){
    Serial.print("d");
    Serial.println(pwr2screen(y));
    for (x = X_OFFSET; x <= 100+ X_OFFSET; x += 2)
      GLCD.SetDot(x,vswr2screen(y),BLACK);
  }

  //draw the vertical grid
  f1 = centerFreq - (spanFreq/2);
  if (f1 < 0)
      f1 = 0;
  f2 = f1 + spanFreq;
  for (f = f1; f <= f2; f += spanFreq/10){
    for (y =0; y <= 50; y += 2)
      GLCD.SetDot(freq2screen(f),y+Y_OFFSET,BLACK);
  }


  for (y = -80; y <= -20; y += 20){
    itoa(y, p, 10);
    GLCD.DrawString(p, 0, pwr2screen(y)-4);
  }

  f1 = centerFreq - (spanFreq/2);
  f2 = f1 + spanFreq;
  stepSize = (f2 - f1)/100;
  int i = 0, vswr_reading;

  for (f = f1; f < f2; f += stepSize){
    takeReading(f);
    delay(50);
    //now take the readings
    analogRead(DBM_READING);
    analogRead(DBM_READING);
    analogRead(DBM_READING);
    
    int r = analogRead(DBM_READING)/5 + dbmOffset;
    plot_readings[i] = r;
    Serial.print(plot_readings[i]);
    Serial.print('-');
 
    if (i == 0)
      GLCD.SetDot(X_OFFSET, pwr2screen(plot_readings[i]),BLACK);
    else
      GLCD.DrawLine(i + X_OFFSET-1, pwr2screen(plot_readings[i-1]), i + X_OFFSET, pwr2screen(plot_readings[i])); 
    i++;
  }

  int current_pos = 50;

  powerHeading(current_pos);
  while (!btnDown()){
    i = enc_read();
    
    if ((i < 0 && current_pos + i >= 0) || 
      (i > 0 && current_pos + i <= 100)){
      current_pos += i;
      powerHeading(current_pos);

/*      GLCD.FillRect(0,0,127,12, WHITE);

      freqtoa(f1 + (stepSize * current_pos), p);
      GLCD.DrawString(p, 0, 0);
      sprintf(p, "%ddbm", plot_readings[current_pos]);
      GLCD.DrawString(p, 80, 0);
      GLCD.DrawLine(current_pos+ X_OFFSET, 8, current_pos + X_OFFSET,11); */
    }
  }
  
  while(btnDown())
    delay(100);
}

