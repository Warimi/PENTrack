/**
 * \file
 * Neutron class definition.
 */

#include "neutron.h"
#include "globals.h"
#include "proton.h"
#include "electron.h"
#include "ndist.h"


const char* NAME_NEUTRON = "neutron";

ofstream TNeutron::endout; ///< endlog file stream
ofstream TNeutron::snapshotout; ///< snapshot file stream
ofstream TNeutron::trackout; ///< tracklog file stream
ofstream TNeutron::hitout; ///< hitlog file stream
ofstream TNeutron::spinout; ///< spinlog file stream
ofstream TNeutron::spinout2; ///< spinlog file stream for doing simultaneous anti-parallel Efield spin integration 
TMicroRoughness TNeutron::MR;

TNeutron::TNeutron(int number, double t, double x, double y, double z, double E, double phi, double theta, int polarisation, TMCGenerator &amc, TGeometry &geometry, TFieldManager *afield)
		: TParticle(NAME_NEUTRON, 0, m_n, mu_nSI, gamma_n, number, t, x, y, z, E, phi, theta, polarisation, amc, geometry, afield){

}

void TNeutron::Transmit(value_type x1, state_type y1, value_type &x2, state_type &y2, int &polarisation,
			const double normal[3], solid *leaving, solid *entering, bool &trajectoryaltered, bool &traversed){
	value_type vnormal = y1[3]*normal[0] + y1[4]*normal[1] + y1[5]*normal[2]; // velocity normal to reflection plane
	material *mat = vnormal < 0 ? &entering->mat : &leaving->mat;
	double prob = mc->UniformDist(0,1);
	double diffprob;

	// specular transmission (refraction)
	value_type Enormal = 0.5*m_n*vnormal*vnormal; // energy normal to reflection plane
	value_type Estep = entering->mat.FermiReal*1e-9 - leaving->mat.FermiReal*1e-9;
	long double k1 = sqrt(Enormal); // wavenumber in first solid (use only real part for transmission!)
	long double k2 = sqrt(Enormal - Estep); // wavenumber in second solid (use only real part for transmission!)
	for (int i = 0; i < 3; i++)
		y2[i + 3] += (k2/k1 - 1)*(normal[i]*vnormal); // refract (scale normal velocity by k2/k1)

	bool UseMRModel = mat->UseMRModel && MR.MRValid(&y1[3], normal, leaving, entering); // check if MicroRoughness model should be applied
	if (UseMRModel)
		diffprob = MR.MRProb(true, &y1[3], normal, leaving, entering); // calculate probability of diffuse scattering using MR model
	else
		diffprob = mat->DiffProb; // use fixed probability of diffuse scattering for simple Lambert model

	if (prob < diffprob){ // diffuse transmission
		double theta_t, phi_t;
		if (UseMRModel){
			double MRmax = MR.MRDistMax(true, &y1[3], normal, leaving, entering);
//			cout << "max: " << MRmax << '\n';
			do{
				phi_t = mc->UniformDist(0, 2*pi);
				theta_t = mc->SinDist(0, pi/2);
			}while (mc->UniformDist(0, MRmax) > MR.MRDist(true, false, &y1[3], normal, leaving, entering, theta_t, phi_t));
		}
		else{
			phi_t = mc->UniformDist(0, 2*pi); // generate random reflection angles (Lambert's law)
			theta_t = mc->SinCosDist(0, 0.5*pi);
		}
		if (vnormal < 0) theta_t = pi - theta_t; // if velocity points into volume invert polar angle
		x2 = x1;
		value_type vabs = sqrt(y2[3]*y2[3] + y2[4]*y2[4] + y2[5]*y2[5]);
		y2[3] = vabs*cos(phi_t)*sin(theta_t);	// new velocity with respect to z-axis
		y2[4] = vabs*sin(phi_t)*sin(theta_t);
		y2[5] = vabs*cos(theta_t);
		RotateVector(&y2[3], normal, &y1[3]); // rotate velocity into coordinate system defined by incoming velocity and plane normal
	}
	traversed = true;
	trajectoryaltered = true;
}

