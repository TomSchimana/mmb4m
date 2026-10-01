/*-*****************************************************************************

MMBasic for Linux (MMB4L)

maths.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "../common/complex_compat.h"
#include "../common/display.h"
#include "../common/mmb4l.h"
#include "../common/interrupt.h"
#include "../common/mmtime.h"
#include "../core/MMBasic.h"
#include "../core/maths.h"
#include "../core/Functions.h"
#include "../third_party/aes.h"

#define CRC4_DEFAULT_POLYNOME       0x03
#define CRC4_ITU                    0x03


// CRC 8
#define CRC8_DEFAULT_POLYNOME       0x07
#define CRC8_DVB_S2                 0xD5
#define CRC8_AUTOSAR                0x2F
#define CRC8_BLUETOOTH              0xA7
#define CRC8_CCITT                  0x07
#define CRC8_DALLAS_MAXIM           0x31                // oneWire
#define CRC8_DARC                   0x39
#define CRC8_GSM_B                  0x49
#define CRC8_SAEJ1850               0x1D
#define CRC8_WCDMA                  0x9B


// CRC 12
#define CRC12_DEFAULT_POLYNOME      0x080D
#define CRC12_CCITT                 0x080F
#define CRC12_CDMA2000              0x0F13
#define CRC12_GSM                   0x0D31


// CRC 16
#define CRC16_DEFAULT_POLYNOME      0x1021
#define CRC16_CHAKRAVARTY           0x2F15
#define CRC16_ARINC                 0xA02B
#define CRC16_CCITT                 0x1021
#define CRC16_CDMA2000              0xC867
#define CRC16_DECT                  0x0589
#define CRC16_T10_DIF               0x8BB7
#define CRC16_DNP                   0x3D65
#define CRC16_IBM                   0x8005
#define CRC16_OPENSAFETY_A          0x5935
#define CRC16_OPENSAFETY_B          0x755B
#define CRC16_PROFIBUS              0x1DCF


// CRC 32
#define CRC32_DEFAULT_POLYNOME      0x04C11DB7
#define CRC32_ISO3309               0x04C11DB7
#define CRC32_CASTAGNOLI            0x1EDC6F41
#define CRC32_KOOPMAN               0x741B8CD7
#define CRC32_KOOPMAN_2             0x32583499
#define CRC32_Q                     0x814141AB


// CRC 64
#define CRC64_DEFAULT_POLYNOME      0x42F0E1EBA9EA3693
#define CRC64_ECMA64                0x42F0E1EBA9EA3693
#define CRC64_ISO64                 0x000000000000001B

#define STATE_VECTOR_LENGTH 624
#define STATE_VECTOR_M      397 /* changes to STATE_VECTOR_LENGTH also require changes to this */

typedef struct tagMTRand {
  unsigned long mt[STATE_VECTOR_LENGTH];
  int index;
} MTRand;

MMFLOAT PI;
void cmd_FFT(const char *pp);
const MMFLOAT chitable[51][15]={
		{0.995,0.99,0.975,0.95,0.9,0.5,0.2,0.1,0.05,0.025,0.02,0.01,0.005,0.002,0.001},
		{0.0000397,0.000157,0.000982,0.00393,0.0158,0.455,1.642,2.706,3.841,5.024,5.412,6.635,7.879,9.550,10.828},
		{0.0100,0.020,0.051,0.103,0.211,1.386,3.219,4.605,5.991,7.378,7.824,9.210,10.597,12.429,13.816},
		{0.072,0.115,0.216,0.352,0.584,2.366,4.642,6.251,7.815,9.348,9.837,11.345,12.838,14.796,16.266},
		{0.207,0.297,0.484,0.711,1.064,3.357,5.989,7.779,9.488,11.143,11.668,13.277,14.860,16.924,18.467},
		{0.412,0.554,0.831,1.145,1.610,4.351,7.289,9.236,11.070,12.833,13.388,15.086,16.750,18.907,20.515},
		{0.676,0.872,1.237,1.635,2.204,5.348,8.558,10.645,12.592,14.449,15.033,16.812,18.548,20.791,22.458},
		{0.989,1.239,1.690,2.167,2.833,6.346,9.803,12.017,14.067,16.013,16.622,18.475,20.278,22.601,24.322},
		{1.344,1.646,2.180,2.733,3.490,7.344,11.030,13.362,15.507,17.535,18.168,20.090,21.955,24.352,26.124},
		{1.735,2.088,2.700,3.325,4.168,8.343,12.242,14.684,16.919,19.023,19.679,21.666,23.589,26.056,27.877},
		{2.156,2.558,3.247,3.940,4.865,9.342,13.442,15.987,18.307,20.483,21.161,23.209,25.188,27.722,29.588},
		{2.603,3.053,3.816,4.575,5.578,10.341,14.631,17.275,19.675,21.920,22.618,24.725,26.757,29.354,31.264},
		{3.074,3.571,4.404,5.226,6.304,11.340,15.812,18.549,21.026,23.337,24.054,26.217,28.300,30.957,32.909},
		{3.565,4.107,5.009,5.892,7.042,12.340,16.985,19.812,22.362,24.736,25.472,27.688,29.819,32.535,34.528},
		{4.075,4.660,5.629,6.571,7.790,13.339,18.151,21.064,23.685,26.119,26.873,29.141,31.319,34.091,36.123},
		{4.601,5.229,6.262,7.261,8.547,14.339,19.311,22.307,24.996,27.488,28.259,30.578,32.801,35.628,37.697},
		{5.142,5.812,6.908,7.962,9.312,15.338,20.465,23.542,26.296,28.845,29.633,32.000,34.267,37.146,39.252},
		{5.697,6.408,7.564,8.672,10.085,16.338,21.615,24.769,27.587,30.191,30.995,33.409,35.718,38.648,40.790},
		{6.265,7.015,8.231,9.390,10.865,17.338,22.760,25.989,28.869,31.526,32.346,34.805,37.156,40.136,42.312},
		{6.844,7.633,8.907,10.117,11.651,18.338,23.900,27.204,30.144,32.852,33.687,36.191,38.582,41.610,43.820},
		{7.434,8.260,9.591,10.851,12.443,19.337,25.038,28.412,31.410,34.170,35.020,37.566,39.997,43.072,45.315},
		{8.034,8.897,10.283,11.591,13.240,20.337,26.171,29.615,32.671,35.479,36.343,38.932,41.401,44.522,46.797},
		{8.643,9.542,10.982,12.338,14.041,21.337,27.301,30.813,33.924,36.781,37.659,40.289,42.796,45.962,48.268},
		{9.260,10.196,11.689,13.091,14.848,22.337,28.429,32.007,35.172,38.076,38.968,41.638,44.181,47.391,49.728},
		{9.886,10.856,12.401,13.848,15.659,23.337,29.553,33.196,36.415,39.364,40.270,42.980,45.559,48.812,51.179},
		{10.520,11.524,13.120,14.611,16.473,24.337,30.675,34.382,37.652,40.646,41.566,44.314,46.928,50.223,52.620},
		{11.160,12.198,13.844,15.379,17.292,25.336,31.795,35.563,38.885,41.923,42.856,45.642,48.290,51.627,54.052},
		{11.808,12.879,14.573,16.151,18.114,26.336,32.912,36.741,40.113,43.195,44.140,46.963,49.645,53.023,55.476},
		{12.461,13.565,15.308,16.928,18.939,27.336,34.027,37.916,41.337,44.461,45.419,48.278,50.993,54.411,56.892},
		{13.121,14.256,16.047,17.708,19.768,28.336,35.139,39.087,42.557,45.722,46.693,49.588,52.336,55.792,58.301},
		{13.787,14.953,16.791,18.493,20.599,29.336,36.250,40.256,43.773,46.979,47.962,50.892,53.672,57.167,59.703},
		{14.458,15.655,17.539,19.281,21.434,30.336,37.359,41.422,44.985,48.232,49.226,52.191,55.003,58.536,61.098},
		{15.134,16.362,18.291,20.072,22.271,31.336,38.466,42.585,46.194,49.480,50.487,53.486,56.328,59.899,62.487},
		{15.815,17.074,19.047,20.867,23.110,32.336,39.572,43.745,47.400,50.725,51.743,54.776,57.648,61.256,63.870},
		{16.501,17.789,19.806,21.664,23.952,33.336,40.676,44.903,48.602,51.966,52.995,56.061,58.964,62.608,65.247},
		{17.192,18.509,20.569,22.465,24.797,34.336,41.778,46.059,49.802,53.203,54.244,57.342,60.275,63.955,66.619},
		{17.887,19.233,21.336,23.269,25.643,35.336,42.879,47.212,50.998,54.437,55.489,58.619,61.581,65.296,67.985},
		{18.586,19.960,22.106,24.075,26.492,36.336,43.978,48.363,52.192,55.668,56.730,59.892,62.883,66.633,69.346},
		{19.289,20.691,22.878,24.884,27.343,37.335,45.076,49.513,53.384,56.896,57.969,61.162,64.181,67.966,70.703},
		{19.996,21.426,23.654,25.695,28.196,38.335,46.173,50.660,54.572,58.120,59.204,62.428,65.476,69.294,72.055},
		{20.707,22.164,24.433,26.509,29.051,39.335,47.269,51.805,55.758,59.342,60.436,63.691,66.766,70.618,73.402},
		{21.421,22.906,25.215,27.326,29.907,40.335,48.363,52.949,56.942,60.561,61.665,64.950,68.053,71.938,74.745},
		{22.138,23.650,25.999,28.144,30.765,41.335,49.456,54.090,58.124,61.777,62.892,66.206,69.336,73.254,76.084},
		{22.859,24.398,26.785,28.965,31.625,42.335,50.548,55.230,59.304,62.990,64.116,67.459,70.616,74.566,77.419},
		{23.584,25.148,27.575,29.787,32.487,43.335,51.639,56.369,60.481,64.201,65.337,68.710,71.893,75.874,78.750},
		{24.311,25.901,28.366,30.612,33.350,44.335,52.729,57.505,61.656,65.410,66.555,69.957,73.166,77.179,80.077},
		{25.041,26.657,29.160,31.439,34.215,45.335,53.818,58.641,62.830,66.617,67.771,71.201,74.437,78.481,81.400},
		{25.775,27.416,29.956,32.268,35.081,46.335,54.906,59.774,64.001,67.821,68.985,72.443,75.704,79.780,82.720},
		{26.511,28.177,30.755,33.098,35.949,47.335,55.993,60.907,65.171,69.023,70.197,73.683,76.969,81.075,84.037},
		{27.249,28.941,31.555,33.930,36.818,48.335,57.079,62.038,66.339,70.222,71.406,74.919,78.231,82.367,85.351},
		{27.991,29.707,32.357,34.764,37.689,49.335,58.164,63.167,67.505,71.420,72.613,76.154,79.490,83.657,86.661}
};
MMFLOAT q[4]={1,0,0,0};
MMFLOAT eInt[3]={0,0,0};
/* An implementation of the MT19937 Algorithm for the Mersenne Twister
 * by Evan Sultanik.  Based upon the pseudocode in: M. Matsumoto and
 * T. Nishimura, "Mersenne Twister: A 623-dimensionally
 * equidistributed uniform pseudorandom number generator," ACM
 * Transactions on Modeling and Computer Simulation Vol. 8, No. 1,
 * January pp.3-30 1998.
 *
 * http://www.sultanik.com/Mersenne_twister
 */
struct tagMTRand *g_myrand=NULL;
#define UPPER_MASK		0x80000000
#define LOWER_MASK		0x7fffffff
#define TEMPERING_MASK_B	0x9d2c5680
#define TEMPERING_MASK_C	0xefc60000

void PFlt(MMFLOAT flt){
    char s[20];
    FloatToStr(s, flt, 4,4, ' ');
    display_puts(s);
}
void PFltComma(MMFLOAT n) {
    display_puts(", "); PFlt(n);
}

void PRet(void){
    display_puts("\r\n");
}

void PInt(int64_t n) {
    char s[20];
    IntToStr(s, (int64_t)n, 10);
    display_puts(s);
}

void PIntComma(int64_t n) {
    display_puts(", "); PInt(n);
}

void PIntH(int64_t n) {
    char s[20];
    IntToStr(s, n, 16);
    display_puts(s);
}

void PIntHComma(int64_t n) {
    display_puts(", "); PIntH(n);
}

void cmd_SensorFusion(char *passcmdline);

void MadgwickQuaternionUpdate(MMFLOAT ax, MMFLOAT ay, MMFLOAT az, MMFLOAT gx, MMFLOAT gy, MMFLOAT gz, MMFLOAT mx, MMFLOAT my, MMFLOAT mz, MMFLOAT beta, MMFLOAT deltat, MMFLOAT *pitch, MMFLOAT *yaw, MMFLOAT *roll)
        {
            MMFLOAT q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
            MMFLOAT norm;
            MMFLOAT hx, hy, _2bx, _2bz;
            MMFLOAT s1, s2, s3, s4;
            MMFLOAT qDot1, qDot2, qDot3, qDot4;

            // Auxiliary variables to avoid repeated arithmetic
            MMFLOAT _2q1mx;
            MMFLOAT _2q1my;
            MMFLOAT _2q1mz;
            MMFLOAT _2q2mx;
            MMFLOAT _4bx;
            MMFLOAT _4bz;
            MMFLOAT _2q1 = 2.0 * q1;
            MMFLOAT _2q2 = 2.0 * q2;
            MMFLOAT _2q3 = 2.0 * q3;
            MMFLOAT _2q4 = 2.0 * q4;
            MMFLOAT _2q1q3 = 2.0 * q1 * q3;
            MMFLOAT _2q3q4 = 2.0 * q3 * q4;
            MMFLOAT q1q1 = q1 * q1;
            MMFLOAT q1q2 = q1 * q2;
            MMFLOAT q1q3 = q1 * q3;
            MMFLOAT q1q4 = q1 * q4;
            MMFLOAT q2q2 = q2 * q2;
            MMFLOAT q2q3 = q2 * q3;
            MMFLOAT q2q4 = q2 * q4;
            MMFLOAT q3q3 = q3 * q3;
            MMFLOAT q3q4 = q3 * q4;
            MMFLOAT q4q4 = q4 * q4;

            // Normalise accelerometer measurement
            norm = sqrt(ax * ax + ay * ay + az * az);
            if (norm == 0.0) return; // handle NaN
            norm = 1.0/norm;
            ax *= norm;
            ay *= norm;
            az *= norm;

            // Normalise magnetometer measurement
            norm = sqrt(mx * mx + my * my + mz * mz);
            if (norm == 0.0) return; // handle NaN
            norm = 1.0/norm;
            mx *= norm;
            my *= norm;
            mz *= norm;

            // Reference direction of Earth's magnetic field
            _2q1mx = 2.0 * q1 * mx;
            _2q1my = 2.0 * q1 * my;
            _2q1mz = 2.0 * q1 * mz;
            _2q2mx = 2.0 * q2 * mx;
            hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
            hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
            _2bx = sqrt(hx * hx + hy * hy);
            _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
            _4bx = 2.0 * _2bx;
            _4bz = 2.0 * _2bz;

            // Gradient decent algorithm corrective step
            s1 = -_2q3 * (2.0 * q2q4 - _2q1q3 - ax) + _2q2 * (2.0 * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
            s2 = _2q4 * (2.0 * q2q4 - _2q1q3 - ax) + _2q1 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q2 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + _2bz * q4 * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
            s3 = -_2q1 * (2.0 * q2q4 - _2q1q3 - ax) + _2q4 * (2.0 * q1q2 + _2q3q4 - ay) - 4.0 * q3 * (1.0 - 2.0 * q2q2 - 2.0 * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
            s4 = _2q2 * (2.0 * q2q4 - _2q1q3 - ax) + _2q3 * (2.0 * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5 - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5 - q2q2 - q3q3) - mz);
            norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
            norm = 1.0/norm;
            s1 *= norm;
            s2 *= norm;
            s3 *= norm;
            s4 *= norm;

            // Compute rate of change of quaternion
            qDot1 = 0.5 * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
            qDot2 = 0.5 * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
            qDot3 = 0.5 * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
            qDot4 = 0.5 * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

            // Integrate to yield quaternion
            q1 += qDot1 * deltat;
            q2 += qDot2 * deltat;
            q3 += qDot3 * deltat;
            q4 += qDot4 * deltat;
            norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
            norm = 1.0/norm;
            q[0] = q1 * norm;
            q[1] = q2 * norm;
            q[2] = q3 * norm;
            q[3] = q4 * norm;

            MMFLOAT ysqr = q3 * q3;


            // roll (x-axis rotation)
            MMFLOAT t0 = +2.0 * (q1 * q2 + q3 * q4);
            MMFLOAT t1 = +1.0 - 2.0 * (q2 * q2 + ysqr);
            *roll = atan2(t0, t1);

            // pitch (y-axis rotation)
            MMFLOAT t2 = +2.0 * (q1 * q3 - q4 * q2);
            t2 = t2 > 1.0 ? 1.0 : t2;
            t2 = t2 < -1.0 ? -1.0 : t2;
            *pitch = asin(t2);

            // yaw (z-axis rotation)
            MMFLOAT t3 = +2.0 * (q1 * q4 + q2 *q3);
            MMFLOAT t4 = +1.0 - 2.0 * (ysqr + q4 * q4);
            *yaw = atan2(t3, t4);

}
void MahonyQuaternionUpdate(MMFLOAT ax, MMFLOAT ay, MMFLOAT az, MMFLOAT gx, MMFLOAT gy, MMFLOAT gz, MMFLOAT mx, MMFLOAT my, MMFLOAT mz, MMFLOAT Ki, MMFLOAT Kp, MMFLOAT deltat, MMFLOAT *yaw, MMFLOAT *pitch, MMFLOAT *roll)        {
            MMFLOAT q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
            MMFLOAT norm;
            MMFLOAT hx, hy, bx, bz;
            MMFLOAT vx, vy, vz, wx, wy, wz;
            MMFLOAT ex, ey, ez;
            MMFLOAT pa, pb, pc;

            // Auxiliary variables to avoid repeated arithmetic
            MMFLOAT q1q1 = q1 * q1;
            MMFLOAT q1q2 = q1 * q2;
            MMFLOAT q1q3 = q1 * q3;
            MMFLOAT q1q4 = q1 * q4;
            MMFLOAT q2q2 = q2 * q2;
            MMFLOAT q2q3 = q2 * q3;
            MMFLOAT q2q4 = q2 * q4;
            MMFLOAT q3q3 = q3 * q3;
            MMFLOAT q3q4 = q3 * q4;
            MMFLOAT q4q4 = q4 * q4;

            // Normalise accelerometer measurement
            norm = sqrt(ax * ax + ay * ay + az * az);
            if (norm == 0.0) return; // handle NaN
            norm = 1.0 / norm;        // use reciprocal for division
            ax *= norm;
            ay *= norm;
            az *= norm;

            // Normalise magnetometer measurement
            norm = sqrt(mx * mx + my * my + mz * mz);
            if (norm == 0.0) return; // handle NaN
            norm = 1.0 / norm;        // use reciprocal for division
            mx *= norm;
            my *= norm;
            mz *= norm;

            // Reference direction of Earth's magnetic field
            hx = 2.0 * mx * (0.5 - q3q3 - q4q4) + 2.0 * my * (q2q3 - q1q4) + 2.0 * mz * (q2q4 + q1q3);
            hy = 2.0 * mx * (q2q3 + q1q4) + 2.0 * my * (0.5 - q2q2 - q4q4) + 2.0 * mz * (q3q4 - q1q2);
            bx = sqrt((hx * hx) + (hy * hy));
            bz = 2.0 * mx * (q2q4 - q1q3) + 2.0 * my * (q3q4 + q1q2) + 2.0 * mz * (0.5 - q2q2 - q3q3);

            // Estimated direction of gravity and magnetic field
            vx = 2.0 * (q2q4 - q1q3);
            vy = 2.0 * (q1q2 + q3q4);
            vz = q1q1 - q2q2 - q3q3 + q4q4;
            wx = 2.0 * bx * (0.5 - q3q3 - q4q4) + 2.0 * bz * (q2q4 - q1q3);
            wy = 2.0 * bx * (q2q3 - q1q4) + 2.0 * bz * (q1q2 + q3q4);
            wz = 2.0 * bx * (q1q3 + q2q4) + 2.0 * bz * (0.5 - q2q2 - q3q3);

            // Error is cross product between estimated direction and measured direction of gravity
            ex = (ay * vz - az * vy) + (my * wz - mz * wy);
            ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
            ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
            if (Ki > 0.0)
            {
                eInt[0] += ex;      // accumulate integral error
                eInt[1] += ey;
                eInt[2] += ez;
            }
            else
            {
                eInt[0] = 0.0;     // prevent integral wind up
                eInt[1] = 0.0;
                eInt[2] = 0.0;
            }

            // Apply feedback terms
            gx = gx + Kp * ex + Ki * eInt[0];
            gy = gy + Kp * ey + Ki * eInt[1];
            gz = gz + Kp * ez + Ki * eInt[2];

            // Integrate rate of change of quaternion
            pa = q2;
            pb = q3;
            pc = q4;
            q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5 * deltat);
            q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5 * deltat);
            q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5 * deltat);
            q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5 * deltat);

            // Normalise quaternion
            norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
            norm = 1.0 / norm;
            q[0] = q1 * norm;
            q[1] = q2 * norm;
            q[2] = q3 * norm;
            q[3] = q4 * norm;
            MMFLOAT ysqr = q3 * q3;


            // roll (x-axis rotation)
            MMFLOAT t0 = +2.0 * (q1 * q2 + q3 * q4);
            MMFLOAT t1 = +1.0 - 2.0 * (q2 * q2 + ysqr);
            *roll = atan2(t0, t1);

            // pitch (y-axis rotation)
            MMFLOAT t2 = +2.0 * (q1 * q3 - q4 * q2);
            t2 = t2 > 1.0 ? 1.0 : t2;
            t2 = t2 < -1.0 ? -1.0 : t2;
            *pitch = asin(t2);

            // yaw (z-axis rotation)
            MMFLOAT t3 = +2.0 * (q1 * q4 + q2 *q3);
            MMFLOAT t4 = +1.0 - 2.0 * (ysqr + q4 * q4);
            *yaw = atan2(t3, t4);
        }

