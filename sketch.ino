#include <Wire.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

int lineNo = 0;
String gcd;
int nstrm = 0;
const int chipSelect = 10; 
String RxString;
int RxStringResOk;
int RxStringResErr;
int flagM = 0;
int k = 0;

int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

String menu[3] = {"Leggi SD!!", "Muovi assi XY", "Muovi asse Z"};  //Italiano

char* myfile;


int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
/*
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  
*/
 // For V1.0 comment the other threshold and use the one below:

 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   



 return btnNONE;  // when all others fail, return this...
}


//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


void setup() {
	Serial.begin(115200);
	while (!Serial) {
		;
		
	}

	Serial.print("\nInitializing SD card...");
	pinMode(10, OUTPUT);
	if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
    
	}
	Serial.println("card initialized.");
	
	lcd.begin(16,2);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Attesa GRBL..");
	delay(1000);
	
	// sveglio GRBL :
	Serial.println(); // invio cr+lf (risponde con 2 ok di fila)
 	delay(1000); // attendo un sec per la risposta
 	Serial.flush();  // per sicurezza svuoto il buffer di ricezione

}

void stream(char* nameFile) {
	File myFile = SD.open(nameFile);
	int nstr = 0;
	while (myFile.available()) {
		gcd = myFile.readStringUntil('\n');
		nstr++;
	}
	myFile.close();
	while (myFile.available()) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("Avvio:");
		lcd.setCursor(7,0);
		lcd.print(String(nameFile));
		gcd = myFile.readStringUntil('\n');
		gcd = gcd.substring(0, gcd.length() - 1);
		Serial.print(gcd);
		Serial.write(0x0A); // seguita solo da /n (LF)
		nstrm++;
		lcd.setCursor(0,1);
		lcd.print(nstr);
		lcd.print("/");
		lcd.print(nstrm);
		lcd.print(" ");
		RxStringResOk = 0;
		while(RxStringResOk <= 0) {
		while (Serial.available() <=0) { delay(5);}
			while (Serial.available()) {
			RxString += char(Serial.read());
			delay(10);
				
			}
			RxStringResOk = RxString.lastIndexOf('ok');
			RxStringResErr = RxString.lastIndexOf('error');
			RxString="";
			if ( RxStringResErr > 0) {
				lcd.clear();
				lcd.setCursor(0,0);
				lcd.print("Errore!!!");
				RxString="";
				myFile.close();
				delay(2000);
				return;
				
			}
			
		}
		lcd_key = read_LCD_buttons();  // read the buttons
		if ( lcd_key == 4) {
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("Ritorno!");
			delay(1000);
			myFile.close();
			return;
			
		}
	}
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Fine invio !!");
	delay(2000);
	myFile.close();
	return;
}

void leggisd() {
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("SD");
	File root = SD.open("/");
	delay(500);
	lcd.setCursor(0,0);
	lcd.print("SD Letta!!!");
	int i = 0;
	int j = 1;
	while(1>2) {
		while (i != j) {
			File entry =  root.openNextFile();
			if (! entry) {
				root.rewindDirectory();
				i=0;
			}
			if (i > j) {
				root.rewindDirectory();
				i=0;
			}
			if (entry.isDirectory()) {
				;
		
			} else {
				myfile = entry.name();
    	   		lcd.setCursor(0,1);
				lcd.print("                ");
				lcd.setCursor(0,1);
				lcd.print(j);
				lcd.print(">");
				lcd.print(myfile);
				entry.close();
				i++;	
			}
			
		}
		delay(100);
		lcd_key = read_LCD_buttons();  // read the buttons
		if ( lcd_key == 2) { if (j > 0) { j--; delay(500); } }
		if ( lcd_key == 1) { j++; delay(500); }
		if ( lcd_key == 4) {
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("Ritorno!");
			delay(1000);
			return;
		}
		if ( lcd_key == 0) {
			lcd.clear();
			lcd.setCursor(0,0);
			lcd.print("Stram!!!");
			delay(500);
			File myFile = SD.open(myfile);
			stream(myfile);
		}
	}
}

void movexy() { ;  }  //todo
void movez() { ;  }  //todo



void loop() {
	lcd_key = read_LCD_buttons();  // read the buttons
	
	if ( lcd_key == 4) { flagM = 1;	}

	if (flagM == 1) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("Menu");
		lcd.setCursor(0,1);
		lcd.print("                ");
		lcd.setCursor(0,1);
		lcd.print(k);
		lcd.print(">");
		lcd.print(menu[k]);
		delay(200);
	}
	
	if ( lcd_key == 2 and flagM == 1 and k>0) { k--; delay(500); } //down
	if ( lcd_key == 1 and flagM == 1 and k<2) { k++; delay(500); } //up
	if ( lcd_key == 3 and flagM == 1) { flagM = 0; delay(500); } //left
	if ( lcd_key == 0 and flagM == 1 and k == 0) { delay(500); leggisd(); }  //right
	if ( lcd_key == 0 and flagM == 1 and k == 1) { delay(500); movexy(); }  //right
	if ( lcd_key == 0 and flagM == 1 and k == 2) { delay(500); movez(); }  //right
}
