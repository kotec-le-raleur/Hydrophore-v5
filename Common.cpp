#include "common.h"

void converts2hm(unsigned long ts, int * hh, int * mm) ;
void GetDateStuff(char* ,byte& Year, byte& Month, byte& Day, byte& DoW, byte& Hour, byte& Minute, byte& Second);
void SetClock(char*) ;
void xprintf(const char * format, ...);
byte Year;
byte Month;
byte Date;
byte DoW;
byte Hour;
byte Minute;
byte Second;

static const unsigned int  NbMAGIC = 0xDEA0;
unsigned int Magic = 0;


extern void GetNow( ST_Temps* tx ) ;
extern int NbCyclesJour;
extern int Nbcycles;
extern void WriteStatInFile();
//*********************************************************************
//*****************  debut des fonctions  *****************************
//*********************************************************************

// Restarts program from beginning but does not reset the peripherals and registers
void software_Reset() {asm volatile ("  jmp 0");}  
void RazStat()
 {     memset(&MemStat,0x00,sizeof(struct ST_Stat)); }
//===============================================================================
void Init_Programme (void)
{
  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0, 0);
  lcd.print(F(_VERSION_)); // print a simple message ligne 0

// test de l'EEprom et rechargement si c'est OK  RAZ sinon !
   RazStat();
   EEPROM.get (0, Magic) ;
   if (Magic != NbMAGIC)    // il faut tout remettre à 0
    { 
      Serial.println("erase EEprom");
      EEPROM.put(0,NbMAGIC);
      EEPROM.put(2,MemStat);
    }
   else 
    {
      Serial.println(" EEprom OK : on recharge Memstat ");
      EEPROM.get (2,MemStat);  //on renseigne MemStat avec le contenu de l'eeprom addr= 2
    }


  //  gestion de la carte SD 2 giga
   if (!SD.begin(SD_CHIPSELECT)) 
   {
     Serial.println(F("Carte hs ou absente"));
     lcd.setCursor(0, 1);                    // colonne 0 sur la deuxième ligne
     lcd.print(F("Pas de SD !!!!  "));       // print a simple message ligne 0
     delay(3000);
     lcd.clear();
     return;
   }
  Serial.println(F("card initialized."));
  lcd.setCursor(0, 1);                // colonne 0 sur la deuxième ligne
  lcd.print("Carte SD OK     ");      // print a simple message ligne 0
  delay(1000);

  if (SD.exists(Fname)) 
  {
    Serial.println(F("hydro_xx.csv exists."));
  }
  else
	{
    DataFile = SD.open(Fname, FILE_WRITE);
    DataFile.close();
    Serial.println("fichier pompe_V5 OK");
	}
  delay(100);
  lcd.clear();
 return;
 }

 
//===============================================================================
void shift_cprev(int cpt)
{
  cprev4 = cprev3;
  cprev3 = cprev2;
  cprev2 = cprev1;
  cprev1 = cpt;
}
//===============================================================================
void shift_tip(unsigned long lasttip)
{
  tip4 = tip3;
  tip3 = tip2;
  tip2 = tip1;
  tip1 = lasttip;
}


//===============================================================================
 void shift_nbjour (unsigned int lastnbjour)
 {
   for (int i=NBSTAT; i>1; i--)
   {
     MemStat.tab_nbjour[i-1] = MemStat.tab_nbjour[i-2];
   }
   MemStat.tab_nbjour[0] = lastnbjour;
 }


//===============================================================================
 void shift_nbsemaine (unsigned int lastnbsemaine)
{
   for (int i=NBSTAT; i>1; i--)
   {
     MemStat.tab_nbsemaine[i-1] = MemStat.tab_nbsemaine[i-2];
   }
   MemStat.tab_nbsemaine[0] = lastnbsemaine;
 }

//===============================================================================
 void shift_nbmois (unsigned int lastnbmois)
 {
   for (int i=NBSTAT; i>1; i--)
   {
     MemStat.tab_nbmois[i-1] = MemStat.tab_nbmois[i-2];
   }
   MemStat.tab_nbmois[0] = lastnbmois;
 }


//===============================================================================
// void GetNow2(ST_Temps* tx ) 
// {
	// tx->yy	= Clock.getYear() + 2000  ;
	// tx->mo	= Clock.getMonth(Century)   ;
	// tx->jj	= Clock.getDate()   ;
	// tx->wd	= Clock.getDoW()   ;
	// tx->hh	= Clock.getHour()   ;
	// tx->mm	= Clock.getMinute()   ;
	// tx->ss	= Clock.getSecond()   ;
	// tx->jour=  TabJours[tx->wd]   ;
 // }
 


//===============================================================================
// detection du changement de jour au passage à minuit getHour==0 et getMinute==0 et getSecond==0
//===============================================================================


//===============================================================================
// detection du changement de semaine au passage à 00:00 et GetDoW==1
//===============================================================================


//===============================================================================
// detection du changement de mois au passage à 00:00 et GetDate==1
//===============================================================================




