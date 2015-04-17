/**
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Oct 22, 2014
 * License: LGPLv3+, Apache License, or MIT, your choice
 */

#include "common/cs_Storage.h"
#include "services/cs_GeneralService.h"
#include "common/cs_MasterBuffer.h"
#include "characteristics/cs_BufferCharacteristic.h"

#if MESHING==1 
#include <protocol/cs_Mesh.h>
#endif

using namespace BLEpp;

// needs to be the same command as defined in the bootloader
#define COMMAND_ENTER_RADIO_BOOTLOADER          1 

GeneralService::GeneralService() :
//		_stack(&stack),
		_temperatureCharacteristic(NULL),
		_firmwareCharacteristic(NULL),
		_selectConfiguration(0xFF) {

	setUUID(UUID(GENERAL_UUID));
	setName("General Service");

	Storage::getInstance().getHandle(PS_ID_GENERAL_SERVICE, _storageHandle);
	loadPersistentStorage();

	init();
}

void GeneralService::init() {
	LOGi("Create general service");

#if TEMPERATURE==1
	LOGi("add Temperature Characteristic");
	addTemperatureCharacteristic();
#else
	LOGi("skip Temperature Characteristic");
#endif

#if FIRMWARE==1
	LOGi("add Firmware Characteristic");
	addFirmwareCharacteristic();
#else
	LOGi("skip Firmware Characteristic");
#endif
#if MESHING==1
	addMeshCharacteristic();
	LOGi("add Mesh Characteristic");
#else
	LOGi("skip Mesh Characteristic");
#endif

#if CONFIGURATION==1
	// if we use configuration characteristics, set up a buffer
	_streamBuffer = new StreamBuffer<uint8_t>();
	MasterBuffer& mb = MasterBuffer::getInstance();
	buffer_ptr_t buffer = NULL;
	uint16_t size = 0;
	mb.getBuffer(buffer, size);

	LOGd("Assign buffer of size %i to stream buffer", size);
	_streamBuffer->assign(buffer, size);

	LOGi("add Set Configuration Characteristic");
	addSetConfigurationCharacteristic();
	LOGi("add Select Configuration Characteristic");
	addSelectConfigurationCharacteristic();
	LOGi("add Get Configuration Characteristic");
	addGetConfigurationCharacteristic();

	_setConfigurationCharacteristic->setValue(buffer);
	_setConfigurationCharacteristic->setMaxLength(size);
	_setConfigurationCharacteristic->setDataLength(size);

	_getConfigurationCharacteristic->setValue(buffer);
	_getConfigurationCharacteristic->setMaxLength(size);
	_getConfigurationCharacteristic->setDataLength(size);

	LOGd("Set both set/get charac to buffer at %p", buffer);
#else
	LOGi("skip Configuration Characteristics");
#endif

}

void GeneralService::loadPersistentStorage() {
	Storage::getInstance().readStorage(_storageHandle, &_storageStruct, sizeof(_storageStruct));
}

void GeneralService::savePersistentStorage() {
	Storage::getInstance().writeStorage(_storageHandle, &_storageStruct, sizeof(_storageStruct));
}

void GeneralService::addTemperatureCharacteristic() {
	_temperatureCharacteristic = new Characteristic<int32_t>();
	addCharacteristic(_temperatureCharacteristic);

	_temperatureCharacteristic->setUUID(UUID(getUUID(), TEMPERATURE_UUID));
	_temperatureCharacteristic->setName("Temperature");
	_temperatureCharacteristic->setDefaultValue(0);
	_temperatureCharacteristic->setNotifies(true);
}