inline static void m_seedRand(MTRand* rand, unsigned long seed) {
  /* set initial seeds to mt[STATE_VECTOR_LENGTH] using the generator
   * from Line 25 of Table 1 in: Donald Knuth, "The Art of Computer
   * Programming," Vol. 2 (2nd Ed.) pp.102.
   */
  rand->mt[0] = seed & 0xffffffff;
  for(rand->index=1; rand->index<STATE_VECTOR_LENGTH; rand->index++) {
    rand->mt[rand->index] = (6069 * rand->mt[rand->index-1]) & 0xffffffff;
  }
}

/**
* Creates a new random number generator from a given seed.
*/
void seedRand(unsigned long seed) {
  m_seedRand(g_myrand, seed);
//  return rand;
}

/**
 * Generates a pseudo-randomly generated long.
 */
unsigned long genRandLong(MTRand* rand) {

  unsigned long y;
  static unsigned long mag[2] = {0x0, 0x9908b0df}; /* mag[x] = x * 0x9908b0df for x = 0,1 */
  if(rand->index >= STATE_VECTOR_LENGTH || rand->index < 0) {
    /* generate STATE_VECTOR_LENGTH words at a time */
    int kk;
    if(rand->index >= STATE_VECTOR_LENGTH+1 || rand->index < 0) {
      m_seedRand(rand, 4357);
    }
    for(kk=0; kk<STATE_VECTOR_LENGTH-STATE_VECTOR_M; kk++) {
      y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk+1] & LOWER_MASK);
      rand->mt[kk] = rand->mt[kk+STATE_VECTOR_M] ^ (y >> 1) ^ mag[y & 0x1];
    }
    for(; kk<STATE_VECTOR_LENGTH-1; kk++) {
      y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk+1] & LOWER_MASK);
      rand->mt[kk] = rand->mt[kk+(STATE_VECTOR_M-STATE_VECTOR_LENGTH)] ^ (y >> 1) ^ mag[y & 0x1];
    }
    y = (rand->mt[STATE_VECTOR_LENGTH-1] & UPPER_MASK) | (rand->mt[0] & LOWER_MASK);
    rand->mt[STATE_VECTOR_LENGTH-1] = rand->mt[STATE_VECTOR_M-1] ^ (y >> 1) ^ mag[y & 0x1];
    rand->index = 0;
  }
  y = rand->mt[rand->index++];
  y ^= (y >> 11);
  y ^= (y << 7) & TEMPERING_MASK_B;
  y ^= (y << 15) & TEMPERING_MASK_C;
  y ^= (y >> 18);
  return y;
}

/**
 * Generates a pseudo-randomly generated double in the range [0..1].
 */
MMFLOAT genRand(MTRand* rand) {
  return((MMFLOAT)genRandLong(rand) / (unsigned long)0xffffffff);
}

MMFLOAT determinant(MMFLOAT **matrix,int size);
void transpose(MMFLOAT **matrix,MMFLOAT **matrix_cofactor,MMFLOAT **newmatrix, int size);
void cofactor(MMFLOAT **matrix,MMFLOAT **newmatrix,int size);
static void floatshellsort(MMFLOAT a[],  int n) {
    long h, l, j;
    MMFLOAT k;
    for (h = n; h /= 2;) {
        for (l = h; l < n; l++) {
            k = a[l];
            for (j = l; j >= h &&  k < a[j - h]; j -= h) {
                a[j] = a[j - h];
            }
            a[j] = k;
        }
    }
}

static MMFLOAT* alloc1df (int n)
{
//    int i;
    MMFLOAT* array;
    if ((array = (MMFLOAT*) GetMemory(n * sizeof(MMFLOAT))) == NULL) {
        error_throw_legacy("Unable to allocate memory for 1D float array...\n");
        exit(0);
    }

//    for (i = 0; i < n; i++) {
//        array[i] = 0.0;
//    }

    return array;
}

static MMFLOAT** alloc2df (int m, int n)
{
    int i;
    MMFLOAT** array;
    if ((array = (MMFLOAT **) GetMemory(m * sizeof(MMFLOAT*))) == NULL) {
        error_throw_legacy("Unable to allocate memory for 2D float array...\n");
        exit(0);
    }

    for (i = 0; i < m; i++) {
        array[i] = alloc1df(n);
    }

    return array;
}

static void dealloc2df (MMFLOAT** array, int m, int n)
{
    int i;
    for (i = 0; i < m; i++) {
        // FreeMemorySafe((void **)&array[i]);
		FreeMemory(array[i]);
    }

    // FreeMemorySafe((void **)&array);
	FreeMemory(array);

}

void Q_Mult(MMFLOAT *q1, MMFLOAT *q2, MMFLOAT *n){
    MMFLOAT a1=q1[0],a2=q2[0],b1=q1[1],b2=q2[1],c1=q1[2],c2=q2[2],d1=q1[3],d2=q2[3];
    n[0]=a1*a2-b1*b2-c1*c2-d1*d2;
    n[1]=a1*b2+b1*a2+c1*d2-d1*c2;
    n[2]=a1*c2-b1*d2+c1*a2+d1*b2;
    n[3]=a1*d2+b1*c2-c1*b2+d1*a2;
    n[4]=q1[4]*q2[4];
}

void Q_Invert(MMFLOAT *q, MMFLOAT *n){
    n[0]=q[0];
    n[1]=-q[1];
    n[2]=-q[2];
    n[3]=-q[3];
    n[4]=q[4];
}
static uint8_t reverse8(uint8_t in)
{
  uint8_t x = in;
  x = (((x & 0xAA) >> 1) | ((x & 0x55) << 1));
  x = (((x & 0xCC) >> 2) | ((x & 0x33) << 2));
  x =          ((x >> 4) | (x << 4));
  return x;
}


static uint16_t reverse16(uint16_t in)
{
  uint16_t x = in;
  x = (((x & 0XAAAA) >> 1) | ((x & 0X5555) << 1));
  x = (((x & 0xCCCC) >> 2) | ((x & 0X3333) << 2));
  x = (((x & 0xF0F0) >> 4) | ((x & 0X0F0F) << 4));
  x = (( x >> 8) | (x << 8));
  return x;
}


static uint16_t reverse12(uint16_t in)
{
  return reverse16(in) >> 4;
}


static uint32_t reverse32(uint32_t in)
{
  uint32_t x = in;
  x = (((x & 0xAAAAAAAA) >> 1)  | ((x & 0x55555555) << 1));
  x = (((x & 0xCCCCCCCC) >> 2)  | ((x & 0x33333333) << 2));
  x = (((x & 0xF0F0F0F0) >> 4)  | ((x & 0x0F0F0F0F) << 4));
  x = (((x & 0xFF00FF00) >> 8)  | ((x & 0x00FF00FF) << 8));
  x = (x >> 16) | (x << 16);
  return x;
}


static uint64_t reverse64(uint64_t in)
{
  uint64_t x = in;
  x = (((x & 0xAAAAAAAAAAAAAAAA) >> 1)  | ((x & 0x5555555555555555) << 1));
  x = (((x & 0xCCCCCCCCCCCCCCCC) >> 2)  | ((x & 0x3333333333333333) << 2));
  x = (((x & 0xF0F0F0F0F0F0F0F0) >> 4)  | ((x & 0x0F0F0F0F0F0F0F0F) << 4));
  x = (((x & 0xFF00FF00FF00FF00) >> 8)  | ((x & 0x00FF00FF00FF00FF) << 8));
  x = (((x & 0xFFFF0000FFFF0000) >> 16) | ((x & 0x0000FFFF0000FFFF) << 16));
  x = (x >> 32) | (x << 32);
  return x;
}


///////////////////////////////////////////////////////////////////////////////////

// CRC POLYNOME = x8 + x5 + x4 + 1 = 1001 1000 = 0x8C
uint8_t crc8(uint8_t *array, uint16_t length, const uint8_t polynome,
             const uint8_t startmask, const uint8_t endmask,
             const uint8_t reverseIn, const uint8_t reverseOut)
{
  uint8_t crc = startmask;
  while (length--)
  {
    // if ((length & 0xFF) == 0) routinechecks();  // RTOS
    uint8_t data = *array++;
    if (reverseIn) data = reverse8(data);
    crc ^= data;
    for (uint8_t i = 8; i; i--)
    {
      if (crc & 0x80)
      {
        crc <<= 1;
        crc ^= polynome;
      }
      else
      {
        crc <<= 1;
      }
    }
  }
  crc ^= endmask;
  if (reverseOut) crc = reverse8(crc);
  return crc;
}


// CRC POLYNOME = x12 + x3 + x2 + 1 =  0000 1000 0000 1101 = 0x80D
uint16_t crc12(const uint8_t *array, uint16_t length, const uint16_t polynome,
               const uint16_t startmask, const uint16_t endmask,
               const uint8_t reverseIn, const uint8_t reverseOut)
{
  uint16_t crc = startmask;
  while (length--)
  {
    // if ((length & 0xFF) == 0) routinechecks();  // RTOS
    uint8_t data = *array++;
    if (reverseIn) data = reverse8(data);

    crc ^= ((uint16_t)data) << 4;
    for (uint8_t i = 8; i; i--)
    {
      if (crc & (1 << 11) )
      {
        crc <<= 1;
        crc ^= polynome;
      }
      else
      {
        crc <<= 1;
      }
    }
  }

  if (reverseOut) crc = reverse12(crc);
  crc ^= endmask;
  return crc;
}


// CRC POLYNOME = x15 + 1 =  1000 0000 0000 0001 = 0x8001
uint16_t crc16(const uint8_t *array, uint16_t length, const uint16_t polynome,
               const uint16_t startmask, const uint16_t endmask,
               const uint8_t reverseIn, const uint8_t reverseOut)
{
  uint16_t crc = startmask;
  while (length--)
  {
    // if ((length & 0xFF) == 0) routinechecks();  // RTOS
    uint8_t data = *array++;
    if (reverseIn) data = reverse8(data);
    crc ^= ((uint16_t)data) << 8;
    for (uint8_t i = 8; i; i--)
    {
      if (crc & (1 << 15))
      {
        crc <<= 1;
        crc ^= polynome;
      }
      else
      {
        crc <<= 1;
      }
    }
  }
  if (reverseOut) crc = reverse16(crc);
  crc ^= endmask;
  return crc;
}


// CRC-CCITT POLYNOME = x13 + X5 + 1 =  0001 0000 0010 0001 = 0x1021
uint16_t crc16_CCITT(uint8_t *array, uint16_t length)
{
  return crc16(array, length, 0x1021, 0xFFFF,0,0,0);
}

// CRC-32 POLYNOME =  x32 + ..... + 1
uint32_t crc32(const uint8_t *array, uint16_t length, const uint32_t polynome,
               const uint32_t startmask, const uint32_t endmask,
               const uint8_t reverseIn, const uint8_t reverseOut)
{
  uint32_t crc = startmask;
  while (length--)
  {
    // if ((length & 0xFF) == 0) routinechecks();  // RTOS
    uint8_t data = *array++;
    if (reverseIn) data = reverse8(data);
    crc ^= ((uint32_t) data) << 24;
    for (uint8_t i = 8; i; i--)
    {
      if (crc & (1UL << 31))
      {
        crc <<= 1;
        crc ^= polynome;
      }
      else
      {
        crc <<= 1;
      }
    }
  }
  crc ^= endmask;
  if (reverseOut) crc = reverse32(crc);
  return crc;
}


