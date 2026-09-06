#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(9, 10);  // CE, CSN
const uint64_t pipe = 0xA85F57EF29LL;

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
  
  Serial.println("Лифт - Режим только передатчика");

  while (true){
    SendRadio();
    Serial.println(nextRadioByte);
    WaitForAllOK();
  }
  
}

void loop() {
  // Serial.println(MotTachos);

}