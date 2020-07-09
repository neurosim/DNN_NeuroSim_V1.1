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
#include <vector>
#include "constant.h"
#include "formula.h"
#include "Param.h"
#include "MultilevelSenseAmp.h"

using namespace std;

extern Param *param;

MultilevelSenseAmp::MultilevelSenseAmp(const InputParameter& _inputParameter, const Technology& _tech, const MemCell& _cell): inputParameter(_inputParameter), tech(_tech), cell(_cell), currentSenseAmp(_inputParameter, _tech, _cell), FunctionUnit() {
	initialized = false;
}

void MultilevelSenseAmp::Initialize(int _numCol, int _levelOutput, double _clkFreq, int _numReadCellPerOperationNeuro, bool _parallel) {
	if (initialized) {
		cout << "[MultilevelSenseAmp] Warning: Already initialized!" << endl;
    } else {

	numCol = _numCol;
	levelOutput = _levelOutput;                // # of bits for A/D output ... 
	clkFreq = _clkFreq;
	numReadCellPerOperationNeuro = _numReadCellPerOperationNeuro;
	parallel = _parallel;
	
	if (parallel) {
		for (int i=0; i<levelOutput-1; i++){
			double R_start = (double) param->resistanceOn / param->numRowSubArray;
			double R_index = (double) param->resistanceOff / param->numRowSubArray;
			double R_this = R_start + (double) (i+1)*R_index/levelOutput;
			Rref.push_back(R_this);
		} // TODO: Nonlinear Quantize
	} else {
		for (int i=0; i<levelOutput-1; i++){
			double R_start = (double) param->resistanceOn;
			double R_index = (double) param->resistanceOff;
			double R_this = R_start + (double) (i+1)*R_index/levelOutput;
			Rref.push_back(R_this);
		} // TODO: Nonlinear Quantize
	}
	
	
	// Initialize SenseAmp
	currentSenseAmp.Initialize((levelOutput-1)*numCol, false, false, clkFreq, numReadCellPerOperationNeuro);        // use real-traced mode ... 

	initialized = true;
	}
}

