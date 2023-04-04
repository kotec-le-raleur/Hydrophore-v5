
//  HYDROPHORE VERSION 5    [Avril 2023 ]
//  mise en place du comptage par jour  ( NbCyclesJour )
//  mise place d'un interprêteur de commandes série
//
//
//
//
//
//

//  HYDROPHORE VERSION 4    [Aout 2022 ]
// comptage du temps entre deux déclenchement de la pompe
// modification de l'affichage et de la mise à  l'heure de la RTC
// [21/12/2021] affichage sur deux chiffres du compteur: donc  valeur en secondes
// MISE à L'heure
// ORDRE :=  YYMMDDwHHMMSS, Avec  'x' a la fin  Dw== jour  de 1 à 7 1=dimanche
// exemple   18 aout 2013 à 16 heure 27
//           1308181162700x
//           1711143001300x


// tableau des valeurs lues sur Analog 0 avec appui des boutons 1,2,3,4 du clavier
// en face avant du boitier de Hydrophore
// pas de bouton === 0
// bouton 1      ===  594  env 600
// bouton 2      ===  802  env 800
// bouton 3      ===  200  env 200
// bouton 4      ===  395  env 400
//  CPU = arduino Pro ou Pro Mini

#include "arduino.h"
#include "common.h"

// instantiation de l'objet horloge DS3231
DS3231 Clock;
bool Century = false;
bool h12Flag, pmFlag;

// instantiation de l'objet LCD et selection des broches sur le LCD panel 16X2
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#define _TEST_ 1
#define LCD_LIGHT 15      // A1 sur la carte arduino UNO
//#define SD_CHIPSELECT 16  // A2 sur la carte arduino UNO
#define PinTest 17        // vers input oscilloscope
#define Clavier_input A0
#define Btn_1 600
#define Btn_2 800
#define Btn_3 200
#define Btn_4 400
#define Btn_delta 50

struct ST_Temps temps;
struct ST_Stat MemStat;
int clavier = 0;
int Nbcycles = 0;
int NbCyclesJour = 0;
int BackG = 0;
int Temper = 0;
int hx, mx;
const byte interruptPin = 2;
char cmd[32];
int i = 0;

volatile unsigned int compteur, cprev;
File DataFile;
File root;
//const char* TabJours[] = {"?","Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
//const char* TabMois[]  = {"?","Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"};+


unsigned int cprev1, cprev2, cprev3, cprev4;
unsigned long tip, tip1, tip2, tip3, tip4;
String Line_1, Line_2;
String Data_SD;
// taches répétitives
Metro Display = Metro(1000);      // décompte du temps ON et display sur le LCD : doit rester à 1000 pour le compte des TIP
Metro TestChange = Metro(29000);  // test sur jour, semaine et mois pour les statistiques
float TempsON;

unsigned long tdebut;
//=========================================================
//== prototypes
//=========================================================

extern void processCommand();
extern float getTemperature();
extern void Init_Programme();
extern void shift_cprev(int);
extern void shift_tip(unsigned long);
extern void Affiche_tip();
extern void SetClock(char*);
extern void software_Reset();
extern void xprintf(const char* format, ...);
extern void converts2hm(unsigned long, int*, int*);
extern void RazStat();
extern void shift_nbjour(unsigned int);
extern void shift_nbsemaine(unsigned int);
extern void shift_nbmois(unsigned int);
extern void Display_MemStat(void);
extern void Display_EEprom(void);
extern bool ChangeJour(void);

void GetNow(ST_Temps* tx);
void WriteDataInFile_boot(void);
void WriteDataInFile();
void ISR_compte();
void DumpSDcard(char * file_name);
//=========================================================
void setup() {
  // Start the I2C interface
  Wire.begin();
  Serial.begin(115200);
  delay(500);
  // pour le test de ISR_compte
  // pinMode (PinTest, OUTPUT);

  // allumage de la diode de retro éclairage du LCD
  pinMode(LCD_LIGHT, OUTPUT);
  digitalWrite(LCD_LIGHT, HIGH);

  // entree interruption 50 Hertz
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISR_compte, FALLING);

  Init_Programme();
  //
 // Serial.println("lecture 1");
  //Display_MemStat();
  // Display_EEprom();
  Serial.println("\n");
  GetNow(&temps);
 // Serial.print("annee: ");
  //Serial.println(temps.yy);
  WriteDataInFile_boot();
  tip = 0;  // tip ==>  temps inter pompage

  //  shift_nbjour (12  );
  //  shift_nbsemaine( 11 );
  //  shift_nbmois( 75 );
}


