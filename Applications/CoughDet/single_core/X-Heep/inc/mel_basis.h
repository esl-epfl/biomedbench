/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////////
// Author:          Stefano Albini                  //
// Contributions:   Dimitrios Samakovlis            //
// Date:            September 2023                  //
//////////////////////////////////////////////////////



#ifndef _MEL_BASIS_SMALL_H_
#define _MEL_BASIS_SMALL_H_

// This file contains the MEL matrix for projecting a power spectrum into a mel basis
// The numbers are hardcoded and taken from python.

//////////////////////////////////////////////////////////////////////////////////
// Note that this only works when the RFFT size used is 2048 (so the output		//
// will be 1025 samples)														//
//////////////////////////////////////////////////////////////////////////////////

#define MEL_ROWS		128
#define MEL_COLUMNS		1025

#define MAX_NZ_ELEMS    50  // max number of non-zero elements 

/* 
    Indexes of the first and last non-zero elements for each row of the mel basis. 
    For each row, the non-zero elements are stored one after the other, so I just
    need to store the indexes of the first and last element
*/
extern const int mel_nz_indexes[MEL_ROWS][2];

extern const float mel_basis[MEL_ROWS][MAX_NZ_ELEMS];

#endif