// CRC-CCITT POLYNOME =  x64 + ..... + 1
// CRC_ECMA64 = 0x42F0E1EBA9EA3693
uint64_t crc64(const uint8_t *array, uint16_t length, const uint64_t polynome,
               const uint64_t startmask, const uint64_t endmask,
               const uint8_t reverseIn, const uint8_t reverseOut)
{
  uint64_t crc = startmask;
  while (length--)
  {
    // if ((length & 0xFF) == 0) routinechecks();  // RTOS
    uint8_t data = *array++;
    if (reverseIn) data = reverse8(data);
    crc ^= ((uint64_t) data) << 56;
    for (uint8_t i = 8; i; i--)
    {
      if (crc & (1ULL << 63))
      {
        crc <<= 1;
        crc ^= polynome;
      }
      else
      {
        crc <<= 1;
      }
    }
  }
  crc ^= endmask;
  if (reverseOut) crc = reverse64(crc);
  return crc;
}
int parseintegerarray(const char *tp, int64_t **a1int, int argno, int dimensions, DIMTYPE *dims, bool ConstantNotAllowed){
	int i,j;
	void *ptr1 = findvar(tp, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
	if((vartbl[VarIndex].type & T_CONST) && ConstantNotAllowed) error_throw_legacy("Cannot change a constant");
	if(dims==NULL)dims=vartbl[VarIndex].dims;
	if(vartbl[VarIndex].type & T_INT) {
		memcpy(dims,vartbl[VarIndex].dims, MAXDIM * sizeof(DIMTYPE));
		*a1int = (int64_t *)ptr1;
		if ((char *)ptr1 != vartbl[VarIndex].val.s) ERROR_SYNTAX;
	} else error_throw_legacy("Argument % must be an integer array",argno);
	int card=1;
	if(dimensions==1 && (dims[0]<=0 || dims[1]>0))error_throw_legacy("Argument % must be a 1D integer point array",argno);
	if(dimensions==2 && (dims[0]<=0 || dims[1]<=0 || dims[2]>0))error_throw_legacy("Argument % must be a 2D integer point array",argno);
	for(i=0;i<MAXDIM;i++){
		j=(dims[i] - mmb_options.base + 1);
		if(j)card *= j;
	}
	return card;
}
int parsenumberarray(const char *tp, MMFLOAT **a1float, int64_t **a1int, int argno, short dimensions, DIMTYPE *dims, bool ConstantNotAllowed){
	int i,j;
	void *ptr1 = findvar(tp, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
	if((vartbl[VarIndex].type & T_CONST) && ConstantNotAllowed) error_throw_legacy("Cannot change a constant");
	if(dims==NULL)dims=vartbl[VarIndex].dims;
	if(vartbl[VarIndex].type & (T_INT | T_NBR)) {
		memcpy(dims,vartbl[VarIndex].dims,  MAXDIM * sizeof(DIMTYPE));
		if(vartbl[VarIndex].type & T_NBR) *a1float = (MMFLOAT *)ptr1;
		else *a1int=(int64_t *)ptr1;
		if((char *)ptr1!=vartbl[VarIndex].val.s)ERROR_SYNTAX;
	} else error_throw_legacy("Argument % must be a numerical array",argno);
	int card=1;
	if(dimensions==1 && (dims[0]<=0 || dims[1]>0))error_throw_legacy("Argument % must be a 1D numerical array",argno);
	if(dimensions==2 && (dims[0]<=0 || dims[1]<=0 || dims[2]>0))error_throw_legacy("Argument % must be a 2D numerical array",argno);
	for(i=0;i<MAXDIM;i++){
		j=(dims[i] - mmb_options.base + 1);
		if(j)card *= j;
	}
	return card;
}
int parsefloatrarray(const char *tp, MMFLOAT **a1float, int argno, int dimensions, DIMTYPE *dims, bool ConstantNotAllowed){
	void *ptr1 = NULL;
	int i,j;
	ptr1 = findvar(tp, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
	if((vartbl[VarIndex].type & T_CONST) && ConstantNotAllowed) error_throw_legacy("Cannot change a constant");
	if(dims==NULL)dims=vartbl[VarIndex].dims;
	if(vartbl[VarIndex].type & T_NBR) {
		memcpy(dims,vartbl[VarIndex].dims,  MAXDIM * sizeof(DIMTYPE));
		*a1float = (MMFLOAT *)ptr1;
		if((char *) ptr1!= vartbl[VarIndex].val.s)ERROR_SYNTAX;
	} else error_throw_legacy("Argument % must be a floating point array",argno);
	int card=1;
	if(dimensions==1 && (dims[0]<=0 || dims[1]>0))error_throw_legacy("Argument % must be a 1D floating point array",argno);
	if(dimensions==2 && (dims[0]<=0 || dims[1]<=0 || dims[2]>0))error_throw_legacy("Argument % must be a 2D floating point array",argno);
	for(i=0;i<MAXDIM;i++){
		j=(dims[i] - mmb_options.base + 1);
		if(j)card *= j;
	}
	return card;
}
int parsearrays(const char *tp, MMFLOAT **a1float, MMFLOAT **a2float,MMFLOAT **a3float, int64_t **a1int, int64_t **a2int, int64_t **a3int){
	int card1,card2,card3;
	getargs(&tp, 5, DELIM_COMMA);
	if(!(argc == 5)) error_throw_legacy("Argument count");
	card1=parsenumberarray(argv[0],a1float,a1int,1,0, NULL, false);
	card2=parsenumberarray(argv[2],a2float,a2int,2,0, NULL, false);
	card3=parsenumberarray(argv[4],a3float,a3int,3,0, NULL, true);
	if(!(card1==card2 && card2==card3))error_throw_legacy("Array size mismatch");
	if(!((*a3float==NULL && *a2float==NULL && *a1float==NULL) || (*a3int==NULL && *a2int==NULL && *a1int==NULL)))error_throw_legacy("Arrays must be all integer or all floating point");
	return card1;
}
int parseany(const char *tp, MMFLOAT **a1float, int64_t **a1int, char ** a1str, int *length, bool stringarray){
	void *ptr1 = findvar(tp, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
	int arraylength;
	if(vartbl[VarIndex].type & T_NBR) {
		if(vartbl[VarIndex].dims[1] != 0) error_throw_legacy("Invalid variable");
		if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
			error_throw_legacy("Argument 1 must be a numerical array");
		}
		arraylength=vartbl[VarIndex].dims[0] - mmb_options.base + 1;
		if(*length==0)*length=arraylength;
		if(*length>arraylength)error_throw_legacy("Array size");
		*a1float = (MMFLOAT *)ptr1;
		if((char *) ptr1!= vartbl[VarIndex].val.s)ERROR_SYNTAX;
	} else if(ptr1 && vartbl[VarIndex].type & T_INT) {
		if(vartbl[VarIndex].dims[1] != 0) error_throw_legacy("Invalid variable");
		if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
			error_throw_legacy("Argument 1 must be a numerical array");
		}
		arraylength=vartbl[VarIndex].dims[0] - mmb_options.base + 1;
		if(*length==0)*length=arraylength;
		if(*length>arraylength)error_throw_legacy("Array size");
		*a1int = (int64_t *)ptr1;
		if((char *) ptr1!= vartbl[VarIndex].val.s)ERROR_SYNTAX;
	} else if(ptr1 && vartbl[VarIndex].type & T_STR && !stringarray) {
		*a1str=(char *)ptr1;
		if(*length==0)*length=**a1str;
		if(**a1str<*length)error_throw_legacy("String size");
	} else if(ptr1 && vartbl[VarIndex].type & T_STR && stringarray) {
		if(vartbl[VarIndex].dims[1] != 0) error_throw_legacy("Invalid variable");
		if(vartbl[VarIndex].dims[0] <= 0) {		// Not an array
			error_throw_legacy("Argument 1 must be a string array");
		}
		arraylength=vartbl[VarIndex].dims[0] - mmb_options.base + 1;
		if(*length==0)*length=arraylength;
		if(*length>arraylength)error_throw_legacy("Array size");
		*a1str=(char *)ptr1;
		if((char *) ptr1!= vartbl[VarIndex].val.s)ERROR_SYNTAX;
		*length=vartbl[VarIndex].size;
		return arraylength;
	} else ERROR_SYNTAX;
	return *length;
}
MMFLOAT farr2d(MMFLOAT *arr,int d1, int a, int b){
	arr+=d1*b+a;
	return *arr;
}
int64_t iarr2d(int64_t *arr,int d1, int a, int b){
	arr+=d1*b+a;
	return *arr;
}

// ---- MATH SINC, as on the PicoMite V6.04.00RC2, and MATH(UPSAMPLE), as in MMBasic for Windows ----

/** Smooths (x_in, y_in) with a windowed sinc (Hamming) kernel into (x_out, y_out), n points each. */
static void sinc_filter(const MMFLOAT *x_in, const MMFLOAT *y_in, int n, MMFLOAT *x_out, MMFLOAT *y_out,
                        MMFLOAT cutoff_freq, int window_size) {
    if (window_size % 2 == 0) window_size++;
    const int center = window_size / 2, pad = window_size / 2, padded_len = n + 2 * pad;
    MMFLOAT *kernel = GetTempMemory(window_size * sizeof(MMFLOAT));
    MMFLOAT kernel_sum = 0.0;
    for (int i = 0; i < window_size; i++) {
        const int t = i - center;
        const MMFLOAT hamming = 0.54 - 0.46 * cos(2.0 * M_PI * i / (window_size - 1));
        kernel[i] = (t == 0) ? 2.0 * cutoff_freq * hamming
                             : (sin(2.0 * M_PI * cutoff_freq * t) / (M_PI * t)) * hamming;
        kernel_sum += kernel[i];
    }
    for (int i = 0; i < window_size; i++) kernel[i] /= kernel_sum;
    MMFLOAT *xp = GetTempMemory(padded_len * sizeof(MMFLOAT));
    MMFLOAT *yp = GetTempMemory(padded_len * sizeof(MMFLOAT));
    for (int i = 0; i < pad; i++) {
        xp[i] = x_in[0]; yp[i] = y_in[0];
        xp[padded_len - 1 - i] = x_in[n - 1]; yp[padded_len - 1 - i] = y_in[n - 1];
    }
    for (int i = 0; i < n; i++) { xp[i + pad] = x_in[i]; yp[i + pad] = y_in[i]; }
    for (int i = 0; i < n; i++) {
        MMFLOAT sx = 0.0, sy = 0.0;
        for (int j = 0; j < window_size; j++) { sx += xp[i + j] * kernel[j]; sy += yp[i + j] * kernel[j]; }
        x_out[i] = sx;
        y_out[i] = sy;
    }
}

/** Resamples (x_in, y_in), n points, onto m evenly spaced points with a windowed sinc. */
static void sinc_filter_interpolate(const MMFLOAT *x_in, const MMFLOAT *y_in, int n, int m,
                                    MMFLOAT *x_out, MMFLOAT *y_out, MMFLOAT cutoff_freq, int window_size) {
    if (n <= 1) return;
    if ((window_size & 1) == 0) window_size++;
    const int half = window_size / 2;
    const MMFLOAT x_start = x_in[0], span = x_in[n - 1] - x_in[0];
    const MMFLOAT step = (m > 1) ? span / (MMFLOAT) (m - 1) : 0.0;
    MMFLOAT sample_step = (x_in[n - 1] - x_in[0]) / (MMFLOAT) (n - 1);
    if (sample_step == 0.0) sample_step = 1.0;
    for (int i = 0; i < m; i++) {
        const MMFLOAT x_target = x_start + step * (MMFLOAT) i;
        x_out[i] = x_target;
        const double idx_f = (double) (x_target - x_start) / (double) sample_step;
        const int idx0 = (int) floor(idx_f);
        const double frac = idx_f - (double) idx0;
        double acc = 0.0, norm = 0.0;
        for (int j = -half; j <= half; j++) {
            const int src = idx0 + j;
            if (src < 0 || src >= n) continue;
            const double t = (double) j - frac;
            const double window = 0.54 - 0.46 * cos(2.0 * M_PI * (double) (j + half) / (double) (window_size - 1));
            double w = (t == 0.0) ? (double) (2.0 * cutoff_freq) : sin(2.0 * M_PI * cutoff_freq * t) / (M_PI * t);
            w *= window;
            acc += (double) y_in[src] * w;
            norm += w;
        }
        y_out[i] = (norm != 0.0) ? (MMFLOAT) (acc / norm) : 0.0;
    }
}

static double sincpi(double x) {
    return (x == 0.0) ? 1.0 : sin(M_PI * x) / (M_PI * x);
}

/** Converts xsamps samples at 'down' Hz into samples at 'up' Hz; returns how many. */
static int upsample(const MMFLOAT *x, MMFLOAT *y, int xsamps, MMFLOAT up, MMFLOAT down) {
    const double down_ratio = down / up;
    const int ysamps = 1 + (int) ((xsamps - 6) / down_ratio);
    for (int j = 0; j < ysamps; j++) {
        const double b = j * down_ratio;
        const int i = (int) b;
        const double a = b - i;
        y[j] = x[i] * sincpi(2 + a) + x[i + 1] * sincpi(1 + a) + x[i + 2] * sincpi(a)
                + x[i + 3] * sincpi(1 - a) + x[i + 4] * sincpi(2 - a) + x[i + 5] * sincpi(3 - a);
    }
    return ysamps;
}


// ---- MATH PID, as on the PicoMite V6.04.00RC2 ----

/** The 14 elements of the float array MATH PID INIT is given, in order. */
typedef struct {
    MMFLOAT Kp, Ki, Kd;              // gains
    MMFLOAT tau;                     // derivative low-pass filter time constant
    MMFLOAT limMin, limMax;          // output limits
    MMFLOAT limMinInt, limMaxInt;    // integrator limits
    MMFLOAT T;                       // sample time, seconds
    MMFLOAT integrator, prevError, differentiator, prevMeasurement;  // controller memory
    MMFLOAT out;
} PidController;

static PidController *pid_channels[MAX_PID_CHANNELS + 1];

static MMFLOAT pid_update(PidController *pid, MMFLOAT setpoint, MMFLOAT measurement) {
    const MMFLOAT error = setpoint - measurement;
    const MMFLOAT proportional = pid->Kp * error;
    pid->integrator = pid->integrator + 0.5 * pid->Ki * pid->T * (error + pid->prevError);
    if (pid->integrator > pid->limMaxInt) pid->integrator = pid->limMaxInt;
    else if (pid->integrator < pid->limMinInt) pid->integrator = pid->limMinInt;
    // Derivative on the measurement, band-limited.
    pid->differentiator = -(2.0 * pid->Kd * (measurement - pid->prevMeasurement)
                            + (2.0 * pid->tau - pid->T) * pid->differentiator)
                          / (2.0 * pid->tau + pid->T);
    pid->out = proportional + pid->integrator + pid->differentiator;
    if (pid->out > pid->limMax) pid->out = pid->limMax;
    else if (pid->out < pid->limMin) pid->out = pid->limMin;
    pid->prevError = error;
    pid->prevMeasurement = measurement;
    return pid->out;
}

/** MATH PID INIT channel, params!(), callback | START channel | STOP channel */
static void cmd_math_pid(const char *tp) {
    const char *pi;
    if ((pi = checkstring(tp, "START"))) {
        const int channel = getint(pi, 1, MAX_PID_CHANNELS);
        if (!pid_channels[channel] || FAILED(interrupt_start_pid(channel))) {
            error_throw_legacy("Channel not initialised");
        }
    } else if ((pi = checkstring(tp, "STOP"))) {
        const int channel = getint(pi, 1, MAX_PID_CHANNELS);
        if (!pid_channels[channel]) error_throw_legacy("Channel not initialised");
        interrupt_set_pid(channel, NULL, 0);
        pid_channels[channel] = NULL;
    } else if ((pi = checkstring(tp, "INIT"))) {
        getargs(&pi, 5, DELIM_COMMA);
        if (argc != 5) ERROR_SYNTAX;
        MMFLOAT *q1 = NULL;
        const int channel = getint(argv[0], 1, MAX_PID_CHANNELS);
        const int card = parsefloatrarray(argv[2], &q1, 2, 1, NULL, true);
        if (card != 14) error_throw_legacy("Argument 2 must be a 14 element floating point array");
        PidController *pid = (PidController *) q1;
        if (pid->T < 0.001) error_throw_legacy("Invalid update rate");
        pid_channels[channel] = pid;
        interrupt_set_pid(channel, GetIntAddress(argv[4]), (int64_t) (pid->T * 1e9));
    } else {
        ERROR_SYNTAX;
    }
}

// ---- MATH AES128 ENCRYPT|DECRYPT CBC|ECB|CTR, as on the PicoMite V6.04.00RC2 ----

/** Sixteen random bytes for an initialisation vector, from the system's generator. */
void aes_random_iv(uint8_t *iv) {
	arc4random_buf(iv, 16);
}

/** A key or initialisation vector: a string, or an integer or float array of 16 bytes. */
void aes_get16(const char *arg, uint8_t *out, const char *size_error) {
	MMFLOAT *afloat=NULL;
	int64_t *aint=NULL;
	char *astr=NULL;
	int length = 0;
	if(parseany(arg, &afloat, &aint, &astr, &length, false) != 16) error_throw_legacy(size_error);
	for(int i = 0; i < 16; i++) {
		if(aint) {
			if(aint[i] < 0 || aint[i] > 255) error_throw_legacy("Key number out of bounds 0-255");
			out[i] = aint[i];
		} else if(afloat) {
			if(afloat[i] < 0 || afloat[i] > 255) error_throw_legacy("Key number out of bounds 0-255");
			out[i] = afloat[i];
		} else {
			out[i] = astr[i + 1];
		}
	}
}

static void math_aes128(const char *tp) {
	const char *q, *p;
	const bool encrypt = (q = checkstring(tp, "ENCRYPT")) != NULL;
	if(!encrypt && !(q = checkstring(tp, "DECRYPT"))) ERROR_SYNTAX;
	const bool ecb = (p = checkstring(q, "ECB")) != NULL;
	const bool cbc = !ecb && (p = checkstring(q, "CBC")) != NULL;
	if(!ecb && !cbc && !(p = checkstring(q, "CTR"))) ERROR_SYNTAX;
	// Encrypting in CBC or CTR puts the initialisation vector in front of the output,
	// decrypting takes it from the front of the input.
	const int ivadd = ecb ? 0 : encrypt ? 16 : -16;
	uint8_t keyx[16], iv[16];
	getargs(&p, 7, DELIM_COMMA);
	if(ivadd == 16 ? argc < 5 : argc != 5) ERROR_SYNTAX;
	aes_get16(argv[0], keyx, "Key must be 16 elements long");
	MMFLOAT *a2float=NULL, *outflt=NULL;
	int64_t *a2int=NULL, *outint=NULL;
	char *a2str=NULL, *outstr=NULL;
	int length = 0;
	const int card = parseany(argv[2], &a2float, &a2int, &a2str, &length, false);
	if(card % 16) error_throw_legacy("input must be multiple of 16 elements long");
	if(card < -ivadd) error_throw_legacy("input must be at least 16 elements long");
	uint8_t *inx = (uint8_t *)GetTempMemory(card + 16);
	length = 0;
	const int card3 = parseany(argv[4], &outflt, &outint, &outstr, &length, false);
	if(card3 != card + ivadd && outstr == NULL) error_throw_legacy("Array size mismatch");
	if(ivadd == 16) {
		if(argc == 7) aes_get16(argv[6], iv, "Initialisation vector must be 16 elements long");
		else aes_random_iv(iv);
	}
	for(int i = 0; i < card; i++) {
		if(a2int) {
			if(a2int[i] < 0 || a2int[i] > 255) error_throw_legacy("input number out of bounds 0-255");
			inx[i] = a2int[i];
		} else if(a2float) {
			if(a2float[i] < 0 || a2float[i] > 255) error_throw_legacy("input number out of bounds 0-255");
			inx[i] = a2float[i];
		} else {
			inx[i] = a2str[i + 1];
		}
	}

	struct AES_ctx ctx;
	uint8_t *result = inx;
	int size = card;
	if(ecb) {
		struct AES_ctx ctxcopy;
		AES_init_ctx(&ctxcopy, keyx);
		for(int i = 0; i < card; i += 16) {
			memcpy(&ctx, &ctxcopy, sizeof(ctx));
			if(encrypt) AES_ECB_encrypt(&ctx, &inx[i]);
			else AES_ECB_decrypt(&ctx, &inx[i]);
		}
	} else if(encrypt) {
		AES_init_ctx_iv(&ctx, keyx, iv);
		if(cbc) AES_CBC_encrypt_buffer(&ctx, inx, card);
		else AES_CTR_xcrypt_buffer(&ctx, inx, card);
	} else {
		AES_init_ctx_iv(&ctx, keyx, inx);
		result = &inx[16];
		size = card - 16;
		if(cbc) AES_CBC_decrypt_buffer(&ctx, result, size);
		else AES_CTR_xcrypt_buffer(&ctx, result, size);
	}

	// The result, behind the initialisation vector when one was made here.
	const int front = ivadd == 16 ? 16 : 0;
	if(outint || outflt) {
		for(int i = 0; i < front; i++) {
			if(outint) outint[i] = iv[i]; else outflt[i] = iv[i];
		}
		for(int i = 0; i < size; i++) {
			if(outint) outint[front + i] = result[i]; else outflt[front + i] = result[i];
		}
	} else if(outstr) {
		if(size + front >= 256) error_throw_legacy("Too many elements for string output");
		memcpy(&outstr[1], iv, front);
		memcpy(&outstr[1 + front], result, size);
		outstr[0] = size + front;
	}
}

void cmd_math(void){
	const char *tp;
    int t = T_NBR;
    MMFLOAT f;
    MMINTEGER i64;
    char *s;
	DIMTYPE dims[MAXDIM]={0};

	skipspace(cmdline);
	if(toupper(*cmdline)=='S'){
		tp = checkstring(cmdline,  "SINC");
		if(tp) {
			// MATH SINC x(), y(), n, [m,] window, frequency, xout(), yout(), as on the PicoMite.
			MMFLOAT *a1float=NULL, *a2float=NULL, *a3float=NULL, *a4float=NULL;
			getargs(&tp, 15, DELIM_COMMA);
			if(argc != 13 && argc != 15) error_throw_legacy("Argument count");
			const int n = getint(argv[4], 1, 10000);
			int m, window;
			MMFLOAT frequency;
			if(argc == 13) {
				m = n;
				window = getint(argv[6], 3, 101);
				frequency = getnumber(argv[8]);
			} else {
				m = getint(argv[6], 1, 20000);
				window = getint(argv[8], 3, 101);
				frequency = getnumber(argv[10]);
			}
			if(frequency <= 0.0 || frequency > 0.5) error_throw_legacy("Frequency must be >0 and <=0.5");
			if(parsefloatrarray(argv[0], &a1float, 1, 1, NULL, false) < n) error_throw_legacy("Array size");
			if(parsefloatrarray(argv[2], &a2float, 2, 1, NULL, false) < n) error_throw_legacy("Array size");
			const int o = (argc == 13) ? 10 : 12;
			if(parsefloatrarray(argv[o], &a3float, 3, 1, NULL, true) < m) error_throw_legacy("Array size");
			if(parsefloatrarray(argv[o + 2], &a4float, 4, 1, NULL, true) < m) error_throw_legacy("Array size");
			if(m == n) sinc_filter(a1float, a2float, n, a3float, a4float, frequency, window);
			else sinc_filter_interpolate(a1float, a2float, n, m, a3float, a4float, frequency, window);
			return;
		}

		tp = checkstring(cmdline,  "SET");
		if(tp) {
			int i,card1=1;
			MMFLOAT *a1float=NULL;
			int64_t *a1int=NULL;
			getargs(&tp, 3, DELIM_COMMA);
			if(!(argc == 3)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[2],&a1float,&a1int,2,0, dims, true);
		    evaluate(argv[0], &f, &i64, &s, &t, false);
		    if(t & T_STR) ERROR_SYNTAX;

			if(a1float!=NULL){
				for(i=0; i< card1;i++)*a1float++ = ((t & T_INT) ? (MMFLOAT)i64 : f);
			} else {
				for(i=0; i< card1;i++)*a1int++ = ((t & T_INT) ? i64 : FloatToInt64(f));
			}
			return;
		}

		tp = checkstring(cmdline,  "SCALE");
		if(tp) {
			int i,card1=1, card2=1;
			MMFLOAT *a1float=NULL,*a2float=NULL, scale;
			int64_t *a1int=NULL, *a2int=NULL;
			getargs(&tp, 5, DELIM_COMMA);
			if(!(argc == 5)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0],&a1float,&a1int,1,0, dims, false);
		    evaluate(argv[2], &f, &i64, &s, &t, false);
		    if(t & T_STR) ERROR_SYNTAX;
		    scale=getnumber(argv[2]);
			card2=parsenumberarray(argv[4],&a2float,&a2int,3,0, dims, true);
			if(card1 != card2)error_throw_legacy("Size mismatch");
			if(scale!=1.0){
				if(a2float!=NULL && a1float!=NULL){
					for(i=0; i< card1;i++)*a2float++ = ((t & T_INT) ? (MMFLOAT)i64 : f) * (*a1float++);
				} else if(a2float!=NULL && a1float==NULL){
					for(i=0; i< card1;i++)(*a2float++) = ((t & T_INT) ? (MMFLOAT)i64 : f) * ((MMFLOAT)*a1int++);
				} else if(a2float==NULL && a1float!=NULL){
					for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(((t & T_INT) ? i64 : FloatToInt64(f)) * (*a1float++));
				} else {
					for(i=0; i< card1;i++)(*a2int++) = ((t & T_INT) ? i64 : FloatToInt64(f)) * (*a1int++);
				}
			} else {
				if(a2float!=NULL && a1float!=NULL){
					for(i=0; i< card1;i++)*a2float++ = *a1float++;
				} else if(a2float!=NULL && a1float==NULL){
					for(i=0; i< card1;i++)(*a2float++) = ((MMFLOAT)*a1int++);
				} else if(a2float==NULL && a1float!=NULL){
					for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(*a1float++);
				} else {
					for(i=0; i< card1;i++)*a2int++ = *a1int++;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "SHIFT");
		if(tp) {
			int i, card1=1, card2=1;
			int64_t *a1int=NULL, *a2int=NULL;
			getargs(&tp, 7, DELIM_COMMA);
			if(!(argc == 5 || argc==7)) error_throw_legacy("Argument count");
			card1=parseintegerarray(argv[0],&a1int,1,0, dims, false);
		    evaluate(argv[2], &f, &i64, &s, &t, false);
		    int shift=getint(argv[2], -63,63);
			card2=parseintegerarray(argv[4],&a2int,3,0, dims, true);
			if(card1 != card2)error_throw_legacy("Size mismatch");
				if(shift>0)for(i=0; i< card1;i++)*a2int++ = (((uint64_t)*a1int++)<<shift);
				else {
					if(argc==7 && checkstring(argv[6], "U")){
						for(i=0; i< card1;i++)*a2int++ = ((uint64_t)(*a1int++)>>(-shift));
					} else {
						for(i=0; i< card1;i++)*a2int++ = ((*a1int++)>>(-shift));
					}
				}
			return;
		}

		tp = checkstring(cmdline,  "SLICE");
		if(tp) {
			int i, j, start, increment, dim[MAXDIM], pos[MAXDIM],off[MAXDIM], dimcount=0, target=-1, toarray=0;
			int64_t *a1int=NULL,*a2int=NULL;
			MMFLOAT *afloat=NULL;
			getargs(&tp, 13, DELIM_COMMA);
			if(argc<7)error_throw_legacy("Argument count");
			parsenumberarray(argv[0],&afloat,&a1int,1,0,dims, false);
			if(!a1int)a1int=(int64_t *)afloat;
			if(dims[1]<=0)error_throw_legacy("Argument 1 must be a 2D or more numerical array");
			for(i=0;i<MAXDIM;i++){
				if(dims[i]-mmb_options.base>0){
					dimcount++;
					dim[i]=dims[i]-mmb_options.base;
				} else dim[i]=0;
			}
			if(((argc-1)/2-1)!=dimcount)error_throw_legacy("Argument count");
			for(i=0; i<dimcount;i++ ){
				if(*argv[i*2 +2]) pos[i]=getint(argv[i*2 +2],mmb_options.base,dim[i]+mmb_options.base)-mmb_options.base;
				else {
					if(target!=-1)error_throw_legacy("Only one index can be omitted");
					target=i;
					pos[i]=1;
				}
			}
			toarray=parsenumberarray(argv[i*2 +2],&afloat,&a2int,i+1,1,dims, true)-1;
			if(!a2int)a2int=(int64_t *)afloat;
			if(dim[target]!=toarray)error_throw_legacy("Size mismatch between slice and target array");
			i=dimcount-1;

			DIAGNOSTIC_IGNORE_ARRAY_BOUNDS
			while(i>=0){
				off[i]=1;
				for(j=0; j<i; j++)off[i]*=(dim[j]+1);
				i--;
			}
			DIAGNOSTIC_RESTORE

			start=1;

			DIAGNOSTIC_IGNORE_MAYBE_UNINITIALIZED
			for(i=0;i<dimcount;i++){
				start+= (pos[i]*off[i]);
			}
			DIAGNOSTIC_RESTORE

			start--;
			increment=off[target];
			start-=increment;
			for(i=0;i<=dim[target];i++)*a2int++ = a1int[start+i*increment];
			return;
		}
		tp = checkstring(cmdline,  "SENSORFUSION");
		if(tp) {
			cmd_SensorFusion((char *)tp);
			return;
		}
	} else if(toupper(*cmdline)=='C') {
		tp = checkstring(cmdline,  "CLAMP");
		if(tp) {
			// MATH CLAMP in(), lo, hi, out(): every element limited to lo..hi, as on the PicoMite.
			MMFLOAT *a1float=NULL, *a2float=NULL;
			int64_t *a1int=NULL, *a2int=NULL;
			getargs(&tp, 7, DELIM_COMMA);
			if(argc != 7) error_throw_legacy("Argument count");
			const int card1 = parsenumberarray(argv[0], &a1float, &a1int, 1, 0, dims, false);
			const MMFLOAT lo = getnumber(argv[2]);
			const MMFLOAT hi = getnumber(argv[4]);
			if(lo > hi) error_throw_legacy("Low limit above high limit");
			const int card2 = parsenumberarray(argv[6], &a2float, &a2int, 4, 0, dims, true);
			if(card1 != card2) error_throw_legacy("Size mismatch");
			const int64_t ilo = FloatToInt64(lo), ihi = FloatToInt64(hi);
			for(int i = 0; i < card1; i++) {
				if(a1float && a2float) {
					const MMFLOAT v = a1float[i];
					a2float[i] = v < lo ? lo : (v > hi ? hi : v);
				} else if(a2float) {
					const MMFLOAT v = (MMFLOAT) a1int[i];
					a2float[i] = v < lo ? lo : (v > hi ? hi : v);
				} else if(a1float) {
					const MMFLOAT v = a1float[i];
					a2int[i] = FloatToInt64(v < lo ? lo : (v > hi ? hi : v));
				} else {
					const int64_t v = a1int[i];
					a2int[i] = v < ilo ? ilo : (v > ihi ? ihi : v);
				}
			}
			return;
		}
		const char *tp1=NULL;
		tp = checkstring(cmdline,  "C_ADD");
		if(tp) {
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			int64_t *a1int=NULL,*a2int=NULL,*a3int=NULL;
			int card=parsearrays(tp, &a1float, &a2float, &a3float, &a1int, &a2int, &a3int);
			if(a1float){
				while(card--){
					*a3float++ = *a1float++ + *a2float++;
				}
			} else {
				while(card--){
					*a3int++ = *a1int++ + *a2int++;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "C_MUL");
		tp1 = checkstring(cmdline,  "C_MULT");
		if(tp || tp1) {
			if(tp1)tp=tp1;
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			int64_t *a1int=NULL,*a2int=NULL,*a3int=NULL;
			int card=parsearrays(tp, &a1float, &a2float, &a3float, &a1int, &a2int, &a3int);
			if(a1float){
				while(card--){
					*a3float++ = *a1float++ * *a2float++;
				}
			} else {
				while(card--){
					*a3int++ = *a1int++ * *a2int++;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "C_AND");
		if(tp) {
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			int64_t *a1int=NULL,*a2int=NULL,*a3int=NULL;
			int card=parsearrays(tp, &a1float, &a2float, &a3float, &a1int, &a2int, &a3int);
			if(a1float){
				while(card--){
					*a3float++ = (MMFLOAT)((int64_t)*a1float++ & (int64_t)*a2float++);
				}
			} else {
				while(card--){
					*a3int++ = *a1int++ & *a2int++;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "C_XOR");
		if(tp) {
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			int64_t *a1int=NULL,*a2int=NULL,*a3int=NULL;
			int card=parsearrays(tp, &a1float, &a2float, &a3float, &a1int, &a2int, &a3int);
			if(a1float){
				while(card--){
					*a3float++ = (MMFLOAT)((int64_t)*a1float++ ^ (int64_t)*a2float++);
				}
			} else {
				while(card--){
					*a3int++ = *a1int++ & *a2int++;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "C_OR");
		if(tp) {
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			int64_t *a1int=NULL,*a2int=NULL,*a3int=NULL;
			int card=parsearrays(tp, &a1float, &a2float, &a3float, &a1int, &a2int, &a3int);
			if(a1float){
				while(card--){
					*a3float++ = (MMFLOAT)((int64_t)*a1float++ | (int64_t)*a2float++);
				}
			} else {
				while(card--){
					*a3int++ = *a1int++ & *a2int++;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "C_SUB");
		if(tp) {
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			int64_t *a1int=NULL,*a2int=NULL,*a3int=NULL;
			int card=parsearrays(tp, &a1float, &a2float, &a3float, &a1int, &a2int, &a3int);
			if(a1float){
				while(card--){
					*a3float++ = *a1float++ - *a2float++;
				}
			} else {
				while(card--){
					*a3int++ = *a1int++ - *a2int++;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "C_DIV");
		if(tp) {
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			int64_t *a1int=NULL,*a2int=NULL,*a3int=NULL;
			int card=parsearrays(tp, &a1float, &a2float, &a3float, &a1int, &a2int, &a3int);
			if(a1float){
				while(card--){
					*a3float++ = *a1float++ / *a2float++;
				}
			} else {
				while(card--){
					*a3int++ = *a1int++ / *a2int++;
				}
			}
			return;
		}
	} else if(toupper(*cmdline)=='V') {
		tp = checkstring(cmdline,  "V_MULT");
		if(tp) {
			int i,j, numcols=0, numrows=0;
			MMFLOAT *a1float=NULL,*a2float=NULL,*a2sfloat=NULL,*a3float=NULL;
			getargs(&tp, 5, DELIM_COMMA);
			if(!(argc == 5)) error_throw_legacy("Argument count");
			parsefloatrarray(argv[0],&a1float,1,2,dims,false);
			numcols=dims[0] - mmb_options.base;
			numrows=dims[1] - mmb_options.base;
			parsefloatrarray(argv[2],&a2float,1,1,dims,false);
			if((dims[0] - mmb_options.base) != numcols)error_throw_legacy("Array size mismatch");
			parsefloatrarray(argv[4],&a3float,1,1,dims,true);
			if((dims[0] - mmb_options.base) != numrows)error_throw_legacy("Array size mismatch");
			if(a3float==a1float || a3float==a2float)error_throw_legacy("Destination array same as source");
			a2sfloat=a2float;
			numcols++;
			numrows++;
			for(i=0;i<numrows;i++){
				a2float=a2sfloat;
				*a3float=0.0;
				for(j=0;j<numcols;j++){
					*a3float= *a3float + ((*a1float++) * (*a2float++));
				}
				a3float++;
			}
			return;
		}

		tp = checkstring(cmdline,  "V_ROTATE");
		if(tp) {
	    // xorigin!, yorigin!,angle!,xin!(), yin!(),xout(1), yout!()
			getargs(&tp, 13, DELIM_COMMA);
			if(!(argc == 13)) error_throw_legacy("Argument count");
			MMFLOAT xorigin=getnumber(argv[0]);
			MMFLOAT yorigin=getnumber(argv[2]);
			MMFLOAT angle=getnumber(argv[4])/ANGLE_CONVERSION;
			MMFLOAT *a1float=NULL, *xfout=NULL, *yfout=NULL, cangle=cos(angle), sangle=sin(angle),x,y;
			int64_t *a1int=NULL, *xiout=NULL, *yiout=NULL;
			int numpoints=parsenumberarray(argv[6],&a1float,&a1int,4,1,dims, false);
			MMFLOAT *xin=GetTempMemory(numpoints * sizeof(MMFLOAT));
			for(int i=0;i<numpoints;i++)xin[i]=(a1float!=NULL ? a1float[i]-xorigin : (MMFLOAT)a1int[i]-xorigin);
			a1float=NULL;
			a1int=NULL;
			if(parsenumberarray(argv[8],&a1float,&a1int,5,1,dims, false)!=numpoints)error_throw_legacy("Array size mismatch");
			MMFLOAT *yin=GetTempMemory(numpoints * sizeof(MMFLOAT));
			for(int i=0;i<numpoints;i++)yin[i]=(a1float!=NULL ? a1float[i]-yorigin : (MMFLOAT)a1int[i]-yorigin);
			a1float=NULL;
			a1int=NULL;
			if(parsenumberarray(argv[10],&xfout,&xiout,6,1,dims, false)!=numpoints)error_throw_legacy("Array size mismatch");
			if(parsenumberarray(argv[12],&yfout,&yiout,7,1,dims, false)!=numpoints)error_throw_legacy("Array size mismatch");
			for(int i=0;i<numpoints;i++){
				x= xin[i] * cangle - yin[i] * sangle + xorigin;
				y= yin[i] * cangle + xin[i] * sangle + yorigin;
				if(xfout)xfout[i]=x;
				else xiout[i]=round(x);
				if(yfout)yfout[i]=y;
				else yiout[i]=round(y);
			}
			return;
		}
		tp = checkstring(cmdline,  "V_NORMALISE");
		if(tp) {
			int j, numrows=0, card2;
			MMFLOAT *a1float=NULL,*a1sfloat=NULL,*a2float=NULL,mag=0.0;
			getargs(&tp, 3, DELIM_COMMA);
			if(!(argc == 3)) error_throw_legacy("Argument count");
			numrows=parsefloatrarray(argv[0],&a1float,1,1, dims, false);
			a1sfloat=a1float;
			card2=parsefloatrarray(argv[2],&a2float,2,1, dims, true);
			if(numrows!=card2)error_throw_legacy("Array size mismatch");
			for(j=0;j<numrows;j++){
				mag+= (*a1sfloat) * (*a1sfloat);
				a1sfloat++;
			}
			mag= sqrt(mag);
			for(j=0;j<numrows;j++){
				*a2float++ = (*a1float++)/mag;
			}
			return;
		}

		tp = checkstring(cmdline,  "V_CROSS");
		if(tp) {
			int j, numcols=0;
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			MMFLOAT a[3],b[3];
			getargs(&tp, 5, DELIM_COMMA);
			if(!(argc == 5)) error_throw_legacy("Argument count");
			numcols=parsefloatrarray(argv[0],&a1float,1,1, dims, false);
			if(numcols!=3)error_throw_legacy("Argument 1 must be a 3 element floating point array");
			numcols=parsefloatrarray(argv[2],&a2float,2,1, dims, false);
			if(numcols!=3)error_throw_legacy("Argument 2 must be a 3 element floating point array");
			numcols=parsefloatrarray(argv[4],&a3float,3,1, dims, true);
			if(numcols!=3)error_throw_legacy("Argument 3 must be a 3 element floating point array");
			for(j=0;j<numcols;j++){
				a[j]=*a1float++;
				b[j]=*a2float++;
			}
			*a3float++ = a[1]*b[2] - a[2]*b[1];
			*a3float++ = a[2]*b[0] - a[0]*b[2];
			*a3float = a[0]*b[1] - a[1]*b[0];
			return;
		}
		tp = checkstring(cmdline,  "V_PRINT");
		if(tp) {
			int j, numcols=0;
			MMFLOAT *a1float=NULL;
			int64_t *a1int=NULL;
			getargs(&tp, 3, DELIM_COMMA);
			if(!(argc == 1 || argc == 3)) error_throw_legacy("Argument count");
			numcols=parsenumberarray(argv[0],&a1float,&a1int,1,1, dims, false);
			// MATH V_PRINT a%(), HEX, as on the PicoMite and CMM2 V6.
			if(argc == 3 && !checkstring(argv[2], "HEX")) ERROR_SYNTAX;
			if(a1float!=NULL){
				if(argc == 3) error_throw_legacy("Trying to print a float in HEX");
				PFlt(*a1float++);
				for(j=1;j<numcols;j++)PFltComma(*a1float++);
				PRet();
			} else if(argc == 3) {
				PIntH(*a1int++);
				for(j=1;j<numcols;j++)PIntHComma(*a1int++);
				PRet();
			} else {
				PInt(*a1int++);
				for(j=1;j<numcols;j++)PIntComma(*a1int++);
				PRet();
			}
			return;
		}
	} else if(toupper(*cmdline)=='M') {
		tp = checkstring(cmdline,  "M_INVERSE");
		if(tp){
			int i, j, n, numcols=0, numrows=0;
			MMFLOAT *a1float=NULL, *a2float=NULL,det;
			getargs(&tp, 3, DELIM_COMMA);
			if(!(argc == 3)) error_throw_legacy("Argument count");
			parsefloatrarray(argv[0], &a1float, 1,2,dims, false);
			numcols=dims[0] - mmb_options.base;
			numrows=dims[1] - mmb_options.base;
			parsefloatrarray(argv[2], &a2float, 2,2,dims, true);
			if(dims[0] - mmb_options.base != numcols || dims[1] - mmb_options.base!= numrows)error_throw_legacy("Array size mismatch");
			if(numcols!=numrows)error_throw_legacy("Array must be square");
			if(a1float==a2float)error_throw_legacy("Same array specified for input and output");
			n=numrows+1;
			MMFLOAT **matrix=alloc2df(n,n);
			for(i=0;i<n;i++){ //load the matrix
				for(j=0;j<n;j++){
					matrix[j][i]=*a1float++;
				}
			}
			det=determinant(matrix,n);
			if(det==0.0){
				dealloc2df(matrix,n,n);
				error_throw_legacy("Determinant of array is zero");
			}
			MMFLOAT **matrix1=alloc2df(n,n);
			cofactor(matrix, matrix1, n);
			for(i=0;i<n;i++){ //load the matrix
				for(j=0;j<n;j++){
					*a2float++=matrix1[j][i];
				}
			}
			dealloc2df(matrix,n,n);
			dealloc2df(matrix1,n,n);

			return;
		}
		tp = checkstring(cmdline,  "M_TRANSPOSE");
		if(tp) {
			int i,j, numcols1=0, numrows1=0, numcols2=0, numrows2=0;
			MMFLOAT *a1float=NULL,*a2float=NULL;
			getargs(&tp, 3, DELIM_COMMA);
			if(!(argc == 3)) error_throw_legacy("Argument count");
			parsefloatrarray(argv[0], &a1float, 1,2,dims, false);
			numcols1=numrows2=dims[0] - mmb_options.base;
			numrows1=numcols2=dims[1] - mmb_options.base;
			parsefloatrarray(argv[2], &a2float,2,2,dims, true);
			if(numcols2 !=dims[0] - mmb_options.base)error_throw_legacy("Array size mismatch");
			if(numrows2 !=dims[1] - mmb_options.base)error_throw_legacy("Array size mismatch");
			numcols1++;
			numrows1++;
			numcols2++;
			numrows2++;
			MMFLOAT **matrix1=alloc2df(numcols1,numrows1);
			MMFLOAT **matrix2=alloc2df(numcols2,numrows2);
			for(i=0;i<numrows1;i++){
				for(j=0;j<numcols1;j++){
					matrix1[j][i]=*a1float++;
				}
			}
			for(i=0;i<numrows1;i++){
				for(j=0;j<numcols1;j++){
					matrix2[i][j]=matrix1[j][i];
				}
			}
			for(i=0;i<numrows2;i++){
				for(j=0;j<numcols2;j++){
					*a2float++=matrix2[j][i];
				}
			}
			dealloc2df(matrix1,numcols1,numrows1);
			dealloc2df(matrix2,numcols2,numrows2);
			return;
		}

		tp = checkstring(cmdline,  "M_MULT");
		if(tp) {
			int i,j, k, numcols1=0, numrows1=0, numcols2=0, numrows2=0, numcols3=0, numrows3=0;
			MMFLOAT *a1float=NULL,*a2float=NULL,*a3float=NULL;
			getargs(&tp, 5, DELIM_COMMA);
			if(!(argc == 5)) error_throw_legacy("Argument count");
			parsefloatrarray(argv[0], &a1float, 1, 2, dims, false);
			numcols1=numrows2=dims[0] - mmb_options.base + 1;
			numrows1=dims[1] - mmb_options.base + 1;
			parsefloatrarray(argv[2], &a2float, 2, 2, dims, false);
			numcols2=dims[0] - mmb_options.base + 1;
			numrows2=dims[1] - mmb_options.base + 1;
			if(numrows2!=numcols1)error_throw_legacy("Input array size mismatch");
			parsefloatrarray(argv[4], &a3float, 3, 2, dims, true);
			numcols3=dims[0] - mmb_options.base + 1;
			numrows3=dims[1] - mmb_options.base + 1;
			if(numcols3!=numcols2 || numrows3!=numrows1)error_throw_legacy("Output array size mismatch");
			if(a3float==a1float || a3float==a2float)error_throw_legacy("Destination array same as source");
//			MMFLOAT **matrix1=alloc2df(numcols1,numrows1);
//			MMFLOAT **matrix2=alloc2df(numcols2,numrows2);
/*			s=a1float;
			for(i=0;i<numrows1;i++){ //load the first matrix
				for(j=0;j<numcols1;j++){
					matrix1[j][i]=*a1float++;
				}
			}
			a1float=s;
			s=a2float;
			for(i=0;i<numrows2;i++){ //load the second matrix
				for(j=0;j<numcols2;j++){
					matrix2[j][i]=*a2float++;
				}
			}
			a2float=s;*/
	// Now calculate the dot products
			for(i=0;i<numrows3;i++){
				for(j=0;j<numcols3;j++){
					*a3float=0.0;
					for(k=0;k<numcols1;k++){
	//					PFlt(farr2d(a1float,numcols1,k,i));PFltComma(matrix1[k][i]);PFltComma(farr2d(a2float,numcols2,j,k));PFltComma(matrix2[j][k]);PRet();
						*a3float+=farr2d(a1float,numcols1,k,i)*farr2d(a2float,numcols2,j,k);
	//					*a3float+= matrix1[k][i] * matrix2[j][k];
					}
					a3float++;
				}
			}

//			dealloc2df(matrix1,numcols1,numrows1);
//			dealloc2df(matrix2,numcols2,numrows2);
			return;
		}
		tp = checkstring(cmdline,  "M_PRINT");
		if(tp) {
			int i,j, numcols=0, numrows=0;
			MMFLOAT *a1float=NULL;
			int64_t *a1int=NULL;
			// need three arrays with same cardinality, second array must be 2 dimensional
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			parsenumberarray(argv[0],&a1float,&a1int,1,2,dims, false);
			numcols=dims[0]+1-mmb_options.base;
			numrows=dims[1]+1-mmb_options.base;
//			MMFLOAT **matrix=alloc2df(numcols,numrows);
//			int64_t **imatrix= (int64_t **)matrix;
			if(a1float!=NULL){
/*				for(i=0;i<numrows;i++){
					for(j=0;j<numcols;j++){
						matrix[j][i]=*a1float++;
					}
				}*/
				for(i=0;i<numrows;i++){
					PFlt(farr2d(a1float,numcols,0,i));
//					PFlt(matrix[0][i]);
					for(j=1;j<numcols;j++){
						PFltComma(farr2d(a1float,numcols,j,i));
//						PFltComma(matrix[j][i]);
					}
					PRet();
				}
			} else {
/*				for(i=0;i<numrows;i++){
					for(j=0;j<numcols;j++){
						imatrix[j][i]=*a1int++;
					}
				}*/
				for(i=0;i<numrows;i++){
					PInt(iarr2d(a1int,numcols,0,i));
//					PInt(imatrix[0][i]);
					for(j=1;j<numcols;j++){
						PIntComma(iarr2d(a1int,numcols,j,i));
//						PIntComma(imatrix[j][i]);
					}
					PRet();
				}
			}
//			dealloc2df(matrix,numcols,numrows);
			return;
		}
	} else if(toupper(*cmdline)=='Q') {

		tp = checkstring(cmdline,  "Q_INVERT");
		if(tp) {
			int card;
			MMFLOAT *q=NULL,*n=NULL;
			getargs(&tp, 3, DELIM_COMMA);
			if(!(argc == 3)) error_throw_legacy("Argument count");
			card=parsefloatrarray(argv[0],&q,1,1, dims, false);
			if(card!=5)error_throw_legacy("Argument 1 must be a 5 element floating point array");
			card=parsefloatrarray(argv[2],&n,2,1, dims, true);
			if(card!=5)error_throw_legacy("Argument 2 must be a 5 element floating point array");
			Q_Invert(q, n);
			return;
		}

		tp = checkstring(cmdline,  "Q_VECTOR");
		if(tp) {
			int card;
			MMFLOAT *q=NULL;
			MMFLOAT mag=0.0;
			getargs(&tp, 7, DELIM_COMMA);
			if(!(argc == 7)) error_throw_legacy("Argument count");
			MMFLOAT x=getnumber(argv[0]);
			MMFLOAT y=getnumber(argv[2]);
			MMFLOAT z=getnumber(argv[4]);
			card=parsefloatrarray(argv[6],&q,4,1, dims, true);
			if(card!=5)error_throw_legacy("Argument 4 must be a 5 element floating point array");
			mag=sqrt(x*x + y*y + z*z) ;//calculate the magnitude
			q[0]=0.0; //create a normalised vector
			q[1]=x/mag;
			q[2]=y/mag;
			q[3]=z/mag;
			q[4]=mag;
			return;
		}

		tp = checkstring(cmdline,  "Q_EULER");
		if(tp) {
			int card;
			MMFLOAT *q=NULL;
			getargs(&tp, 7, DELIM_COMMA);
			if(!(argc == 7)) error_throw_legacy("Argument count");
			MMFLOAT yaw=-getnumber(argv[0])/ANGLE_CONVERSION;
			MMFLOAT pitch=getnumber(argv[2])/ANGLE_CONVERSION;
			MMFLOAT roll=getnumber(argv[4])/ANGLE_CONVERSION;
			card=parsefloatrarray(argv[6],&q,4,1, dims, true);
			if(card!=5)error_throw_legacy("Argument 4 must be a 5 element floating point array");
			MMFLOAT s1=sin(pitch/2);
			MMFLOAT c1=cos(pitch/2);
			MMFLOAT s2=sin(yaw/2);
			MMFLOAT c2=cos(yaw/2);
			MMFLOAT s3=sin(roll/2);
			MMFLOAT c3=cos(roll/2);
			q[1] = s1 * c2 * c3 - c1 * s2 * s3;
			q[2] = c1 * s2 * c3 + s1 * c2 * s3;
			q[3] = c1 * c2 * s3 - s1 * s2 * c3;
			q[0] = c1 * c2 * c3 + s1 * s2 * s3;
			q[4]=1.0;
			return;
		}

		tp = checkstring(cmdline,  "Q_CREATE");
		if(tp) {
			int card;
			MMFLOAT *q=NULL;
			MMFLOAT mag=0.0;
			getargs(&tp, 9, DELIM_COMMA);
			if(!(argc == 9)) error_throw_legacy("Argument count");
			MMFLOAT theta=getnumber(argv[0]);
			MMFLOAT x=getnumber(argv[2]);
			MMFLOAT y=getnumber(argv[4]);
			MMFLOAT z=getnumber(argv[6]);
			card=parsefloatrarray(argv[8],&q,5,1, dims, true);
			if(card!=5)error_throw_legacy("Argument 4 must be a 5 element floating point array");
			MMFLOAT sineterm= sin(theta/2.0/ANGLE_CONVERSION);
			q[0]=cos(theta/2.0);
			q[1]=x* sineterm;
			q[2]=y* sineterm;
			q[3]=z* sineterm;
			mag=sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]) ;//calculate the magnitude
			q[0]=q[0]/mag; //create a normalised quaternion
			q[1]=q[1]/mag;
			q[2]=q[2]/mag;
			q[3]=q[3]/mag;
			q[4]=1.0;
			return;
		}

		tp = checkstring(cmdline,  "Q_MULT");
		if(tp) {
			MMFLOAT *q1=NULL,*q2=NULL,*n=NULL;
			int card;
			getargs(&tp, 5, DELIM_COMMA);
			if(!(argc == 5)) error_throw_legacy("Argument count");
			card=parsefloatrarray(argv[0],&q1,1,1, dims, false);
			if(card!=5)error_throw_legacy("Argument 1 must be a 5 element floating point array");
			card=parsefloatrarray(argv[2],&q2,2,1, dims, false);
			if(card!=5)error_throw_legacy("Argument 2 must be a 5 element floating point array");
			card=parsefloatrarray(argv[4],&n,31,1, dims, true);
			if(card!=5)error_throw_legacy("Argument 3 must be a 5 element floating point array");
			Q_Mult(q1, q2, n);
			return;
		}

		tp = checkstring(cmdline,  "Q_ROTATE");
		if(tp) {
			int card;
			MMFLOAT *q1=NULL,*v1=NULL,*n=NULL;
			MMFLOAT temp[5], qtemp[5];
			getargs(&tp, 5, DELIM_COMMA);
			if(!(argc == 5)) error_throw_legacy("Argument count");
			card=parsefloatrarray(argv[0],&q1,1,1, dims, false);
			if(card!=5)error_throw_legacy("Argument 1 must be a 5 element floating point array");
			card=parsefloatrarray(argv[2],&v1,2,1, dims, false);
			if(card!=5)error_throw_legacy("Argument 2 must be a 5 element floating point array");
			card=parsefloatrarray(argv[4],&n,31,1, dims, true);
			if(card!=5)error_throw_legacy("Argument 3 must be a 5 element floating point array");
			Q_Mult(q1, v1, temp);
			Q_Invert(q1, qtemp);
			Q_Mult(temp, qtemp, n);
			return;
		}
	} else {
		tp = checkstring(cmdline,  "AES128");
		if(tp) {
			math_aes128(tp);
			return;
		}
		tp = checkstring(cmdline,  "PID");
		if(tp) {
			cmd_math_pid(tp);
			return;
		}
		tp = checkstring(cmdline,  "ADD");
		if(tp) {
			int i,card1=1, card2=1;
			MMFLOAT *a1float=NULL,*a2float=NULL, scale;
			int64_t *a1int=NULL, *a2int=NULL;
			getargs(&tp, 5, DELIM_COMMA);
			if(!(argc == 5)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0], &a1float, &a1int, 1, 0,dims, false);
		    evaluate(argv[2], &f, &i64, &s, &t, false);
		    if(t & T_STR) ERROR_SYNTAX;
		    scale=getnumber(argv[2]);
			card2=parsenumberarray(argv[4], &a2float, &a2int, 3, 0,dims, true);
			if(card1 != card2)error_throw_legacy("Array size mismatch");
			if(scale!=0.0){
				if(a2float!=NULL && a1float!=NULL){
					for(i=0; i< card1;i++)*a2float++ = ((t & T_INT) ? (MMFLOAT)i64 : f) + (*a1float++);
				} else if(a2float!=NULL && a1float==NULL){
					for(i=0; i< card1;i++)(*a2float++) = ((t & T_INT) ? (MMFLOAT)i64 : f) + ((MMFLOAT)*a1int++);
				} else if(a2float==NULL && a1float!=NULL){
					for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(((t & T_INT) ? i64 : FloatToInt64(f)) + (*a1float++));
				} else {
					for(i=0; i< card1;i++)(*a2int++) = ((t & T_INT) ? i64 : FloatToInt64(f)) + (*a1int++);
				}
			} else {
				if(a2float!=NULL && a1float!=NULL){
					for(i=0; i< card1;i++)*a2float++ = *a1float++;
				} else if(a2float!=NULL && a1float==NULL){
					for(i=0; i< card1;i++)(*a2float++) = ((MMFLOAT)*a1int++);
				} else if(a2float==NULL && a1float!=NULL){
					for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(*a1float++);
				} else {
					for(i=0; i< card1;i++)*a2int++ = *a1int++;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "POWER");
		if(tp) {
			int i,card1=1, card2=1;
			MMFLOAT *a1float=NULL,*a2float=NULL, scale;
			int64_t *a1int=NULL, *a2int=NULL;
			getargs(&tp, 5, DELIM_COMMA);
			if(!(argc == 5)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0], &a1float, &a1int, 1, 0,dims, false);
		    evaluate(argv[2], &f, &i64, &s, &t, false);
		    if(t & T_STR) ERROR_SYNTAX;
		    scale=getnumber(argv[2]);
			card2=parsenumberarray(argv[4], &a2float, &a2int, 3, 0,dims, true);
			if(card1 != card2)error_throw_legacy("Array size mismatch");
			if(scale!=1.0){
				if(a2float!=NULL && a1float!=NULL){
					for(i=0; i< card1;i++)*a2float++ = pow(*a1float++,(t & T_INT) ? (MMFLOAT)i64 : f);
				} else if(a2float!=NULL && a1float==NULL){
					for(i=0; i< card1;i++)(*a2float++) = pow((MMFLOAT)*a1int++,((t & T_INT) ? (MMFLOAT)i64 : f));
				} else if(a2float==NULL && a1float!=NULL){
					for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(pow(*a1float++,((t & T_INT) ? i64 : FloatToInt64(f))));
				} else {
					for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(pow(*a1int++,(t & T_INT) ? i64 : FloatToInt64(f)));
				}
			} else {
				if(a2float!=NULL && a1float!=NULL){
					for(i=0; i< card1;i++)*a2float++ = *a1float++;
				} else if(a2float!=NULL && a1float==NULL){
					for(i=0; i< card1;i++)(*a2float++) = ((MMFLOAT)*a1int++);
				} else if(a2float==NULL && a1float!=NULL){
					for(i=0; i< card1;i++)(*a2int++) = FloatToInt64(*a1float++);
				} else {
					for(i=0; i< card1;i++)*a2int++ = *a1int++;
				}
			}
			return;
		}

		tp = checkstring(cmdline,  "WINDOW");
		if(tp) {
			int i,card1=1, card2=1;
			MMFLOAT *a1float=NULL,*a2float=NULL, outmin,outmax, inmin=1.5e+308 , inmax=-1.5e308;
			int64_t *a1int=NULL, *a2int=NULL;
			getargs(&tp, 11, DELIM_COMMA);
			if(!(argc == 7 || argc==11)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0], &a1float, &a1int, 1, 0,dims, false);
		    outmin=getnumber(argv[2]);
			outmax=getnumber(argv[4]);
			card2=parsenumberarray(argv[6], &a2float, &a2int, 4, 0,dims, true);
			if(card1 != card2)error_throw_legacy("Size mismatch");
			for(i=0; i< card1;i++){
				if(a1float!=NULL){ //find min and max if in is a float
					if(a1float[i]<inmin)inmin=a1float[i];
					if(a1float[i]>inmax)inmax=a1float[i];
				} else {
					if(a1int[i]<inmin)inmin=(MMFLOAT)a1int[i];
					if(a1int[i]>inmax)inmax=(MMFLOAT)a1int[i];
				}
			}
			if(argc==11){
				void *ptr1 = findvar(argv[8], V_FIND);
				if(!(vartbl[VarIndex].type & (T_NBR | T_INT))) error_throw_legacy("Invalid variable");
				if(vartbl[VarIndex].type==T_INT)*(long long int *)ptr1=(long long int)inmin;
				else *(MMFLOAT *)ptr1=inmin;
				void *ptr2 = findvar(argv[10], V_FIND);
				if(!(vartbl[VarIndex].type & (T_NBR | T_INT))) error_throw_legacy("Invalid variable");
				if(vartbl[VarIndex].type==T_INT)*(long long int *)ptr2=(long long int)inmax;
				else *(MMFLOAT *)ptr2=inmax;
			}
			if(a2float!=NULL && a1float!=NULL){ //in and out are floats
				for(i=0; i< card1;i++)a2float[i] = ((a1float[i]-inmin)/(inmax-inmin))*(outmax-outmin)+outmin;
			} else if(a2float==NULL && a1float!=NULL){ //in is a float and out is an integer
				for(i=0; i< card1;i++)a2int[i] =(long long int)(((a1float[i]-inmin)/(inmax-inmin))*(outmax-outmin)+outmin);
			} else if(a2float!=NULL && a1float==NULL){ //in is an integer and out is a float
				for(i=0; i< card1;i++)a2float[i] =((((MMFLOAT)a1int[i]-inmin)/(inmax-inmin))*(outmax-outmin)+outmin);
			} else {  // in and out are integers
				for(i=0; i< card1;i++)a2int[i] =(long long int)((((MMFLOAT)a1int[i]-inmin)/(inmax-inmin))*(outmax-outmin)+outmin);
			}
			return;
		}


		tp = checkstring(cmdline,  "RANDOMIZE");
		if(tp) {
			int i;
			getargs(&tp,1, DELIM_COMMA);
			if(argc==1)i = getinteger(argv[0]);
			else i= mmtime_now_ns() / 1000; // time_us_32();
			if(i < 0) error_throw_legacy("Number out of bounds");
			if(g_myrand==NULL)g_myrand=(struct tagMTRand *)GetMemory(sizeof(struct tagMTRand));
			seedRand(i);
			return;
		}
		tp = checkstring(cmdline,  "INTERPOLATE");
		if(tp) {
			int i,card1, card2, card3;
			MMFLOAT *a1float=NULL,*a2float=NULL, *a3float=NULL, scale, tmp1, tmp2, tmp3;
			int64_t *a1int=NULL, *a2int=NULL, *a3int=NULL;
			getargs(&tp, 7, DELIM_COMMA);
			if(!(argc == 7)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0], &a1float, &a1int, 1, 0, dims, false);
		    evaluate(argv[4], &f, &i64, &s, &t, false);
		    if(t & T_STR) ERROR_SYNTAX;
		    scale=getnumber(argv[4]);
			card2=parsenumberarray(argv[2], &a2float, &a2int, 3, 0, dims, false);
			card3=parsenumberarray(argv[6], &a3float, &a3int, 4, 0, dims, true);
			if((card1 != card2) || (card1!=card3))error_throw_legacy("Size mismatch");
			if(a3int!=NULL){
				if((a1int==a2int) || (a1int==a3int) || (a2int==a3int))error_throw_legacy("Arrays must be different");
				for(i=0; i< card1;i++){
					if(a1int!=NULL)tmp1=(MMFLOAT)*a1int++;
					else tmp1=*a1float++;
					if(a2int!=NULL)tmp2=(MMFLOAT)*a2int++;
					else tmp2=*a2float++;
					tmp3=(tmp2-tmp1)*scale + tmp1;
					*a3int++=FloatToInt64(tmp3);
				}
			} else {
				if((a1float==a2float) || (a1float==a3float) || (a2float==a3float))error_throw_legacy("Arrays must be different");
				for(i=0; i< card1;i++){
					if(a1int!=NULL)tmp1=(MMFLOAT)*a1int++;
					else tmp1=*a1float++;
					if(a2int!=NULL)tmp2=(MMFLOAT)*a2int++;
					else tmp2=*a2float++;
					tmp3=(tmp2-tmp1)*scale + tmp1;
					*a3float++=tmp3;
				}
			}
			return;
		}
		tp = checkstring(cmdline,  "INSERT");
		if(tp) {
			int i, j, start, increment, dim[MAXDIM], pos[MAXDIM],off[MAXDIM], dimcount=0, target=-1;
			int64_t *a1int=NULL,*a2int=NULL;
			MMFLOAT *afloat=NULL;
			getargs(&tp, 13, DELIM_COMMA);
			if(argc<7)error_throw_legacy("Argument count");
			parsenumberarray(argv[0],&afloat,&a1int,1,0,dims, false);
			if(!a1int)a1int=(int64_t *)afloat;
			if(dims[1]<=0)error_throw_legacy("Argument 1 must be a 2D or more numerical array");
			for(i=0;i<MAXDIM;i++){
				if(dims[i]-mmb_options.base>0){
					dimcount++;
					dim[i]=dims[i]-mmb_options.base;
				} else dim[i]=0;
			}
			if(((argc-1)/2-1)!=dimcount)error_throw_legacy("Argument count");
			for(i=0; i<dimcount;i++ ){
				if(*argv[i*2 +2]) pos[i]=getint(argv[i*2 +2],mmb_options.base,dim[i]+mmb_options.base)-mmb_options.base;
				else {
					if(target!=-1)error_throw_legacy("Only one index can be omitted");
					target=i;
					pos[i]=1;
				}
			}
			parsenumberarray(argv[i*2 +2],&afloat,&a2int,i+1,1,dims,true);
			if(!a2int)a2int=(int64_t *)afloat;
			if(target==-1)return;
			if(dim[target]+mmb_options.base!=dims[0])error_throw_legacy("Size mismatch between insert and target array");
			i=dimcount-1;

			DIAGNOSTIC_IGNORE_ARRAY_BOUNDS
			while(i>=0){
				off[i]=1;
				for(j=0; j<i; j++)off[i]*=(dim[j]+1);
				i--;
			}
			DIAGNOSTIC_RESTORE

			start=1;

			DIAGNOSTIC_IGNORE_MAYBE_UNINITIALIZED
			for(i=0;i<dimcount;i++){
				start+= (pos[i]*off[i]);
			}
			DIAGNOSTIC_RESTORE

			start--;
			increment=off[target];
			start-=increment;
			for(i=0;i<=dim[target];i++) a1int[start+i*increment]=*a2int++;
			return;
		}
		tp = checkstring(cmdline,  "FFT");
		if(tp) {
			cmd_FFT(tp);
			return;
		}
	}

	ERROR_SYNTAX;
}
fcplx getComplex(const char *in){
	int64_t i=getinteger(in);
	fcplx x;
	memcpy(&x, &i, 8);
	return x;
}
void retComplex(fcplx in){
	iret=0;
	memcpy(&iret, &in, 8);
	targ=T_INT;
}

// ---- MATH(BASE64 ENCODE|DECODE in, out), as on the PicoMite V6.04.00RC2 ----

static const char b64_chr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned b64_int(unsigned ch) {
    if (ch == '+') return 62;
    if (ch == '/') return 63;
    if (ch == '=') return 64;
    if (ch >= '0' && ch <= '9') return ch + 4;
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26;
    return 0;
}

unsigned b64e_size(unsigned in_size) { return 4 * ((in_size + 2) / 3); }
unsigned b64d_size(unsigned in_size) { return (3 * in_size) / 4; }

unsigned b64_encode(const unsigned char *in, unsigned in_len, unsigned char *out) {
    unsigned j = 0, k = 0, s[3] = { 0 };
    for (unsigned i = 0; i < in_len; i++) {
        s[j++] = in[i];
        if (j == 3) {
            out[k + 0] = b64_chr[(s[0] & 255) >> 2];
            out[k + 1] = b64_chr[((s[0] & 0x03) << 4) + ((s[1] & 0xF0) >> 4)];
            out[k + 2] = b64_chr[((s[1] & 0x0F) << 2) + ((s[2] & 0xC0) >> 6)];
            out[k + 3] = b64_chr[s[2] & 0x3F];
            j = 0;
            k += 4;
        }
    }
    if (j) {
        if (j == 1) s[1] = 0;
        out[k + 0] = b64_chr[(s[0] & 255) >> 2];
        out[k + 1] = b64_chr[((s[0] & 0x03) << 4) + ((s[1] & 0xF0) >> 4)];
        out[k + 2] = (j == 2) ? b64_chr[((s[1] & 0x0F) << 2)] : '=';
        out[k + 3] = '=';
        k += 4;
    }
    return k;
}

unsigned b64_decode(const unsigned char *in, unsigned in_len, unsigned char *out) {
    unsigned j = 0, k = 0, s[4];
    for (unsigned i = 0; i < in_len; i++) {
        s[j++] = b64_int(in[i]);
        if (j == 4) {
            out[k + 0] = ((s[0] & 255) << 2) + ((s[1] & 0x30) >> 4);
            if (s[2] != 64) {
                out[k + 1] = ((s[1] & 0x0F) << 4) + ((s[2] & 0x3C) >> 2);
                if (s[3] != 64) {
                    out[k + 2] = ((s[2] & 0x03) << 6) + s[3];
                    k += 3;
                } else {
                    k += 2;
                }
            } else {
                k += 1;
            }
            j = 0;
        }
    }
    return k;
}

void fun_math(void){
	const char *tp, *tp1;
	DIMTYPE dims[MAXDIM]={0};
	skipspace(ep);
	if(toupper(*ep)=='A'){
		tp = checkstring(ep,  "ATAN3");
		if(tp) {
			MMFLOAT y,x,z;
			getargs(&tp, 3, DELIM_COMMA);
			if(argc != 3)ERROR_SYNTAX;
			y=getnumber(argv[0]);
			x=getnumber(argv[2]);
			z=atan2(y,x);
			if (z < 0.0) z = z + 2 * M_PI;
			fret=z;
	 		fret *=ANGLE_CONVERSION;
			targ = T_NBR;
			return;
		}
	} else if(toupper(*ep)=='B') {
		tp = checkstring(ep,  "BASE64");
		if(tp) {
			// Input and output are each a string variable, or an integer or float array of bytes.
			const char *q;
			const bool encode = (q = checkstring(tp, "ENCODE")) != NULL;
			if(!encode && !(q = checkstring(tp, "DECODE"))) ERROR_SYNTAX;
			MMFLOAT *infloat=NULL, *outfloat=NULL;
			int64_t *inint=NULL, *outint=NULL;
			char *instr=NULL, *outstr=NULL;
			getargs(&q, 3, DELIM_COMMA);
			if(argc != 3) ERROR_SYNTAX;
			int length = 0;
			const int card = parseany(argv[0], &infloat, &inint, &instr, &length, false);
			length = 0;
			const int card2 = parseany(argv[2], &outfloat, &outint, &outstr, &length, false);
			unsigned char *in = GetTempMemory(card + 1);
			for(int i = 0; i < card; i++) {
				if(inint) {
					if(inint[i] < 0 || inint[i] > 255) error_throw_legacy("Key number out of bounds 0-255");
					in[i] = inint[i];
				} else if(infloat) {
					if(infloat[i] < 0 || infloat[i] > 255) error_throw_legacy("Key number out of bounds 0-255");
					in[i] = infloat[i];
				} else {
					in[i] = instr[i + 1];
				}
			}
			const unsigned need = encode ? b64e_size(card) : b64d_size(card);
			if(!outstr && card2 < (int) need) error_throw_legacy("Output array too small");
			if(outstr && need > 255) error_throw_legacy("Output exceeds string size");
			unsigned char *out = GetTempMemory(b64e_size(card + 1) + 4);
			const int size = encode ? b64_encode(in, card, out) : b64_decode(in, card, out);
			for(int i = 0; i < size; i++) {
				if(outint) outint[i] = out[i];
				else if(outfloat) outfloat[i] = out[i];
			}
			if(outstr) {
				memcpy(&outstr[1], out, size);
				outstr[0] = size;
			}
			iret = size;
			targ = T_INT;
			return;
		}
	} else if(toupper(*ep)=='P') {
		tp = checkstring(ep,  "PID");
		if(tp) {
			// MATH(PID channel, setpoint, measurement): the controller's output.
			getargs(&tp, 5, DELIM_COMMA);
			if(argc != 5) ERROR_SYNTAX;
			const int channel = getint(argv[0], 1, MAX_PID_CHANNELS);
			const MMFLOAT setpoint = getnumber(argv[2]);
			const MMFLOAT measurement = getnumber(argv[4]);
			if(!pid_channels[channel]) error_throw_legacy("Channel not configured");
			if(!interrupt_pid_active(channel)) error_throw_legacy("Channel not active");
			fret = pid_update(pid_channels[channel], setpoint, measurement);
			targ = T_NBR;
			return;
		}
	} else if(toupper(*ep)=='C') {
		if(ep[1]=='_'){
			if((tp=checkstring(&ep[2], "REAL"))){
				fret=(MMFLOAT)crealf(getComplex(tp));
				targ=T_NBR;
			} else if((tp=checkstring(&ep[2], "IMAG"))){
				fret=(MMFLOAT)cimagf(getComplex(tp));
				targ=T_NBR;
			} else if((tp=checkstring(&ep[2], "MOD"))){
				MMFLOAT a=(MMFLOAT)crealf(getComplex(tp));
				MMFLOAT b=(MMFLOAT)cimagf(getComplex(tp));
				a*=a;
				b*=b;
				b+=a;
				fret=sqrt(b);
				targ=T_NBR;
			} else if((tp=checkstring(&ep[2], "PHASE"))){
				MMFLOAT a=(MMFLOAT)crealf(getComplex(tp));
				MMFLOAT b=(MMFLOAT)cimagf(getComplex(tp));
				fret=atan2(b,a)*ANGLE_CONVERSION;
				targ=T_NBR;
			} else if((tp=checkstring(&ep[2], "CARG"))){
				fret=(MMFLOAT)cargf(getComplex(tp));
				targ=T_NBR;
			} else if((tp=checkstring(&ep[2], "ADD"))){
				getargs(&tp,3, DELIM_COMMA);
				fcplx x = caddf(getComplex(argv[0]), getComplex(argv[2]));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "MUL"))){
				getargs(&tp,3, DELIM_COMMA);
				fcplx x = cmulf(getComplex(argv[0]), getComplex(argv[2]));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "SUB"))){
				getargs(&tp,3, DELIM_COMMA);
				fcplx x = csubf(getComplex(argv[0]), getComplex(argv[2]));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "DIV"))){
				getargs(&tp,3, DELIM_COMMA);
				fcplx x = cdivf(getComplex(argv[0]), getComplex(argv[2]));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "POW"))){
				getargs(&tp,3, DELIM_COMMA);
				fcplx x=cpowf(getComplex(argv[0]),getComplex(argv[2]));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "CONJ"))){
				fcplx x=conjf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "ACOS"))){
				fcplx x=cacosf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "ASIN"))){
				fcplx x=casinf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "ATAN"))){
				fcplx x=catanf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "SIN"))){
				fcplx x=csinf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "COS"))){
				fcplx x=ccosf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "TAN"))){
				fcplx x=ctanf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "SINH"))){
				fcplx x=csinhf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "COSH"))){
				fcplx x=ccoshf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "TANH"))){
				fcplx x=ctanhf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "ASINH"))){
				fcplx x=casinhf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "ACOSH"))){
				fcplx x=cacoshf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "ATANH"))){
				fcplx x=catanhf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "EXP"))){
				fcplx x=cexpf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "LOG"))){
				fcplx x=clogf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "ABS"))){
				float mag = cabsf(getComplex(tp));
				fcplx x = FCOMPLEX(mag, 0.0f);
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "SQRT"))){
				fcplx x=csqrtf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "PROJ"))){
				fcplx x=cprojf(getComplex(tp));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "CPLX"))){
				getargs(&tp,3, DELIM_COMMA);
				fcplx x = FCOMPLEX(getnumber(argv[0]), getnumber(argv[2]));
				retComplex(x);
			} else if((tp=checkstring(&ep[2], "POLAR"))){
				getargs(&tp,3, DELIM_COMMA);
				MMFLOAT r=getnumber(argv[0]);
				MMFLOAT theta=getnumber(argv[2])/ANGLE_CONVERSION;
				MMFLOAT stheta=sin(theta)*r;
				MMFLOAT ctheta=cos(theta)*r;
				fcplx x = FCOMPLEX(ctheta, stheta);
				retComplex(x);
			} else ERROR_SYNTAX;
			return;
		}
		tp = checkstring(ep,  "CRC8");
		if(tp) {
		    int i;
		    MMFLOAT *a1float=NULL;
		    int64_t *a1int=NULL;
			getargs(&tp,13, DELIM_COMMA);
			if(argc<1)ERROR_SYNTAX;
			uint8_t polynome=CRC8_DEFAULT_POLYNOME;
			uint8_t startmask=0;
			uint8_t endmask=0;
			uint8_t reverseIn=false;
			uint8_t reverseOut=false;
			char *a1str=NULL;
			int length=0;
			if(argc>1 && *argv[2])length=getint(argv[2],1,65535);
			length=parseany(argv[0], &a1float, &a1int, &a1str, &length, false);
			uint8_t *array=GetTempMemory(length);
			if(argc>3 && *argv[4])polynome=getint(argv[4],0,255);
			if(argc>5 && *argv[6])startmask=getint(argv[6],0,255);
			if(argc>7 && *argv[8])endmask=getint(argv[8],0,255);
			if(argc>9 && *argv[10])reverseIn=getint(argv[10],0,1);
			if(argc==13 && *argv[12])reverseOut=getint(argv[12],0,1);
			for(i=0;i<length;i++){
				if(a1float){
					if(a1float[i]>255)error_throw_legacy("Variable > 255");
					else array[i]=(uint8_t)a1float[i];
				} else if(a1int){
					if(a1int[i]>255)error_throw_legacy("Variable > 255");
					else array[i]=(uint8_t)a1int[i];
				} else 	memcpy(array,&a1str[1],length);
			}
			iret=crc8(array, length, polynome, startmask, endmask, reverseIn, reverseOut);
			targ=T_INT;
			return;
		}
		tp = checkstring(ep,  "CRC12");
		if(tp) {
		    int i;
		    MMFLOAT *a1float=NULL;
		    int64_t *a1int=NULL;
			getargs(&tp,13, DELIM_COMMA);
			if(argc<1)ERROR_SYNTAX;
			uint16_t polynome=CRC12_DEFAULT_POLYNOME;
			uint16_t startmask=0;
			uint16_t endmask=0;
			uint8_t reverseIn=false;
			uint8_t reverseOut=false;
			char *a1str=NULL;
			int length=0;
			if(argc>1 && *argv[2])length=getint(argv[2],1,65535);
			length=parseany(argv[0], &a1float, &a1int, &a1str, &length, false);
			uint8_t *array=GetTempMemory(length);
			if(argc>3 && *argv[4])polynome=getint(argv[4],0,4095);
			if(argc>5 && *argv[6])startmask=getint(argv[6],0,4095);
			if(argc>7 && *argv[8])endmask=getint(argv[8],0,4095);
			if(argc>9 && *argv[10])reverseIn=getint(argv[10],0,1);
			if(argc==13 && *argv[12])reverseOut=getint(argv[12],0,1);
			for(i=0;i<length;i++){
				if(a1float){
					if(a1float[i]>255)error_throw_legacy("Variable > 255");
					else array[i]=(uint8_t)a1float[i];
				} else if(a1int){
					if(a1int[i]>255)error_throw_legacy("Variable > 255");
					else array[i]=(uint8_t)a1int[i];
				} else 	memcpy(array,&a1str[1],length);
			}
			iret=crc12(array, length, polynome, startmask, endmask, reverseIn, reverseOut);
			targ=T_INT;
			return;
		}
		tp = checkstring(ep,  "CRC16");
		if(tp) {
		    int i;
		    MMFLOAT *a1float=NULL;
		    int64_t *a1int=NULL;
			getargs(&tp,13, DELIM_COMMA);
			if(argc<1)ERROR_SYNTAX;
			uint16_t polynome=CRC16_DEFAULT_POLYNOME;
			uint16_t startmask=0;
			uint16_t endmask=0;
			uint8_t reverseIn=false;
			uint8_t reverseOut=false;
			char *a1str=NULL;
			int length=0;
			if(argc>1 && *argv[2])length=getint(argv[2],1,65535);
			length=parseany(argv[0], &a1float, &a1int, &a1str, &length, false);
			uint8_t *array=GetTempMemory(length);
			if(argc>3 && *argv[4])polynome=getint(argv[4],0,65535);
			if(argc>5 && *argv[6])startmask=getint(argv[6],0,65535);
			if(argc>7 && *argv[8])endmask=getint(argv[8],0,65535);
			if(argc>9 && *argv[10])reverseIn=getint(argv[10],0,1);
			if(argc==13 && *argv[12])reverseOut=getint(argv[12],0,1);
			for(i=0;i<length;i++){
				if(a1float){
					if(a1float[i]>255)error_throw_legacy("Variable > 255");
					else array[i]=(uint8_t)a1float[i];
				} else if(a1int){
					if(a1int[i]>255)error_throw_legacy("Variable > 255");
					else array[i]=(uint8_t)a1int[i];
				} else 	memcpy(array,&a1str[1],length);
			}
			iret=crc16(array, length, polynome, startmask, endmask, reverseIn, reverseOut);
			targ=T_INT;
			return;
		}
		tp = checkstring(ep,  "CRC32");
		if(tp) {
		    int i;
		    MMFLOAT *a1float=NULL;
		    int64_t *a1int=NULL;
			getargs(&tp,13, DELIM_COMMA);
			if(argc<1)ERROR_SYNTAX;
			uint32_t polynome=CRC32_DEFAULT_POLYNOME;
			uint32_t startmask=0;
			uint32_t endmask=0;
			uint8_t reverseIn=false;
			uint8_t reverseOut=false;
			char *a1str=NULL;
			int length=0;
			if(argc>1 && *argv[2])length=getint(argv[2],1,65535);
			length=parseany(argv[0], &a1float, &a1int, &a1str, &length, false);
			uint8_t *array=GetTempMemory(length);
			if(argc>3 && *argv[4])polynome=getint(argv[4],0,0xFFFFFFFF);
			if(argc>5 && *argv[6])startmask=getint(argv[6],0,0xFFFFFFFF);
			if(argc>7 && *argv[8])endmask=getint(argv[8],0,0xFFFFFFFF);
			if(argc>9 && *argv[10])reverseIn=getint(argv[10],0,1);
			if(argc==13 && *argv[12])reverseOut=getint(argv[12],0,1);
			for(i=0;i<length;i++){
				if(a1float){
					if(a1float[i]>255)error_throw_legacy("Variable > 255");
					else array[i]=(uint8_t)a1float[i];
				} else if(a1int){
					if(a1int[i]>255)error_throw_legacy("Variable > 255");
					else array[i]=(uint8_t)a1int[i];
				} else 	memcpy(array,&a1str[1],length);
			}
			iret=crc32(array, length, polynome, startmask, endmask, reverseIn, reverseOut);
			targ=T_INT;
			return;
		}
		tp = checkstring(ep,  "COSH");
		if(tp) {
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			fret=cosh(getnumber(argv[0]));
			targ=T_NBR;
			return;
		}
		tp = checkstring(ep,  "CROSSING");
		if(tp){
		    MMFLOAT *a1float=NULL;
		    int64_t *a1int=NULL;
			int arraylength=0;
			MMFLOAT crossing=0.0;
			int direction=1;
			int found=-1;
			getargs(&tp,5, DELIM_COMMA);
			if(argc<1)ERROR_SYNTAX;
			if(argc>=3 && *argv[2])crossing = getnumber(argv[2]);
			if(argc==5) direction=getint(argv[4],-1,1);
			if(direction==0)error_throw_legacy("Valid are -1 and 1");
			arraylength=parsenumberarray(argv[0],&a1float,&a1int,1,1,dims, false);
			for(int i=0;i<arraylength-3;i++){
				if(a1float){
					if(a1float[i]<crossing && a1float[i+2]>crossing && (a1float[i+1]>=a1float[i] && a1float[i+1]<=a1float[i+2]) && direction==1){
						found=i+1;
						break;
					}
					if(a1float[i]>crossing && a1float[i+2]<crossing && (a1float[i+1]<=a1float[i] && a1float[i+1]>=a1float[i+2]) && direction==-1){
						found=i+1;
						break;
					}
				} else {
					if(a1int[i]<crossing && a1int[i+2]>crossing && (a1int[i+1]>=a1int[i] && a1int[i+1]<=a1int[i+2]) && direction==1){
						found=i+1;
						break;
					}
					if(a1int[i]>crossing && a1int[i+2]<crossing && (a1int[i+1]<=a1int[i] && a1int[i+1]>=a1int[i+2]) && direction==-1){
						found=i+1;
						break;
					}
				}
			}
			if(found==-1){ //try a slower moving slope
				for(int i=0;i<arraylength-5;i++){
					if(a1float){
						if(a1float[i+1]<=crossing && a1float[i+3]>=crossing && (a1float[i+2]>=a1float[i+1] && a1float[i+2]<=a1float[i+3]) && direction==1){
							if(a1float[i]<a1float[i+2] && a1float[i+4]>a1float[i+2]){
								found=i+2;
								break;
							}
						}
						if(a1float[i+1]>=crossing && a1float[i+3]<=crossing && (a1float[i+2]<=a1float[i+1] && a1float[i+2]>=a1float[i+3]) && direction==-1){
							if(a1float[i]>a1float[i+2] && a1float[i+4]<a1float[i+2]){
								found=i+2;
								break;
							}
						}
					} else {
						if(a1int[i+1]<=crossing && a1int[i+3]>=crossing && (a1int[i+2]>=a1int[i+1] && a1int[i+2]<=a1int[i+3]) && direction==1){
							if(a1int[i]<a1int[i+2] && a1int[i+4]>a1int[i+2]){
								found=i+2;
								break;
							}
						}
						if(a1int[i+1]>=crossing && a1int[i+3]<=crossing && (a1int[i+2]<=a1int[i+1] && a1int[i+2]>=a1int[i+3]) && direction==-1){
							if(a1int[i]>a1int[i+2] && a1int[i+4]<a1int[i+2]){
								found=i+2;
								break;
							}
						}
					}
				}
			}
			targ=T_INT;
			iret=found;
			return;
		}
		tp = checkstring(ep,  "CORREL");
		if(tp) {
		    int i,card1=1, card2=1;
		    MMFLOAT *a1float=NULL, *a2float=NULL, mean1=0, mean2=0;
		    MMFLOAT *a3float=NULL, *a4float=NULL;
		    MMFLOAT axb=0, a2=0, b2=0;
		    int64_t *a1int=NULL, *a2int=NULL;
		    getargs(&tp, 3, DELIM_COMMA);
		    if(!(argc == 3)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0],&a1float,&a1int,1,0,dims, false);
			card2=parsenumberarray(argv[2],&a2float,&a2int,2,0,dims, false);
			if(card1!=card2)error_throw_legacy("Array size mismatch");
			a3float=GetTempMemory(card1*sizeof(MMFLOAT));
			a4float=GetTempMemory(card1*sizeof(MMFLOAT));
			if(a1float!=NULL){
				for(i=0; i< card1;i++)a3float[i] = (*a1float++);
			} else {
				for(i=0; i< card1;i++)a3float[i] = (MMFLOAT)(*a1int++);
			}
			if(a2float!=NULL){
				for(i=0; i< card1;i++)a4float[i] = (*a2float++);
			} else {
				for(i=0; i< card1;i++)a4float[i] = (MMFLOAT)(*a2int++);
			}
			for(i=0;i<card1;i++){
				mean1+=a3float[i];
				mean2+=a4float[i];
			}
			mean1/=card1;
			mean2/=card1;
			for(i=0;i<card1;i++){
				a3float[i]-=mean1;
				a2+=(a3float[i]*a3float[i]);
				a4float[i]-=mean2;
				b2+=(a4float[i]*a4float[i]);
				axb+=(a3float[i]*a4float[i]);
			}
			targ=T_NBR;
			fret=axb/sqrt(a2*b2);
			return;
		}
	tp = (checkstring(ep,  "CHI_P"));
	tp1 = (checkstring(ep,  "CHI"));
	if(tp || tp1) {
			int chi_p=1;
			if(tp1){
				tp=tp1;
				chi_p=0;
			}
			int i,j, df, numcols=0, numrows=0;
			MMFLOAT *a1float=NULL,*rows=NULL, *cols=NULL, chi=0, prob, chi_prob;
			MMFLOAT total=0.0;
			int64_t *a1int=NULL;
			{
				getargs(&tp, 1, DELIM_COMMA);
				if(!(argc == 1)) error_throw_legacy("Argument count");
				parsenumberarray(argv[0],&a1float,&a1int,1,2,dims, false);
				numcols=dims[0]+1-mmb_options.base;
				numrows=dims[1]+1-mmb_options.base;
				df=(numcols-1)*(numrows-1);
				if(df<1)error_throw_legacy("Needs at least 2 rows and 2 columns");
				if(df>50)error_throw_legacy("Too many degrees of freedom (max 50)");
				MMFLOAT **observed=alloc2df(numcols,numrows);
				MMFLOAT **expected=alloc2df(numcols,numrows);
				rows=alloc1df(numrows);
				cols=alloc1df(numcols);
				if(a1float!=NULL){
					for(i=0;i<numrows;i++){
						for(j=0;j<numcols;j++){
							observed[j][i]=*a1float++;
							total+=observed[j][i];
							rows[i]+=observed[j][i];
						}
					}
				} else {
					for(i=0;i<numrows;i++){
						for(j=0;j<numcols;j++){
							observed[j][i]=(MMFLOAT)(*a1int++);
							total+=observed[j][i];
							rows[i]+=observed[j][i];
						}
					}
				}
				for(j=0;j<numcols;j++){
					for(i=0;i<numrows;i++){
						cols[j]+=observed[j][i];
					}
				}
				for(i=0;i<numrows;i++){
					for(j=0;j<numcols;j++){
						expected[j][i]=cols[j]*rows[i]/total;
						expected[j][i]=((observed[j][i]-expected[j][i]) * (observed[j][i]-expected[j][i]) / expected[j][i]);
						chi+=expected[j][i];
					}
				}
				prob=chitable[df][7];
				if(chi>prob){
					i=7;
					while(i<15 && chi>=chitable[df][i])i++;
					chi_prob=chitable[0][i-1];
				} else {
					i=7;
					while(i>=0 && chi<=chitable[df][i])i--;
					chi_prob=chitable[0][i+1];
				}
				dealloc2df(observed,numcols,numrows);
				dealloc2df(expected,numcols,numrows);
				FreeMemory(rows);
				FreeMemory(cols);
				// FreeMemorySafe((void **)&rows);
				// FreeMemorySafe((void **)&cols);
				targ=T_NBR;
				fret=(chi_p ? chi_prob*100 : chi);
				return;
			}
		}

	} else if(toupper(*ep)=='D') {

		tp = checkstring(ep,  "DOTPRODUCT");
		if(tp) {
			int i;
			int card1,card2;
			MMFLOAT *a1float=NULL, *a2float=NULL;
			// need two arrays with same cardinality
			getargs(&tp, 3, DELIM_COMMA);
			if(!(argc == 3)) error_throw_legacy("Argument count");
			card1=parsefloatrarray(argv[0],&a1float,1,1,dims, false);
			card2=parsefloatrarray(argv[2],&a2float,2,1,dims, false);
			if(card1!=card2)error_throw_legacy("Array size mismatch");
			fret=0;
			for(i=0;i<card1;i++){
				fret = fret + ((*a1float++) * (*a2float++));
			}
			targ = T_NBR;
			return;
		}
	} else if(toupper(*ep)=='L') {
		tp = checkstring(ep,  "LOG10");
		if(tp) {
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			fret=log10(getnumber(argv[0]));
			targ=T_NBR;
			return;
		}

	} else if(toupper(*ep)=='M') {
		tp = checkstring(ep,  "M_DETERMINANT");
		if(tp){
			int i, j, n, numcols=0, numrows=0;
			MMFLOAT *a1float=NULL;
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			parsefloatrarray(argv[0],&a1float,1,2,dims, false);
			numcols=dims[0]+1-mmb_options.base;
			numrows=dims[1]+1-mmb_options.base;
			if(numcols!=numrows)error_throw_legacy("Array must be square");
			n=numrows;
			MMFLOAT **matrix=alloc2df(n,n);
			for(i=0;i<n;i++){ //load the matrix
				for(j=0;j<n;j++){
					matrix[j][i]=*a1float++;
				}
			}
			fret=determinant(matrix,n);
			dealloc2df(matrix,numcols,numrows);
			targ=T_NBR;

			return;
		}

		tp = checkstring(ep,  "MAX");
		if(tp) {
			int i,card1=1;
			MMFLOAT *a1float=NULL, max=-3.0e+38;
			int64_t *a1int=NULL;
			long long int *temp=NULL;
			getargs(&tp, 3, DELIM_COMMA);
//			if(!(argc == 1)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0],&a1float,&a1int,1,0,dims, false);
			if(argc==3){
				if(dims[1] > 0) {		// Not an array
					error_throw_legacy("Argument 1 must be a 1D numerical array");
				}
				temp = findvar(argv[2], V_FIND);
				if(!(vartbl[VarIndex].type & T_INT)) error_throw_legacy("Invalid variable");
			}

			if(a1float!=NULL){
				for(i=0; i< card1;i++){
					if((*a1float)>max){
						max=(*a1float);
						if(temp!=NULL){
							*temp=i+mmb_options.base;
						}
					}
					a1float++;
				}
			} else {
				for(i=0; i< card1;i++){
					if(((MMFLOAT)(*a1int))>max){
						max=(MMFLOAT)(*a1int);
						if(temp!=NULL){
							*temp=i+mmb_options.base;
						}
					}
					a1int++;
				}
			}
			targ=T_NBR;
			fret=max;
			return;
		}
		tp = checkstring(ep,  "MIN");
		if(tp) {
			int i,card1=1;
			MMFLOAT *a1float=NULL, min=3.0e+38;
			int64_t *a1int=NULL;
			long long int *temp=NULL;
			getargs(&tp, 3, DELIM_COMMA);
//			if(!(argc == 1)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0],&a1float,&a1int,1,0,dims, false);
			if(argc==3){
				if(dims[1] > 0) {		// Not an array
					error_throw_legacy("Argument 1 must be a 1D numerical array");
				}
				temp = findvar(argv[2], V_FIND);
				if(!(vartbl[VarIndex].type & T_INT)) error_throw_legacy("Invalid variable");
			}
			if(a1float!=NULL){
				for(i=0; i< card1;i++){
					if((*a1float)<min){
						min=(*a1float);
						if(temp!=NULL){
							*temp=i+mmb_options.base;
						}
					}
					a1float++;
				}
			} else {
				for(i=0; i< card1;i++){
					if(((MMFLOAT)(*a1int))<min){
						min=(MMFLOAT)(*a1int);
						if(temp!=NULL){
							*temp=i+mmb_options.base;
						}
					}
					a1int++;
				}
			}
			targ=T_NBR;
			fret=min;
			return;
		}
		tp = checkstring(ep,  "MAGNITUDE");
		if(tp) {
			int i;
			int numcols=0;
			MMFLOAT *a1float=NULL;
			MMFLOAT mag=0.0;
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			numcols=parsefloatrarray(argv[0],&a1float,1,0,dims, false);
			for(i=0;i<numcols;i++){
				mag = mag + ((*a1float) * (*a1float));
				a1float++;
			}
			fret=sqrt(mag);
			targ = T_NBR;
			return;
		}

		tp = checkstring(ep,  "MEAN");
		if(tp) {
			int i,card1=1;
			MMFLOAT *a1float=NULL, mean=0;
			int64_t *a1int=NULL;
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0],&a1float,&a1int,1,0,dims, false);
			if(a1float!=NULL){
				for(i=0; i< card1;i++)mean+= (*a1float++);
			} else {
				for(i=0; i< card1;i++)mean+= (MMFLOAT)(*a1int++);
			}
			targ=T_NBR;
			fret=mean/(MMFLOAT)card1;
			return;
		}

		tp = checkstring(ep,  "MEDIAN");
		if(tp) {
			int i,card1, card2=1;
			MMFLOAT *a1float=NULL, *a2float=NULL;
			int64_t *a2int=NULL;
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			card2=parsenumberarray(argv[0],&a2float,&a2int,1,0,dims,false);
			card1=card2;
			card2=(card2-1)/2;
			a1float=GetTempMemory(card1*sizeof(MMFLOAT));
			if(a2float!=NULL){
				for(i=0; i< card1;i++)a1float[i] = (*a2float++);
			} else {
				for(i=0; i< card1;i++)a1float[i] = (MMFLOAT)(*a2int++);
			}
			floatshellsort(a1float,  card1);
			targ=T_NBR;
			if(card1 & 1)fret=a1float[card2];
			else fret=(a1float[card2]+a1float[card2+1])/2.0;
			return;
		}
	} else if(toupper(*ep)=='S') {

		tp = checkstring(ep,  "SINH");
		if(tp) {
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			fret=sinh(getnumber(argv[0]));
			targ=T_NBR;
			return;
		}

		tp = checkstring(ep,  "SD");
		if(tp) {
			int i,card1=1;
			MMFLOAT *a2float=NULL, *a1float=NULL, mean=0, var=0, deviation;
			int64_t *a2int=NULL, *a1int=NULL;
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0],&a1float,&a1int,1,0,dims, false);
			if(a1float!=NULL){
				a2float=a1float;
				for(i=0; i< card1;i++)mean+= (*a2float++);
			} else {
				a2int=a1int;
				for(i=0; i< card1;i++)mean+= (MMFLOAT)(*a2int++);
			}
			mean=mean/(MMFLOAT)card1;
			if(a1float!=NULL){
				for(i=0; i< card1;i++){
					deviation = (*a1float++) - mean;
					var += deviation * deviation;
				}
			} else {
				for(i=0; i< card1;i++){
					deviation = (MMFLOAT)(*a1int++) - mean;
					var += deviation * deviation;
				}
			}
			targ=T_NBR;

            // Standard Deviation Formula
            //
            // There are two formulas for standard deviation: one for population standard deviation
            // and another for sample standard deviation.
            //
            // Population Standard Deviation Formula σ (sigma) = √[(Σ(X - μ)²) / N]
            //
            // where:
            //   σ (sigma) is the population standard deviation
            //   X is each data point
            //   μ (mu) is the population mean
            //   Σ (sigma) represents the sum of
            //   N is the population size
            //
            // Sample Standard Deviation Formula
            //
            // s = √[(Σ(X - x̄)²) / (n - 1)]
            //
            // where:
            //   s is the sample standard deviation
            //   X is each data point
            //   x̄ (x-bar) is the sample mean
            //   Σ (sigma) represents the sum of
            //   n is the sample size
            //
            // Note the difference between the two formulas: the population standard deviation
            // formula uses N (population size) in the denominator, while the sample standard
            // deviation formula uses n - 1 (degrees of freedom) in the denominator. This is because
            // the sample standard deviation is an estimate of the population standard deviation,
            // and the degrees of freedom account for the uncertainty in the estimate.

			// MMBasic uses the "Sample Standard Deviation Formula".
			fret=sqrt(var/(card1-1));
			return;
		}

		tp = checkstring(ep,  "SUM");
		if(tp) {
			int i,card1=1;
			MMFLOAT *a1float=NULL, sum=0;
			int64_t *a1int=NULL;
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			card1=parsenumberarray(argv[0],&a1float,&a1int,1,0,dims, false);
			if(a1float!=NULL){
				for(i=0; i< card1;i++)sum+= (*a1float++);
			} else {
				for(i=0; i< card1;i++)sum+= (MMFLOAT)(*a1int++);
			}
			targ=T_NBR;
			fret=sum;
			return;
		}
	} else if(toupper(*ep)=='U') {
		tp = checkstring(ep,  "UPSAMPLE");
		if(tp) {
			// MATH(UPSAMPLE in(), out(), infreq, outfreq): the number of samples written, as in MMBasic for Windows.
			MMFLOAT *q1=NULL, *v1=NULL;
			getargs(&tp, 7, DELIM_COMMA);
			if(argc != 7) error_throw_legacy("Argument count");
			const int insize = parsefloatrarray(argv[0], &q1, 1, 1, NULL, false);
			const int outsize = parsefloatrarray(argv[2], &v1, 2, 1, NULL, true);
			if(insize < 7) error_throw_legacy("Input array too small");
			const MMFLOAT infreq = getnumber(argv[4]);
			if(infreq <= 0) error_throw_legacy("Invalid input frequency");
			const MMFLOAT outfreq = getnumber(argv[6]);
			if(outfreq < infreq) error_throw_legacy("Invalid output frequency");
			MMFLOAT *buf = GetTempMemory((int) (outfreq / infreq * (MMFLOAT) insize + 10) * sizeof(MMFLOAT));
			const int got = upsample(q1, buf, insize - 1, outfreq, infreq);
			if(got > outsize) error_throw_legacy("Output array size");
			memcpy(v1, buf, got * sizeof(MMFLOAT));
			fret = got;
			targ = T_NBR;
			return;
		}
	} else if(toupper(*ep)=='T') {

		tp = checkstring(ep,  "TANH");
		if(tp) {
			getargs(&tp, 1, DELIM_COMMA);
			if(!(argc == 1)) error_throw_legacy("Argument count");
			fret=tanh(getnumber(argv[0]));
			targ=T_NBR;
			return;
		}
	} else if(toupper(*ep)=='R') {
		tp = checkstring(ep,  "RAND");
		if(tp) {
			if(g_myrand==NULL){
				g_myrand=(struct tagMTRand *)GetMemory(sizeof(struct tagMTRand));
				seedRand(mmtime_now_ns() / 1000);
			}
			fret=genRand(g_myrand);
			targ = T_NBR;
			return;
		}
	}
	ERROR_SYNTAX;
}

