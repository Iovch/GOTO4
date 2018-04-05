/*
 * GOTO4_6.h Written by Igor Ovchinnikov 18/07/2016
 */
 
long Stepper_step(long ipSteps, unsigned uStepPin, unsigned uDirPin, unsigned uStepsPS)
{
 long iSteps=ipSteps, lRetVal=0;
 if((uStepPin>53)||(uDirPin>53)) return lRetVal;
 
 if(iSteps > 0) digitalWrite(uDirPin,  LOW);
 if(iSteps < 0) digitalWrite(uDirPin,  HIGH);
 iSteps=abs(iSteps);

 while (iSteps>0)
 {
  digitalWrite(uStepPin,  HIGH);
  delay(1000/uStepsPS);
  delayMicroseconds(1000*(1000%uStepsPS));
  digitalWrite(uStepPin,  LOW);
  iSteps--;
  if (ipSteps>0) lRetVal++; else lRetVal--;
 }
 return lRetVal;
}

void Stepper_X_step(int ipSteps)
{
  Stepper_step(ipSteps, DX_STEP_PIN, DX_DIR_PIN, imStepsXPS);
}

void Stepper_Y_step(int ipSteps)
{
  Stepper_step(ipSteps, DY_STEP_PIN, DY_DIR_PIN, imStepsYPS);
}

void Stepper_Z_step(int ipSteps)
{
  Stepper_step(ipSteps, DZ_STEP_PIN, DZ_DIR_PIN, imStepsZPS);
}

void AscFoSw(void)
{
 if(analogRead(SW_FOC_SENCE)<200)
 {
  bFocus=true;
  analogWrite(LIHT_FOC_PIN,150); // Светодиод фокусера включен
 }
 else
 {
  bFocus=false;
  analogWrite(LIHT_FOC_PIN,0); // Светодиод фокусера выключен
 }
}


// Функция int AskJoy() возвращает при ее вызове следующие значения:

//    0 - когда ничего не надо делать
//    1 - когда надо сделать микрошаг вперед по оси Х
//   16 - когда надо сделать полныйшаг вперед по оси Х
//    4 - когда надо сделать микрошаг назад по оси Х
//   64 - когда надо сделать полныйшаг назад по оси Х
//    2 - когда надо сделать микрошаг вверх по оси У
//   32 - когда надо сделать полныйшаг вверх по оси У
//    8 - когда надо сделать микрошаг вниз по оси У
//  128 - когда надо сделать полныйшаг вниз по оси У
//  256 - включить/отключить трекинг

int AskJOY()
{
  int iA1=0, iA2=0, iA3=0;
  int iRetValue=0;

  iA1 = analogRead(X_JOY_SENCE);
  iA2 = analogRead(Y_JOY_SENCE);
  iA3 = analogRead(SW_JOY_SENCE);
    
  if(iA1<15)                { iRetValue=iRetValue | 16; } // Полный шаг X+
  if(iA1>=15 && iA1 < 470)  { iRetValue=iRetValue |  1; } // Микрошаг X+
  if(iA1>575 && iA1<=1000)  { iRetValue=iRetValue |  4; } // Микрошаг X-
  if(iA1>1000)              { iRetValue=iRetValue | 64; } // Полный шаг X-
  
  if(iA2<15)                { iRetValue=iRetValue | 32; } // Полный шаг Y+
  if(iA2>=15  && iA2 < 470) { iRetValue=iRetValue |  2; } // Микрошаг Y+
  if(iA2>575  && iA2<=1000) { iRetValue=iRetValue |  8; } // Микрошаг Y-
  if(iA2>1000)              { iRetValue=iRetValue | 128;} // Полный шаг Y-

  if(iA3<500) {iRetValue=iRetValue | 256; delay(250);}    // Включить/отключить трекинг

  return iRetValue;
}

