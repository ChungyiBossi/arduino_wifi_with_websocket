// Board: Matrix MR101

int MR101_A1A = 16;
int MR101_A1B = 4;
int val_A1A;
int val_A1B;
void setup() {
  // put your setup code here, to run once:
  pinMode(MR101_A1A, INPUT);
  pinMode(MR101_A1B, INPUT);

  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

  if (val_A1A != digitalRead(MR101_A1A)) {
    Serial.print("Val A change..., from: ");
    Serial.print(val_A1A);
    Serial.print(" to ");
    Serial.println(digitalRead(MR101_A1A));
    val_A1A = digitalRead(MR101_A1A);   
  }

  if (val_A1B != digitalRead(MR101_A1B))
  {
    Serial.print("Val B change..., from: ");
    Serial.print(val_A1B);
    Serial.print(" to ");
    Serial.println(digitalRead(MR101_A1B));
    val_A1B = digitalRead(MR101_A1B);
    if (val_A1B == 1) {
      Serial.println("Rotating....");
    } else {
      Serial.println("Rotate finish....");
    }
      
  } 


}