static size_t reverse_bits(size_t val, int width) {
	size_t result = 0;
	for (int i = 0; i < width; i++, val >>= 1)
		result = (result << 1) | (val & 1U);
	return result;
}

bool Fft_transformRadix2(cplx vec[], size_t n, bool inverse) {
	// Length variables
	int levels = 0;  // Compute levels = floor(log2(n))
	for (size_t temp = n; temp > 1U; temp >>= 1)
		levels++;
	if ((size_t)1U << levels != n)
		return false;  // n is not a power of 2

	// Trigonometric tables
	if (SIZE_MAX / sizeof(cplx) < n / 2)
		return false;
	cplx *exptable = GetMemory((n / 2) * sizeof(cplx));
	if (exptable == NULL)
		return false;
	for (size_t i = 0; i < n / 2; i++) {
		double angle = (inverse ? 2 : -2) * M_PI * i / n;
		exptable[i] = cexp(DCOMPLEX(0.0, angle));
	}

	// Bit-reversed addressing permutation
	for (size_t i = 0; i < n; i++) {
		size_t j = reverse_bits(i, levels);
		if (j > i) {
			cplx temp = vec[i];
			vec[i] = vec[j];
			vec[j] = temp;
		}
	}

	// Cooley-Tukey decimation-in-time radix-2 FFT
	for (size_t size = 2; size <= n; size *= 2) {
		size_t halfsize = size / 2;
		size_t tablestep = n / size;
		for (size_t i = 0; i < n; i += size) {
			for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
				size_t l = j + halfsize;
				cplx temp = cmul(vec[l], exptable[k]);
				vec[l] = csub(vec[j], temp);
				vec[j] = cadd(vec[j], temp);
			}
		}
		if (size == n)  // Prevent overflow in 'size *= 2'
			break;
	}

	FreeMemory((void *)exptable);
	return true;
}


