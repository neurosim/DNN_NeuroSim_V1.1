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
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include "Bus.h"
#include "SubArray.h"
#include "constant.h"
#include "formula.h"
#include "ProcessingUnit.h"
#include "Param.h"
#include "AdderTree.h"
#include "Bus.h"
#include "DFF.h"

using namespace std;

extern Param *param;

AdderTree *adderTreeNM;
Bus *busInputNM;
Bus *busOutputNM;
DFF *bufferInputNM;
DFF *bufferOutputNM;

AdderTree *adderTreeCM;
Bus *busInputCM;
Bus *busOutputCM;
DFF *bufferInputCM;
DFF *bufferOutputCM;

void ProcessingUnitInitialize(SubArray *& subArray, InputParameter& inputParameter, Technology& tech, MemCell& cell, int _numSubArrayRowNM, int _numSubArrayColNM, int _numSubArrayRowCM, int _numSubArrayColCM) {

	/*** circuit level parameters ***/
	switch(param->memcelltype) {
		case 3:     cell.memCellType = Type::FeFET; break;
		case 2:	    cell.memCellType = Type::RRAM; break;
		case 1:	    cell.memCellType = Type::SRAM; break;
		case -1:	break;
		default:	exit(-1);
	}
	switch(param->accesstype) {
		case 4:	    cell.accessType = none_access;  break;
		case 3:	    cell.accessType = diode_access; break;
		case 2:	    cell.accessType = BJT_access;   break;
		case 1:	    cell.accessType = CMOS_access;  break;
		case -1:	break;
		default:	exit(-1);
	}				
					
	switch(param->transistortype) {
		case 3:	    inputParameter.transistorType = TFET;          break;
		case 2:	    inputParameter.transistorType = FET_2D;        break;
		case 1:	    inputParameter.transistorType = conventional;  break;
		case -1:	break;
		default:	exit(-1);
	}
	
	switch(param->deviceroadmap) {
		case 2:	    inputParameter.deviceRoadmap = LSTP;  break;
		case 1:	    inputParameter.deviceRoadmap = HP;    break;
		case -1:	break;
		default:	exit(-1);
	}
	
	subArray = new SubArray(inputParameter, tech, cell);
	adderTreeNM = new AdderTree(inputParameter, tech, cell);
	busInputNM = new Bus(inputParameter, tech, cell);
	busOutputNM = new Bus(inputParameter, tech, cell);
	bufferInputNM = new DFF(inputParameter, tech, cell);
	bufferOutputNM = new DFF(inputParameter, tech, cell);
	adderTreeCM = new AdderTree(inputParameter, tech, cell);
	busInputCM = new Bus(inputParameter, tech, cell);
	busOutputCM = new Bus(inputParameter, tech, cell);
	bufferInputCM = new DFF(inputParameter, tech, cell);
	bufferOutputCM = new DFF(inputParameter, tech, cell);
		
	/* Create SubArray object and link the required global objects (not initialization) */
	inputParameter.temperature = param->temp;   // Temperature (K)
	inputParameter.processNode = param->technode;    // Technology node
	tech.Initialize(inputParameter.processNode, inputParameter.deviceRoadmap, inputParameter.transistorType);
	
	cell.resistanceOn = param->resistanceOn;	                                // Ron resistance at Vr in the reported measurement data (need to recalculate below if considering the nonlinearity)
	cell.resistanceOff = param->resistanceOff;	                                // Roff resistance at Vr in the reported measurement dat (need to recalculate below if considering the nonlinearity)
	cell.resistanceAvg = (cell.resistanceOn + cell.resistanceOff)/2;            // Average resistance (for energy estimation)
	cell.readVoltage = param->readVoltage;	                                    // On-chip read voltage for memory cell
	cell.readPulseWidth = param->readPulseWidth;
	cell.accessVoltage = param->accessVoltage;                                       // Gate voltage for the transistor in 1T1R
	cell.resistanceAccess = param->resistanceAccess;
	cell.featureSize = param->featuresize; 

	if (cell.memCellType == Type::SRAM) {   // SRAM
		cell.heightInFeatureSize = param->heightInFeatureSizeSRAM;                   // Cell height in feature size
		cell.widthInFeatureSize = param->widthInFeatureSizeSRAM;                     // Cell width in feature size
		cell.widthSRAMCellNMOS = param->widthSRAMCellNMOS;
		cell.widthSRAMCellPMOS = param->widthSRAMCellPMOS;
		cell.widthAccessCMOS = param->widthAccessCMOS;
		cell.minSenseVoltage = param->minSenseVoltage;
	} else {
		cell.heightInFeatureSize = (cell.accessType==CMOS_access)? param->heightInFeatureSize1T1R : param->heightInFeatureSizeCrossbar;         // Cell height in feature size
		cell.widthInFeatureSize = (cell.accessType==CMOS_access)? param->widthInFeatureSize1T1R : param->widthInFeatureSizeCrossbar;            // Cell width in feature size
	} 

	subArray->XNORparallelMode = param->XNORparallelMode;               
	subArray->XNORsequentialMode = param->XNORsequentialMode;             
	subArray->BNNparallelMode = param->BNNparallelMode;                
	subArray->BNNsequentialMode = param->BNNsequentialMode;              
	subArray->conventionalParallel = param->conventionalParallel;                  
	subArray->conventionalSequential = param->conventionalSequential;                 
	subArray->numRow = param->numRowSubArray;
	subArray->numCol = param->numRowSubArray;
	subArray->levelOutput = param->levelOutput;
	subArray->numColMuxed = param->numColMuxed;               // How many columns share 1 read circuit (for neuro mode with analog RRAM) or 1 S/A (for memory mode or neuro mode with digital RRAM)
    subArray->clkFreq = param->clkFreq;                       // Clock frequency
	subArray->relaxArrayCellHeight = param->relaxArrayCellHeight;
	subArray->relaxArrayCellWidth = param->relaxArrayCellWidth;
	subArray->numReadPulse = param->numBitInput;
	subArray->avgWeightBit = param->cellBit;
	subArray->numCellPerSynapse = param->numColPerSynapse;
	
	int numRow = param->numRowSubArray;
	int numCol = param->numColSubArray;
	
	if (subArray->numColMuxed > numCol) {                      // Set the upperbound of numColMuxed
		subArray->numColMuxed = numCol;
	}

	subArray->numReadCellPerOperationFPGA = numCol;	           // Not relevant for IMEC
	subArray->numWriteCellPerOperationFPGA = numCol;	       // Not relevant for IMEC
	subArray->numReadCellPerOperationMemory = numCol;          // Define # of SRAM read cells in memory mode because SRAM does not have S/A sharing (not relevant for IMEC)
	subArray->numWriteCellPerOperationMemory = numCol/8;       // # of write cells per operation in SRAM memory or the memory mode of multifunctional memory (not relevant for IMEC)
	subArray->numReadCellPerOperationNeuro = numCol;           // # of SRAM read cells in neuromorphic mode
	subArray->numWriteCellPerOperationNeuro = numCol;	       // For SRAM or analog RRAM in neuro mode
    subArray->maxNumWritePulse = MAX(cell.maxNumLevelLTP, cell.maxNumLevelLTD);

	int numSubArrayRowNM = _numSubArrayRowNM;
	int numSubArrayColNM = _numSubArrayColNM;
	int numSubArrayRowCM = _numSubArrayRowCM;
	int numSubArrayColCM = _numSubArrayColCM;

	/*** initialize modules ***/
	subArray->Initialize(numRow, numCol, param->unitLengthWireResistance);        // initialize subArray
	subArray->CalculateArea();
	
	if (param->novelMapping) {
		if (param->parallelRead) {
			adderTreeNM->Initialize(numSubArrayRowNM, log2((double)param->levelOutput)+param->numBitInput+1, ceil((double)numSubArrayColNM*(double)numCol/(double)param->numColMuxed));
		} else {
			adderTreeNM->Initialize(numSubArrayRowNM, (log2((double)numRow)+param->cellBit-1)+param->numBitInput+1, ceil((double)numSubArrayColNM*(double)numCol/(double)param->numColMuxed));
		}
		
		bufferInputNM->Initialize(param->numBitInput*numRow, param->clkFreq);
		if (param->parallelRead) {
			bufferOutputNM->Initialize((numCol/param->numColMuxed)*(log2((double)param->levelOutput)+param->numBitInput+adderTreeNM->numStage), param->clkFreq);
		} else {
			bufferOutputNM->Initialize((numCol/param->numColMuxed)*((log2((double)numRow)+param->cellBit-1)+param->numBitInput+adderTreeNM->numStage), param->clkFreq);
		}
		
		busInputNM->Initialize(HORIZONTAL, numSubArrayRowNM, numSubArrayColNM, 0, numRow, subArray->height, subArray->width);
		busOutputNM->Initialize(VERTICAL, numSubArrayRowNM, numSubArrayColNM, 0, numCol, subArray->height, subArray->width);
	}
	if (param->parallelRead) {
		adderTreeCM->Initialize(numSubArrayRowCM, log2((double)param->levelOutput)+param->numBitInput+1, ceil((double)numSubArrayColCM*(double)numCol/(double)param->numColMuxed));
	} else {
		adderTreeCM->Initialize(numSubArrayRowCM, (log2((double)numRow)+param->cellBit-1)+param->numBitInput+1, ceil((double)numSubArrayColCM*(double)numCol/(double)param->numColMuxed));
	}
	
	bufferInputCM->Initialize(param->numBitInput*numRow, param->clkFreq);
	if (param->parallelRead) {
		bufferOutputCM->Initialize((numCol/param->numColMuxed)*(log2((double)param->levelOutput)+param->numBitInput+adderTreeCM->numStage), param->clkFreq);
	} else {
		bufferOutputCM->Initialize((numCol/param->numColMuxed)*((log2((double)numRow)+param->cellBit-1)+param->numBitInput+adderTreeCM->numStage), param->clkFreq);
	}
	
	busInputCM->Initialize(HORIZONTAL, numSubArrayRowCM, numSubArrayColCM, 0, numRow, subArray->height, subArray->width);
	busOutputCM->Initialize(VERTICAL, numSubArrayRowCM, numSubArrayColCM, 0, numCol, subArray->height, subArray->width);
}


