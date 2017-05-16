// GRBL senza uso del pc!
// Shield usate: datalogger(per sd), lcd shield
// pin liberi: A3, 2, 3

#include <Wire.h>
#include <SD.h>
#include <LiquidCrystal.h>

int state, prevstate = 0;
int nextEncoderState[4] = { 2, 0, 3, 1 };
int prevEncoderState[4] = { 1, 3, 0, 2 };

const int chipSelect = 10;

String RxString;
int n_file = 0;
char* file_name[50];

String words[6] = {"Attesa GRBL...", "Errore SD", "SD OK!!!", "Errore!!!", "Stop!!!", "Fine invio!!!"};  //Italiano

LiquidCrystal lcd( 3, 2, 4, 5, 6, 7 );

int pinenc = A0;

bool isGcode(char* filename) {
  int8_t len = strlen(filename);
  bool result;
  if (  strstr(strlwr(filename + (len - 4)), ".gcd")
     || strstr(strlwr(filename + (len - 3)), ".nc")
     || strstr(strlwr(filename + (len - 4)), ".cnc")
     || strstr(strlwr(filename + (len - 4)), ".txt")
    ) {
    result = true;
  } else {
    result = false;
  }
  return result;
}

void setup()
{
	pinMode(8, INPUT);
  	pinMode(9, INPUT);
  	digitalWrite(8, HIGH);
  	digitalWrite(9, HIGH);
	pinMode(pinenc,INPUT);
	pinMode(A1,OUTPUT);
	digitalWrite(pinenc, HIGH);
	Serial.begin(115200);
	while (!Serial) { ; }

	lcd.begin(20, 4);
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(words[0]);
	lcd.setCursor(0, 1);

	pinMode(10, OUTPUT);
	if (!SD.begin(chipSelect))
	{
		lcd.print(words[1]);
		return;
	}

	if (SD.exists("nameFile.lst")) { SD.remove("nameFile.lst"); }
	File root = SD.open("/");
	if(!root) { return; }
	int j = 0;
	while(true)
	{
		File entry =  root.openNextFile();
		if (!entry) { entry.close(); break; }
		if (entry.isDirectory()) { delay(10); }
		else
		{
			if ( isGcode(entry.name()) )
			{
				file_name[n_file] = entry.name();
				Serial.println(file_name[n_file]);
				n_file++;
			}
		}
	}
	root.close();
	delay(1000);
//	lcd.print(n_file);
//	File myfile = SD.open("nameFile.lst", FILE_WRITE);
//	if(!myfile) { return; }
//	while(n_file != j)
//	{
//		lcd.print(file_name[j]);
  //  	myfile.write(file_name[j]);
    //	myfile.close();
    //	j++;
//	}
//	myfile.close();
	
//	lcd.print(words[2]);

	// sveglio GRBL :
	delay(1000);
	Serial.println("\030"); // invio cr+lf
	delay(500);
	serialread();
	//RxString.remove(0,2);
	//RxString.remove(20);
	lcd.setCursor(0,0);
	lcd.print(RxString);
	delay(2000);
	RxString = "";
}

int encoder()
{
	state = (digitalRead(9) << 1) | digitalRead(8);
  if (state != prevstate) {
    if (state == nextEncoderState[prevstate]) {
       beep();
       return 1;
    } else if (state == prevEncoderState[prevstate]) {
       beep(); 
       return 0;
    }
    prevstate = state;
  }
}

void beep()
{
	digitalWrite(A1, HIGH);
	delay(2);
	digitalWrite(A1, LOW);
}

void serialread()
{
	while (Serial.available() <= 0)
	{
		delay(5);
	}
	while (Serial.available())
	{
		RxString += char(Serial.read());
		delay(10);
	}
}

void leggisd()
{
	int i = 0;
	while(true)
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("File presenti: ");
		lcd.print(n_file);
		lcd.setCursor(0, 1);
		lcd.print(">");
		lcd.print(file_name[i]);
		Serial.println(file_name[i]);

		if ( encoder() == 0)
		{
			if (i > 1)
			{
				i--;
				delay(500);
			}
		}
		if ( encoder() == 1)
		{
			if (i < n_file)
			{
				i++;
				delay(500);
			}
		}
		digitalWrite(pinenc, HIGH);
		if ( digitalRead(pinenc) == LOW)
		{
			beep();
			delay(10);
			stream(i);
			break;
		}
		delay(500);
	}
}

void stream(int i)
{
	String gcd;
	int nstrm = 0;
	int nstr = 0;
	File strFile = SD.open(file_name[i]);

	while (!strFile.available())
	{
		delay(1000);
		return;
	}
	while (strFile.available())
	{
		gcd = strFile.readStringUntil('\n');
		nstr++;
	}
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Stream!!!");
	delay(1000);
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Avvio:");
	lcd.setCursor(7, 0);
	lcd.print(file_name[i]);
	delay(1000);
	strFile.seek(0);
	while (strFile.available())
	{
		gcd = strFile.readStringUntil('\n');
		gcd = gcd.substring(0, gcd.length() - 1);
		Serial.print(gcd);
		Serial.write(0x0A); // seguita solo da /n (LF)
		Serial.flush();
		nstrm++;
		lcd.setCursor(0, 1);
		lcd.print(nstr);
		lcd.print("/");
		lcd.print(nstrm);
		lcd.print(" ");
		lcd.setCursor(0, 2);
		lcd.print(gcd);
		while(RxString.lastIndexOf('ok') <= 0)
		{
			serialread();
			if ( RxString.lastIndexOf('error') > 0)
			{
				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print(RxString);
				lcd.setCursor(0, 1);
				lcd.print(words[3]);
				RxString = "";
				strFile.close();
				delay(1000);
				break;
			}
		}
		RxString = "";
		//readpos();
	}
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(words[5]);
	delay(2000);
	strFile.close();
	return;
}

/* void readpos()
{
	Serial.print("?");
	Serial.write(0x0A); // seguita solo da /n (LF)
	serialread();
	String Mpos = RxString;
	String Wpos = RxString;
	int M = RxString.indexOf('M');
	Mpos.remove(0,M+5);
	Mpos.remove(20);
	lcd.setCursor(0,2);
	lcd.print(Mpos);
	int W = RxString.indexOf('W');
	Wpos.remove(0,W+5);
	Wpos.remove(20);
	lcd.setCursor(0,3);
	lcd.print(Wpos);
	RxString="";
	return;
}
*/

void loop()
{
		leggisd();
		delay(10);
}