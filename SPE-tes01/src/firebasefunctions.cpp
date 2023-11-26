#include "header.h"

void firebaseSetup(){
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the sevice account credentials and private key (required) */
  config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
  config.service_account.data.project_id = FIREBASE_PROJECT_ID;
  config.service_account.data.private_key = PRIVATE_KEY;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  firebaseData.setBSSLBufferSize(12960 /* Rx buffer size in bytes from 512 - 16384 */, 12960 /* Tx buffer size in bytes from 512 - 16384 */);

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);
}

void dataLogging (){
  bool hasUploaded = false; // indikator data log sudah dilakukan/belum
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }

  String time = timeClient.getFormattedTime();
  currentHour = timeClient.getHours();
  currentMinute = timeClient.getMinutes();
  currentSecond = timeClient.getSeconds();
  daynumber = timeClient.getDay();
 
  dateFormat = timeClient.getFormattedDate();
  date = dateFormat.substring(0, 10);

  if (daynumber == 1) {
    day = "Mon";
  }
  else if (daynumber == 2) {
    day = "Tue";
  }
  else if (daynumber == 3) {
    day = "Wed";
  }
  else if (daynumber == 4) {
    day = "Thu";
  }
  else if (daynumber == 5) {
    day = "Fri";
  }
  else if (daynumber == 6) {
    day = "Sat";
  }
  else if (daynumber == 7) {
    day = "Sun";
  }
  
  Firebase.getInt(firebaseData, hourPath);
  hour = firebaseData.intData();
  Firebase.getInt(firebaseData, minutePath);
  minute = firebaseData.intData();

  Serial.print("Data Logging: ");
  Serial.println(hasUploaded);
  Serial.println("Scheduled time: ");
  Serial.print(hour);
  Serial.println(minute);
  Serial.println("Current time: ");
  Serial.print(currentHour);
  Serial.println(currentMinute);

  nextMinute = minute + 1 % 60;
  if (nextMinute == 0) {
    nextHour = hour + 1 % 24;
  }
  else {
    nextHour = hour;
  }

  if (currentHour == hour && currentMinute == minute && currentSecond >= 0 && currentSecond <= 59 && hasUploaded == false)
    {
      Serial.print("we uploading data log\n");
      Firebase.getInt(firebaseData, logIndexPath);
      int logIndex = firebaseData.intData();
      int nextIndex = logIndex + 1;
      std::string indexPath = "/LogTest/" + std::to_string(logIndex);
    
      json.clear();
      json.set("/fullness", percentage);
      json.set("/weight", kg);
      json.set("/status", status);
      json.set("/date", date);
      json.set("/day", day);
      json.set("/time", time);

      Firebase.setJSON(firebaseData, indexPath, json);
      Firebase.setInt(firebaseData, logIndexPath, nextIndex);
      hasUploaded = true; // Set the flag to indicate that upload has been done
    }
    else if (currentHour == nextHour && currentMinute == nextMinute && currentSecond >= 0 && currentSecond <= 59 && hasUploaded == true) {
      Serial.print("ayy we done logging\n");
      hasUploaded = false; // Reset the flag to allow upload in the next interval
    }

}

void sendFirebase() {
  Serial.print("Firebase connection status: ");
  Serial.println(Firebase.ready() ? "true" : "false");
  // Check if Firebase connection is ready
  if (Firebase.ready()) {
    if (percentage >= 95) {
      pickupStatus = "Ready for pickup";
    } 
    else {
      pickupStatus = "Not ready yet";
    }

    json.clear();
    json.set("/capacity1", percentage);
    json.set("/weight1", kg);
    json.set("/status", status);
    json.set("/pick", pickupStatus);
    json.set("/lat", latitude);
    json.set("/long", longitude);
    Firebase.updateNode(firebaseData, bin1Path, json);

    if (pickupStatus != checkPickupStatus) {
      checkPickupStatus = pickupStatus;
      
      if (checkPickupStatus == "Ready for pickup") {
        Firebase.getInt(firebaseData, bin1PickCount);
        int counter = firebaseData.intData();
        int counterPlus = counter + 1;
        Firebase.setInt(firebaseData, bin1PickCount, counterPlus);
      } 
      else if (checkPickupStatus == "Not ready yet") { 
        Firebase.getInt(firebaseData, bin1PickCount);
        int counter = firebaseData.intData();
        int counterMin = counter - 1;
        int updatedCounter = max(counterMin, 0);
        Firebase.setInt(firebaseData, bin1PickCount, updatedCounter);
      }
    }
    dataLogging();
  } 
  else {
    Serial.println("Firebase not ready");
    firebaseSetup();
  }
}

