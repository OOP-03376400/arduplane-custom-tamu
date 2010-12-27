/*
	AP_RcChannel.cpp - Radio library for Arduino
	Code by Jason Short, James Goppert. DIYDrones.com
	
	This library is free software; you can redistribute it and / or
		modify it under the terms of the GNU Lesser General Public
		License as published by the Free Software Foundation; either
		version 2.1 of the License, or (at your option) any later version.

*/

#include <math.h>
#include <avr/eeprom.h>
#include "AP_RcChannel.h"
#include <AP_Common.h>

void AP_RcChannel::readRadio() {
	// apply reverse
	uint16_t pwmRadio = APM_RC.InputCh(_ch);
	setPwm(pwmRadio);
}

void
AP_RcChannel::setPwm(uint16_t pwm)
{
	//Serial.printf("reverse: %s\n", (_reverse)?"true":"false");

	// apply reverse
	if(_reverse) pwm = int16_t(_pwmNeutral-pwm) + _pwmNeutral;

	//Serial.printf("pwm after reverse: %d\n", pwm);

	// apply filter
	if(_filter){
		if(_pwm == 0)
			_pwm = pwm;
		else
			_pwm = ((pwm + _pwm) >> 1);		// Small filtering
	}else{
		_pwm = pwm;
	}

	//Serial.printf("pwm after filter: %d\n", _pwm);

	// apply deadzone
	_pwm = (abs(_pwm - _pwmNeutral) < _pwmDeadZone) ? _pwmNeutral : _pwm;

	//Serial.printf("pwm after deadzone: %d\n", _pwm);
	APM_RC.OutputCh(_ch,_pwm);
}

void
AP_RcChannel::setPosition(float position)
{
	setPwm(_positionToPwm(position));
}

void
AP_RcChannel::mixRadio(uint16_t infStart)
{
	uint16_t pwmRadio = APM_RC.InputCh(_ch);
	float inf = abs( int16_t(pwmRadio - _pwmNeutral) );
	inf = min(inf, infStart);
	inf = ((infStart - inf) /infStart);
	setPwm(_pwm*inf + pwmRadio); 
}

uint16_t
AP_RcChannel::_positionToPwm(const float & position)
{
	uint16_t pwm;
	//Serial.printf("position: %f\n", position);
	if(position < 0)
		pwm = position * int16_t(_pwmNeutral - _pwmMin) / _scale + _pwmNeutral;
	else
		pwm = position * int16_t(_pwmMax - _pwmNeutral) / _scale + _pwmNeutral;
	constrain(pwm,_pwmMin,_pwmMax);
	return pwm;
}

float
AP_RcChannel::_pwmToPosition(const uint16_t & pwm)
{
	float position;
	if(pwm < _pwmNeutral)
		position = _scale * int16_t(pwm - _pwmNeutral)/ int16_t(_pwmNeutral - _pwmMin);
	else
		position = _scale * int16_t(pwm - _pwmNeutral)/ int16_t(_pwmMax - _pwmNeutral);
	constrain(position,-_scale,_scale);
	return position;
}

// ------------------------------------------