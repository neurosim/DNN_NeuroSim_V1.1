/*******************************************************************************
* Copyright (c) 2015-2017
* School of Electrical, Computer and Energy Engineering, Arizona State University
* PI: Prof. Shimeng Yu
* All rights reserved.
* 
* This source code is part of NeuroSim - a device-circuit-algorithm framework to benchmark 
* neuro-inspired architectures with synaptic devices(e.g., SRAM and emerging non-volatile memory). 
* Copyright of the model is maintained by the developers, and the model is distributed under 
* the terms of the Creative Commons Attribution-NonCommercial 4.0 International Public License 
* http://creativecommons.org/licenses/by-nc/4.0/legalcode.
* The source code is free and you can redistribute and/or modify it
* by providing that the following conditions are met:
* 
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
* 
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* Developer list: 
*   Pai-Yu Chen	    Email: pchen72 at asu dot edu 
*                    
*   Xiaochen Peng   Email: xpeng15 at asu dot edu
********************************************************************************/

#include <cmath>
#include <iostream>
#include "constant.h"
#include "formula.h"
#include "BitShifter.h"

using namespace std;

BitShifter::BitShifter(const InputParameter& _inputParameter, const Technology& _tech, const MemCell& _cell): inputParameter(_inputParameter), tech(_tech), cell(_cell), dff(_inputParameter, _tech, _cell), FunctionUnit() {
	initialized = false;
}

void BitShifter::Initialize(int _numUnit, int _numBit, double _clkFreq) {
	if (initialized)
		cout << "[BitShifter] Warning: Already initialized!" << endl;
	
	numUnit = _numUnit;
	numBit = _numBit;
	clkFreq = _clkFreq;
	numDff = numBit * numUnit;	
	
	dff.Initialize(numDff, clkFreq);
	
	initialized = true;
}

void BitShifter::CalculateArea(double _newHeight, double _newWidth, AreaModify _option) {
	if (!initialized) {
		cout << "[BitShifter] Error: Require initialization first!" << endl;
	} else {
		double hInv, wInv, hNand, wNand;
		
		dff.CalculateArea(NULL, NULL, NONE);
		area = dff.area;
		
		if (_newWidth && _option==NONE) {
			width = _newWidth;
			height = area/width;
		} else {
			height = _newHeight;
            width = area/height;
		}
		
		// Modify layout
		newHeight = _newHeight;
		newWidth = _newWidth;
		switch (_option) {
			case MAGIC:
				MagicLayout();
				break;
			case OVERRIDE:
				OverrideLayout();
				break;  
			default:    // NONE
				break;
		}

	}
}

void BitShifter::CalculateLatency(double numRead) {
	if (!initialized) {
		cout << "[BitShifter] Error: Require initialization first!" << endl;
	} else {
		readLatency = 0;
		dff.CalculateLatency(1e20, 1);
		readLatency += dff.readLatency; // read out parallely
		readLatency *= numRead;    
	}
}

void BitShifter::CalculatePower(double numRead) {
	if (!initialized) {
		cout << "[BitShifter] Error: Require initialization first!" << endl;
	} else {
		leakage = 0;
		readDynamicEnergy = 0;
		
		dff.CalculatePower(numRead, numDff);	
		readDynamicEnergy += dff.readDynamicEnergy;
		leakage += dff.leakage;
	}
}

void BitShifter::PrintProperty(const char* str) {
	FunctionUnit::PrintProperty(str);
}

