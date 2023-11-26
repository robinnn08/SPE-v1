#include "header.h"

void Open_Bin() {
  if (digitalRead(IR_PIN) == HIGH) {
    servo.write(180);
  }
  else {
    servo.write(0);
  }
}

void getLocation() {
  if (ss.available() > 0) {
    Serial.println("GPS Serial Connection Available");
    gps.encode(ss.read());
    if (gps.location.isValid()) {
      latitude = (gps.location.lat());
      longitude = (gps.location.lng());
    }
    else {
      latitude = 0.0;
      longitude = 0.0;
    }
  } 
  else {
    Serial.println("GPS Serial Connection Not Available");
  }
}

void getReadings() {
  // Get Percentage
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  jarak = duration * SOUND_SPEED / 2;

  percentage = (height - jarak) * 100 / height;

  // Get Weight
  if (!scale.is_ready()) {
    Serial.println("HX711 not found.");
  }
  else {
    weight = scale.get_units();
    kg = weight / 1000;
  }

  if (percentage >= 95) {
    pickupStatus = "Ready for pickup";
    status = "Available";
  }
  else {
    pickupStatus = "Not ready yet";
    status = "Full";
  }
  getLocation();
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" g");

  Serial.print("Kilos: ");
  Serial.print(kg);
  Serial.println(" kg");

  Serial.print("Percentage: ");
  Serial.print(percentage);
  Serial.println(" %");

  Serial.print("Distance: ");
  Serial.print(jarak);
  Serial.println(" cm");

  Serial.print("Latitude: ");
  Serial.println(latitude, 6);
  Serial.print("Longitude: ");
  Serial.println(longitude, 6);

  Serial.println();
}

void Seven_Segment() {
  if (!isnan(weight) && !isnan(percentage)) {
    wdigit1 = int(kg) / 10;
    wdigit2 = int(kg) % 10;
    wfraction1 = int(kg * 10) % 10;
    wfraction2 = int(kg * 100) % 10;

    disp.Number(8, wdigit1);
    disp.Numberdp(7, wdigit2);
    disp.Number(6, wfraction1);
    disp.Number(5, wfraction2);

    pdigit1 = int(percentage) / 10;
    pdigit2 = int(percentage) % 10;

    disp.Number(3, pdigit1);
    disp.Number(2, pdigit2);
  }
  else {
    disp.Clear(); 
  }
}

void powerSwitching() {
  bool hasSwitched = false;
  
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }

  int relayHour = timeClient.getHours();
  
  if (relayHour >= 18 && relayHour < 7 && hasSwitched == false) {
    digitalWrite(RELAY, HIGH);
    hasSwitched = true;
  }
  else if (relayHour >= 7 && relayHour < 18 && hasSwitched == true) {
    digitalWrite(RELAY, LOW);
    hasSwitched = false;
  }
}
