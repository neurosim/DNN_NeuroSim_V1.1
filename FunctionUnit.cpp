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

#include <iostream>
#include <fstream>
#include "FunctionUnit.h"

using namespace std;

FunctionUnit::FunctionUnit() {
	height = width = 0;
	area = 0;
	emptyArea = 0;
	usedArea = 0;
	totalArea = 0;	// may use it if the circuit units are scattered
	readLatency = writeLatency = 0;
	readDynamicEnergy = writeDynamicEnergy = 0;
	leakage = 0;

	newWidth = newHeight = 0;
	readPower = writePower = 0;
}

void FunctionUnit::PrintProperty(const char* str) {
	cout << "---------------------------------------------------------" << endl;
	cout << str << endl;
	cout << "Area = " << height*1e6 << "um x " << width*1e6 << "um = " << area*1e12 << "um^2" << endl;
	if (totalArea)
		cout << "Total Area = " << totalArea*1e12 << "um^2" << endl;
	cout << "Timing:" << endl;
	cout << " - Read Latency = " << readLatency*1e9 << "ns" << endl;
	cout << " - Write Latency = " << writeLatency*1e9 << "ns" << endl;
	cout << "Power:" << endl;
	cout << " - Read Dynamic Energy = " << readDynamicEnergy*1e12 << "pJ" << endl;
	cout << " - Write Dynamic Energy = " << writeDynamicEnergy*1e12 << "pJ" << endl;
	cout << " - Leakage Power = " << leakage*1e6 << "uW" << endl;
	// cout << " - Read Power = " << readPower*1e6 << "uW" << endl;
	// cout << " - Write Power = " << writePower*1e6 << "uW" << endl;
}

void FunctionUnit::SaveOutput(const char* str) {
	ofstream outfile;                                           
	outfile.open("SynapticCOREoutput.txt", ios::app);     
	outfile << "---------------------------------------------------------" << endl;
	outfile << str << endl;
	outfile << "Area = " << height*1e6 << "um x " << width*1e6 << "um = " << area*1e12 << "um^2" << endl;
	if (totalArea)
		outfile << "Total Area = " << totalArea*1e12 << "um^2" << endl;
	outfile << "Timing:" << endl;
	outfile << " - Read Latency = " << readLatency*1e9 << "ns" << endl;
	outfile << " - Write Latency = " << writeLatency*1e9 << "ns" << endl;
	outfile << "Power:" << endl;
	outfile << " - Read Dynamic Energy = " << readDynamicEnergy*1e12 << "pJ" << endl;
	outfile << " - Write Dynamic Energy = " << writeDynamicEnergy*1e12 << "pJ" << endl;
	outfile << " - Leakage Power = " << leakage*1e6 << "uW" << endl;
	// outfile << " - Read Power = " << readPower*1e6 << "uW" << endl;
	// outfile << " - Write Power = " << writePower*1e6 << "uW" << endl;
	outfile << '\n' << endl;
	outfile.close();
}

void FunctionUnit::MagicLayout() {
	if (newHeight) {
		width = area / newHeight;
		height = newHeight;
	} else if (newWidth) {
		height = area / newWidth;
		width = newWidth;
	}
}

void FunctionUnit::OverrideLayout() {
	if (newHeight && newWidth) {
		height = newHeight;
		width = newWidth;
	} else {
		puts("Need to provide both newHeight and newWidth for OverrideLayout()");
		exit(-1);
	}
	area = height * width;
}

