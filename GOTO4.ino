/*
 * GOTO4_7.ino Written by Igor Ovchinnikov 13/10/2016
 */

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#include "Config4_7.h" // Config4_7.h должен находиться в тойже папке, что GOTO4_7.ino

int imStepsXPS = iStepsXPS*iXStepX; //Микрошагов в секунду на двигателе X
int imStepsYPS = iStepsYPS*iYStepX; //Микрошагов в секунду на двигателе Y
int imStepsZPS = iStepsZPS*iZStepX; //Микрошагов в секунду на двигателе Z

unsigned long ulSPRA = iStepsDX*dRDX*iXStepX; //Микрошагов двигателя X на полный оборот оси прямого восхождения
unsigned long ulSPDE = iStepsDY*dRDY*iYStepX; //Микрошагов двигателя Y на полный оборот оси склонений

const unsigned long StarMSPS=86164091; //Милисекунд в Звездных сутках

double udRAStepsPMS=double(ulSPRA)/double(StarMSPS); //Микрошагов двигателя X на 1 мс

int iMovement = 0;   //Может пригодиться 
int iLastMovement=0; //Может пригодиться

unsigned long ulMilisec=0;   //Виртуальное время трекера
unsigned long ulPortTimer=0; //Таймер порта

boolean bDebug  = false;  //Режим отладки
boolean bRun    = false;  //Трекинг выключен изначально
boolean bLCD    = false;  //LCD врет
boolean bForceX = false;  //Ускоренный режим Х
boolean bForceY = false;  //Ускоренный режим Y
boolean bForceZ = false;  //Ускоренный режим Z
boolean bAlignment=false; //Монтировка не выровнена
boolean bFocus=false;     //Фокусер выключен

unsigned long ulRA=0;      //Текущее (исходное) значение прямого восхождения
unsigned long ulDE=0;      //Текущее (исходное) значение склонения
unsigned long ulToRA=0;    //Целевое значение прямого восхождения
unsigned long ulToDE=0;    //Целевое значение склонения

long ulFomSts=0;  //Микрошагов фокусера

const unsigned long MVRA = 0xFFFFFFFF;  //Максимальное значение величины прямого восхождения
const unsigned long MVDE = 0xFFFFFFFF;  //Максимальное значение величины склонения

unsigned long VRAperSTEP=MVRA/ulSPRA; //Единиц прямого восхождения на 1 шаг двигателя
unsigned long VDEperSTEP=MVDE/ulSPDE; //Единиц склонения на 1 шаг двигателя

unsigned long dVRAperSTEP=MVRA/StarMSPS*1000/imStepsXPS; //Поправка вращения Земли на 1 шаг ДПВ
unsigned long dVDEperSTEP=0; //Поправка (доворот) ДСК в единицах СК на 1 шаг ДСК

double udRaIncPMS=double(MVRA)/double(StarMSPS); //Увеличение прямого восходжения на 1 мс простоя монтировки

String STR= "", STR1="", STR2="";
String LCDString1="  Arduino GOTO4 ";
String LCDString2="  Ready to Use  ";

#include "GOTO4_7.h" // GOTO4_7.h должен находиться в тойже папке, что GOTO4_7.ino

int AskControl()
{
  AscFoSw();
  return AskJOY();
}

String GetString ()
{
  String STR="";
  char c;
  if (!Serial.available() && ((millis()-ulPortTimer) >= 1000)) {ulPortTimer=millis(); STR="e"; return STR;}
  while (Serial.available())  //если есть что читать;
  {
    c = Serial.read();       //читаем символ
    if (c!='\n' && c!='\r' ) //если это не спецсимвол прибавляем к строке
    STR += c;
   delay(1); //Необходимая задержка цикла, для синхронизации порта при 9600 бит/сек
  }
  return STR;
}

int GetSubStr ()
{
  int i;
  i=STR.indexOf(',');
  STR2=STR.substring(i+1);
  if (i<=1) STR1="";
  else STR1=STR.substring(1,i);
};