void TNeutron::Reflect(value_type x1, state_type y1, value_type &x2, state_type &y2, int &polarisation,
			const double normal[3], solid *leaving, solid *entering, bool &trajectoryaltered, bool &traversed){
	value_type vnormal = y1[3]*normal[0] + y1[4]*normal[1] + y1[5]*normal[2]; // velocity normal to reflection plane
	//particle was neither transmitted nor absorbed, so it has to be reflected
	double prob = mc->UniformDist(0,1);
	material *mat = vnormal < 0 ? &entering->mat : &leaving->mat;
	double diffprob;
	bool UseMRModel = mat->UseMRModel && MR.MRValid(&y1[3], normal, leaving, entering);
	if (UseMRModel)
		diffprob = MR.MRProb(false, &y1[3], normal, leaving, entering);
	else
		diffprob = mat->DiffProb;
//	cout << "prob: " << diffprob << '\n';
	if (prob >= diffprob){
		//************** specular reflection **************
//				printf("Specular reflection! Erefl=%LG neV\n",Enormal*1e9);
		x2 = x1;
		for (int i = 0; i < 6; i++)
			y2[i] = y1[i];
		y2[3] -= 2*vnormal*normal[0]; // reflect velocity
		y2[4] -= 2*vnormal*normal[1];
		y2[5] -= 2*vnormal*normal[2];
	}
	else{
		//************** diffuse reflection ************
		double phi_r, theta_r;
		if (UseMRModel){
			double MRmax = MR.MRDistMax(false, &y1[3], normal, leaving, entering);
//			cout << "max: " << MRmax << '\n';
			do{
				phi_r = mc->UniformDist(0, 2*pi);
				theta_r = mc->SinDist(0, pi/2);
			}while (mc->UniformDist(0, MRmax) > MR.MRDist(false, false, &y1[3], normal, leaving, entering, theta_r, phi_r));
		}
		else{
			phi_r = mc->UniformDist(0, 2*pi); // generate random reflection angles (Lambert's law)
			theta_r = mc->SinCosDist(0, 0.5*pi);
		}
		if (vnormal > 0) theta_r = pi - theta_r; // if velocity points out of volume invert polar angle
		x2 = x1;
		for (int i = 0; i < 3; i++)
			y2[i] = y1[i];
		value_type vabs = sqrt(y1[3]*y1[3] + y1[4]*y1[4] + y1[5]*y1[5]);
		y2[3] = vabs*cos(phi_r)*sin(theta_r);	// new velocity with respect to z-axis
		y2[4] = vabs*sin(phi_r)*sin(theta_r);
		y2[5] = vabs*cos(theta_r);
		RotateVector(&y2[3], normal, &y1[3]); // rotate velocity into coordinate system defined by incoming velocity and plane normal
//				printf("Diffuse reflection! Erefl=%LG neV w_e=%LG w_s=%LG\n",Enormal*1e9,phi_r/conv,theta_r/conv);
	}

	if (mc->UniformDist(0,1) < entering->mat.SpinflipProb){
		polarisation *= -1;
		Nspinflip++;
	}

	traversed = false;
	trajectoryaltered = true;
}

void TNeutron::OnHit(value_type x1, state_type y1, value_type &x2, state_type &y2, int &polarisation,
			const double normal[3], solid *leaving, solid *entering, bool &trajectoryaltered, bool &traversed){
	value_type vnormal = y1[3]*normal[0] + y1[4]*normal[1] + y1[5]*normal[2]; // velocity normal to reflection plane
	value_type Enormal = 0.5*m_n*vnormal*vnormal; // energy normal to reflection plane
	trajectoryaltered = false;
	traversed = true;
	value_type prob = mc->UniformDist(0,1);

	value_type Estep = entering->mat.FermiReal*1e-9 - leaving->mat.FermiReal*1e-9;
//		cout << "Leaving " << leaving->ID << " Entering " << entering->ID << " Enormal = " << Enormal << " Estep = " << Estep;
	if (Enormal > Estep){ // transmission only possible if E > Estep
		double k1 = sqrt(Enormal); // wavenumber in first solid (use only real part for transmission!)
		double k2 = sqrt(Enormal - Estep); // wavenumber in second solid (use only real part for transmission!)
		double transprob = 4*k1*k2/(k1 + k2)/(k1 + k2); // transmission probability
//			cout << " TransProb = " << transprob << '\n';
		if (prob < transprob) // -> transmission
			Transmit(x1, y1, x2, y2, polarisation, normal, leaving, entering, trajectoryaltered, traversed);
		else // no transmission -> reflection
			Reflect(x1, y1, x2, y2, polarisation, normal, leaving, entering, trajectoryaltered, traversed);
	}
	else{
		double k1 = sqrt(Enormal); // wavenumber in first solid (only real part)
		complex<double> iEstep(Estep, -entering->mat.FermiImag*1e-9); // potential step using imaginary potential V - i*W of second solid
		complex<double> k2 = sqrt(Enormal - iEstep); // wavenumber in second solid (including imaginary part)
		double reflprob = pow(abs((k1 - k2)/(k1 + k2)), 2); // reflection probability
		if (entering->mat.UseMRModel){
			double kc = sqrt(2*m_n*Estep)*ele_e/hbar;
			double addtrans = 2*pow(entering->mat.RMSRoughness, 2)*kc*kc/(1 + 0.85*kc*entering->mat.CorrelLength + 2*kc*kc*pow(entering->mat.CorrelLength, 2));
			reflprob = 1 - (1 - reflprob)*sqrt(1 + addtrans); // second order correction for reflection on MicroRoughness surfaces
		}
//			cout << " ReflProb = " << reflprob << '\n';
		if (prob > reflprob){ // -> absorption on reflection
			x2 = x1;
			y2 = y1; // set end point to point right before collision and set ID to absorbed
			StopIntegration(ID_ABSORBED_ON_SURFACE, x2, y2, polarisation, *entering);
			traversed = false;
			trajectoryaltered = true;
		}
		else // no absorption -> reflection
			Reflect(x1, y1, x2, y2, polarisation, normal, leaving, entering, trajectoryaltered, traversed);
	}

}


