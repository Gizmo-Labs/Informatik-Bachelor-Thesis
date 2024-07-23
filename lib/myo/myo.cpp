#include "myo.h"

/* ENABLE SERIAL DEBUGGING BY SETTING myo.debug = true, **REQUIRES Serial.begin()** */

// The remote service we wish to connect to.
static NimBLEUUID serviceUUID("d5060001-a904-deb9-4748-2c7f4a124842");
static NimBLERemoteCharacteristic *pRemoteCharacteristic;
static NimBLEAddress *pServerAddress;
static NimBLEAdvertisedDevice *advDevice;
static uint32_t scanTime = 0 * 1000; // In milliseconds, 0 = scan forever

// Initialize status variables
boolean armband::connected = false;
boolean armband::detected = false;
boolean armband::debug = false;

/********************************************************************************************************
    BLUETOOTH CALLBACKS
 ********************************************************************************************************/

// Scan for BLE servers and find the first one that advertises the Myo ID service
class MyoAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{

  // Called for each advertising BLE server.
  void onResult(NimBLEAdvertisedDevice *advertisedDevice)
  {

    armband::debug ? Serial.print("Advertised Device found: ") : 0;
    armband::debug ? Serial.println(advertisedDevice->toString().c_str()) : 0;

    // We have found a Myo, check if it contains the service ID we are looking for.
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID))
    {

      armband::debug ? Serial.println("Found our Service") : 0;

      // Stop scanning
      NimBLEDevice::getScan()->stop();

      // Save the device reference in a global for the client to use
      advDevice = advertisedDevice;

      // Update detection status
      armband::detected = true;
    }
  }
};

// Set the connect and disconnect behaviours
class MyoClientCallbacks : public NimBLEClientCallbacks
{

  void onConnect(NimBLEClient *pClient)
  {
    if (armband::debug)
      Serial.println("Connected");

    // Update connection status
    armband::connected = true;
  }

  void onDisconnect(NimBLEClient *pClient)
  {
    if (armband::debug)
    {
      Serial.printf("%s Disconnected\n", pClient->getPeerAddress().toString().c_str());
    }

    // Update connection status
    armband::connected = false;

    // Update detection status
    armband::detected = false;

    // Disconnect the client
    pClient->disconnect();
  }  
};

/********************************************************************************************************
    CONNECTION
 ********************************************************************************************************/
static MyoClientCallbacks clientCB;

bool connectToServer()
{
  NimBLEClient *pClient = nullptr;

  /** Check if we have a client we should reuse first **/
  if (NimBLEDevice::getClientListSize())
  {
    /** Special case when we already know this device, we send false as the
     *  second argument in connect() to prevent refreshing the service database.
     *  This saves considerable time and power.
     */
    pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
    if (pClient)
    {
      if (!pClient->connect(advDevice, false))
      {
        if (armband::debug)
          Serial.println("Reconnect failed");
        return false;
      }

      if (armband::debug)
        Serial.println("Reconnected client");
    }
    /** We don't already have a client that knows this device,
     *  we will check for a client that is disconnected that we can use.
     */
    else
    {
      pClient = NimBLEDevice::getDisconnectedClient();
    }
  }

  /** No client to reuse? Create a new one. */
  if (!pClient)
  {
    if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS)
    {
      if (armband::debug)
        Serial.println("Max clients reached - no more connections available");
      return false;
    }

    pClient = NimBLEDevice::createClient();

    if (armband::debug)
      Serial.println("New client created");
  }

  pClient->setClientCallbacks(&clientCB, false);
  /** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
   *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
   *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
   *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
   */
  pClient->setConnectionParams(12, 12, 0, 51);
  /** Set how long we are willing to wait for the connection to complete (milliseconds), default is 30000. */

  if (!pClient->connect(advDevice))
  {
    /** Created a client but failed to connect, don't need to keep it as it has no data */
    if (armband::debug)
      Serial.println("Failed to connect, try again!");
    return false;
  }

  if (armband::debug)
  {
    Serial.print("Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());
    Serial.print("RSSI: ");
    Serial.println(pClient->getRssi());
  }

  // Obtain a reference to the service we are after in the remote BLE server.
  NimBLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    if (armband::debug)
    {
      Serial.print("No Service found for Service UUID --> ");
      Serial.println(serviceUUID.toString().c_str());
    }
    pClient->disconnect();
    return false;
  }

  if (armband::debug)
  {
    Serial.println("Done with Myo!");
  }

  return true;
}