vector<double> ProcessingUnitCalculateArea(SubArray *subArray, int numSubArrayRow, int numSubArrayCol, bool NMpe, double *height, double *width, double *bufferArea) {
	vector<double> areaResults;
	*height = 0;
	*width = 0;
	*bufferArea = 0;
	double area = 0;
	
	subArray->CalculateArea();
	if (NMpe) {
		adderTreeNM->CalculateArea(NULL, subArray->width, NONE);
		bufferInputNM->CalculateArea(numSubArrayRow*subArray->height, NULL, NONE);
		bufferOutputNM->CalculateArea(NULL, numSubArrayCol*subArray->width, NONE);
		
		busInputNM->CalculateArea(1, true); 
		busOutputNM->CalculateArea(1, true);	
		area += subArray->usedArea * (numSubArrayRow*numSubArrayCol) + adderTreeNM->area + bufferInputNM->area + bufferOutputNM->area;
		
		*height = sqrt(area);
		*width = area/(*height);
		
		areaResults.push_back(area);
		areaResults.push_back(subArray->areaADC*(numSubArrayRow*numSubArrayCol));
		areaResults.push_back(subArray->areaAccum*(numSubArrayRow*numSubArrayCol)+adderTreeNM->area);
		areaResults.push_back(subArray->areaOther*(numSubArrayRow*numSubArrayCol)+ bufferInputNM->area + bufferOutputNM->area);
		areaResults.push_back(subArray->areaArray*(numSubArrayRow*numSubArrayCol));
	} else {
		adderTreeCM->CalculateArea(NULL, subArray->width, NONE);
		bufferInputCM->CalculateArea(numSubArrayRow*subArray->height, NULL, NONE);
		bufferOutputCM->CalculateArea(NULL, numSubArrayCol*subArray->width, NONE);
		
		busInputCM->CalculateArea(1, true); 
		busOutputCM->CalculateArea(1, true);	
		area += subArray->usedArea * (numSubArrayRow*numSubArrayCol) + adderTreeCM->area + bufferInputCM->area + bufferOutputCM->area;
		
		*height = sqrt(area);
		*width = area/(*height);
		
		areaResults.push_back(area);
		areaResults.push_back(subArray->areaADC*(numSubArrayRow*numSubArrayCol));
		areaResults.push_back(subArray->areaAccum*(numSubArrayRow*numSubArrayCol)+adderTreeCM->area);
		areaResults.push_back(subArray->areaOther*(numSubArrayRow*numSubArrayCol)+ bufferInputCM->area + bufferOutputCM->area);
		areaResults.push_back(subArray->areaArray*(numSubArrayRow*numSubArrayCol));
	}
	
	return areaResults;
}