//=======================================================================
void loop() {
  //------------------------------ tache 1  ---------------------
  //  tache 1 décrémentation du timer et display LCD tache annulée avec la version 4
  if (Display.check() == 1)  // période = 1000 milliSecondes
  {
    tdebut = millis();
    tip++;  // compteur de temps inter pompage 1 seconde
            //	lcd.clear();
    Line_1 = "";
    Line_2 = "";
    Data_SD = "";
    GetNow(&temps);  // maj de la structure temps

    //Serial.print(tip);
    //DPF("tip=",tip);
    //xprintf(" %d:%d:%d", temps.hh, temps.mm, temps.ss);
    //xprintf("- %d/%d/%d\n",  temps.jj, temps.mo, temps.yy) ;

    if (temps.hh < 10) { Line_1 += "0"; }
    Line_1 += temps.hh;
    Line_1 += ":";
    if (temps.mm < 10) { Line_1 += "0"; }
    Line_1 += temps.mm;
    //  suppression des secondes
    //  Line_1 += ":";
    //  if (temps.ss <10){Line_1 += "0";}
    //  Line_1 += temps.ss;
    converts2hm(tip, &hx, &mx);
    Line_1 += " ";
    if (hx < 10) { Line_1 += "0"; }
    Line_1 += hx;
    Line_1 += ":";
    if (mx < 10) { Line_1 += "0"; }
    Line_1 += mx;

    lcd.setCursor(0, 0);  // colonne 0 ligne 0
    lcd.print(Line_1);    // affichage ligne 0 sur le LCD
    //	lcd.setCursor(0, 1);  // colonne 0 ligne 1 == la deuxieme ligne en fait
    //	lcd.print(Line_2);
    //  DPF("display time=", millis()-tdebut);
  }

  //------------------------------ tache 2  ---------------------
  // test de changement de jour  période = 60 secondes
  if (TestChange.check() == 1) {
    PRTL("tache 2 on");
    Data_SD = "";
    if (temps.hh < 10) { Data_SD += "0"; }
    Data_SD = temps.hh;
    Data_SD += ":";
    if (temps.mm < 10) { Data_SD += "0"; }
    Data_SD += temps.mm;
    Data_SD += ":";
    if (temps.ss < 10) { Data_SD += "0"; }
    Data_SD += temps.ss;
    Serial.println(Data_SD);

    //  GetNow (&temps);  // maj de la structure tp  lue dans le circuit Rtc DS3231

    if ((temps.hh == 0) && (temps.mm == 0) && (temps.ss < 30)) {
      PRTL("detection jour ok");
      DPF("nbcycle total", Nbcycles);
      DPF("NbCycles/jour", NbCyclesJour);
      shift_nbjour(NbCyclesJour);
      WriteStatInFile();
      NbCyclesJour = 0;
    }
  }

  //------------------------------ tache 3  par defaut ------------
  //  si le compteur n'est pas à zéro c'est qu'il y a eu des interruptions
  // compteur = 0;
  if (compteur == 0) {
    delay(5);  // rien, on passe === parasites
  } else {
    Serial.println(F("cp>0 start"));
    cprev = compteur;
    delay(100);
    while (compteur) {
      if ((cprev == compteur) && (cprev > 100))  // au moins 2 secondes de 220V sinon c'est un parasite  15 mai 2017
      {
        Nbcycles++;
        NbCyclesJour++;
        shift_cprev(compteur);  // décalage des valeurs vers la droite
        shift_tip(tip);         // idem pour les tip
        UpdateLCD();
        delay(100);
        //        Serial.print("stop: compteur="); Serial.println(compteur);
        TempsON = (float)(compteur / 5.0);
        //      	Serial.print("stop: Temps   ="); Serial.println(TempsON);
        WriteDataInFile();
        compteur = 0;
        tip = 0;
      } else {
        cprev = compteur;
        delay(100);
      }
    }
  }

  UpdateLCD();
  // test du clavier
  //  on teste le bouton 1 pour changer l'affichage avec les temps inter-pompage

  clavier = analogRead(Clavier_input);
  if ((clavier > Btn_1 - Btn_delta) && (clavier < Btn_1 + Btn_delta)) {
    Serial.println(F("Bouton 1 presse"));
    Affiche_tip();
    while (analogRead(Clavier_input) > 20) delay(5);
    Serial.println(F("Bouton 1 relache"));
    delay(500);
  }

  if ((clavier > Btn_2 - Btn_delta) && (clavier < Btn_2 + Btn_delta))  // affichage stat par jours sur 8 jours
  {
    Serial.println(F("Bouton 2 presse"));
    compteur = 800 + random(5000);
    delay(200);
  }

  if ((clavier > Btn_3 - Btn_delta) && (clavier < Btn_3 + Btn_delta))  // affichage stat par jours
  {
    Serial.println(F("Bouton 3 presse"));
    Display_MemStat();
    delay(800);
  }

  if ((clavier > Btn_4 - Btn_delta) && (clavier < Btn_4 + Btn_delta)) {
    Serial.println(F("Bouton 4 presse reboot"));
    delay(1000);
    software_Reset();
  }


  if (Serial.available()) {
      Serial.println(F("processCommand"));
      while (Serial.available())
       {
        char c = Serial.read();
        if (c != '\n') {
          cmd[i++] = toupper(c);
        } else {
          cmd[i] = '\0';
          break;
        }
      }

      DPF("cmd  =", cmd);
      DPF("index=",i);
      switch (cmd[0]) {
        case 'H':  // mise à l'heure
          if (i == 14) {SetClock(cmd); PRTL("modif heure");}  // bonne longueur
          break;

        case '+':  // + 1 heure
          if (temps.hh < 23) { Clock.setHour(temps.hh + 1); }
          break;

        case '-':  // - 1 heure
          if (temps.hh > 0) { Clock.setHour(temps.hh - 1); }
          break;

        // case 'D':  // dump de la SD card
		    // root = SD.open("/");
			  // printDirectory(root, 0);
			  // Serial.println("done!");

        // DumpSDcard(Fname);
        // break;

        case 'R':  // reset
           if (i == 5) {software_Reset(); PRTL("Reset"); }  // bonne longueur
          break;

        default:
          PRTL("Bad cmd");
      }
      memset(cmd, 0, sizeof(cmd));
      i = 0;  // raz de cmd et de l'index
      } 
      delay(50);
     
  }  // fin du loop

