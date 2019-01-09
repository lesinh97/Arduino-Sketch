
#define LED_PIN 13
 
#define CMD_SEND_BEGIN  "AT+CIPSEND=0"
#define CMD_SEND_END    "AT+CIPCLOSE=0"
 
#define TDHshop_PROTOCOL_HTTP     80
#define TDHshop_PROTOCOL_HTTPS    443
#define TDHshop_PROTOCOL_FTP      21
#define TDHshop_PROTOCOL_CURRENT  TDHshop_PROTOCOL_HTTP
 
#define TDHshop_CHAR_CR     0x0D
#define TDHshop_CHAR_LF     0x0A
 
#define TDHshop_STRING_EMPTY  ""
 
#define TDHshop_DELAY_SEED  1000
#define TDHshop_DELAY_1X    (1*TDHshop_DELAY_SEED)
#define TDHshop_DELAY_2X    (2*TDHshop_DELAY_SEED)
#define TDHshop_DELAY_3X    (3*TDHshop_DELAY_SEED)
#define TDHshop_DELAY_4X    (4*TDHshop_DELAY_SEED)
#define TDHshop_DELAY_0X    (5*TDHshop_DELAY_SEED)
bool hasRequest = false;


 void setup()
{
  
  delay(TDHshop_DELAY_0X);
  Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, LOW);
    initESP8266();
}
 void loop()
{
  while(Serial.available())
  {   
    bufferingRequest(Serial.read());
  }
    if(hasRequest == true) 
  {
    String htmlResponse = "<!doctype html>"
    "<html>"
      "<head>"
        "<title>TDHshop.com.vn</title>"
      "</head>"
      "<body>"
         "<h1>ESP8266 DEMO</h1>"
        "<h2>Temperature: </h2>"
        "<p>" +   "&#8451" + "</p>"
        "<h2>Humidity: </h2>"
        "<p>" +   "%" + "</p>"
       
        "<form action='' method='GET'>"
          "<input type='radio' name='LED' name='TDHshop' value='TDHshop_ON' /> LED ON<br/>"
          "<input type='radio' name='LED' name='D2K' value='TDHshop_OFF' /> LED OFF<br/>"
          "<input type='submit' value='Submit' />"
        "</form>"
      "</body>"
     "</html>";
    
    String beginSendCmd = String(CMD_SEND_BEGIN) + "," + htmlResponse.length();
    deliverMessage(beginSendCmd, TDHshop_DELAY_1X);
    deliverMessage(htmlResponse, TDHshop_DELAY_1X);
    deliverMessage(CMD_SEND_END, TDHshop_DELAY_1X);
    hasRequest = false;
  }
}
 void initESP8266()
{
  deliverMessage("AT+RST", TDHshop_DELAY_2X);
  deliverMessage("AT+CWMODE=2", TDHshop_DELAY_3X);
  deliverMessage("AT+CWSAP=\"MINHDEPTRAI\",\"123456789\",1,4", TDHshop_DELAY_3X);
  deliverMessage("AT+CIFSR", TDHshop_DELAY_1X);
  deliverMessage("AT+CIPMUX=1", TDHshop_DELAY_1X); // d? cho phép các k?t n?i TCP
  deliverMessage(String("AT+CIPSERVER=1,") + TDHshop_PROTOCOL_CURRENT, TDHshop_DELAY_1X);  //d? t?o 1 TCP server
}
 void bufferingRequest(char c)
{
  static String bufferData = TDHshop_STRING_EMPTY;
   switch (c)
  {
    case TDHshop_CHAR_CR:
      break;
    case TDHshop_CHAR_LF:
    {
      TDHshopProcedure(bufferData);
      bufferData = TDHshop_STRING_EMPTY;
    }
      break;
    default:
      bufferData += c;
  }
} 
 void TDHshopProcedure(const String& command)
{ 
  hasRequest = command.startsWith("+IPD,");
  
  if(command.indexOf("TDHshop_OFF") != -1)
  { 
    digitalWrite(LED_PIN, LOW);
  }
  else if(command.indexOf("TDHshop_ON") != -1)
  { 
    digitalWrite(LED_PIN, HIGH);
  }
}
 void deliverMessage(const String& msg, int dt)
{
  Serial.println(msg);
  delay(dt);

}