void GeneralService::addFirmwareCharacteristic() {
	_firmwareCharacteristic = new Characteristic<int32_t>();
	addCharacteristic(_firmwareCharacteristic);

	_firmwareCharacteristic->setUUID(UUID(getUUID(), FIRMWARE_UUID));
	_firmwareCharacteristic->setName("Update firmware");
	_firmwareCharacteristic->setDefaultValue(0);
	_firmwareCharacteristic->setWritable(true);
	_firmwareCharacteristic->onWrite([&](const int32_t& value) -> void {
			if (value != 0) {
				LOGi("Update firmware");
				uint8_t err_code;
				err_code = sd_power_gpregret_clr(0xFF);
				APP_ERROR_CHECK(err_code);
				uint32_t cmd = COMMAND_ENTER_RADIO_BOOTLOADER;
				err_code = sd_power_gpregret_set(cmd);
				APP_ERROR_CHECK(err_code);
				sd_nvic_SystemReset();				
			} else {
				LOGi("Update firmware? Write nonzero value");
			}
		});
}

#if MESHING==1
void GeneralService::addMeshCharacteristic() {
	buffer_ptr_t buffer = MasterBuffer::getInstance().getBuffer();

	_meshCharacteristic = new Characteristic<buffer_ptr_t>();
	addCharacteristic(_meshCharacteristic);

	_meshCharacteristic->setUUID(UUID(getUUID(), MESH_UUID));
	_meshCharacteristic->setName("Mesh");
	_meshCharacteristic->setWritable(true);
	_meshCharacteristic->onWrite([&](const buffer_ptr_t& value) -> void {
			LOGi("Send mesh message");

			MeshMessage msg;
			msg.assign(_meshCharacteristic->getValue(), _meshCharacteristic->getValueLength());

			uint8_t handle = msg.handle();
			uint8_t val = msg.value();
			CMesh &mesh = CMesh::getInstance();
			mesh.send(handle, val);
		});

	_meshCharacteristic->setValue(buffer);
	_meshCharacteristic->setMaxLength(MM_SERIALIZED_SIZE);
	_meshCharacteristic->setDataLength(0);

}
#endif

void GeneralService::addSetConfigurationCharacteristic() {
	_setConfigurationCharacteristic = new Characteristic<buffer_ptr_t>();
	addCharacteristic(_setConfigurationCharacteristic);

	_setConfigurationCharacteristic->setUUID(UUID(getUUID(), SET_CONFIGURATION_UUID));
	_setConfigurationCharacteristic->setName("Set Configuration");
	_setConfigurationCharacteristic->setWritable(true);
	_setConfigurationCharacteristic->onWrite([&](const buffer_ptr_t& value) -> void {

			if (!value) {
				LOGw("No value on writing to config. Bail out");
			} else {
				LOGi("Write value!");
				MasterBuffer& mb = MasterBuffer::getInstance();
				if (!mb.isLocked()) {
					mb.lock();
					uint8_t type = _streamBuffer->type();
					LOGi("Write configuration type: %i", (int)type);
					uint8_t *payload = _streamBuffer->payload();
					uint8_t length = _streamBuffer->length();
					writeToStorage(type, length, payload);
					mb.unlock();
				} else {
					LOGe("Buffer is locked. Cannot be written!");
				}
			}
		});
}

void GeneralService::writeToStorage(uint8_t type, uint8_t length, uint8_t* payload) {
	switch(type) {
	case CONFIG_NAME_UUID: {
		LOGd("Write name");
		std::string str = std::string((char*)payload, length);
		LOGd("Set name to: %s", str.c_str());
		setBLEName(str);
		Storage::setString(str, _storageStruct.device_name);
		savePersistentStorage();
		break;
	}
	case CONFIG_FLOOR_UUID: {
		LOGd("Set floor level");
		if (length != 1) {
			LOGw("We do not account for buildings of more than 255 floors yet");
			return;
		}
		uint8_t floor = payload[0];
		LOGi("Set floor to %i", floor);
		Storage::setUint8(floor, _storageStruct.floor);
		savePersistentStorage();
		break;
	}
	case CONFIG_NEARBY_TIMEOUT_UUID: {
		if (length != 2) {
			LOGw("Nearby timeout is of type uint16");
			return;
		}
		uint16_t counts = ((uint16_t*)payload)[0]; //TODO: other byte order?
		LOGd("setNearbyTimeout(%i)", counts);
//		setNearbyTimeout(counts);
		break;
	}
	default:
		LOGw("There is no such configuration type (%i)! Or not yet implemented!", type);
	}
}

