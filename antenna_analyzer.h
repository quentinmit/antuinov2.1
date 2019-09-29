#ifndef ANTENNA_ANALYZER_H
#define ANTENNA_ANALYZER_H

/* I/O ports to read the tuning mechanism */
#define ENC_A (A3)
#define ENC_B (A1)
#define FBUTTON (A2)

#define DBM_READING (A6)

/* offsets into the EEPROM storage for calibration */
#define MASTER_CAL 0
#define LAST_FREQ 4
#define OPEN_HF 8
#define OPEN_VHF 12
#define OPEN_UHF 16
#define LAST_SPAN 20
#define LAST_MODE 24

//to switch on/off various clocks
#define SI_CLK0_CONTROL  16      // Register definitions
#define SI_CLK1_CONTROL 17
#define SI_CLK2_CONTROL 18

#define IF_FREQ  (24993000l)
#define MODE_ANTENNA_ANALYZER 0
#define MODE_MEASUREMENT_RX 1
#define MODE_NETWORK_ANALYZER 2

extern const int PROGMEM vswr[];

void active_delay(int delay_by);
int openReading(unsigned long f);
void takeReading(long newfreq);
int calibrateClock();
int calibrateMeter();

#define MAX_SPANS 8
extern long spans[];

extern uint32_t xtal_freq_calibrated;

extern unsigned long mode;
extern char b[32], c[32];

extern unsigned long frequency, centerFreq, spanFreq;
extern int selectedSpan;
extern int dbmOffset;
extern int prevReading;

#endif /* ANTENNA_ANALYZER_H */
