#include <Servo.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x8E, 0x8D, 0xBE, 0x8F, 0xFE, 0xEE };
char server[] = "ec2-54-213-70-98.us-west-2.compute.amazonaws.com";
EthernetClient client;

int serial = 0;
int inValue = 90;
int outValue = 0;
int sensorNum = -1;
Servo myservo8;
Servo myservo9;

char *req;
char contype[] = "Content-Type: application/x-www-form-urlencoded";
char conclose[] = "Connection: close";

void setup(){
  req = "GET /   HTTP/1.0";
  Serial.begin(9600);
  Serial.println("Setup Beginning");
  myservo8.attach(8);
  myservo9.attach(9);
  
  if(Ethernet.begin(mac)==0){
    Serial.println("Failed to configure Ethernet with DHCP");
    while(true);
  }
  else {
    Serial.println("Phew");
  }
  delay(1000);
  Serial.println("connecting...");
  
  while(!client.connect(server,4000)){
    Serial.println("Connection failed, trying again in 1s");
    delay(1000);
  }
  
  Serial.println("connected");
  client.println("GET /sensors/num HTTP/1.0");
  client.println(contype);
  client.println(conclose);
  client.println();
  
  char c = 0;
  while(sensorNum == -1){
    if(client.available()){
      c = client.read();
      sprint(c);
    }
    else if(c)
      sensorNum = c-49;
    else delay(100);
  }
  Serial.print("Got sensor number: ");
  Serial.println((char)(sensorNum+48));
  if(sensorNum<=9){
    req[5] = (char)(sensorNum+48);
  }
  else{
    req[5] = (char)((sensorNum/10)+48);
    req[6] = (char)((sensorNum%10)+48);
  }
  sprintln(req);
  stop();
}

void loop(){
  while (!client.connected()) {
    stop();
    sprintln("Connecting...");
    if(client.connect(server, 4000))
      break;
  }
  sprintln("Sending request");
  request();
  inValue = httpTrim();
  
  outValue = map(inValue, 0, 1023, 0, 179);
  //Serial.println(String("Value: ") + outValue);
  
  myservo8.write(outValue);
  myservo9.write(outValue);
  
  delay(50);
}

void request(){
  client.println(req);
  client.println(contype);
  client.println(conclose);
  client.println();
}

int httpTrim(){
  int ret = -1;
  while(ret == -1){
    //sprintln("Loop 1");
    while(client.available()){
      char c = client.read();
      sprint(c);
      if(c == '\n' || c =='\r'){
        c = client.read();
        sprint(c);
        if(c == '\n' || c == '\r'){
          sprintln("Loop 2");
          c = client.read();
          if(c == '\n' || c == '\r'){
            sprintln("Loop 3");
            c = client.read();
            sprintln(c);
            while(!(c>='0' && c<='9')){
              c = client.read();
              sprintln(c);
            }
            sprint("Loop 4");
            while(c >= '0' && c <= '9'){
              sprint("Got number: ");
              sprintln(c);
              int x = c-48;
              if(ret>=0){
                ret*=10;
                ret+=x;
              }
              else ret = x;
              c = client.read();
            }
            return ret;
          }
        }
      }
    }
  }
}

void stop(){
  while(client.available())
    client.read();
  client.flush();
  if(!client.connected()){
    client.stop();
    sprintln("Stopping...");
  }
}

void sprintln(char str[]){
  if(serial)
    Serial.println(str);
}

void sprintln(char c){
  if(serial)
    Serial.println(c);
}

void sprint(char str[]){
  if(serial)
    Serial.print(str);
}

void sprint(char c){
  if(serial)
    Serial.print(c);
}