unsigned long StrToHEX (String STR)
{
  int  i;
  char c;
  unsigned long ulVal=0;
  for (i=0; i<STR.length(); i++)
  {
   ulVal=ulVal*16;
   c=STR.charAt(i);
   switch (c) 
    {
      case 'f': ;
      case 'F': ulVal++;
      case 'e': ;
      case 'E': ulVal++;
      case 'd': ;
      case 'D': ulVal++;
      case 'c': ;
      case 'C': ulVal++;
      case 'b': ;
      case 'B': ulVal++;
      case 'a': ;
      case 'A': ulVal++;
      case '9': ulVal++;
      case '8': ulVal++;
      case '7': ulVal++;
      case '6': ulVal++;
      case '5': ulVal++;
      case '4': ulVal++;
      case '3': ulVal++;
      case '2': ulVal++;
      case '1': ulVal++;
    };
  };
 return ulVal;
};

String HexTo8D (unsigned long Hex)
{
  String STR0="";
  char c = '0';
  if (Hex<0x10000000) STR0 += c;
  if (Hex<0x1000000)  STR0 += c;
  if (Hex<0x100000)   STR0 += c;
  if (Hex<0x10000)    STR0 += c;
  if (Hex<0x1000)     STR0 += c;
  if (Hex<0x100)      STR0 += c;
  if (Hex<0x10)       STR0 += c;
  return STR0;
};

void HexRaToString(unsigned long ulRaVal, unsigned long ulMaxRaVal)
{
  double udRa=0;
  int iRaH=0, iRaM=0, iRaS=0;
  udRa=double(ulRaVal>>8)/double(ulMaxRaVal>>8);
  iRaH=udRa*24;
  udRa=udRa-double(iRaH)/24;
  iRaM=udRa*24*60;
  udRa=udRa-double(iRaM)/24/60;
  iRaS=round(udRa*24*60*60);
  LCDString1=String("Ra="+String(iRaH)+"h"+String(iRaM)+"m"+String(iRaS)+"s");
  switch (iStDX)
  {
  case -1: {LCDString1=LCDString1+" S"; break;}
  case  1: {LCDString1=LCDString1+" N"; break;}
  }
}

void HexDeToString(unsigned long ulDeVal, unsigned long ulMaxDeVal)
{
  double udDe=0;
  int iDeG=0, iDeM=0, iDeS=0;
  udDe=double(ulDeVal>>8)/double(ulMaxDeVal>>8);
  if(udDe>0.5){udDe=udDe-1;}
  iDeG=udDe*360;
  udDe=udDe-double(iDeG)/360;
  iDeM=udDe*360*60;
  udDe=udDe-double(iDeM)/360/60;
  iDeM=abs(iDeM);
  iDeS=round(abs(udDe*360*60*60));
  LCDString2=String("De="+String(iDeG)+"*"+String(iDeM)+"m"+String(iDeS)+"s");
  switch (iStDY)
  {
  case -1: {LCDString2=LCDString2+" N/E"; break;}
  case  1: {LCDString2=LCDString2+" N/W"; break;}
  }
}

int LCDPrintString (String str, int row, int kol)
{
  int i=0;
  while (i<16 && i<str.length())
  {
    lcd.setCursor(kol-1+i, row-1);
    lcd.print(str.charAt(i));
    i++;
  }
}

int LCDPrintSTR (char* str, int row, int kol)
{
  int i=0;
  while (i<16 && str[i]!='\0')
  {
    lcd.setCursor(kol-1+i, row-1);
    lcd.print(str[i]);
    i++;
  }
}

void LCDPrint()
 {
  if(!bLCD) // Экваториальный режим
  {
   if(bAlignment)
   {
    HexRaToString(ulRA, MVRA);
    HexDeToString(ulDE, MVDE);
   }
   else
   {
    if (iStDY==-1) LCDString1="   N/W or S/E   "; //Телескоп слева от полярной оси
    if (iStDY== 1) LCDString1="   N/E or S/W   "; //Телескоп справа от полярной оси
    if (iStDY== 0) LCDString1=" Arduino GOTO4  "; //Телескоп не сориентирован по склонению
    if (iStDX== 0) LCDString1+=" RAERR"; //Не задано направление ведения телескопа
    if (bRun){LCDString2=" TRACKING"; if(iStDX>0) LCDString2+=" SOUTH"; if(iStDX<0) LCDString2+=" NORHT";}
    else      LCDString2="    STOPPED     ";
   }
    lcd.clear();
//  SetLCDLight();
  LCDPrintString(LCDString1,1,1);
  LCDPrintString(LCDString2,2,1);
  bLCD=true;
  }
 }

