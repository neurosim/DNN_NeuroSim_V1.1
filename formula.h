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

#ifndef NEUROSIM_FORMULA_H_
#define NEUROSIM_FORMULA_H_

#include "Technology.h"

#define MAX(a,b) (((a)> (b))?(a):(b))
#define MIN(a,b) (((a)< (b))?(a):(b))

/* Calculate MOSFET gate capacitance */
double CalculateGateCap(double width, Technology tech);

double CalculateGateArea(
		int gateType, int numInput,
		double widthNMOS, double widthPMOS,
		double heightTransistorRegion, Technology tech,
		double *height, double *width);

/* Calculate the capacitance of a logic gate */
void CalculateGateCapacitance(
		int gateType, int numInput,
		double widthNMOS, double widthPMOS,
		double heightTransistorRegion, Technology tech,
		double *capInput, double *capOutput);

double CalculateDrainCap(
		double width, int type,
		double heightTransistorRegion, Technology tech);

double CalculateGateLeakage(
		int gateType, int numInput,
		double widthNMOS, double widthPMOS,
		double temperature, Technology tech);

double CalculateOnResistance(double width, int type, double temperature, Technology tech);

double CalculateTransconductance(double width, int type, Technology tech);

double horowitz(double tr, double beta, double rampInput, double *rampOutput);

double CalculatePassGateArea(double widthNMOS, double widthPMOS, Technology tech, int numFold, double *height, double *width);

double NonlinearResistance(double R, double NL, double Vw, double Vr, double V);

#endif /* FORMULA_H_ */
