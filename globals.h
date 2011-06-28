#ifndef GLOBALS_H_
#define GLOBALS_H_

#define KENNZAHL_UNKNOWN 0
#define KENNZAHL_NOT_FINISH 1
#define KENNZAHL_HIT_BOUNDARIES 2
#define KENNZAHL_NRERROR 3
#define KENNZAHL_DECAYED 4
#define KENNZAHL_INITIAL_NOT_FOUND 5

#define FILLING_PHASE 1
#define CLEANING_PHASE 2
#define RAMPUP_PHASE 3
#define FULLFIELD_PHASE 4
#define RAMPDOWN_PHASE 5
#define COUNTING_PHASE 6

#define NEUTRON 1
#define PROTON 2
#define BF_ONLY 3
#define BF_CUT 4
#define INTERACTIVE 5
#define ELECTRON 6
#define GEOMETRY 7

#define POLARISATION_GOOD 1
#define POLARISATION_BAD 2
#define POLARISATION_NONE 3

#define OUTPUT_EVERYTHING 1
#define OUTPUT_ENDPOINTS 2
#define OUTPUT_NOTHING 5
#define OUTPUT_EVERYTHINGandSPIN 3
#define OUTPUT_ENDPOINTSandSPIN 4

#include <cstdarg>
#include <cstring>
#include <string>

using namespace std;

void percent(long double x, long double x1, long double len, int &perc); // print percent of (x-x1)/len
// rotate vector into new coordinate sys whose z-axis lies on NORMALIZED vector n (active transformation)
void RotateVector(long double v[3], long double n[3]);
void BOOST(long double beta[3], long double p[4]); // Lorentz boost of four-vector p into frame moving in arbitrary direction with v/c = beta
long double ProtonSpectrum(long double E);
long double ElectronSpectrum(long double E);

// physical constants
const long double pi = 3.1415926535897932384626;
const long double ele_e = 1.602176487E-19;      //elementary charge (C)
const long double gravconst = 9.80665;	// g (m/s)
const long double conv = pi/180.;	// deg to rad conversion factor
const long double mu0 = 4*pi*1e-7;      //permeability (Vs/Am)
const long double m_n = 1.674927211E-27/ele_e;	//neutron mass (eV/c^2)
const long double m_p = 1.672621637E-27/ele_e;	//proton mass (eV/c^2)
const long double m_e = 9.10938215e-31/ele_e;	//electron mass (eV/c^2)
const long double c_0 = 299792458;	// lightspeed (m/s)
const long double hquer = 1.05457266e-34;	// planck constant (Js)
const long double mu_nSI = -0.96623641e-26;	// Neutron magn Mom (J/T)
const long double gamma_n = -1.83247185e8; // = 2*mu_n/hquer gyromagnetic ratio of neutron (1/Ts)

const long double lengthconv = 0.01 , Bconv = 1e-4, Econv = 1e2;    // Einheiten aus field Tabelle (cgs) und Programm (si) abgleichen

extern long double StorageTime; // max. simulation time

extern int jobnumber;
extern string inpath, outpath; 
extern int ausgabewunsch; // user choice for Ausgabewunsch


#endif /*GLOBALS_H_*/