void cmd_FFT(const char *pp){
    const char *tp;
	PI = atan2(1, 1) * 4;
	DIMTYPE dims[MAXDIM]={0};
    cplx *a1cplx=NULL, *a2cplx=NULL;
    MMFLOAT *a3float=NULL, *a4float=NULL, *a5float;
    int i, card1,card2, powerof2=0;
	tp = checkstring(pp,  "MAGNITUDE");
	if(tp) {
		getargs(&tp,3, DELIM_COMMA);
		card1=parsefloatrarray(argv[0],&a3float,1,1,dims, false);
		card2=parsefloatrarray(argv[2],&a4float,2,1,dims, true);
	    if(card1 !=card2)error_throw_legacy("Array size mismatch");
	    for(i=1;i<65536;i*=2){
	    	if(card1==i)powerof2=1;
	    }
	    if(!powerof2)error_throw_legacy("array size must be a power of 2");
        a1cplx=(cplx *)GetTempMemory((card1)*16);
        a5float=(MMFLOAT *)a1cplx;
        for(i=0;i<card1;i++){a5float[i*2]=a3float[i];a5float[i*2+1]=0;}
        Fft_transformRadix2(a1cplx, card1, 0);
//	    fft((MMFLOAT *)a1cplx,size+1);
	    for(i=0;i<card1;i++)a4float[i]=cabs(a1cplx[i]);
		return;
	}
	tp = checkstring(pp,  "PHASE");
	if(tp) {
		getargs(&tp,3, DELIM_COMMA);
		card1=parsefloatrarray(argv[0],&a3float,1,1,dims, false);
		card2=parsefloatrarray(argv[2],&a4float,2,1,dims, true);
	    if(card1 !=card2)error_throw_legacy("Array size mismatch");
	    for(i=1;i<65536;i*=2){
	    	if(card1==i)powerof2=1;
	    }
	    if(!powerof2)error_throw_legacy("array size must be a power of 2");
        a1cplx=(cplx *)GetTempMemory((card1)*16);
        a5float=(MMFLOAT *)a1cplx;
        for(i=0;i<card1;i++){a5float[i*2]=a3float[i];a5float[i*2+1]=0;}
        Fft_transformRadix2(a1cplx, card1, 0);
//	    fft((MMFLOAT *)a1cplx,size+1);
	    for(i=0;i<card1;i++)a4float[i]=carg(a1cplx[i]);
		return;
	}
	tp = checkstring(pp,  "INVERSE");
	if(tp) {
		getargs(&tp,3, DELIM_COMMA);
		card1=parsefloatrarray(argv[0],&a4float,1,2,dims, false);
		int size=dims[1] - mmb_options.base +1;
		a1cplx=(cplx *)a4float;
		card2=parsefloatrarray(argv[2],&a3float,2,1,dims, true);
		if(card2 !=size)error_throw_legacy("Array size mismatch");
		for(i=1;i<65536;i*=2){
			if(card2==i)powerof2=1;
		}
		if(!powerof2)error_throw_legacy("array size must be a power of 2");
		a2cplx=(cplx *)GetTempMemory((card2)*16);
		memcpy(a2cplx,a1cplx,card2*16);
		for(i=0;i<card2;i++)a2cplx[i]=conj(a2cplx[i]);
		Fft_transformRadix2(a2cplx, card2, 0);
		for(i=0;i<card2;i++)a2cplx[i]=cdivr(conj(a2cplx[i]), (double)card2);
		for(i=0;i<card2;i++)a3float[i]=creal(a2cplx[i]);
		return;
	}
	getargs(&pp,3, DELIM_COMMA);
	card1=parsefloatrarray(argv[0],&a3float,1,1,dims, false);
	card2=parsefloatrarray(argv[2],&a4float,2,2,dims, true);
    a2cplx = (cplx *)a4float;
    if((dims[1] - mmb_options.base + 1) !=card1)error_throw_legacy("Array size mismatch");
    for(i=1;i<65536;i*=2){
    	if(card1==i)powerof2=1;
    }
    if(!powerof2)error_throw_legacy("array size must be a power of 2");
    for(i=0;i<card1;i++){a4float[i*2]=a3float[i];a4float[i*2+1]=0;}
//    fft((MMFLOAT *)a2cplx,size+1);
    Fft_transformRadix2(a2cplx, card1, 0);
}
// The time since the last MATH SENSORFUSION, at most one second; on the
// Colour Maximite 2 this was its AHRSTimer.
static MMFLOAT sensorfusion_seconds(void) {
    static int64_t last_ns = 0;
    const int64_t now = mmtime_now_ns();
    MMFLOAT t = (MMFLOAT) (now - last_ns) / 1e9;
    last_ns = now;
    return t > 1.0 ? 1.0 : t;
}