void MultilevelSenseAmp::CalculateArea(double heightArray, double widthArray, AreaModify _option) {
	if (!initialized) {
		cout << "[MultilevelSenseAmp] Error: Require initialization first!" << endl;
	} else {
		
		area = 0;
		height = 0;
		width = 0;
		
		if (widthArray && _option==NONE) {
			currentSenseAmp.CalculateUnitArea();
			currentSenseAmp.CalculateArea(widthArray);
			area = currentSenseAmp.area;
			width = widthArray;
			height = area / width;
		} else if (heightArray && _option==NONE) {
			currentSenseAmp.CalculateUnitArea();
			currentSenseAmp.CalculateArea(heightArray);
			area = currentSenseAmp.area;
			height = heightArray;
			width = area / height;
		} else {
			cout << "[MultilevelSenseAmp] Error: No width or height assigned for the multiSenseAmp circuit" << endl;
			exit(-1);
		}
		// Assume the Current Mirrors are on the same row and the total width of them is smaller than the adder or DFF
		
		// Modify layout
		newHeight = heightArray;
		newWidth = widthArray;
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

void MultilevelSenseAmp::CalculateLatency(const vector<double> &columnResistance, double numColMuxed, double numRead) {
	if (!initialized) {
		cout << "[MultilevelSenseAmp] Error: Require initialization first!" << endl;
	} else {
		readLatency = 0;
		double LatencyCol = 0;
		for (double j=0; j<columnResistance.size(); j++){
			double T_Col = 0;
			T_Col = GetColumnLatency(columnResistance[j]);
			if (columnResistance[j] == columnResistance[j]) {
				LatencyCol = max(LatencyCol, T_Col);
			} else {
				LatencyCol = LatencyCol;
			}
			if (LatencyCol < 1e-9) {
				LatencyCol = 1e-9;
			} else if (LatencyCol > 10e-9) {
				LatencyCol = 10e-9;
			}
		}
		readLatency += LatencyCol*numColMuxed;
		readLatency *= numRead;
	}
}

void MultilevelSenseAmp::CalculatePower(const vector<double> &columnResistance, double numRead) {
	if (!initialized) {
		cout << "[MultilevelSenseAmp] Error: Require initialization first!" << endl;
	} else {
		leakage = 0;
		readDynamicEnergy = 0;
		for (double i=0; i<columnResistance.size(); i++) {
			double E_Col = 0;
			E_Col = GetColumnPower(columnResistance[i]);
			if (columnResistance[i] == columnResistance[i]) {
				readDynamicEnergy += E_Col;
			} else {
				readDynamicEnergy += 0;
			}
		}
		readDynamicEnergy *= numRead;
	}
} 

void MultilevelSenseAmp::PrintProperty(const char* str) {
	FunctionUnit::PrintProperty(str);
}


double MultilevelSenseAmp::GetColumnLatency(double columnRes) {
	double Column_Latency = 0;
	double up_bound = 3, mid_bound = 1.1, low_bound = 0.9;
	double T_max = 0;
	
	if (((double) 1/columnRes == 0) || (columnRes == 0)) {
		Column_Latency = 0;
	} else {
		if (param->deviceroadmap == 1) {  // HP
			Column_Latency = 1e-9;
		} else {                         // LP
			if (param->technode == 130) {
				T_max = (0.2679*log(columnRes/1000)+0.0478)*1e-9;   // T_max = (0.2679*log(R_BL/1000)+0.0478)*10^-9;

				for (int i=1; i<levelOutput-1; i++){
					double ratio = Rref[i]/columnRes;
					double T = 0;
					if (ratio >= 20 || ratio <= 0.05) {
						T = 1e-9;
					} else {
						if (ratio <= low_bound){
							T = T_max * (3.915*pow(ratio,3)-5.3996*pow(ratio,2)+2.4653*ratio+0.3856);  // y = 3.915*x^3-5.3996*x^2+2.4653*x+0.3856;
						} else if (mid_bound <= ratio <= up_bound){
							T = T_max * (0.0004*pow(ratio,4)-0.0087*pow(ratio,3)+0.0742*pow(ratio,2)-0.2725*ratio+1.2211);  // y = 0.0004*x^4-0.0087*x^3+0.0742*x^2-0.2725*x+1.2211;
						} else if (ratio>up_bound){
							T = T_max * (0.0004*pow(ratio,4)-0.0087*pow(ratio,3)+0.0742*pow(ratio,2)-0.2725*ratio+1.2211);
						} else {
							T = T_max;
						}
					}
					Column_Latency = max(Column_Latency, T);
				}
			} else if (param->technode == 90) {
				T_max = (0.0586*log(columnRes/1000)+1.41)*1e-9;   // T_max = (0.0586*log(R_BL/1000)+1.41)*10^-9;

				for (int i=1; i<levelOutput-1; i++){
					double ratio = Rref[i]/columnRes;
					double T = 0;
					if (ratio >= 20 || ratio <= 0.05) {
						T = 1e-9;
					} else {
						if (ratio <= low_bound){
							T = T_max * (3.726*pow(ratio,3)-5.651*pow(ratio,2)+2.8249*ratio+0.3574);    // y = 3.726*x^3-5.651*x^2+2.8249*x+0.3574;
						} else if (mid_bound <= ratio <= up_bound){
							T = T_max * (0.0000008*pow(ratio,4)-0.00007*pow(ratio,3)+0.0017*pow(ratio,2)-0.0188*ratio+0.9835);  // y = 0.0000008*x^4-0.00007*x^3+0.0017*x^2-0.0188*x+0.9835;
						} else if (ratio>up_bound){
							T = T_max * (0.0000008*pow(ratio,4)-0.00007*pow(ratio,3)+0.0017*pow(ratio,2)-0.0188*ratio+0.9835);
						} else {
							T = T_max;
						}
					}
					Column_Latency = max(Column_Latency, T);
				}
			} else if (param->technode == 65) {
				T_max = (0.1239*log(columnRes/1000)+0.6642)*1e-9;   // T_max = (0.1239*log(R_BL/1000)+0.6642)*10^-9;

				for (int i=1; i<levelOutput-1; i++){
					double ratio = Rref[i]/columnRes;
					double T = 0;
					if (ratio >= 20 || ratio <= 0.05) {
						T = 1e-9;
					} else {
						if (ratio <= low_bound){
							T = T_max * (1.3899*pow(ratio,3)-2.6913*pow(ratio,2)+2.0483*ratio+0.3202);    // y = 1.3899*x^3-2.6913*x^2+2.0483*x+0.3202;
						} else if (mid_bound <= ratio <= up_bound){
							T = T_max * (0.0036*pow(ratio,4)-0.0363*pow(ratio,3)+0.1043*pow(ratio,2)-0.0346*ratio+1.0512);   // y = 0.0036*x^4-0.0363*x^3+0.1043*x^2-0.0346*x+1.0512;
						} else if (ratio>up_bound){
							T = T_max * (0.0036*pow(ratio,4)-0.0363*pow(ratio,3)+0.1043*pow(ratio,2)-0.0346*ratio+1.0512);
						} else {
							T = T_max;
						}
					}
					Column_Latency = max(Column_Latency, T);
				}
			} else if (param->technode == 45 || param->technode == 32) {
				T_max = (0.0714*log(columnRes/1000)+0.7651)*1e-9;    // T_max = (0.0714*log(R_BL/1000)+0.7651)*10^-9;

				for (int i=1; i<levelOutput-1; i++){
					double ratio = Rref[i]/columnRes;
					double T = 0;
					if (ratio >= 20 || ratio <= 0.05) {
						T = 1e-9;
					} else {
						if (ratio <= low_bound){
							T = T_max * (3.7949*pow(ratio,3)-5.6685*pow(ratio,2)+2.6492*ratio+0.4807);    // y = 3.7949*x^3-5.6685*x^2+2.6492*x+0.4807
						} else if (mid_bound <= ratio <= up_bound){
							T = T_max * (0.000001*pow(ratio,4)-0.00006*pow(ratio,3)+0.0001*pow(ratio,2)-0.0171*ratio+1.0057);   // 0.000001*x^4-0.00006*x^3+0.0001*x^2-0.0171*x+1.0057;
						} else if (ratio>up_bound){
							T = T_max * (0.000001*pow(ratio,4)-0.00006*pow(ratio,3)+0.0001*pow(ratio,2)-0.0171*ratio+1.0057);
						} else {
							T = T_max;
						}
					}
					Column_Latency = max(Column_Latency, T);
				}
			} else {   // technode below and equal to 22nm
				Column_Latency = 1e-9;
			}
		}
	}
	return Column_Latency;
}



double MultilevelSenseAmp::GetColumnPower(double columnRes) {
	double Column_Power = 0;
	double Column_Energy = 0;

	if ((double) 1/columnRes == 0) { 
		Column_Power = 1e-6;
	} else if (columnRes == 0) {
		Column_Power = 0;
	} else {
		if (param->deviceroadmap == 1) {  // HP
			if (param->technode == 130) {
				Column_Power = (0.00001*log(columnRes/1000.0)+8.8898)*1e-6;
			} else if (param->technode == 90) {
				Column_Power = (0.0002*log(columnRes/1000.0)+10.09)*1e-6;
			} else if (param->technode == 65) {
				Column_Power = (0.0001*log(columnRes/1000.0)+6.9579)*1e-6;
			} else if (param->technode == 45) {
				Column_Power = (0.0037*log(columnRes/1000.0)+7.7017)*1e-6;
			} else if (param->technode == 32){  
				Column_Power = (0.0064*log(columnRes/1000.0)+7.9648)*1e-6;
			} else if (param->technode == 22){   
				Column_Power = (0.0087*log(columnRes/1000.0)+2.1939)*1e-6;
			} else if (param->technode == 14){  
				Column_Power = (0.0087*log(columnRes/1000.0)+1)*1e-6;
			} else if (param->technode == 10){  
				Column_Power = (0.0087*log(columnRes/1000.0)+0.7)*1e-6;
			} else {   // 7nm
				Column_Power = (0.0087*log(columnRes/1000.0)+0.5)*1e-6;
			}
		} else {                         // LP
			if (param->technode == 130) {
				Column_Power = (0.2811*log(columnRes/1000.0)+6.0809)*1e-6;
			} else if (param->technode == 90) {
				Column_Power = (0.0578*log(columnRes/1000.0)+7.6102)*1e-6;
			} else if (param->technode == 65) {
				Column_Power = (0.0710*log(columnRes/1000.0)+6.4147)*1e-6;
			} else if (param->technode == 45) {
				Column_Power = (0.0710*log(columnRes/1000.0)+6.4147)*1e-6;
			} else if (param->technode == 32){  
				Column_Power = (0.0251*log(columnRes/1000.0)+4.7835)*1e-6;
			} else if (param->technode == 22){   
				Column_Power = (0.0516*log(columnRes/1000.0)+3.2349)*1e-6;
			} else if (param->technode == 14){  
				Column_Power = (0.0516*log(columnRes/1000.0)+2.2)*1e-6;
			} else if (param->technode == 10){  
				Column_Power = (0.0516*log(columnRes/1000.0)+1.7)*1e-6;
			} else {   // 7nm
				Column_Power = (0.0516*log(columnRes/1000.0)+1.3)*1e-6;
			}
		}
	}
	
	double up_bound = 3, mid_bound = 1.1, low_bound = 0.9;
	double T_max = 0;
	if (((double) 1/columnRes == 0) || (columnRes == 0)) {
		Column_Energy += 0;
	} else {
		if (param->deviceroadmap == 1) {  // HP
			Column_Energy = Column_Power*1e-9*(levelOutput-1);
		} else {                         // LP
			if (param->technode == 130) {
				T_max = (0.2679*log(columnRes/1000)+0.0478)*1e-9;   // T_max = (0.2679*log(R_BL/1000)+0.0478)*10^-9;

				for (int i=1; i<levelOutput-1; i++){
					double ratio = Rref[i]/columnRes;
					double T = 0;
					if (ratio >= 20 || ratio <= 0.05) {
						T = 1e-9;
					} else {
						if (ratio <= low_bound){
							T = T_max * (3.915*pow(ratio,3)-5.3996*pow(ratio,2)+2.4653*ratio+0.3856);  // y = 3.915*x^3-5.3996*x^2+2.4653*x+0.3856;
						} else if (mid_bound <= ratio <= up_bound){
							T = T_max * (0.0004*pow(ratio,4)-0.0087*pow(ratio,3)+0.0742*pow(ratio,2)-0.2725*ratio+1.2211);  // y = 0.0004*x^4-0.0087*x^3+0.0742*x^2-0.2725*x+1.2211;
						} else if (ratio>up_bound){
							T = T_max * (0.0004*pow(ratio,4)-0.0087*pow(ratio,3)+0.0742*pow(ratio,2)-0.2725*ratio+1.2211);
						} else {
							T = T_max;
						}
					}
					Column_Energy += Column_Power*T;
				}
			} else if (param->technode == 90) {
				T_max = (0.0586*log(columnRes/1000)+1.41)*1e-9;   // T_max = (0.0586*log(R_BL/1000)+1.41)*10^-9;

				for (int i=1; i<levelOutput-1; i++){
					double ratio = Rref[i]/columnRes;
					double T = 0;
					if (ratio >= 20 || ratio <= 0.05) {
						T = 1e-9;
					} else {
						if (ratio <= low_bound){
							T = T_max * (3.726*pow(ratio,3)-5.651*pow(ratio,2)+2.8249*ratio+0.3574);    // y = 3.726*x^3-5.651*x^2+2.8249*x+0.3574;
						} else if (mid_bound <= ratio <= up_bound){
							T = T_max * (0.0000008*pow(ratio,4)-0.00007*pow(ratio,3)+0.0017*pow(ratio,2)-0.0188*ratio+0.9835);  // y = 0.0000008*x^4-0.00007*x^3+0.0017*x^2-0.0188*x+0.9835;
						} else if (ratio>up_bound){
							T = T_max * (0.0000008*pow(ratio,4)-0.00007*pow(ratio,3)+0.0017*pow(ratio,2)-0.0188*ratio+0.9835);
						} else {
							T = T_max;
						}
					}
					Column_Energy += Column_Power*T;
				}
			} else if (param->technode == 65) {
				T_max = (0.1239*log(columnRes/1000)+0.6642)*1e-9;   // T_max = (0.1239*log(R_BL/1000)+0.6642)*10^-9;

				for (int i=1; i<levelOutput-1; i++){
					double ratio = Rref[i]/columnRes;
					double T = 0;
					if (ratio >= 20 || ratio <= 0.05) {
						T = 1e-9;
					} else {
						if (ratio <= low_bound){
							T = T_max * (1.3899*pow(ratio,3)-2.6913*pow(ratio,2)+2.0483*ratio+0.3202);    // y = 1.3899*x^3-2.6913*x^2+2.0483*x+0.3202;
						} else if (mid_bound <= ratio <= up_bound){
							T = T_max * (0.0036*pow(ratio,4)-0.0363*pow(ratio,3)+0.1043*pow(ratio,2)-0.0346*ratio+1.0512);   // y = 0.0036*x^4-0.0363*x^3+0.1043*x^2-0.0346*x+1.0512;
						} else if (ratio>up_bound){
							T = T_max * (0.0036*pow(ratio,4)-0.0363*pow(ratio,3)+0.1043*pow(ratio,2)-0.0346*ratio+1.0512);
						} else {
							T = T_max;
						}
					}
					Column_Energy += Column_Power*T;
				}
			} else if (param->technode == 45 || param->technode == 32) {
				T_max = (0.0714*log(columnRes/1000)+0.7651)*1e-9;    // T_max = (0.0714*log(R_BL/1000)+0.7651)*10^-9;

				for (int i=1; i<levelOutput-1; i++){
					double ratio = Rref[i]/columnRes;
					double T = 0;
					if (ratio >= 20 || ratio <= 0.05) {
						T = 1e-9;
					} else {
						if (ratio <= low_bound){
							T = T_max * (3.7949*pow(ratio,3)-5.6685*pow(ratio,2)+2.6492*ratio+0.4807);    // y = 3.7949*x^3-5.6685*x^2+2.6492*x+0.4807
						} else if (mid_bound <= ratio <= up_bound){
							T = T_max * (0.000001*pow(ratio,4)-0.00006*pow(ratio,3)+0.0001*pow(ratio,2)-0.0171*ratio+1.0057);   // 0.000001*x^4-0.00006*x^3+0.0001*x^2-0.0171*x+1.0057;
						} else if (ratio>up_bound){
							T = T_max * (0.000001*pow(ratio,4)-0.00006*pow(ratio,3)+0.0001*pow(ratio,2)-0.0171*ratio+1.0057);
						} else {
							T = T_max;
						}
					}
					Column_Energy += Column_Power*T;
				}
			} else {   // technode below and equal to 22nm
				Column_Energy += Column_Power*1e-9*(levelOutput-1);
			}
		}
	}
	return Column_Energy;
}