//===============================================================================
void Affiche_tip()
{
int hh,mm;	
	lcd.clear();
	Line_1 = ""; Line_2 = "";
	
	    converts2hm(tip1, &hh, &mm) ;
 		if (hh <10){Line_1 += "0";}
		Line_1 += hh;
		Line_1 += ":";
		if (mm <10){Line_1 += "0";}
		Line_1 += mm;
		Line_1 += " ";
    Line_1 += Nbcycles;
		Line_1 += " ";
	    converts2hm(tip2, &hh, &mm) ;
 		if (hh <10){Line_1 += "0";}
		Line_1 += hh;
		Line_1 += ":";
		if (mm <10){Line_1 += "0";}
		Line_1 += mm;
		
	    converts2hm(tip3, &hh, &mm) ;
 		if (hh <10){Line_2 += "0";}
		Line_2 += hh;
		Line_2 += ":";
		if (mm <10){Line_2 += "0";}
		Line_2 += mm;
		Line_2 += " ";
    Line_2 += NbCyclesJour;
		Line_2 += " ";

	    converts2hm(tip4, &hh, &mm) ;
 		if (hh <10){Line_2 += "0";}
		Line_2 += hh;
		Line_2 += ":";
		if (mm <10){Line_2 += "0";}
		Line_2 += mm;
 	
		lcd.setCursor(0, 0);  // colonne 0 ligne 0 
		lcd.print(Line_1); 
		lcd.setCursor(0, 1);  // colonne 0 ligne 1 == la deuxieme ligne 
		lcd.print(Line_2); 
    delay(1000);
  	lcd.clear();
        
	return;
}
 
//===============================================================================
void converts2hm(unsigned long ts, int * hh, int * mm) 
{
    ts /= 60;
    *mm = (int)ts % 60;
    ts /= 60;
    *hh = (int)ts % 24;
}
 
//===============================================================================

 void xprintf(const char * format, ...)
{
  va_list ap;
  va_start(ap, format);
  int size = vsnprintf(nullptr, 0, format, ap) + 1;
  if (size > 0) {
    va_end(ap);
    va_start(ap, format);
    char buf[size + 1];
    vsnprintf(buf, size, format, ap);
    Serial.write(buf);
  //  Terminal.write(buf);
  }
  va_end(ap);
}

//===============================================================================
 void GetDateStuff(char *InString, byte& Year, byte& Month, byte& Day, byte& DoW, byte& Hour, byte& Minute, byte& Second) {
	// The stuff coming in should be in the order HYYMMDDwHHMMSS, with an 'H' at the beginning.
   // 2304021174100x
	
	byte Temp1, Temp2;
	
	Serial.println(InString);
	// Read Year first
	Temp1 = (byte)InString[1] -48;
	Temp2 = (byte)InString[2] -48;
	Year = Temp1*10 + Temp2;
	// now month
	Temp1 = (byte)InString[3] -48;
	Temp2 = (byte)InString[4] -48;
	Month = Temp1*10 + Temp2;
	// now date
	Temp1 = (byte)InString[5] -48;
	Temp2 = (byte)InString[6] -48;
	Day = Temp1*10 + Temp2;
	// now Day of Week
	DoW = (byte)InString[7] - 48;		
	// now Hour
	Temp1 = (byte)InString[8] -48;
	Temp2 = (byte)InString[9] -48;
	Hour = Temp1*10 + Temp2;
	// now Minute
	Temp1 = (byte)InString[10] -48;
	Temp2 = (byte)InString[11] -48;
	Minute = Temp1*10 + Temp2;
	// now Second
	Temp1 = (byte)InString[12] -48;
	Temp2 = (byte)InString[13] -48;
	Second = Temp1*10 + Temp2;
	
	Serial.print(Hour);   Serial.print(":");
	Serial.print(Minute); Serial.print(":");
	Serial.print(Second); Serial.print("  ");
	Serial.print(Day);    Serial.print("/");
	Serial.print(Month);  Serial.print("/");
	Serial.print(Year);   Serial.print(" #j ");
	Serial.print(DoW);    Serial.println(" end");
}

void SetClock(char* cmd) {

	// If something is coming in on the serial line, it's
	// a time correction so set the clock accordingly.

		GetDateStuff(cmd, Year, Month, Date, DoW, Hour, Minute, Second);
		Clock.setClockMode(false);	// set to 24h
		//setClockMode(true);	// set to 12h
		Clock.setYear(Year);
		Clock.setMonth(Month);
		Clock.setDate(Date);
		Clock.setDoW(DoW);
		Clock.setHour(Hour);
		Clock.setMinute(Minute);
		Clock.setSecond(Second);
    delay(55);
		Serial.flush();
//		software_Reset();  // faire un reboot ici
	}



 //===============================================================================
void Display_EEprom()
{
unsigned int val=0;
Serial.println("EEPROM ");
for (int i=0; i<NBSTAT;i++)
  { 
    EEPROM.get(2+(2*i),val);
    Serial.print(val);Serial.print("  ");
  }
  Serial.println(" ");
//*********************
for (int i=0; i<NBSTAT;i++)
  { 
    EEPROM.get((NBSTAT*2)+2+(i*2),val);
    Serial.print(val);Serial.print("  ");
  }
 Serial.println(" "); 
//**********************
for (int i=0; i<NBSTAT;i++)
  { 
    EEPROM.get((NBSTAT*4)+2+(i*2),val);
    Serial.print(val);Serial.print("  ");
  }
  Serial.println("");
}


//===============================================================================
void Display_MemStat()
{
Serial.println("MemStat");
for (int i=0; i<NBSTAT;i++)
  { 
    Serial.print(MemStat.tab_nbjour[i]);Serial.print("  ");
  }
  Serial.println(" ");
//*****************************************
 for (int i=0; i<NBSTAT;i++)
  { 
    Serial.print(MemStat.tab_nbsemaine[i]);Serial.print("  ");
  }
  Serial.println("");
//*****************************************
for (int i=0; i<NBSTAT;i++)
    { 
    Serial.print(MemStat.tab_nbmois[i]);Serial.print("  ");
  }
 Serial.println("");
 }