void action(String STRA)
{
  char cAction;
  cAction=STRA.charAt(0);
  switch (cAction)
  {
    case 'e': {Serial.print(HexTo8D(ulRA)); Serial.print(ulRA,HEX); Serial.print(","); Serial.print(HexTo8D(ulDE)); Serial.print(ulDE,HEX); Serial.print("#"); break;}
//    case 'd': {Serial.println(ulFomSts); break;}
    case 'r': {To_PRADEC(); Serial.print("#"); break;}
  };
}

int Force_X(boolean bForce)
{
 int iXSX=0; 
 if(!bForceX && bForce) //Включаем полношаговый режим
 {
   iXSX = 1; //Кратность шага драйвера X
   digitalWrite(DX_FORCE_PIN, LOW);
   imStepsXPS = iStepsXPS*iXSX; //Шагов в секунду на двигателе X
   ulSPRA = iStepsDX*dRDX*iXSX; //Шагов двигателя X на полный оборот оси прямого восхождения
   VRAperSTEP=MVRA/ulSPRA;      //Единиц прямого восхождения на 1 шаг двигателя
   dVRAperSTEP=MVRA/StarMSPS*1000/imStepsXPS; //Поправка вращения Земли на 1 шаг ДПВ
   bForceX=true;
 }
 if(bForceX && !bForce) //Включаем микрошаговый режим
 {
   iXSX = iXStepX; //Кратность шага драйвера X
   digitalWrite(DX_FORCE_PIN, HIGH);
   imStepsXPS = 500; //Микрошагов в секунду на двигателе X
   ulSPRA = iStepsDX*dRDX*iXSX; //Микрошагов двигателя X на полный оборот оси прямого восхождения
   VRAperSTEP=MVRA/ulSPRA;      //Единиц прямого восхождения на 1 шаг двигателя
   dVRAperSTEP=MVRA/StarMSPS*1000/imStepsXPS; //Поправка вращения Земли на 1 шаг ДПВ
   bForceX=false;
  }
}

int Force_Y(boolean bForce)
{
  int iYSX=0;
  if(!bForceY && bForce) //Включаем полношаговый режим
  {
    iYSX = 1; //Кратность шага драйвера Y
    digitalWrite(DY_FORCE_PIN, LOW);
    imStepsYPS = iStepsYPS*iYSX; //Шагов в секунду на двигателе Y
    ulSPDE = iStepsDY*dRDY*iYSX; //Шагов двигателя Y на полный оборот оси склонений
    VDEperSTEP=MVDE/ulSPDE;      //Единиц склонения на 1 шаг двигателя
    dVDEperSTEP=0;               //Поправка (доворот) ДСК в единицах СК на 1 шаг ДСК
    bForceY=true;
   }
  if(bForceY && !bForce) //Включаем микрошаговый режим
  {
    iYSX = iYStepX; //Кратность шага драйвера Y
    digitalWrite(DY_FORCE_PIN, HIGH);
    imStepsYPS = 500; //Микрошагов в секунду на двигателе Y
    ulSPDE = iStepsDY*dRDY*iYSX; //Микрошагов двигателя Y на полный оборот оси склонений
    VDEperSTEP=MVDE/ulSPDE;      //Единиц склонения на 1 шаг двигателя
    dVDEperSTEP=7;               //Поправка (доворот) ДСК в единицах СК на 1 шаг ДСК
    bForceY=false;
  }
}

int Force_Z(boolean bForce)
{
  int iZSX=0;
  if(!bForceZ && bForce) //Включаем полношаговый режим
  {
    iZSX = 1; //Кратность шага драйвера Z
    digitalWrite(DZ_FORCE_PIN, LOW);
    imStepsZPS = iStepsZPS*iZSX; //Шагов в секунду на двигателе Z
    bForceZ=true;
   }
  if(bForceZ && !bForce) //Включаем микрошаговый режим
  {
    iZSX = iZStepX; //Кратность шага драйвера Z
    digitalWrite(DZ_FORCE_PIN, HIGH);
    imStepsZPS = 500; //Микрошагов в секунду на двигателе Z
    bForceZ=false;
  }
}