bool GeneralService::readFromStorage(uint8_t type) {
	switch(type) {
	case CONFIG_NAME_UUID: {
		LOGd("Read name");
		std::string str = getBLEName();
		_streamBuffer->fromString(str);
		_streamBuffer->setType(type);

		LOGd("Name read %s", str.c_str());

		return true;
	}
	case CONFIG_FLOOR_UUID: {
		LOGd("Read floor");
		loadPersistentStorage();
		uint8_t plen = 1;
		uint8_t payload[plen];
		Storage::getUint8(_storageStruct.floor, payload[0], 0);
		_streamBuffer->setPayload(payload, plen);
		_streamBuffer->setType(type);

		LOGd("Floor level set in payload: %i with len %i", _streamBuffer->payload()[0], _streamBuffer->length());

		return true;
	}
	default: {
		LOGd("There is no such configuration type (%i), or not yet implemented.", type);
	}
	}
	return false;
}

void GeneralService::writeToConfigCharac() {
	uint16_t len = _streamBuffer->getDataLength();
	uint8_t value = _streamBuffer->payload()[0];
	buffer_ptr_t pntr = _getConfigurationCharacteristic->getValue();
	LOGd("Write to config length %i and value %i", len, value);
	
	struct stream_t<uint8_t> *stream = (struct stream_t<uint8_t>*) pntr;

	LOGd("Write to config at %p with payload length %i and value %i", pntr, stream->length, stream->payload[0]);

	_getConfigurationCharacteristic->setDataLength(_streamBuffer->getDataLength());
	_getConfigurationCharacteristic->notify();
}

void GeneralService::addSelectConfigurationCharacteristic() {
	_selectConfigurationCharacteristic = new Characteristic<uint8_t>();
	addCharacteristic(_selectConfigurationCharacteristic);

	_selectConfigurationCharacteristic->setUUID(UUID(getUUID(), SELECT_CONFIGURATION_UUID));
	_selectConfigurationCharacteristic->setName("Select Configuration");
	_selectConfigurationCharacteristic->setWritable(true);
	_selectConfigurationCharacteristic->onWrite([&](const uint8_t& value) -> void {
			if (value < CONFIG_TYPES) {
				LOGd("Select configuration type: %i", (int)value);
				_selectConfiguration = value;
			} else {
				LOGe("Cannot select %i", value);
			}
		});
}

void GeneralService::addGetConfigurationCharacteristic() {
	_getConfigurationCharacteristic = new Characteristic<buffer_ptr_t>();
	addCharacteristic(_getConfigurationCharacteristic);

	_getConfigurationCharacteristic->setUUID(UUID(getUUID(), GET_CONFIGURATION_UUID));
	_getConfigurationCharacteristic->setName("Get Configuration");
	_getConfigurationCharacteristic->setWritable(false);
}

std::string & GeneralService::getBLEName() {
	_name = "Unknown";
	if (_stack) {
		return _name = _stack->getDeviceName();
	}
	return _name;
}

void GeneralService::setBLEName(const std::string &name) {
	if (name.length() > 31) {
		log(ERROR, "Name is too long");
		return;
	}
	if (_stack) {
		_stack->updateDeviceName(name);
	}
}

void GeneralService::writeToTemperatureCharac(int32_t temperature) {
	*_temperatureCharacteristic = temperature;
}
	
void GeneralService::tick() {
	if (_temperatureCharacteristic) {
		int32_t temp;
		temp = getTemperature();
		writeToTemperatureCharac(temp);
#ifdef MICRO_VIEW
		// Update temperature at the display
		write("1 %i\r\n", temp);
#endif
	}

	if (_getConfigurationCharacteristic) {
		if (_selectConfiguration != 0xFF) {
			bool success = readFromStorage(_selectConfiguration);
			if (success) {
				writeToConfigCharac();
			}
			// only write once
			_selectConfiguration = 0xFF;
		}
	}
}