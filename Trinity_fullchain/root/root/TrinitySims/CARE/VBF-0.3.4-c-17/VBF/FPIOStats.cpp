/*
 * FPIOStats.cpp -- statistics for FPIO
 * by Filip Pizlo, 2002, 2003
 */

#include "FPIO.h"

#include <stdio.h>

void FPStatisticsAware::addBlitz(uword64 amount) {
    if (stats!=NULL) stats->addBlitz(amount);
}

void FPStatisticsAware::addSoak(uword64 amount) {
    if (stats!=NULL) stats->addSoak(amount);
}

FPIOStatistics::FPIOStatistics():
    sum_blitz(0lu),
    sum_soak(0lu),
    num_blitz(0lu),
    num_soak(0lu)
{
}

FPIOStatistics::FPIOStatistics(const FPIOStatistics &other) {
    sum_blitz=other.sum_blitz;
    sum_soak=other.sum_soak;
    num_blitz=other.num_blitz;
    num_soak=other.num_soak;
}

FPIOStatistics &FPIOStatistics::operator=(const FPIOStatistics &other) {
    sum_blitz=other.sum_blitz;
    sum_soak=other.sum_soak;
    num_blitz=other.num_blitz;
    num_soak=other.num_soak;
    return *this;
}

FPIOStatistics::~FPIOStatistics() {
}

void FPIOStatistics::addBlitz(uword64 amount) {
    sum_blitz+=amount;
    num_blitz++;
}

void FPIOStatistics::addSoak(uword64 amount) {
    sum_soak+=amount;
    num_soak++;
}

uword64 FPIOStatistics::getSumBlitz() {
    return sum_blitz;
}

uword64 FPIOStatistics::getNumBlitz() {
    return num_blitz;
}

uword64 FPIOStatistics::getSumSoak() {
    return sum_soak;
}

uword64 FPIOStatistics::getNumSoak() {
    return num_soak;
}

uword64 FPIOStatistics::getMeanBlitz() {
    uword64 ret;
    if (num_blitz) {
        ret=sum_blitz/num_blitz;
    } else {
        ret=0lu;
    }
    return ret;
}

uword64 FPIOStatistics::getMeanSoak() {
    uword64 ret;
    if (num_soak) {
        ret=sum_soak/num_soak;
    } else {
        ret=0lu;
    }
    return ret;
}

void FPIOStatistics::resetStatistics() {
    sum_blitz=0;
    sum_soak=0;
    num_blitz=0;
    num_soak=0;
}











// **************************************************************************
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 
// 2011, 2012 Purdue University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//  this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are 
// those of the authors and should not be interpreted as representing official
// policies, either expressed or implied, of Purdue University or the VERITAS 
// Collaboration.
// **************************************************************************