void To_PRADEC(void)
{
  int DirectRA=0;
  int DirectDE=0;
  unsigned long uldRA=0;
  unsigned long uldDE=0;
  unsigned long ulStartMilis=millis();

  GetSubStr ();
  ulToRA=StrToHEX (STR1);
  ulToDE=StrToHEX (STR2);
  
  Force_X(true);
  Force_Y(true);
  
  if (ulToRA > ulRA) {uldRA = (ulToRA-ulRA); DirectRA=  1;}
  if (ulToRA < ulRA) {uldRA = (ulRA-ulToRA); DirectRA= -1;}
  if (uldRA > MVRA/2) {uldRA = MVRA-uldRA; DirectRA = -(DirectRA);}

  if (ulToDE > ulDE) {uldDE = (ulToDE-ulDE); DirectDE=  1;}
  if (ulToDE < ulDE) {uldDE = (ulDE-ulToDE); DirectDE= -1;}
  if (uldDE > MVDE/2) {uldDE = MVDE-uldDE; DirectDE = -(DirectDE);}
  
  if (uldRA > MVRA/2) return; //Ошибка в расчете шагов по прямому восхождению
  if (uldDE > MVDE/2) return; //Ошибка в расчете шагов по склонению
  
  while ((((uldRA > VRAperSTEP) && iStDX!=0) || ((uldDE > VDEperSTEP) && iStDY!= 0)) && bAlignment)
  {
    if (uldRA > VRAperSTEP)
    {
      if (DirectRA > 0) {Stepper_X_step(-iStDX); uldRA-=(VRAperSTEP+dVRAperSTEP); ulRA+=(VRAperSTEP+dVRAperSTEP);}
      if (DirectRA < 0) {Stepper_X_step( iStDX); uldRA-=(VRAperSTEP-dVRAperSTEP); ulRA-=(VRAperSTEP-dVRAperSTEP);}
      if (uldRA > MVRA/2) uldRA =0;
    } else Force_X(false);
    if (uldDE > VDEperSTEP)
    {
      if (DirectDE > 0) {Stepper_Y_step(-iStDY); uldDE-=(VDEperSTEP+dVDEperSTEP); ulDE+=(VDEperSTEP+dVDEperSTEP);}
      if (DirectDE < 0) {Stepper_Y_step( iStDY); uldDE-=(VDEperSTEP+dVDEperSTEP); ulDE-=(VDEperSTEP+dVDEperSTEP);}
      if (uldDE > MVDE/2) uldDE =0;
    } else Force_Y(false);
   if ((millis()-ulStartMilis)>1000) {bLCD=false; LCDPrint(); STR="e"; action(STR); ulStartMilis=millis();}
  }
  if (!bAlignment)  //Первая команда GOTO задает координаты наведения телескопа, без его реального перемещения
  {
    ulRA=ulToRA;
    ulDE=ulToDE;
    bAlignment=true;
  }
  bLCD=false;
 };

