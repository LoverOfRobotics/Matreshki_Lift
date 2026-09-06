#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(9, 10);  // CE, CSN
const uint64_t pipe = 0xA85F57EF29LL;

#define MotTachoPin1 2
#define MotTachoPin2 4
#define MotA 5
#define MotB 6
#define MotPWM 3
#define ServoPin 8
#define Button 7

Servo rampServo;

volatile long MotTachos = 0;
long now_tachos = 0;
long tachos_down = 5000;   //Модуль числа

const byte ramp_open = 30;
const byte ramp_closed = 150;

byte nextRadioByte = 10;
void SendRadio(){
  Serial.println("Sending: " + String(nextRadioByte));
  for (int i = 0; i < 15; i++){
    // Serial.println("Sending: " + String(nextRadioByte));
    if (radio.write(&nextRadioByte, 1) == false)
      Serial.println("Ошибка");
    delay(80);
  }
  nextRadioByte++;
  delay(250);
}

void ClearRadioBuff(){
  byte receivedData = 0;
  while (radio.available()) {           // если есть принятые данные
  radio.read(&receivedData, 1);    // читаем 1 байт
  }
}
void WaitForAllOK(long long timeout_ms = 5000){
  Serial.println("Waiting for all OK"); 
  ClearRadioBuff();
  delay(1500);    //Ждем, чтоб все предыдущее закончилось
  radio.startListening();
  radio.openReadingPipe(0, pipe);

  byte count = 0;
  bool nums[4] = {false, false, false, false};
  long long start_ms = millis();
  byte receivedData = 0;

  while (count < 4 && millis() - start_ms < timeout_ms) {     //change 3 to 4 !!!
    if (count == 0) start_ms = millis();

    if (radio.available()) {           // если есть принятые данные
      radio.read(&receivedData, 1);    // читаем 1 байт
      Serial.println("Received: " + String(receivedData));
      switch (receivedData)
      {
      case 1:
        if (nums[0] == false) count++;
        nums[0] = true;
        break;
      case 2:
        if (nums[1] == false) count++;
        nums[1] = true;
        break;
      case 3:
        if (nums[2] == false) count++;
        nums[2] = true;
        break;
      case 4:
        if (nums[3] == false) count++;
        nums[3] = true;
        break;
      default:
        break;
      }
    }
    delay(50);
  }
  Serial.println("Count: " + String(count));

  radio.closeReadingPipe(0);
  radio.openWritingPipe(pipe);
  radio.stopListening();
  Serial.println("Got all OKs");
}

void MotTacho(){
  if (digitalRead(MotTachoPin1) == 1){
    if (digitalRead(MotTachoPin2) == 0){
      MotTachos += 1;
    }
    else{
      MotTachos += 1;
    }
  }
  else {
    if (digitalRead(MotTachoPin2) == 0){
      MotTachos += 1;
    }
    else{
      MotTachos += 1;
    }
  }
}


void LiftDown(int maxSpeed = 200){
  MotTachos = 0;  //Сбрасываем счётчик импульсов
  while (MotTachos < tachos_down){
    analogWrite(MotPWM, (int)maxSpeed);
    digitalWrite(MotA, LOW);
    digitalWrite(MotB, HIGH);
  }
  digitalWrite(MotA, 0);
  digitalWrite(MotB, 0);
}

void LiftUp(int maxSpeed = 200, bool plus_tachos = true){
  MotTachos = 0;  //Сбрасываем счётчик импульсов
  if (plus_tachos) tachos_down += 500;
  while (MotTachos < tachos_down){
    analogWrite(MotPWM, (int)maxSpeed);
    digitalWrite(MotA, HIGH);
    digitalWrite(MotB, LOW);
  }
  if (plus_tachos) tachos_down -= 500;
  digitalWrite(MotA, 0);
  digitalWrite(MotB, 0);
}

void rampClose(){
  rampServo.write(ramp_open);
  for (byte i = ramp_open; i < ramp_closed; i++){
    rampServo.write(i);
    delay(20);
  }
  rampServo.write(ramp_closed);
  delay(250);
}

void rampOpen(){
  rampServo.write(ramp_closed);
  for (byte i = ramp_closed; i > ramp_open; i--){
    rampServo.write(i);
    delay(20);
  }
  rampServo.write(ramp_open);
  delay(250);
}


void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.setChannel(0x67);
  radio.setDataRate(RF24_250KBPS);   // скорость 1 Мбит/с
  radio.setPALevel(RF24_PA_MAX);    	// Уровень питания усилителя RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX ((RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm).
  radio.setAutoAck(false);
  radio.setPayloadSize(1);
  radio.openWritingPipe(pipe);
  radio.stopListening();
  
  Serial.println("Лифт");


  pinMode(MotTachoPin1, INPUT);
  pinMode(MotTachoPin2, INPUT);
  attachInterrupt(digitalPinToInterrupt(MotTachoPin1), MotTacho, CHANGE);

  pinMode(MotA, OUTPUT);
  pinMode(MotB, OUTPUT);
  pinMode(MotPWM, OUTPUT);

  pinMode(Button, INPUT_PULLUP);

  rampServo.attach(ServoPin);
  rampServo.write(90);

  while (true) {};
  // rampServo.write(ramp_closed);


  //Поднятие в начале
  if (digitalRead(Button) == 0)   //Нажата
    LiftUp();
  
  delay(250);

  //===Код для демонстрации===
  while (digitalRead(Button) == 1) {}
  Serial.println("Button");
  SendRadio();
  nextRadioByte = 10;
  delay(400);
  
  while (digitalRead(Button) == 1) {}
  Serial.println("Button");
  rampOpen();
  LiftDown();
  delay(1000);
  LiftUp(200, false);
  rampClose();
  delay(1000);
  

  //===Код для демонстрации===


  while (digitalRead(Button) == 1) {}
  Serial.println("Button");
  delay(7000);

  rampOpen();
  SendRadio();
  delay(10000);
  LiftDown();
  delay(10000);
  // LiftUp();
  // rampClose();
  
  // rampClose();
  // rampOpen();
  // LiftDown();
  // LiftUp();
  WaitForAllOK();

  while (true){
    SendRadio();
    Serial.println(nextRadioByte);
    if (nextRadioByte == 18) break;
    WaitForAllOK();
  }
  delay(10000);
  LiftUp();

  //Ждём кнопку
  // while (digitalRead(Button) == 1) {}
  delay(20000);
  rampClose();

}

void loop() {
  // Serial.println(MotTachos);

}