bool TNeutron::OnStep(value_type x1, state_type y1, value_type &x2, state_type &y2, int &polarisation, solid currentsolid){
	bool result = false;
	if (currentsolid.mat.FermiImag > 0){
		complex<long double> E(0.5*m_n*(y1[3]*y1[3] + y1[4]*y1[4] + y1[5]*y1[5]), currentsolid.mat.FermiImag*1e-9); // E + i*W
		complex<long double> k = sqrt(2*m_n*E)*ele_e/hbar; // wave vector
		double l = sqrt(pow(y2[0] - y1[0], 2) + pow(y2[1] - y1[1], 2) + pow(y2[2] - y1[2], 2)); // travelled length
		double abspath = mc->ExpDist(2*imag(k)); // exponential probability decay
		if (abspath < l){
			x2 = x1 + abspath/l*(x2 - x1); // if absorbed, chose a random time between x1 and x2
			for (int i = 0; i < 6; i++)
				stepper.calc_state(x2, y2);
			StopIntegration(ID_ABSORBED_IN_MATERIAL, x2, y2, polarisation, currentsolid);
			printf("Absorption!\n");
			result = true; // stop integration
		}
	}

	// do special calculations for neutrons (spinflipcheck, snapshots, etc)
	if (neutdist == 1)
		fillndist(x1, &y1[0], x2, &y2[0]); // write spatial neutron distribution
/*
	if (field){
		long double B[4][4];
		field->BField(y1[0],y1[1],y1[2],x1,B);
*//*
		if (B[3][0] > 0){
			// spin flip properties according to Vladimirsky and thumbrule
			vlad = vladimirsky(B[0][0], B[1][0], B[2][0],
							   B[0][1], B[0][2], B[0][3], B[1][1], B[1][2], B[1][3], B[2][1], B[2][2], B[2][3], B[3][0],
							   y1[3], y1[4], y1[5]);
			frac = thumbrule(B[0][0], B[1][0], B[2][0],
							   B[0][1], B[0][2], B[0][3], B[1][1], B[1][2], B[1][3], B[2][1], B[2][2], B[2][3], B[3][0],
							   y1[3], y1[4], y1[5]);
			vladtotal *= 1-vlad;
			if (vlad > 1e-99)
				vladmax = max(vladmax,log10(vlad));
			if (frac > 1e-99)
				thumbmax = max(thumbmax,log10(frac));
		}
*/
/*			long double B2[4][4];
		field->BField(y2[0],y2[1],y2[2],x2,B2);
		long double sp = BruteForceIntegration(x1,y1,B,x2,y2,B2); // integrate spinflip probability
//			if (1-sp > 1e-30) logBF = log10(1-sp);
//			else logBF = -99;
//			BFsurvprob *= sp;
		// flip the spin with a probability of 1-BFsurvprob
		if (flipspin && mc->UniformDist(0,1) < 1-sp)
		{
			polarisation *= -1;
			Nspinflip++;
			printf("\n The spin has flipped! Number of flips: %i\n",Nspinflip);
			result = true;
		}
	}
*/
	return result;
}


void TNeutron::Decay(){
	double E_p, E_e, phi_p, phi_e, theta_p, theta_e;
	int pol_p, pol_e;
	TParticle *p;
	mc->NeutronDecay(&yend[3], E_p, E_e, phi_p, phi_e, theta_p, theta_e, pol_p, pol_e);
	p = new TProton(particlenumber, tend, yend[0], yend[1], yend[2], E_p, phi_p, theta_p, pol_p, *mc, *geom, field);
	secondaries.push_back(p);
	p = new TElectron(particlenumber, tend, yend[0], yend[1], yend[2], E_e, phi_e, theta_e, pol_e, *mc, *geom, field);
	secondaries.push_back(p);
}


double TNeutron::Epot(value_type t, state_type y, int polarisation, TFieldManager *field, solid sld){
	return TParticle::Epot(t, y, polarisation, field, sld) + sld.mat.FermiReal*1e-9;
}