void reaction () //Обработка команд ПУ
  {
   int iKey=0, iLastKey=0;

// Здесь мы договариваемся, что функция int AskControl(),
// к чему бы она ни была привязана, возвращает при ее вызове следующие значения:

//   0 - когда ничего не надо делать
//   1 - когда надо сделать микрошаг вперед по оси Х
//  16 - когда надо сделать полныйшаг вперед по оси Х
//   4 - когда надо сделать микрошаг назад по оси Х
//  64 - когда надо сделать полныйшаг назад по оси Х
//   2 - когда надо сделать микрошаг вверх по оси У
//  32 - когда надо сделать полныйшаг вверх по оси У
//   8 - когда надо сделать микрошаг вниз по оси У
// 128 - когда надо сделать полныйшаг вниз по оси У
// 256 - включить/отключить трекинг

  do 
   {
    iMovement=0;
    iKey=AskControl();
    if(!bFocus)
     {
    if ((iKey &   4)==  4 && iStDX!=0) {Force_X(false); Stepper_X_step(-iStDX); iMovement=iMovement |   4;} // Микрошаг назад
    if ((iKey &  64)== 64 && iStDX!=0) {Force_X(true);  Stepper_X_step(-iStDX); iMovement=iMovement |  64;} // Полный шаг назад
    if ((iKey &   1)==  1 && iStDX!=0) {Force_X(false); Stepper_X_step( iStDX); iMovement=iMovement |   1;} // Микрошаг вперед
    if ((iKey &  16)== 16 && iStDX!=0) {Force_X(true);  Stepper_X_step( iStDX); iMovement=iMovement |  16;} // Полный шаг вперед  
    if ((iKey &   8)==  8 && iStDY!=0) {Force_Y(false); Stepper_Y_step( iStDY); iMovement=iMovement |   8;} // Микрошаг вниз
    if ((iKey & 128)==128 && iStDY!=0) {Force_Y(true);  Stepper_Y_step( iStDY); iMovement=iMovement | 128;} // Полный шаг вниз
    if ((iKey &   2)==  2 && iStDY!=0) {Force_Y(false); Stepper_Y_step(-iStDY); iMovement=iMovement |   2;} // Микрошаг вверх
    if ((iKey &  32)== 32 && iStDY!=0) {Force_Y(true);  Stepper_Y_step(-iStDY); iMovement=iMovement |  32;} // Полный шаг вверх    
     }
   else
     {
      if ((iKey &   8)==  8 && iStDZ!=0 && (ulFomSts < FO_END_STEP*long(iZStepX)))
         {Force_Z (false); Stepper_Z_step(iStDZ); ulFomSts+=1; iMovement=iMovement | 8;}          // Микрошаг фокусера F+
      if ((iKey & 128)==128 && iStDZ!=0 && (ulFomSts < (FO_END_STEP-1)*long(iZStepX)))
         {Force_Z (true);  Stepper_Z_step(iStDZ); ulFomSts+=iZStepX; iMovement=iMovement | 128;}  // Полный шаг фокусера F+
      if ((iKey &   2)==  2 && iStDZ!=0 && ulFomSts > 0)
         {Force_Z (false); Stepper_Z_step(-iStDZ); ulFomSts-=1; iMovement=iMovement |   2;}       // Микрошаг фокусера F-
      if ((iKey &  32)== 32 && iStDZ!=0 && ulFomSts >= iZStepX)
         {Force_Z (true);  Stepper_Z_step(-iStDZ); ulFomSts-=iZStepX; iMovement=iMovement |  32;} // Полный шаг фокусера F-
     }
    if ((iKey & 256)==256) 
     {
     if(!bRun) {bRun=true;  bLCD=false; bForceX = true; Force_X(false); ulMilisec=millis();} //Включить  ведение  (Tracking ON)
     else      {bRun=false; bLCD=false;} //Отключить ведение (Tracking OFF)
     iMovement=iMovement | 256;
     }
    if (iKey != iLastKey){LCDCOR(iKey); iLastKey=iKey;  bLCD=false;}
   } while (iKey!=0);
  if (iMovement!=0) ulMilisec=millis(); 
  LCDPrint();
 } 