void armband::connect()
{
  if (!armband::connected)
  {
    // Initialize BLE scan
    NimBLEDevice::init("");

#ifdef ESP_PLATFORM
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
#else
    NimBLEDevice::setPower(9); /** +9db */
#endif

    // Scan for Devices
    NimBLEScan *pBLEScan = NimBLEDevice::getScan();

    // Set callbacks for finding devices
    pBLEScan->setAdvertisedDeviceCallbacks(new MyoAdvertisedDeviceCallbacks());

    /** Set scan interval (how often) and window (how long) in milliseconds */
    pBLEScan->setInterval(25);
    pBLEScan->setWindow(15);

    // Active scan uses more power, but get results faster
    pBLEScan->setActiveScan(true);
    uint8_t counter = 0;

    // Keep scanning untill a Myo is detected
    while (!armband::detected && counter < 1)
    {
      pBLEScan->start(2000);
      counter++;
    }

    if (!armband::detected)
    {
      pClient->disconnect();
    }

    // Attempt Server connection
    connectToServer();
  }
}

/********************************************************************************************************
    INFO
 ********************************************************************************************************/

void armband::get_info()
{
  if (armband::connected)
  {
    BLEUUID tservice = BLEUUID("d5060001-a904-deb9-4748-2c7f4a124842");
    BLEUUID tcharacteristic = BLEUUID("d5060101-a904-deb9-4748-2c7f4a124842");
    std::string stringt;
    stringt = armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->readValue();
    armband::fw_serial_number[0] = stringt[0];
    armband::fw_serial_number[1] = stringt[1];
    armband::fw_serial_number[2] = stringt[2];
    armband::fw_serial_number[3] = stringt[3];
    armband::fw_serial_number[4] = stringt[4];
    armband::fw_serial_number[5] = stringt[5];
    armband::fw_serial_number[6] = stringt[6];
    armband::fw_unlock_pose = (byte)stringt[8] * 256 + (byte)stringt[7];
    armband::fw_active_classifier_type = stringt[9];
    armband::fw_active_classifier_index = stringt[10];
    armband::fw_has_custom_classifier = stringt[11];
    armband::fw_stream_indicating = stringt[12];
    armband::fw_sku = stringt[13];
    armband::fw_reserved[0] = stringt[14];
    armband::fw_reserved[1] = stringt[15];
    armband::fw_reserved[2] = stringt[16];
    armband::fw_reserved[3] = stringt[17];
    armband::fw_reserved[4] = stringt[18];
    armband::fw_reserved[5] = stringt[19];
    armband::fw_reserved[6] = stringt[20];
    armband::fw_reserved[7] = stringt[21];
  }
}

void armband::get_firmware()
{
  if (armband::connected)
  {
    BLEUUID tservice = BLEUUID("d5060001-a904-deb9-4748-2c7f4a124842");
    BLEUUID tcharacteristic = BLEUUID("d5060201-a904-deb9-4748-2c7f4a124842");
    std::string stringt;
    stringt = armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->readValue();
    armband::fw_major = (byte)stringt[1] * 256 + (byte)stringt[0];
    armband::fw_minor = (byte)stringt[3] * 256 + (byte)stringt[2];
    armband::fw_patch = (byte)stringt[5] * 256 + (byte)stringt[4];
    armband::fw_hardware_rev = (byte)stringt[7] * 256 + (byte)stringt[6];
  }
}

/********************************************************************************************************
    NOTIFICATIONS
 ********************************************************************************************************/

NimBLERemoteCharacteristic *armband::emg_notification(uint8_t on_off)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("d5060005-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic0 = NimBLEUUID("d5060105-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic1 = NimBLEUUID("d5060205-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic2 = NimBLEUUID("d5060305-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic3 = NimBLEUUID("d5060405-a904-deb9-4748-2c7f4a124842");
    uint8_t NotifyOn[] = {on_off, 0x00};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic0)->getDescriptor((uint16_t)0x2902)->writeValue((uint8_t *)NotifyOn, sizeof(NotifyOn), true);
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic1)->getDescriptor((uint16_t)0x2902)->writeValue((uint8_t *)NotifyOn, sizeof(NotifyOn), true);
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic2)->getDescriptor((uint16_t)0x2902)->writeValue((uint8_t *)NotifyOn, sizeof(NotifyOn), true);
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic3)->getDescriptor((uint16_t)0x2902)->writeValue((uint8_t *)NotifyOn, sizeof(NotifyOn), true);
    return armband::pClient->getService(NimBLEUUID(tservice))->getCharacteristic(NimBLEUUID(tcharacteristic0));
  }
  else
  {
    return 0;
  }
}

