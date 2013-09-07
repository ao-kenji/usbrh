/* 
 * $Id: calc.c,v 1.1.1.1 2006/07/22 15:26:25 aoyama Exp $
 * 
 * Copyright (c) 2006 Kenji AOYAMA <aoyama@nk-home.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* prototypes */
float calc_temp(unsigned int);
float calc_humid(unsigned int, float);

/*
 * Based on:
 * "SHTxx Humidity & Temperature Sensmitter Application Note Sample
 *  Code"  by Sensirion AG.
 */

#define C1 -4.0
#define C2 0.0405
#define C3 -0.0000028
#define T1 0.01
#define T2 0.00008

float
calc_temp(unsigned int ticks)
{
	return ((float)(ticks - 4000) / 100);
}

float
calc_humid(unsigned int ticks, float temp)
{
	float h_linear, h_true;

	h_linear = C3 * (float)ticks * (float)ticks + C2 * (float)ticks + C1;
	h_true = (temp - 25.0) * (T1 + T2 * ticks) + h_linear;

	return h_true;
}
