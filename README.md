# Old MSDOS code stuff
This is a random assortment of my old pieces of MSDOS code, some newer but all of it for DOS.
Some of it reworked from TASM and such into NASM and C code tested against newer OpenWatcom v2 compiler.
Mostly, its old and just recompiled against OWv2.

## D - Directory Lister
> DIR command for a coloured lister. Configurable etc.

## Dram Check
> Dumps the DDR info from the VIA VT82C496G chipset

## E820
> Fake E820 bios hook if your bios doesnt have E820 functionality.
> Also an E820 list dumper..

## InfoD32
> Prints DOS32 executable information, all version 3.0 - 3.5

## Once
> Use in your autoexec.bat to run something once per day

## RDFL
> RDF Loader. Posted this to usenet back in the day... looong time ago.
> Lets you build an RDF like a COM and boostrap it with a tiny COM loader.

## SBDSP
> Pulls the SoundBlaster DSP, major/minor versions and the DSP string.

## SCInfo
> Prints sound card info from environment variables.
> Supports SB, GUS, WSS, Adlib Gold

## TSR Skeleton
> A small NASM stub I use for all my TSR's. Will load itself as high into UMB if possible, etc. Takes care of all the memory checking/dos verion tests for umb's etc, detaching etc

---

# License
I consider this old junk, and of no intrinsic value.
Consider everything in this repo unless stated otherwise, to be under the BSD 3 clause

## BSD 3-Clause License
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

The names of its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

