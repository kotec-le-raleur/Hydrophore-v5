// common.h pour hydrophore V4
// modif  V3--> V4  12 aout 2022


#ifndef MY_COMMON_H // include guard
#define MY_COMMON_H
#define _VERSION_  "Hydrophore V5.1"

#include "arduino.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <DS3231.h>
#include <Metro.h> 
#include <EEPROM.h> 

#define  LCD_LIGHT      15   // A1 sur la carte arduino UNO
#define  SD_CHIPSELECT  16   // A2 sur la carte arduino UNO
#define  PinTest       17   // vers input oscilloscope
#define  Fname         "Pompe_V5.csv"   // mise a jour 29/06/2022
#define  Fname_2       "boot_log.txt"   // ajout    du 23/11/2021
#define  Fname_3       "stat_log.txt"   // ajout    13 aout 2022
#define  PRT           Serial.print
#define  PRTL          Serial.println
#define  DX(x,y)       Serial.print(x);Serial.println(y,HEX)
#define  DP(x,y)       Serial.print(x);Serial.println(y)
#define  DPF(msg,val)  Serial.print(F(msg));Serial.println(val)
#define  NBSTAT        9

extern LiquidCrystal lcd;
extern File     DataFile;
extern unsigned int  cprev1, cprev2 , cprev3, cprev4;
extern unsigned long  tip1, tip2 , tip3, tip4;
extern DS3231 Clock;
//extern const char* TabJours[] ;
//extern char* TabMois[];
extern bool Century;
extern String Line_1, Line_2	;
extern struct ST_Stat MemStat;

struct ST_Stat
  {
    unsigned int tab_nbjour[NBSTAT];
    unsigned int tab_nbsemaine[NBSTAT];
    unsigned int tab_nbmois[NBSTAT];
  } ;


 struct ST_Temps
  {
    unsigned int yy;
    unsigned int mo;
    unsigned int jj;
    unsigned int wd;
    unsigned int hh;
    unsigned int mm;
    unsigned int ss;
    char *       jour;
  } ;

#endif
