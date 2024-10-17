#include <WiFiNINA.h>
#include <DFRobot_AS7341.h>

char ssid[]= "SSID";
char pass[]= "PASSWORD";
int status = WL_IDLE_STATUS;

//Enter the IP adress of your device on which server is hosted
IPAddress server(192,168,1,20); // create an object server on stack memory using the class IPAddress

WiFiClient client; // initialize the wifi client library

//Define AS7341 library
DFRobot_AS7341 as7341;
//Define AS7341 data objects:
DFRobot_AS7341::sModeOneData_t data1; // data storage for first set of channels (mode:eF1F4ClearNIR)
DFRobot_AS7341::sModeTwoData_t data2;// data storage for 2nd set of channels (mode:eF5F8ClearNIR)

//Define potentio meter pin:
#define pot A0

//Define the class button pins:
#define class_1 2
#define class_2 3
#define class_3 4
#define class_4 5

// Define the control LED pins"
#define green 6
#define red 7

//Define data holders 
int pot_val,class_1_val,class_2_val,class_3_val,class_4_val;


void setup()
 {
  Serial.begin(9600);
//Class button settings
//ENSURING that when button is not pressed the the pins read HIGH
pinMode(class_1,INPUT_PULLUP);
pinMode(class_2,INPUT_PULLUP);
pinMode(class_3,INPUT_PULLUP);
pinMode(class_4,INPUT_PULLUP);
//Control LED settings:
pinMode(green,OUTPUT);
pinMode(red,OUTPUT);

//Detect i I2C can communicate properly
while(as7341.begin()!=0)
{
  Serial.println("I2C init failed, please check if the wire connection is correct");
  delay(1000);

}

//Enable the builtin LED on the AS7341 sensor.
as7341.enableLed(true);

//Check for the Wifi modules:
if(WiFi.status()==WL_NO_MODULE)
{
  Serial.println("Connection Failed!");
  while(true);
}
//Atttenpt to connect to the WiFi network:
while(status!=WL_CONNECTED)
{
  Serial.println("Attempting to connect to Wifi!! ");
  status = WiFi.begin(ssid,pass);
  delay(10000);
}

}

void loop() {
  read_controls();
  adjust_brigtness();

  //Start spectrum measurement:
  //Channel mapping mode: 1 (eF1F4ClearNIR)
  as7341.startMeasure(as7341.eF1F4ClearNIR);
  //Read the value of sensor data channel 0~5
  data1= as7341.readSpectralDataOne();
  //channel mapping mode : 2 eF5F8ClearNIR
  as7341.startMeasure(as7341.eF5F8ClearNIR);
  //read the value of sensor data channel 6~nir
  data2= as7341.readSpectralDataTwo();

  //print data:
  Serial.print("F1(405-425nm): "); Serial.println(data1.ADF1);
  Serial.print("F2(435-455nm): "); Serial.println(data1.ADF2);
  Serial.print("F3(470-490nm): "); Serial.println(data1.ADF3);
  Serial.print("F4(505-525nm): "); Serial.println(data1.ADF4);
  Serial.print("F5(545-565nm): "); Serial.println(data2.ADF5);
  Serial.print("F6(580-600nm): "); Serial.println(data2.ADF6);
  Serial.print("F7(620-640nm): "); Serial.println(data2.ADF7);
  Serial.print("F8(670-690nm): "); Serial.println(data2.ADF8);
  // CLEAR and NIR:
  Serial.print("Clear_1: "); Serial.println(data1.ADCLEAR);
  Serial.print("NIR_1: "); Serial.println(data1.ADNIR);
  Serial.print("Clear_2: "); Serial.println(data2.ADCLEAR);
  Serial.print("NIR_2: "); Serial.println(data2.ADNIR);
  Serial.print("\n------------------------------\n");
  delay(1000); 
 //send this data to PHP web application depending on the selected ripeness class[0-3]
 
 if(!class_1_val)
 make_a_get_request("/vegAndFruitesDataLogger/", "0");
 
 if(!class_2_val)
 make_a_get_request("/vegAndFruitesDataLogger/", "1");
 
 if(!class_3_val)
 make_a_get_request("/vegAndFruitesDataLogger/", "2");
 
 if(!class_4_val)
 make_a_get_request("/vegAndFruitesDataLogger/", "3");
 


}

void read_controls()
{
  //Potentiometer:
  pot_val = analogRead(pot);
  // class buttons:
  class_1_val= digitalRead(class_1);
  class_2_val= digitalRead(class_2);
  class_3_val= digitalRead(class_3);
  class_4_val= digitalRead(class_4);
  
}

void adjust_brigtness()
{
  //set brigntness of in built led according to the value of potentiometer(1~20 corresponds to current 4mA,6mA,8mA.....,42mA)
  int brigtness = map(pot_val,0,1023,1,21); // scale the value of potential wiz in the range of 0~1023 to 1~20
  Serial.print("\nBrigtness: ");
  Serial.println(brigtness);
  as7341.controlLed(brigtness);
}

void make_a_get_request(String application, String _class)
{
  // Connect to the web application named vegAndFruitesDataLogger.
  if(client.connect(server,80))
  {
    //if successful:
    Serial.println("\n\nConnected to the server!");
    //Create the query string:
    String query = application + "?F1="+data1.ADF1+"&F2="+data1.ADF2+"&F3="+data1.ADF3+"&F4="+data1.ADF4+"&F5="+data2.ADF5+"&F6="+data2.ADF6+"&F7="+data2.ADF7+"&F8="+data2.ADF8+"&nir_1="+data1.ADNIR+"&nir_2="+data2.ADNIR+"&class="+_class;
    //Make an HTTP Get request:
    client.println("GET "+ query + " HTTP/1.1");
    client.println("Host: 192.168.1.20");
    client.println("Connection: close");
    client.println();
  }
  else
  {
    Serial.println("Server Error!");
    digitalWrite(red,HIGH);
  }
  delay(2000); // wait 2 seconds after connection...
  // get the response from web application
  String response="";
  while(client.available())
  {
    char c = client.read(); // read byte by byte
    response+= c;
  }
  if(response!= "")
  {
    Serial.println(response);
    Serial.println("\n");
    //check whether the transferred data is inserted successfully or not:
    if(response.indexOf("Data Inserted Successfully!")>=0)
    {
      digitalWrite(green,HIGH);
    }
    else
    {
      digitalWrite(red,HIGH);
    }
  }
  //Turn off the leds
  delay(3000);
  digitalWrite(green,LOW);
  digitalWrite(red,LOW);
}






