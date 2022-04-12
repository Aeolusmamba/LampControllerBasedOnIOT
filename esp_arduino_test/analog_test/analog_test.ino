boolean pushButton;
int ledPin = 10;
int button = 12;
int brightness = 128;
bool left = false;  //left == true => to left；left == false => to right

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  pinMode(button, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  pushButton = digitalRead(button);
  if(pushButton == LOW){
    if(!left){
      brightness++;
      if(brightness == 255){
        left = true;
        brightness--; 
      }
    }else{
      brightness--;
      if(brightness == 0){
        left = false;
        brightness++;
      }
    }
    analogWrite(ledPin, brightness);
    Serial.println(brightness);
    delay(10);
  }
}