double ProcessingUnitCalculatePerformance(SubArray *subArray, const vector<vector<double> > &newMemory, const vector<vector<double> > &oldMemory, 
											const vector<vector<double> > &inputVector,
											int arrayDupRow, int arrayDupCol, int numSubArrayRow, int numSubArrayCol, int weightMatrixRow,
											int weightMatrixCol, int numInVector, MemCell& cell, bool NMpe, double *readLatency, double *readDynamicEnergy, double *leakage, 
											double *bufferLatency, double *bufferDynamicEnergy, double *icLatency, double *icDynamicEnergy,
											double *coreLatencyADC, double *coreLatencyAccum, double *coreLatencyOther, double *coreEnergyADC, 
											double *coreEnergyAccum, double *coreEnergyOther) {
	
	/*** define how many subArray are used to map the whole layer ***/
	*readLatency = 0;
	*readDynamicEnergy = 0;
	*leakage = 0;
	*bufferLatency = 0;
	*bufferDynamicEnergy = 0;
	*icLatency = 0;
	*icDynamicEnergy = 0;
	*coreEnergyADC = 0;
	*coreEnergyAccum = 0;
	*coreEnergyOther = 0;
	*coreLatencyADC = 0;
	*coreLatencyAccum = 0;
	*coreLatencyOther = 0;
	
	double subArrayReadLatency, subArrayReadDynamicEnergy, subArrayLeakage, subArrayLatencyADC, subArrayLatencyAccum, subArrayLatencyOther;

	if (arrayDupRow*arrayDupCol > 1) {
		// weight matrix is duplicated among subArray
		if (arrayDupRow < numSubArrayRow || arrayDupCol < numSubArrayCol) {
			// a couple of subArrays are mapped by the matrix
			// need to redefine the data-grab start-point
			for (int i=0; i<ceil((double) weightMatrixRow/(double) param->numRowSubArray); i++) {
				for (int j=0; j<ceil((double) weightMatrixCol/(double) param->numColSubArray); j++) {
					int numRowMatrix = min(param->numRowSubArray, weightMatrixRow-i*param->numRowSubArray);
					int numColMatrix = min(param->numColSubArray, weightMatrixCol-j*param->numColSubArray);
					
					if ((i*param->numRowSubArray < weightMatrixRow) && (j*param->numColSubArray < weightMatrixCol) && (i*param->numRowSubArray < weightMatrixRow) ) {
						// assign weight and input to specific subArray
						vector<vector<double> > subArrayMemory;
						subArrayMemory = CopySubArray(newMemory, i*param->numRowSubArray, j*param->numColSubArray, numRowMatrix, numColMatrix);
						vector<vector<double> > subArrayInput;
						subArrayInput = CopySubInput(inputVector, i*param->numRowSubArray, numInVector, numRowMatrix);
						
						subArrayReadLatency = 0;
						subArrayLatencyADC = 0;
						subArrayLatencyAccum = 0;
						subArrayLatencyOther = 0;

						for (int k=0; k<numInVector; k++) {                 // calculate single subArray through the total input vectors
							double activityRowRead = 0;
							vector<double> input; 
							input = GetInputVector(subArrayInput, k, &activityRowRead);
							subArray->activityRowRead = activityRowRead;
							
							int cellRange = pow(2, param->cellBit);
							if (param->parallelRead) {
								subArray->levelOutput = param->levelOutput;               // # of levels of the multilevelSenseAmp output
							} else {
								subArray->levelOutput = cellRange;
							}
							
							vector<double> columnResistance;
							columnResistance = GetColumnResistance(input, subArrayMemory, cell, param->parallelRead, subArray->resCellAccess);
							
							subArray->CalculateLatency(1e20, columnResistance);
							subArray->CalculatePower(columnResistance);
							
							subArrayReadLatency += subArray->readLatency;
							*readDynamicEnergy += subArray->readDynamicEnergy;
							subArrayLeakage = subArray->leakage;

							subArrayLatencyADC += subArray->readLatencyADC;
							subArrayLatencyAccum += subArray->readLatencyAccum;
							subArrayLatencyOther += subArray->readLatencyOther;
							
							*coreEnergyADC += subArray->readDynamicEnergyADC;
							*coreEnergyAccum += subArray->readDynamicEnergyAccum;
							*coreEnergyOther += subArray->readDynamicEnergyOther;
						}
						if (NMpe) {
							adderTreeNM->CalculateLatency((int)(numInVector/param->numBitInput)*param->numColMuxed, ceil((double) weightMatrixRow/(double) param->numRowSubArray), 0);
							adderTreeNM->CalculatePower((int)(numInVector/param->numBitInput)*param->numColMuxed, ceil((double) weightMatrixRow/(double) param->numRowSubArray));
							*readLatency = MAX(subArrayReadLatency + adderTreeNM->readLatency, (*readLatency));
							*readDynamicEnergy += adderTreeNM->readDynamicEnergy;
							*coreLatencyADC = MAX(subArrayLatencyADC, (*coreLatencyADC));
							*coreLatencyAccum = MAX(subArrayLatencyAccum + adderTreeNM->readLatency, (*coreLatencyAccum));
							*coreLatencyOther = MAX(subArrayLatencyOther, (*coreLatencyOther));
							*coreEnergyAccum += adderTreeNM->readDynamicEnergy;
						} else {
							adderTreeCM->CalculateLatency((int)(numInVector/param->numBitInput)*param->numColMuxed, ceil((double) weightMatrixRow/(double) param->numRowSubArray), 0);
							adderTreeCM->CalculatePower((int)(numInVector/param->numBitInput)*param->numColMuxed, ceil((double) weightMatrixRow/(double) param->numRowSubArray));
							*readLatency = MAX(subArrayReadLatency + adderTreeCM->readLatency, (*readLatency));
							*readDynamicEnergy += adderTreeCM->readDynamicEnergy;
							*coreLatencyADC = MAX(subArrayLatencyADC, (*coreLatencyADC));
							*coreLatencyAccum = MAX(subArrayLatencyAccum + adderTreeCM->readLatency, (*coreLatencyAccum));
							*coreLatencyOther = MAX(subArrayLatencyOther, (*coreLatencyOther));
							*coreEnergyAccum += adderTreeCM->readDynamicEnergy;
						}
					}
				}
			}
			// considering speedup, the latency of processing each layer is decreased
			*readLatency = (*readLatency)/(arrayDupRow*arrayDupCol);
			*coreLatencyADC = (*coreLatencyADC)/(arrayDupRow*arrayDupCol);
			*coreLatencyAccum = (*coreLatencyAccum)/(arrayDupRow*arrayDupCol);
			*coreLatencyOther = (*coreLatencyOther)/(arrayDupRow*arrayDupCol);
		} else {
			// assign weight and input to specific subArray
			vector<vector<double> > subArrayMemory;
			subArrayMemory = CopySubArray(newMemory, 0, 0, weightMatrixRow, weightMatrixCol);
			vector<vector<double> > subArrayInput;
			subArrayInput = CopySubInput(inputVector, 0, numInVector, weightMatrixRow);

			subArrayReadLatency = 0;
			subArrayLatencyADC = 0;
			subArrayLatencyAccum = 0;
			subArrayLatencyOther = 0;

			for (int k=0; k<numInVector; k++) {                 // calculate single subArray through the total input vectors
				double activityRowRead = 0;
				vector<double> input;
				input = GetInputVector(subArrayInput, k, &activityRowRead);
				subArray->activityRowRead = activityRowRead;
				int cellRange = pow(2, param->cellBit);
				
				if (param->parallelRead) {
					subArray->levelOutput = param->levelOutput;               // # of levels of the multilevelSenseAmp output
				} else {
					subArray->levelOutput = cellRange;
				}
				
				vector<double> columnResistance;
				columnResistance = GetColumnResistance(input, subArrayMemory, cell, param->parallelRead, subArray->resCellAccess);
				
				subArray->CalculateLatency(1e20, columnResistance);
				subArray->CalculatePower(columnResistance);
				
				subArrayReadLatency += subArray->readLatency;
				*readDynamicEnergy += subArray->readDynamicEnergy;
				subArrayLeakage = subArray->leakage;
				
				subArrayLatencyADC += subArray->readLatencyADC;
				subArrayLatencyAccum += subArray->readLatencyAccum;
				subArrayLatencyOther += subArray->readLatencyOther;
				
				*coreEnergyADC += subArray->readDynamicEnergyADC;
				*coreEnergyAccum += subArray->readDynamicEnergyAccum;
				*coreEnergyOther += subArray->readDynamicEnergyOther;
			}
			
			// do not pass adderTree 
			*readLatency = subArrayReadLatency/(arrayDupRow*arrayDupCol);
			*coreLatencyADC = subArrayLatencyADC/(arrayDupRow*arrayDupCol);
			*coreLatencyAccum = subArrayLatencyAccum/(arrayDupRow*arrayDupCol);
			*coreLatencyOther = subArrayLatencyOther/(arrayDupRow*arrayDupCol);
		}
	} else {
		// weight matrix is further partitioned inside PE (among subArray) --> no duplicated
		for (int i=0; i<numSubArrayRow/*ceil((double) weightMatrixRow/(double) param->numRowSubArray)*/; i++) {
			for (int j=0; j<numSubArrayCol/*ceil((double) weightMatrixCol/(double) param->numColSubArray)*/; j++) {
				if ((i*param->numRowSubArray < weightMatrixRow) && (j*param->numColSubArray < weightMatrixCol) && (i*param->numRowSubArray < weightMatrixRow) ) {
					int numRowMatrix = min(param->numRowSubArray, weightMatrixRow-i*param->numRowSubArray);
					int numColMatrix = min(param->numColSubArray, weightMatrixCol-j*param->numColSubArray);
					// assign weight and input to specific subArray
					vector<vector<double> > subArrayMemory;
					subArrayMemory = CopySubArray(newMemory, i*param->numRowSubArray, j*param->numColSubArray, numRowMatrix, numColMatrix);
					vector<vector<double> > subArrayInput;
					subArrayInput = CopySubInput(inputVector, i*param->numRowSubArray, numInVector, numRowMatrix);
					
					subArrayReadLatency = 0;
					subArrayLatencyADC = 0;
					subArrayLatencyAccum = 0;
					subArrayLatencyOther = 0;
					
					for (int k=0; k<numInVector; k++) {                 // calculate single subArray through the total input vectors
						double activityRowRead = 0;
						vector<double> input;
						input = GetInputVector(subArrayInput, k, &activityRowRead);
						subArray->activityRowRead = activityRowRead;
						
						int cellRange = pow(2, param->cellBit);
						if (param->parallelRead) {
							subArray->levelOutput = param->levelOutput;               // # of levels of the multilevelSenseAmp output
						} else {
							subArray->levelOutput = cellRange;
						}
						
						vector<double> columnResistance;
						columnResistance = GetColumnResistance(input, subArrayMemory, cell, param->parallelRead, subArray->resCellAccess);

						subArray->CalculateLatency(1e20, columnResistance);
						subArray->CalculatePower(columnResistance);
						
						subArrayReadLatency += subArray->readLatency;
						*readDynamicEnergy += subArray->readDynamicEnergy;
						subArrayLeakage = subArray->leakage;
						
						subArrayLatencyADC += subArray->readLatencyADC;
						subArrayLatencyAccum += subArray->readLatencyAccum;
						subArrayLatencyOther += subArray->readLatencyOther;
						
						*coreEnergyADC += subArray->readDynamicEnergyADC;
						*coreEnergyAccum += subArray->readDynamicEnergyAccum;
						*coreEnergyOther += subArray->readDynamicEnergyOther;
						
					}
					*readLatency = MAX(subArrayReadLatency, (*readLatency));
					*coreLatencyADC = MAX(subArrayLatencyADC, (*coreLatencyADC));
					*coreLatencyAccum = MAX(subArrayLatencyAccum, (*coreLatencyAccum));
					*coreLatencyOther = MAX(subArrayLatencyOther, (*coreLatencyOther));
				}
			}
		}
		if (NMpe) {
			adderTreeNM->CalculateLatency((int)(numInVector/param->numBitInput)*param->numColMuxed, ceil((double) weightMatrixRow/(double) param->numRowSubArray), 0);
			adderTreeNM->CalculatePower((int)(numInVector/param->numBitInput)*param->numColMuxed, ceil((double) weightMatrixRow/(double) param->numRowSubArray));
			*readLatency += adderTreeNM->readLatency;
			*coreLatencyAccum += adderTreeNM->readLatency;
			*readDynamicEnergy += adderTreeNM->readDynamicEnergy;
			*coreEnergyAccum += adderTreeNM->readDynamicEnergy;
		} else {
			adderTreeCM->CalculateLatency((int)(numInVector/param->numBitInput)*param->numColMuxed, ceil((double) weightMatrixRow/(double) param->numRowSubArray), 0);
			adderTreeCM->CalculatePower((int)(numInVector/param->numBitInput)*param->numColMuxed, ceil((double) weightMatrixRow/(double) param->numRowSubArray));
			*readLatency += adderTreeCM->readLatency;
			*coreLatencyAccum += adderTreeCM->readLatency;
			*readDynamicEnergy += adderTreeCM->readDynamicEnergy;
			*coreEnergyAccum += adderTreeCM->readDynamicEnergy;
		}
		
	}
	//considering buffer activation: no matter speedup or not, the total number of data transferred is fixed
	// input buffer: total num of data loaded in = weightMatrixRow*numInVector
	// output buffer: total num of data transferred = weightMatrixRow*numInVector/param->numBitInput (total num of IFM in the PE) *adderTree->numAdderTree*adderTree->numAdderBit (bit precision of OFMs) 
	if (NMpe) {
		bufferInputNM->CalculateLatency(0, numInVector*ceil((double) weightMatrixRow/(double) param->numRowSubArray));
		bufferOutputNM->CalculateLatency(0, numInVector/param->numBitInput);
		bufferInputNM->CalculatePower(weightMatrixRow/param->numRowPerSynapse, numInVector);
		bufferOutputNM->CalculatePower(weightMatrixCol/param->numColPerSynapse*adderTreeNM->numAdderBit, numInVector/param->numBitInput);
		
		busInputNM->CalculateLatency(weightMatrixRow/param->numRowPerSynapse*numInVector/(busInputNM->busWidth)); 
		busInputNM->CalculatePower(busInputNM->busWidth, weightMatrixRow/param->numRowPerSynapse*numInVector/(busInputNM->busWidth));
		
		if (param->parallelRead) {
			busOutputNM->CalculateLatency((weightMatrixCol/param->numColPerSynapse*log2((double)param->levelOutput)*numInVector/param->numBitInput)/(busOutputNM->numRow*busOutputNM->busWidth));
			busOutputNM->CalculatePower(busOutputNM->numRow*busOutputNM->busWidth, (weightMatrixCol/param->numColPerSynapse*log2((double)param->levelOutput)*numInVector/param->numBitInput)/(busOutputNM->numRow*busOutputNM->busWidth));
		} else {
			busOutputNM->CalculateLatency((weightMatrixCol/param->numColPerSynapse*(log2((double)param->numRowSubArray)+param->cellBit-1)*numInVector/param->numBitInput)/(busOutputNM->numRow*busOutputNM->busWidth));
			busOutputNM->CalculatePower(busOutputNM->numRow*busOutputNM->busWidth, (weightMatrixCol/param->numColPerSynapse*(log2((double)param->numRowSubArray)+param->cellBit-1)*numInVector/param->numBitInput)/(busOutputNM->numRow*busOutputNM->busWidth));
		}

		*bufferLatency = bufferInputNM->readLatency + bufferOutputNM->readLatency;
		*icLatency = busInputNM->readLatency + busOutputNM->readLatency;
		*bufferDynamicEnergy += bufferInputNM->readDynamicEnergy + bufferOutputNM->readDynamicEnergy;
		*icDynamicEnergy += busInputNM->readDynamicEnergy + busOutputNM->readDynamicEnergy;
		*leakage = subArrayLeakage*numSubArrayRow*numSubArrayCol + adderTreeNM->leakage + bufferInputNM->leakage + bufferOutputNM->leakage;
	} else {
		bufferInputCM->CalculateLatency(0, numInVector*ceil((double) weightMatrixRow/(double) param->numRowSubArray));
		bufferOutputCM->CalculateLatency(0, numInVector/param->numBitInput);
		bufferInputCM->CalculatePower(weightMatrixRow/param->numRowPerSynapse, numInVector);
		bufferOutputCM->CalculatePower(weightMatrixCol/param->numColPerSynapse*adderTreeNM->numAdderBit, numInVector/param->numBitInput);
		
		busInputCM->CalculateLatency(weightMatrixRow/param->numRowPerSynapse*numInVector/(busInputCM->busWidth)); 
		busInputCM->CalculatePower(busInputCM->busWidth, weightMatrixRow/param->numRowPerSynapse*numInVector/(busInputCM->busWidth));
		
		if (param->parallelRead) {
			busOutputCM->CalculateLatency((weightMatrixCol/param->numColPerSynapse*log2((double)param->levelOutput)*numInVector/param->numBitInput)/(busOutputCM->numRow*busOutputCM->busWidth));
			busOutputCM->CalculatePower(busOutputCM->numRow*busOutputCM->busWidth, (weightMatrixCol/param->numColPerSynapse*log2((double)param->levelOutput)*numInVector/param->numBitInput)/(busOutputCM->numRow*busOutputCM->busWidth));
		} else {
			busOutputCM->CalculateLatency((weightMatrixCol/param->numColPerSynapse*(log2((double)param->numRowSubArray)+param->cellBit-1)*numInVector/param->numBitInput)/(busOutputCM->numRow*busOutputCM->busWidth));
			busOutputCM->CalculatePower(busOutputCM->numRow*busOutputCM->busWidth, (weightMatrixCol/param->numColPerSynapse*(log2((double)param->numRowSubArray)+param->cellBit-1)*numInVector/param->numBitInput)/(busOutputCM->numRow*busOutputCM->busWidth));
		}

		*bufferLatency = bufferInputCM->readLatency + bufferOutputCM->readLatency;
		*icLatency = busInputCM->readLatency + busOutputCM->readLatency;
		*bufferDynamicEnergy += bufferInputCM->readDynamicEnergy + bufferOutputCM->readDynamicEnergy;
		*icDynamicEnergy += busInputCM->readDynamicEnergy + busOutputCM->readDynamicEnergy;
		*leakage = subArrayLeakage*numSubArrayRow*numSubArrayCol + adderTreeCM->leakage + bufferInputCM->leakage + bufferOutputCM->leakage;
	}
	*readLatency += (*bufferLatency) + (*icLatency);
	*readDynamicEnergy += (*bufferDynamicEnergy) + (*icDynamicEnergy);
	*coreLatencyOther += (*bufferLatency) + (*icLatency);
	*coreEnergyOther += (*bufferDynamicEnergy) + (*icDynamicEnergy);
}