void cmd_SensorFusion(char *passcmdline){
    char *p;
    if((p = checkstring( passcmdline,  "MADGWICK")) != NULL) {
    getargs(&p, 25, DELIM_COMMA);
    if(argc < 23) error_throw_legacy("Incorrect number of parameters");
        MMFLOAT t;
        MMFLOAT *pitch, *yaw, *roll;
        MMFLOAT ax; MMFLOAT ay; MMFLOAT az; MMFLOAT gx; MMFLOAT gy; MMFLOAT gz; MMFLOAT mx; MMFLOAT my; MMFLOAT mz; MMFLOAT beta;
        ax=getnumber(argv[0]);
        ay=getnumber(argv[2]);
        az=getnumber(argv[4]);
        gx=getnumber(argv[6]);
        gy=getnumber(argv[8]);
        gz=getnumber(argv[10]);
        mx=getnumber(argv[12]);
        my=getnumber(argv[14]);
        mz=getnumber(argv[16]);
        pitch = findvar(argv[18], V_FIND);
        if(!(vartbl[VarIndex].type & T_NBR)) error_throw_legacy("Invalid variable");
        roll = findvar(argv[20], V_FIND);
        if(!(vartbl[VarIndex].type & T_NBR)) error_throw_legacy("Invalid variable");
        yaw = findvar(argv[22], V_FIND);
        if(!(vartbl[VarIndex].type & T_NBR)) error_throw_legacy("Invalid variable");
        beta = 0.5;
        if(argc == 25) beta=getnumber(argv[24]);
        t=sensorfusion_seconds();
        MadgwickQuaternionUpdate(ax, ay, az, gx, gy, gz, mx, my, mz, beta, t, pitch, yaw, roll);
        return;
    }
    if((p = checkstring( passcmdline,  "MAHONY")) != NULL) {
    getargs(&p, 27, DELIM_COMMA);
    if(argc < 23) error_throw_legacy("Incorrect number of parameters");
        MMFLOAT t;
        MMFLOAT *pitch, *yaw, *roll;
        MMFLOAT Kp, Ki;
        MMFLOAT ax; MMFLOAT ay; MMFLOAT az; MMFLOAT gx; MMFLOAT gy; MMFLOAT gz; MMFLOAT mx; MMFLOAT my; MMFLOAT mz;
        ax=getnumber(argv[0]);
        ay=getnumber(argv[2]);
        az=getnumber(argv[4]);
        gx=getnumber(argv[6]);
        gy=getnumber(argv[8]);
        gz=getnumber(argv[10]);
        mx=getnumber(argv[12]);
        my=getnumber(argv[14]);
        mz=getnumber(argv[16]);
        pitch = findvar(argv[18], V_FIND);
        if(!(vartbl[VarIndex].type & T_NBR)) error_throw_legacy("Invalid variable");
        roll = findvar(argv[20], V_FIND);
        if(!(vartbl[VarIndex].type & T_NBR)) error_throw_legacy("Invalid variable");
        yaw = findvar(argv[22], V_FIND);
        if(!(vartbl[VarIndex].type & T_NBR)) error_throw_legacy("Invalid variable");
        Kp=10.0 ; Ki=0.0;
        if(argc >= 25)Kp=getnumber(argv[24]);
        if(argc == 27)Ki=getnumber(argv[26]);
        t=sensorfusion_seconds();
        MahonyQuaternionUpdate(ax, ay, az, gx, gy, gz, mx, my, mz, Ki, Kp, t, yaw, pitch, roll) ;
        return;
    }
    error_throw_legacy("Invalid command");
}