NimBLERemoteCharacteristic *armband::imu_notification(uint8_t on_off)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("d5060002-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic = NimBLEUUID("d5060402-a904-deb9-4748-2c7f4a124842");
    uint8_t NotifyOn[] = {on_off, 0x00};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->getDescriptor((uint16_t)0x2902)->writeValue((uint8_t *)NotifyOn, sizeof(NotifyOn), true);
    return armband::pClient->getService(NimBLEUUID(tservice))->getCharacteristic(NimBLEUUID(tcharacteristic));
  }
  else
  {
    return 0;
  }
}

NimBLERemoteCharacteristic *armband::battery_notification(uint8_t on_off)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("0000180f-0000-1000-8000-00805f9b34fb");
    NimBLEUUID tcharacteristic = NimBLEUUID("00002a19-0000-1000-8000-00805f9b34fb");
    uint8_t NotifyOn[] = {on_off, 0x00};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->getDescriptor((uint16_t)0x2902)->writeValue((uint8_t *)NotifyOn, sizeof(NotifyOn), true);
    return armband::pClient->getService(NimBLEUUID(tservice))->getCharacteristic(NimBLEUUID(tcharacteristic));
  }
  else
  {
    return 0;
  }
}

NimBLERemoteCharacteristic *armband::gesture_notification(uint8_t on_off)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("d5060003-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic = NimBLEUUID("d5060103-a904-deb9-4748-2c7f4a124842");
    int IndicateOn[] = {2 * on_off, 0x00};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->getDescriptor(((uint16_t)0x2902))->writeValue((uint8_t *)IndicateOn, sizeof(IndicateOn), true);
    return armband::pClient->getService(NimBLEUUID(tservice))->getCharacteristic(NimBLEUUID(tcharacteristic));
  }
  else
  {
    return 0;
  }
}

/********************************************************************************************************
    COMMANDS
 ********************************************************************************************************/

void armband::set_myo_mode(uint8_t emg_mode, uint8_t imu_mode, uint8_t clf_mode)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("d5060001-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic = NimBLEUUID("d5060401-a904-deb9-4748-2c7f4a124842");
    uint8_t writeVal[] = {0x01, 0x03, emg_mode, imu_mode, clf_mode};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->writeValue(writeVal, sizeof(writeVal));
  }
}

void armband::set_sleep_mode(uint8_t sleep_mode)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("d5060001-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic = NimBLEUUID("d5060401-a904-deb9-4748-2c7f4a124842");
    uint8_t writeVal[] = {0x09, 0x01, sleep_mode};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->writeValue(writeVal, sizeof(writeVal));
  }
}

void armband::vibration(uint8_t duration)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("d5060001-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic = NimBLEUUID("d5060401-a904-deb9-4748-2c7f4a124842");
    uint8_t writeVal[] = {0x03, 0x01, duration};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->writeValue(writeVal, sizeof(writeVal));
  }
}

void armband::user_action(uint8_t action_type)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("d5060001-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic = NimBLEUUID("d5060401-a904-deb9-4748-2c7f4a124842");
    uint8_t writeVal[] = {myohw_command_user_action, 0x01, action_type};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->writeValue(writeVal, sizeof(writeVal));
  }
}

void armband::unlock(uint8_t unlock_mode)
{
  if (armband::connected)
  {
    NimBLEUUID tservice = NimBLEUUID("d5060001-a904-deb9-4748-2c7f4a124842");
    NimBLEUUID tcharacteristic = NimBLEUUID("d5060401-a904-deb9-4748-2c7f4a124842");
    uint8_t writeVal[] = {0x0a, 0x01, unlock_mode};
    armband::pClient->getService(tservice)->getCharacteristic(tcharacteristic)->writeValue(writeVal, sizeof(writeVal));
  }
}