//   0 - когда ничего не надо делать
//   1 - когда надо сделать микрошаг вперед по оси Х
//  16 - когда надо сделать полныйшаг вперед по оси Х
//   4 - когда надо сделать микрошаг назад по оси Х
//  64 - когда надо сделать полныйшаг назад по оси Х
//   2 - когда надо сделать микрошаг вверх по оси У
//  32 - когда надо сделать полныйшаг вверх по оси У
//   8 - когда надо сделать микрошаг вниз по оси У
// 128 - когда надо сделать полныйшаг вниз по оси У

void LCDCOR (int pKey)
{
 if(true)
 {
  if(pKey!=0 && !bAlignment) LCDPrintSTR (" Correction ", 2, 1);
  switch (pKey)
  {
   case   1: if(iStDX!=0) LCDPrintSTR ("  > ", 2, 13); else LCDPrintSTR ("N/S Position ERR", 2, 1); break; 
   case   2: if(iStDY!=0) LCDPrintSTR ("  ^ ", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case   3: if(iStDX!=0) LCDPrintSTR (" ^> ", 2, 13); else LCDPrintSTR ("N/S Position ERR", 2, 1); break;    
   case   4: if(iStDX!=0) LCDPrintSTR (" <  ", 2, 13); else LCDPrintSTR ("N/S Position ERR", 2, 1); break;
   case   6: if(iStDX!=0) LCDPrintSTR (" <^ ", 2, 13); else LCDPrintSTR ("N/S Position ERR", 2, 1); break;    
   case   8: if(iStDY!=0) LCDPrintSTR ("  v ", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case   9: if(iStDY!=0) LCDPrintSTR (" v> ", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case  12: if(iStDY!=0) LCDPrintSTR (" <v ", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case  16: if(iStDX!=0) LCDPrintSTR ("  >>", 2, 13); else LCDPrintSTR ("N/S Position ERR", 2, 1); break;
   case  32: if(iStDY!=0) LCDPrintSTR (" ^^ ", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case  33: if(iStDY!=0) LCDPrintSTR (" ^^>", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case  36: if(iStDY!=0) LCDPrintSTR ("<^^ ", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case  48: if(iStDY!=0) LCDPrintSTR ("^^>>", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case  64: if(iStDX!=0) LCDPrintSTR ("<<  ", 2, 13); else LCDPrintSTR ("N/S Position ERR", 2, 1); break;
   case  96: if(iStDX!=0) LCDPrintSTR ("<<^^", 2, 13); else LCDPrintSTR ("N/S Position ERR", 2, 1); break;
   case 128: if(iStDY!=0) LCDPrintSTR (" vv ", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case 129: if(iStDY!=0) LCDPrintSTR (" vv>", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case 132: if(iStDY!=0) LCDPrintSTR ("<vv ", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case 144: if(iStDY!=0) LCDPrintSTR ("vv>>", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
   case 192: if(iStDY!=0) LCDPrintSTR ("<<vv", 2, 13); else LCDPrintSTR ("E/W Position ERR", 2, 1); break;
  };
 }
 else
 {
 
 }
}

// Добавлено в версии 4_6:

boolean FoStart()
{
  boolean bRetVal=false;
  int iAF = analogRead(FO_SENCE_PIN);
  if (iAF < 25  && FO_START_VAL == LOW ) bRetVal=true;
  if (iAF > 998 && FO_START_VAL == HIGH) bRetVal=true;
  return bRetVal;
}

void FoInit(void)
{
  while (!FoStart())
  {
    Stepper_Z_step(-iStDZ);
  }
  ulFomSts=0;
}