/*Finding transpose of cofactor of matrix*/
void transpose(MMFLOAT **matrix,MMFLOAT **matrix_cofactor,MMFLOAT **newmatrix, int size)
{
    int i,j;
    MMFLOAT d;
	MMFLOAT **m_transpose=alloc2df(size,size);
	MMFLOAT **m_inverse=alloc2df(size,size);

    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            m_transpose[i][j]=matrix_cofactor[j][i];
        }
    }
    d=determinant(matrix,size);
    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            m_inverse[i][j]=m_transpose[i][j] / d;
        }
    }

    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            newmatrix[i][j]=m_inverse[i][j];
        }
    }
	dealloc2df(m_transpose,size,size);
	dealloc2df(m_inverse,size,size);
}
/*calculate cofactor of matrix*/
void cofactor(MMFLOAT **matrix,MMFLOAT **newmatrix,int size)
{
	MMFLOAT **m_cofactor=alloc2df(size,size);
	MMFLOAT **matrix_cofactor=alloc2df(size,size);
    int p,q,m,n,i,j;
    for (q=0;q<size;q++)
    {
        for (p=0;p<size;p++)
        {
            m=0;
            n=0;
            for (i=0;i<size;i++)
            {
                for (j=0;j<size;j++)
                {
                    if (i != q && j != p)
                    {
                       m_cofactor[m][n]=matrix[i][j];
                       if (n<(size-2))
                          n++;
                       else
                       {
                           n=0;
                           m++;
                       }
                    }
                }
            }
            matrix_cofactor[q][p]=pow(-1,q + p) * determinant(m_cofactor,size-1);
        }
    }
    transpose(matrix, matrix_cofactor, newmatrix, size);
	dealloc2df(m_cofactor,size,size);
	dealloc2df(matrix_cofactor,size,size);

}
/*For calculating Determinant of the Matrix . this function is recursive*/
MMFLOAT determinant(MMFLOAT **matrix,int size)
{
   MMFLOAT s=1,det=0;
   int i,j,m,n,c;
   if (size==0)
       return 1;
   if (size==1)
   {
       return (matrix[0][0]);
   }
   MMFLOAT **m_minor=alloc2df(size,size);
   {
       det=0;
       for (c=0;c<size;c++)
       {
           m=0;
           n=0;
           for (i=0;i<size;i++)
           {
               for (j=0;j<size;j++)
               {
                   m_minor[i][j]=0;
                   if (i != 0 && j != c)
                   {
                      m_minor[m][n]=matrix[i][j];
                      if (n<(size-2))
                         n++;
                      else
                      {
                          n=0;
                          m++;
                      }
                   }
               }
           }
           det=det + s * (matrix[0][c] * determinant(m_minor,size-1));
           s=-1 * s;
       }
   }
   dealloc2df(m_minor,size,size);
   return (det);
}