void setup()
{
  lcd.init();           
  lcd.backlight();
  pinMode(FO_SENCE_PIN, INPUT_PULLUP); 
  pinMode(ENABLE_XYZ_PIN,  OUTPUT);  // ENABLE XYZ PIN
  digitalWrite(ENABLE_XYZ_PIN, LOW); // LOW
  pinMode(DX_STEP_PIN,  OUTPUT);     // DX STEP PIN
  digitalWrite(DX_STEP_PIN, LOW);    // LOW
  pinMode(DX_DIR_PIN,  OUTPUT);      // DX DIR PIN
  digitalWrite(DX_DIR_PIN, LOW);     // LOW
  pinMode(DX_FORCE_PIN,  OUTPUT);    // DX FORCE PIN
  digitalWrite(DX_FORCE_PIN, HIGH);  // HIGH
  pinMode(DY_STEP_PIN,  OUTPUT);     // DY STEP PIN
  digitalWrite(DY_STEP_PIN, LOW);    // LOW
  pinMode(DY_DIR_PIN,  OUTPUT);      // DY DIR PIN
  digitalWrite(DY_DIR_PIN, LOW);     // LOW
  pinMode(DY_FORCE_PIN,  OUTPUT);    // DY FORCE PIN
  digitalWrite(DY_FORCE_PIN, HIGH);  // HIGH
  pinMode(DZ_STEP_PIN,  OUTPUT);     // DZ STEP PIN
  digitalWrite(DZ_STEP_PIN, LOW);    // LOW
  pinMode(DZ_DIR_PIN,  OUTPUT);      // DZ DIR PIN
  digitalWrite(DZ_DIR_PIN, LOW);     // LOW
  pinMode(DZ_FORCE_PIN,  OUTPUT);    // DZ FORCE PIN
  digitalWrite(DZ_FORCE_PIN, HIGH);  // HIGH
  pinMode(LIHT_FOC_PIN, OUTPUT);     // Пин светодиода фокусера 
  analogWrite(LIHT_FOC_PIN,LOW);       // выключен
  pinMode(SW_FOC_SENCE, INPUT_PULLUP); // Сенсор кнопки фокусера
  pinMode(SW_JOY_SENCE, INPUT_PULLUP); // Сенсор кнопки джойстика
  pinMode(X_JOY_SENCE, INPUT);         // Сенсор оси X джойстика
  pinMode(Y_JOY_SENCE, INPUT);         // Сенсор оси Y джойстика
  pinMode(13,  OUTPUT);  // LED на плате ардуино
  digitalWrite(13, LOW); // Выключен
  Serial.begin(9600);    // Подключаем COM порт
  Serial.flush();        // Сбрасываем содержимое COM порта
  delay(100);
  AscFoSw();             // Определяем нажатие кнопки фокусера
  if(bFocus){            // Если нажата кнопка фокусера
   Force_Z (true);       // Ускоряем фокусер
   FoInit();}            // Переводим фокусер в исходную позицию
  else {ulFomSts=long(iZStepX)*FO_START_STEP;} // Считаем, что фокусер в исходной позиции
  ulMilisec=millis();    // Фиксируем время начала работы
 }

void loop()
{
 long LoopTime=0;
 long StepsNeed=0;
 long RaIncNeed=0;
 STR = GetString();
 action(STR);
 reaction ();
 LoopTime=millis()-ulMilisec;
 if(bRun)
 {
 Force_X(false); //Микрошаговый режим
 StepsNeed=LoopTime*udRAStepsPMS;
 if(StepsNeed>=1)
  {
   digitalWrite(13, HIGH); // Зажигаем LCD
   Stepper_X_step(StepsNeed*iStDX);  // Шагаем
   ulMilisec+=double(StepsNeed)/udRAStepsPMS; // Виртуальное время трекера
   digitalWrite(13, LOW); // Тушим LCD
  }
 if (bDebug) {Serial.print(" StepsNeed "); Serial.print(StepsNeed); Serial.print(" udRAStepsPMS "); Serial.println(udRAStepsPMS);}
 }
 else
 {
  if (bAlignment)
  {
   LoopTime=millis()-ulMilisec;
   RaIncNeed=LoopTime*udRaIncPMS;
   if(RaIncNeed>=MVRA/24/60/60)
   {
    ulRA+=RaIncNeed;
    ulMilisec=millis();
    bLCD=false;
   }
  } 
 }
 LCDPrint();
}

