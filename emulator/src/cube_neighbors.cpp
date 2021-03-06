/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * Sifteo Thundercracker simulator
 * Micah Elizabeth Scott <micah@misc.name>
 *
 * Copyright <c> 2011 Sifteo, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "cube_neighbors.h"
#include "cube_hardware.h"

namespace Cube {


void Neighbors::attachCubes(Cube::Hardware *cubes)
{
    /*
     * The system is giving us a pointer to the other cubes, so
     * that we can scan their neighbor state on demand.
     */ 
    otherCubes = cubes;
}

void Neighbors::ioTick(CPU::em8051 &cpu)
{
    static const uint8_t outPinLUT[] = { PIN_0_TOP, PIN_1_LEFT, PIN_2_BOTTOM, PIN_3_RIGHT };

    if (!otherCubes) {
        // So lonely...
        return;
    }

    uint8_t inputPins = cpu.mSFR[DIR];                 // Which pins are in input mode?
    uint8_t driveHigh = cpu.mSFR[PORT] & ~inputPins;   // Pins we're driving high

    // Rising driven-edge detector. This is our pulse transmit state.
    uint8_t driveEdge = driveHigh & ~prevDriveHigh;
    prevDriveHigh = driveHigh;

    /*
     * Precalculate a mask of all sides that can receive right now.
     * If our input isn't working (e.g. its pin direction is wrong)
     * or if all sides are masked, this mask can be zero.
     */

    uint8_t nextMask = 0;

    if (inputPins & PIN_IN)
        for (unsigned s = 0; s < NUM_SIDES; s++)
            if (inputPins & outPinLUT[s])
                nextMask |= 1 << s;

    inputMask = nextMask;

    /*
     * Propagate outgoing edges to anyone who can hear them.
     * Edges are much much less frequent than clock ticks, so we
     * try to do as much of this work as possible on the transmit
     * side.
     */

    if (driveEdge) {
        Tracer::log(&cpu, "NEIGHBOR: Send pulse (pins %02x)", driveEdge);

        for (unsigned mySide = 0; mySide < NUM_SIDES; mySide++) {
            uint8_t mySideBit = outPinLUT[mySide];
            
            if (mySideBit & driveEdge) {
                // We're transmitting on this side

                // If we're listening, we'll hear an echo
                receivedPulse(cpu);
                
                Tracer::log(&cpu, "NEIGHBOR: Matrix, side %d: %02x %02x %02x %02x", mySide,
                            mySides[mySide].otherSides[0],
                            mySides[mySide].otherSides[1],
                            mySides[mySide].otherSides[2],
                            mySides[mySide].otherSides[3]);

                // Look at each other-side bitmask...
                for (unsigned otherSide = 0; otherSide < NUM_SIDES; otherSide++) {
                    uint32_t sideMask = mySides[mySide].otherSides[otherSide];

                    /*
                     * Loop over all neighbored cubes on this
                     * side. Should just be zero or one cubes, but
                     * it's easy to stay generic here.
                     */

                    while (sideMask) {
                        int otherCube = __builtin_ffs(sideMask) - 1;
                        transmitPulse(cpu, otherCube, otherSide);
                        sideMask ^= 1 << otherCube;
                    }
                }
            }
        }
    }
}

void Neighbors::transmitPulse(CPU::em8051 &cpu, unsigned otherCube, uint8_t otherSide)
{
    Hardware &dest = otherCubes[otherCube];

    if (dest.neighbors.isSideReceiving(otherSide)) {
        Tracer::log(&cpu, "NEIGHBOR: Sending pulse to %d.%d", otherCube, otherSide);
        receivedPulse(dest.cpu);
    } else {
        Tracer::log(&cpu, "NEIGHBOR: Pulse to %d.%d was masked", otherCube, otherSide);
    }
}


};  // namespace Cube