// ---- ARRAY SET, ADD, SLICE and INSERT, as on the PicoMite V6.04.00RC2: MATH's own
//      numeric forms, extended to string arrays ----

typedef struct {
    char *base;
    int type;         // T_INT, T_NBR or T_STR
    int elsize;       // bytes per element
    int ndims;
    int dim[MAXDIM];  // upper bound minus OPTION BASE, per dimension
    int count;
} ArrayRef;

static ArrayRef array_ref(const char *arg, int argno, bool disallow_const) {
    ArrayRef a = { 0 };
    a.base = findvar(arg, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (disallow_const && (vartbl[VarIndex].type & T_CONST)) ERROR_CANNOT_CHANGE_A_CONSTANT;
    if (vartbl[VarIndex].dims[0] <= 0 || a.base != vartbl[VarIndex].val.s) {
        error_throw_ex(kError, "Argument % must be an array", argno);
    }
    a.type = vartbl[VarIndex].type & (T_INT | T_NBR | T_STR);
    a.elsize = (a.type & T_STR) ? vartbl[VarIndex].size + 1 : 8;
    a.count = 1;
    for (int i = 0; i < MAXDIM && vartbl[VarIndex].dims[i] != 0; i++) {
        a.dim[i] = vartbl[VarIndex].dims[i] - mmb_options.base;
        a.count *= a.dim[i] + 1;
        a.ndims++;
    }
    return a;
}

/** Copies element si of src to element di of dst, converting between integer and float. */
static void array_copy_element(const ArrayRef *src, int si, const ArrayRef *dst, int di) {
    char *from = src->base + (size_t) si * src->elsize;
    char *to = dst->base + (size_t) di * dst->elsize;
    if ((src->type & T_STR) != (dst->type & T_STR)) error_throw_ex(kError, "Arrays must be the same type");
    if (src->type & T_STR) {
        if ((unsigned char) *from >= dst->elsize) ERROR_STRING_TOO_LONG;
        memcpy(to, from, (unsigned char) *from + 1);
    } else if (src->type == dst->type) {
        memcpy(to, from, 8);
    } else if (dst->type & T_NBR) {
        *(MMFLOAT *) to = (MMFLOAT) *(int64_t *) from;
    } else {
        *(int64_t *) to = FloatToInt64(*(MMFLOAT *) from);
    }
}

/** Numeric ARRAY forms are MATH's: run MATH with the same arguments. */
static void array_as_math(const char *sub) {
    static char buf[STRINGSIZE * 2];
    snprintf(buf, sizeof(buf), "%s %s", sub, cmdline);
    cmdline = buf;
    cmd_math();
}

/** ARRAY SET value, array() */
void cmd_array_set(void) {
    const char *args = cmdline;
    getargs(&cmdline, 3, DELIM_COMMA);
    if (argc != 3) ERROR_ARGUMENT_COUNT;
    findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (!(vartbl[VarIndex].type & T_STR)) {
        cmdline = args;
        array_as_math("SET");
        return;
    }
    ArrayRef a = array_ref(argv[2], 2, true);
    const char *s = getstring(argv[0]);
    if ((unsigned char) *s >= a.elsize) ERROR_STRING_TOO_LONG;
    memset(a.base, 0, (size_t) a.count * a.elsize);
    for (int i = 0; i < a.count; i++) memcpy(a.base + (size_t) i * a.elsize, s, (unsigned char) *s + 1);
}

/** ARRAY ADD in(), value, out(): adds value to each number, or appends it to each string. */
void cmd_array_add(void) {
    const char *args = cmdline;
    getargs(&cmdline, 5, DELIM_COMMA);
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (!(vartbl[VarIndex].type & T_STR)) {
        cmdline = args;
        array_as_math("ADD");
        return;
    }
    ArrayRef in = array_ref(argv[0], 1, false);
    ArrayRef out = array_ref(argv[4], 3, true);
    if (!(out.type & T_STR)) error_throw_ex(kError, "Arrays must be the same type");
    if (in.count != out.count) error_throw_ex(kError, "Array size mismatch");
    const char *s = getstring(argv[2]);
    char tmp[STRINGSIZE];
    for (int i = 0; i < in.count; i++) {
        const char *from = in.base + (size_t) i * in.elsize;
        const int len = (unsigned char) *from + (unsigned char) *s;
        if (len >= out.elsize) ERROR_STRING_TOO_LONG;
        memcpy(tmp + 1, from + 1, (unsigned char) *from);
        memcpy(tmp + 1 + (unsigned char) *from, s + 1, (unsigned char) *s);
        tmp[0] = len;
        memcpy(out.base + (size_t) i * out.elsize, tmp, len + 1);
    }
}

/**
 * The element offsets of the slice ARRAY SLICE and ARRAY INSERT work on: all
 * indices given but one, in argv[2], argv[4] ... Returns the first element
 * and puts the step in *increment; *length is how many elements.
 */
static int array_slice_start(const ArrayRef *a, int argc, char **argv, int *increment, int *length) {
    if (a->ndims < 2) error_throw_ex(kError, "Argument 1 must be a 2D or more array");
    if ((argc - 1) / 2 - 1 != a->ndims) ERROR_ARGUMENT_COUNT;
    int target = -1, pos[MAXDIM] = { 0 }, off[MAXDIM];
    for (int i = 0; i < a->ndims; i++) {
        if (*argv[i * 2 + 2]) {
            pos[i] = getint(argv[i * 2 + 2], mmb_options.base, a->dim[i] + mmb_options.base) - mmb_options.base;
        } else {
            if (target != -1) error_throw_ex(kError, "Only one index can be omitted");
            target = i;
        }
    }
    if (target == -1) error_throw_ex(kError, "One index must be omitted");
    int start = 0;
    for (int i = 0; i < a->ndims; i++) {
        off[i] = 1;
        for (int j = 0; j < i; j++) off[i] *= a->dim[j] + 1;
        if (i != target) start += pos[i] * off[i];
    }
    *increment = off[target];
    *length = a->dim[target] + 1;
    return start;
}

/** ARRAY SLICE source(), [d1] [, d2] ..., dest(): one line of source() into the 1D dest(). */
void cmd_array_slice(void) {
    getargs(&cmdline, 2 * MAXDIM + 3, DELIM_COMMA);
    if (argc < 7) ERROR_ARGUMENT_COUNT;
    ArrayRef src = array_ref(argv[0], 1, false);
    int increment, length;
    const int start = array_slice_start(&src, argc, argv, &increment, &length);
    ArrayRef dst = array_ref(argv[argc - 1], (argc + 1) / 2, true);
    if (dst.ndims != 1 || dst.count != length) error_throw_ex(kError, "Size mismatch between slice and target array");
    for (int k = 0; k < length; k++) array_copy_element(&src, start + k * increment, &dst, k);
}

/** ARRAY INSERT target(), [d1] [, d2] ..., source(): the 1D source() into one line of target(). */
void cmd_array_insert(void) {
    getargs(&cmdline, 2 * MAXDIM + 3, DELIM_COMMA);
    if (argc < 7) ERROR_ARGUMENT_COUNT;
    ArrayRef dst = array_ref(argv[0], 1, true);
    int increment, length;
    const int start = array_slice_start(&dst, argc, argv, &increment, &length);
    ArrayRef src = array_ref(argv[argc - 1], (argc + 1) / 2, false);
    if (src.ndims != 1 || src.count != length) error_throw_ex(kError, "Size mismatch between slice and source array");
    for (int k = 0; k < length; k++) array_copy_element(&src, k, &dst, start + k * increment);
}

