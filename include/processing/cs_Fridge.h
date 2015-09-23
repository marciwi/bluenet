/**
 * Author: Bart van Vliet
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Jul 15, 2015
 * License: LGPLv3+, Apache License, or MIT, your choice
 */

#pragma once

#include <cstdint>

#include <events/cs_EventListener.h>

#define FRIDGE_UPDATE_FREQUENCY 0.2 // hz

class Fridge : EventListener {
public:
	Fridge();
	void tick();
	static void staticTick(Fridge* ptr) {
		ptr->tick();
	}
	void startTicking();
	void stopTicking();

	void handleEvent(uint16_t evt, void* p_data, uint16_t length);

private:
	uint32_t _appTimerId;
	int8_t _minTemp;
	int8_t _maxTemp;

};


