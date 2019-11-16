#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFA, 0xED};
IPAddress ip(192, 168, 1, 120);
unsigned int localPort = 420;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,

EthernetUDP Udp;

// 2, 3, 4, 5, 6, 7
bool lightStatus[] = {false, false, false, false, false, false};

void updateLights(int pin, bool on) {
  if (pin >= 2 && pin < 8) {
    digitalWrite(pin, on ? HIGH : LOW);
    lightStatus[pin - 2] = on;
  }
}

bool lightsAreOn(int pin) {
  return lightStatus[pin - 2];
}

const char * lightStatusHuman (int pin) {
  return lightsAreOn(pin) ? "on" : "off";
}

void printIP(IPAddress ipAddress) {
  for (int i = 0; i < 4; i++) {
    Serial.print(ipAddress[i], DEC);
    if (i < 3) {
      Serial.print(".");
    }
  }
}

//void printListening () {
//  Serial.print("Listining on ");
//  printIP(ip);
//
//  char out[8];
//  sprintf(out, ":%d", localPort);
//  Serial.println(out);
//}

void printRemote() {
  printIP(Udp.remoteIP());
  Serial.print(":");
  Serial.println(Udp.remotePort());
}

void printPacket() {
  char out[3];
  sprintf(out, "%x %x", packetBuffer[0], packetBuffer[1]);
  Serial.println(out);
}

void setup() {
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.begin(9600);
//  printListening();

  for (int i = 2; i <= 7;i++) {
    pinMode(i, OUTPUT);
    updateLights(i, lightsAreOn(i));
  }
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    Serial.print("(");
    Serial.print(packetSize);
    Serial.print(") ");
    printRemote();

    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);

    if (packetSize == 1) {
      Serial.println("Got all status request");
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      char out[6];
      for (int i=2;i<=7;i++) {
        sprintf(out, "%d %s\n", i, lightStatusHuman(i));
        Udp.write(out);
      }
      Udp.endPacket();
      return;
    }

    if (packetSize == 2) {
      printPacket();
      int pin = packetBuffer[0];
      byte msg = packetBuffer[1];
      switch(msg) {
      case 0:
        Serial.println("Got status request");
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        char out[2];
        sprintf(out, "%d ", pin);
        Udp.write(out);
        Udp.write(lightsAreOn(pin) ? "on" : "off");
        Udp.endPacket();
        break;
      case 1:
        if (!lightsAreOn(pin)) break;
        Serial.println("Turning lights off");
        updateLights(pin, false);
        break;
      default:
        if (lightsAreOn(pin)) break;
        Serial.println("Turning lights on");
        updateLights(pin, true);
      }
    }
  }
}