vector<vector<double> > CopySubArray(const vector<vector<double> > &orginal, int positionRow, int positionCol, int numRow, int numCol) {
	vector<vector<double> > copy;
	for (int i=0; i<numRow; i++) {
		vector<double> copyRow;
		for (int j=0; j<numCol; j++) {
			copyRow.push_back(orginal[positionRow+i][positionCol+j]);
		}
		copy.push_back(copyRow);
		copyRow.clear();
	}
	return copy;
	copy.clear();
} 


vector<vector<double> > CopySubInput(const vector<vector<double> > &orginal, int positionRow, int numInputVector, int numRow) {
	vector<vector<double> > copy;
	for (int i=0; i<numRow; i++) {
		vector<double> copyRow;
		for (int j=0; j<numInputVector; j++) {
			copyRow.push_back(orginal[positionRow+i][j]);
		}
		copy.push_back(copyRow);
		copyRow.clear();
	}
	return copy;
	copy.clear();
}


vector<double> GetInputVector(const vector<vector<double> > &input, int numInput, double *activityRowRead) {
	vector<double> copy;
	for (int i=0; i<input.size(); i++) {
		double x = input[i][numInput];
		copy.push_back(x);   
	}  
	double numofreadrow = 0;  // initialize readrowactivity parameters
	for (int i=0; i<input.size(); i++) {
		if (copy[i] != 0) {
			numofreadrow += 1;
		}else {
			numofreadrow += 0;
		}
	}
	double totalnumRow = input.size();
	*(activityRowRead) = numofreadrow/totalnumRow;
	return copy;
	copy.clear();
} 


