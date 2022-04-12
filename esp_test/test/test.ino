void setup() {
Serial.begin(9600);
}

void loop() {
int i = 1;
float f = i;
if(f == 1)
Serial.println("Hello,NodeMCU!");
}
