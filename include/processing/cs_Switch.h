/**
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: May 19, 2016
 * License: LGPLv3+
 */
#pragma once

#include <ble/cs_Nordic.h>
#include <cfg/cs_Boards.h>
#include <cfg/cs_Config.h>
#include <drivers/cs_Serial.h>
#include <drivers/cs_PWM.h>
#include <cfg/cs_StateVars.h>

class Switch {
public:
	//! Gets a static singleton (no dynamic memory allocation)
	static Switch& getInstance() {
		static Switch instance;
		return instance;
	}

	void turnOff() {
		setValue(0);
	}

	void turnOn() {
		setValue(255);
	}

	void dim(uint8_t value) {
		setValue(value);
	}

	void setValue(uint8_t value) {
		_switchValue = value;
		PWM::getInstance().setValue(0, value);

		if (value) {
			EventDispatcher::getInstance().dispatch(EVT_POWER_ON, &_switchValue, 1);
		} else {
			EventDispatcher::getInstance().dispatch(EVT_POWER_OFF, &_switchValue, 1);
		}

		StateVars::getInstance().setStateVar(SV_SWITCH_STATE, value);
	}

	uint8_t getValue() {
		return _switchValue;
	}

	void relayOn() {
#if HAS_RELAY
		LOGi("trigger relay on pin for %d ms", RELAY_HIGH_DURATION);
		nrf_gpio_pin_set(PIN_GPIO_RELAY_ON);
		nrf_delay_ms(RELAY_HIGH_DURATION);
		nrf_gpio_pin_clear(PIN_GPIO_RELAY_ON);

		// todo: update switch state?
#endif
	}

	void relayOff() {
#if HAS_RELAY
		LOGi("trigger relay off pin for %d ms", RELAY_HIGH_DURATION);
		nrf_gpio_pin_set(PIN_GPIO_RELAY_OFF);
		nrf_delay_ms(RELAY_HIGH_DURATION);
		nrf_gpio_pin_clear(PIN_GPIO_RELAY_OFF);

		// todo: update switch state?
#endif
	}

private:
	Switch() :
		_switchValue(0)
	{
		LOGi("creating switch");

		PWM& pwm = PWM::getInstance();
		pwm.init(PWM::config1Ch(1600L, PIN_GPIO_SWITCH));

#if HAS_RELAY
		nrf_gpio_cfg_output(PIN_GPIO_RELAY_OFF);
		nrf_gpio_pin_clear(PIN_GPIO_RELAY_OFF);
		nrf_gpio_cfg_output(PIN_GPIO_RELAY_ON);
		nrf_gpio_pin_clear(PIN_GPIO_RELAY_ON);
#endif
	};

	uint8_t _switchValue;

};