vector<double> GetColumnResistance(const vector<double> &input, const vector<vector<double> > &weight, MemCell& cell, bool parallelRead, double resCellAccess) {
	vector<double> resistance;
	vector<double> conductance;
	double columnG = 0; 
	
	for (int j=0; j<weight[0].size(); j++) {
		int activatedRow = 0;
		columnG = 0;
		for (int i=0; i<weight.size(); i++) {
			if (cell.memCellType == Type::RRAM) {	// eNVM
				double totalWireResistance;
				if (cell.accessType == CMOS_access) {
					totalWireResistance = (double) 1.0/weight[i][j] + (j + 1) * param->wireResistanceRow + (weight.size() - i) * param->wireResistanceCol + cell.resistanceAccess;
				} else {
					totalWireResistance = (double) 1.0/weight[i][j] + (j + 1) * param->wireResistanceRow + (weight.size() - i) * param->wireResistanceCol;
				}
				if ((int) input[i] == 1) {
					columnG += (double) 1.0/totalWireResistance;
					activatedRow += 1 ;
				} else {
					columnG += 0;
				}
			} else if (cell.memCellType == Type::FeFET) {
				double totalWireResistance;
				totalWireResistance = (double) 1.0/weight[i][j] + (j + 1) * param->wireResistanceRow + (weight.size() - i) * param->wireResistanceCol;
				if ((int) input[i] == 1) {
					columnG += (double) 1.0/totalWireResistance;
					activatedRow += 1 ;
				} else {
					columnG += 0;
				}
				
			} else if (cell.memCellType == Type::SRAM) {	
				// SRAM: weight value do not affect sense energy --> read energy calculated in subArray.cpp (based on wireRes wireCap etc)
				double totalWireResistance = (double) (resCellAccess + param->wireResistanceCol);
				if ((int) input[i] == 1) {
					columnG += (double) 1.0/totalWireResistance;
					activatedRow += 1 ;
				} else {
					columnG += 0;
				}
			}
		}
		
		if (cell.memCellType == Type::RRAM || cell.memCellType == Type::FeFET) {
			if (!parallelRead) {  
				conductance.push_back((double) columnG/activatedRow);
			} else {
				conductance.push_back(columnG);
			}
		} else {
			conductance.push_back(columnG);
		}
	}
	// covert conductance to resistance
	for (int i=0; i<weight[0].size(); i++) {
		resistance.push_back((double) 1.0/conductance[i]);
	}
		
	return resistance;
	resistance.clear();
} 







