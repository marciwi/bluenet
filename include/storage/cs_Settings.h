/*
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: May 11, 2015
 * License: LGPLv3+
 */
#pragma once

#include <events/cs_EventTypes.h>
#include <drivers/cs_Storage.h>
#include <structs/cs_StreamBuffer.h>

/** Configuration types
 *
 * Use in the characteristic to read and write configurations in <CommonService>.
 */
enum ConfigurationTypes {
	CONFIG_NAME_UUID                        = Configuration_Base,
	CONFIG_DEVICE_TYPE_UUID                 = 0x01,
	CONFIG_ROOM_UUID                        = 0x02,
	CONFIG_FLOOR_UUID                       = 0x03,
	CONFIG_NEARBY_TIMEOUT_UUID              = 0x04,
	CONFIG_PWM_FREQ_UUID                    = 0x05,
	CONFIG_IBEACON_MAJOR                    = 0x06,
	CONFIG_IBEACON_MINOR                    = 0x07,
	CONFIG_IBEACON_UUID                     = 0x08,
	CONFIG_IBEACON_RSSI                     = 0x09,
	CONFIG_WIFI_SETTINGS                    = 0x0A, //! 10
	CONFIG_TX_POWER                         = 0x0B, //! 11
	CONFIG_ADV_INTERVAL                     = 0x0C, //! 12
	CONFIG_PASSKEY							= 0x0D, //! 13
	CONFIG_MIN_ENV_TEMP                     = 0x0E, //! 14
	CONFIG_MAX_ENV_TEMP                     = 0x0F, //! 15
	CONFIG_SCAN_DURATION                    = 0x10, //! 16
	CONFIG_SCAN_SEND_DELAY                  = 0x11, //! 17
	CONFIG_SCAN_BREAK_DURATION              = 0x12, //! 18
	CONFIG_BOOT_DELAY                       = 0x13, //! 19
	CONFIG_MAX_CHIP_TEMP                    = 0x14, //! 20
	CONFIG_SCAN_FILTER                      = 0x15, //! 21
	CONFIG_SCAN_FILTER_SEND_FRACTION        = 0x16, //! 22
	CONFIG_CURRENT_LIMIT                    = 0x17, //! 23
	CONFIG_MESH_ENABLED                     = 0x18, //! 24
	CONFIG_ENCRYPTION_ENABLED               = 0x19, //! 25
	CONFIG_IBEACON_ENABLED                  = 0x1A, //! 26
	CONFIG_SCANNER_ENABLED                  = 0x1B, //! 27
	CONFIG_CONT_POWER_SAMPLER_ENABLED       = 0x1C, //! 28
	CONFIG_TRACKER_ENABLED                  = 0x1D, //! 29
	CONFIG_ADC_SAMPLE_RATE                  = 0x1E, //! 30
	CONFIG_POWER_SAMPLE_BURST_INTERVAL      = 0x1F, //! 31
	CONFIG_POWER_SAMPLE_CONT_INTERVAL       = 0x20, //! 32
	CONFIG_POWER_SAMPLE_CONT_NUM_SAMPLES    = 0x21, //! 33
	CONFIG_TYPES
};

/**
 * Load settings from and save settings to persistent storage.
 */
class Settings {

private:
	Settings();

	//! This class is singleton, deny implementation
	Settings(Settings const&);
	//! This class is singleton, deny implementation
	void operator=(Settings const &);

public:
	static Settings& getInstance() {
		static Settings instance;
		return instance;
	}

	void init();

	bool isInitialized() {
		return _initialized;
	}

	/** Read the configuration from the buffer and store in working memory.
	 *  If persistent is true, also store in FLASH
	 */
	void writeToStorage(uint8_t type, uint8_t* payload, uint16_t length, bool persistent = true);

	/** Read the configuration from storage and write to streambuffer (to be read from characteristic)
	 */
	bool readFromStorage(uint8_t type, StreamBuffer<uint8_t>* streamBuffer);

	/** Return the struct containing the configuration values in current working memory
	 */
//	ps_configuration_t& getConfig();

	/** Get a handle to the persistent storage struct and load it from FLASH into working memory.
	 *
	 * Persistent storage is implemented in FLASH. Just as with SSDs, it is important to realize that
	 * writing less than a minimal block strains the memory just as much as flashing the entire block.
	 * Hence, there is an entire struct that can be filled and flashed at once.
	 */
	void loadPersistentStorage();

	/** Save the whole configuration struct (working memory) to FLASH.
	 */
	void savePersistentStorage();

	/** Save the whole iBeacon struct to FLASH
	 */
	void saveIBeaconPersistent();

	/** Retrieve the Bluetooth name from the object representing the BLE stack.
	 *
	 * @return name of the device
	 */
	std::string & getBLEName();

	/** Write the Bluetooth name to the object representing the BLE stack.
	 *
	 * This updates the Bluetooth name immediately, however, it does not update the name persistently. It
	 * has to be written to FLASH in that case.
	 */
	void setBLEName(const std::string &name, bool persistent = true);

	bool isEnabled(uint8_t type);
	bool updateFlag(uint8_t type, bool value, bool persistent);

	void factoryReset(uint32_t resetCode);

	bool get(uint8_t type, void* target);
	bool get(uint8_t type, void* target, uint16_t& size );
	bool set(uint8_t type, void* target, bool persistent = false, uint16_t size = 0);

protected:

	bool _initialized;

	//! handle to storage (required to write to and read from FLASH)
	pstorage_handle_t _storageHandle;

	//! struct that storage object understands
	ps_configuration_t _storageStruct;

	//! non-persistent configuration options
	std::string _wifiSettings;

	Storage* _storage;

	bool verify(uint8_t type, uint8_t* payload, uint8_t length);

	uint8_t* getStorageItem(uint8_t type);
	uint16_t getSettingsItemSize(uint8_t type);

	bool readFlag(uint8_t type, bool& value);
	void initFlags();

	/**
	 * Helper functions to write single item from the configuration struct to storage (FLASH).
	 */
	void savePersistentStorageItem(uint32_t* item);
	void savePersistentStorageItem(int32_t* item);
	void savePersistentStorageItem(uint8_t* item, uint8_t size = 1);

};

