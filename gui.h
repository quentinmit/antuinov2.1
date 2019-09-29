#ifndef GUI_H
#define GUI_H

int btnDown();
int enc_read(void);
void freqtoa(unsigned long f, char *s);

void updateScreen();
void updateMeter();
void doMenu();
void calibration_mode();

#endif /* GUI_H */