//=================================================================================
/* void DumpSDcard(char * file_name)
{   
    DataFile.close(); delay(100);
	DPF("Ouverture du fichier: ",file_name);
	DataFile = SD.open(file_name, FILE_READ);
	if (DataFile) {
		PRTL("Contenu du fichier :");
		while (DataFile.available()) {
		Serial.write(DataFile.read());
		}
    DataFile.close();
	} 
	else 
	{
		PRTL("Impossible d'ouvrir le fichier.");
	}
}
 */
//=================================================================================
/* void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
 */

  void UpdateLCD(void) {
    lcd.setCursor(12, 0);  // colonne 9 ligne 1
    lcd.print(F("#   "));
    lcd.setCursor(13, 0);  // colonne 12 ligne 1
    lcd.print(NbCyclesJour);


    lcd.setCursor(0, 1);  // colonne 0 ligne 1
    lcd.print(F("                "));

    //lcd.setCursor(0, 1);  // colonne 0 ligne 1
    //lcd.print(sLigne2);
    lcd.setCursor(0, 1);  // colonne 0 ligne 1
    lcd.print(cprev1 / 50, DEC);
    lcd.setCursor(4, 1);  // colonne 0 ligne 1
    lcd.print(cprev2 / 50, DEC);
    lcd.setCursor(8, 1);  // colonne 0 ligne 1
    lcd.print(cprev3 / 50, DEC);
    lcd.setCursor(12, 1);  // colonne 0 ligne 1
    lcd.print(cprev4 / 50, DEC);
  }
  //=================================================================================
  void WriteDataInFile() {
    DataFile = SD.open(Fname, FILE_WRITE);
   if (DataFile)
   {
        DPF("Data carte SD OK", DataFile);
        if (temps.hh < 10) { Data_SD += "0"; }
        Data_SD = temps.hh;
        Data_SD += ":";
        if (temps.mm < 10) { Data_SD += "0"; }
        Data_SD += temps.mm;
        Data_SD += " ";
        Data_SD += temps.jj;
        Data_SD += "/";
        Data_SD += temps.mo;
        Data_SD += "/";
        Data_SD += temps.yy;
        Data_SD += ";";
        Data_SD += String(compteur / 50);
        //	Data_SD += ";";
        //	Data_SD += String (Temper);
        // print to the serial port too:
        Serial.println(Data_SD);
    }
    else 
    {
        DPF("data carte SD non prete", DataFile);
        return;
    }

    // if the file is available, write to it:
    if (DataFile) {
      DataFile.println(Data_SD);
      DataFile.close();
    }
  }
  //=================================================================================
  void WriteStatInFile() {
    DataFile = SD.open(Fname_3, FILE_WRITE);
    if (DataFile) {
        DPF("Stat carte SD OK", DataFile);

        if (temps.hh < 10) { Data_SD += "0"; }
        Data_SD = temps.hh;
        Data_SD += ":";
        if (temps.mm < 10) { Data_SD += "0"; }
        Data_SD += temps.mm;
        Data_SD += " ";
        Data_SD += temps.jj;
        Data_SD += "/";
        Data_SD += temps.mo;
        Data_SD += "/";
        Data_SD += temps.yy;
        Data_SD += ";";
        Data_SD += String(MemStat.tab_nbjour[0]);
        Data_SD += ";";
        Data_SD += String(Nbcycles);
        Data_SD += ";";
        Data_SD += String(MemStat.tab_nbsemaine[0]);
        Data_SD += ";";
        Data_SD += String(MemStat.tab_nbmois[0]);
        Data_SD += ";";
        // print to the serial port too:
        Serial.println(Data_SD);
        // if the file is available, write to it:
    }
    else 
    {
        DPF("Stat carte SD non prete", DataFile);
        return;
    }
    
    if (DataFile) {
      DataFile.println(Data_SD);
      DataFile.close();
    }
  }
  //=================================================================================
  void WriteDataInFile_boot() {
    DataFile = SD.open(Fname_2, FILE_WRITE);
    if (DataFile) {
      DPF("Boot carte SD OK", DataFile);
    if (temps.hh < 10) { Data_SD += "0"; }
    Data_SD = temps.hh;
    Data_SD += ":";
    if (temps.mm < 10) { Data_SD += "0"; }
    Data_SD += temps.mm;
    Data_SD += " ";
    Data_SD += temps.jj;
    Data_SD += "/";
    Data_SD += temps.mo;
    Data_SD += "/";
    Data_SD += temps.yy;
    Data_SD += "  __reboot  Temp=";
    Data_SD += String(Temper);
    // print to the serial port too:
    Serial.println(Data_SD);
    }
    else 
  {
        DPF("Boot carte SD non prete", DataFile); 
        return;
    }
   
    // if the file is available, write to it:
    if (DataFile) {
      DataFile.println(Data_SD);
      DataFile.close();
    }
  }
  //=================================================================================
  void GetNow(ST_Temps * tx) {
    tx->yy = Clock.getYear() + 2000;
    tx->mo = Clock.getMonth(Century);
    tx->jj = Clock.getDate();
    tx->wd = Clock.getDoW();
    tx->hh = Clock.getHour(h12Flag, pmFlag);
    tx->mm = Clock.getMinute();
    tx->ss = Clock.getSecond();
    //	tx->jour=  TabJours[tx->wd]   ;
  }

  //=================================================================================
  // Fonction appelée par l'interruption FALLING sur int1 (pin 2)
  void ISR_compte() {
    compteur++;
    // Serial.print("!");
    // verification avec l'oscilloscope
    // digitalWrite(PinTest, HIGH);
    // delayMicroseconds(1);
    // digitalWrite(PinTest, LOW);
  }