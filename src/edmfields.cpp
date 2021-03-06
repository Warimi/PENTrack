
/**
 * \file
 * The EDM fields used in the Ramsey Cycle
*/

#include <cmath>
#include "edmfields.h"
#include <iostream>
#include <stdlib.h>


//TEDMStaticB0GradZField constructor
TEDMStaticB0GradZField::TEDMStaticB0GradZField(double xoff, double yoff, double zoff, double ang1, double ang2, double abz, double adB0zdz, bool AC, double frq, double tstart1, double tend1, double pshift, double bW, double _xmax, double _xmin, double _ymax, double _ymin, double _zmax, double _zmin, std::string Bscale): TField(Bscale, "1") {
	edmB0xoff = xoff;
	edmB0yoff = yoff;
	edmB0zoff = zoff;
	pol_ang1 = ang1;
	azm_ang2 = ang2;
	edmB0z0 = abz;
	edmdB0z0dz = adB0zdz;
	ac = AC;
	f = frq;
	on1 = tstart1;
	off1 = tend1;
	phase = pshift;
	BoundaryWidth = bW;
	xmax = _xmax;
	xmin = _xmin;
	ymax = _ymax;
	ymin = _ymin;
	zmax = _zmax;
	zmin = _zmin;

	//Rotation Matrix for Bfield
	Rot1[0][0]=cos(ang1)*cos(ang2);
	Rot1[1][0]=-sin(ang2);
	Rot1[2][0]=sin(ang1)*cos(ang2);
	Rot1[0][1]=cos(ang1)*sin(ang2);
	Rot1[1][1]=cos(ang2);
	Rot1[2][1]=sin(ang1)*sin(ang2);
	Rot1[0][2]=-sin(ang1);
	Rot1[1][2]=0;
	Rot1[2][2]=cos(ang1);
	
	//generate inverted indicies rotation matrix
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			Rot2[j][i]=Rot1[i][j];
		}
	}
	
	//Rotation matrix for dB
	Rot3[0][0]=cos(ang1)*cos(ang2);
	Rot3[0][1]=-sin(ang2);
	Rot3[0][2]=sin(ang1)*cos(ang2);
	Rot3[1][0]=cos(ang1)*sin(ang2);
	Rot3[1][1]=cos(ang2);
	Rot3[1][2]=sin(ang1)*sin(ang2);
	Rot3[2][0]=-sin(ang1);
	Rot3[2][1]=0;
	Rot3[2][2]=cos(ang1);


	//Theory values for derivatives in the BField frame
	dB[0] = -edmdB0z0dz/2; 			//dBxdx
	dB[1] = 0;				//dBydx
	dB[2] = 0; 				//dBzdx
	dB[3] = 0;  				//dBxdy
	dB[4] = -edmdB0z0dz/2;			//dBydy
	dB[5] = 0;				//dBzdy
	dB[6] = 0;				//dBxdz
	dB[7] = 0;				//dBydz
	dB[8] = edmdB0z0dz;			//dBzdz
	
	for (int i = 0; i < 6; i++){
		for (int j = 0; j < 3; j++) {
			Bd[i][j] = 0;
		}
	}
	
	// change in B field in this coordinate system to be transformed into original coordinate system. //Bd[0]= Bdx/d(x,y,z) //Bd[1]= Bdy/d(x,y,z) //Bd[2]= Bdz/d(x,y,z)
	Bd[0][0] = dB[0];
	Bd[0][1] = dB[3];
	Bd[0][2] = dB[6];
	Bd[1][0] = dB[1];
	Bd[1][1] = dB[4];
	Bd[1][2] = dB[7];
	Bd[2][0] = dB[2];
	Bd[2][1] = dB[5];
	Bd[2][2] = dB[8];
	

	
	//apply rotation matrix to Change in Bfield parameters
	for(int k=0;k<3;k++){
		for(int i=0;i<3;i++){
		   for(int j=0;j<3;j++){
			   Bd[k+3][i]+=Rot3[i][j]*Bd[k][j];
			}
		}
	}


	//rearrangements of vectors for 2nd transform of Bfield. Bdx = Bd(x',y',z')/dx, where z' is in the direction of the B field
	Bd[0][0]=Bd[3][0];
	Bd[0][1]=Bd[4][0];
	Bd[0][2]=Bd[5][0];
	Bd[1][0]=Bd[3][1];	//Bdy = Bd(x',y',z')/dy
	Bd[1][1]=Bd[4][1];
	Bd[1][2]=Bd[5][1];
	Bd[2][0]=Bd[3][2];	//Bdz = Bd(x',y',z')/dz
	Bd[2][1]=Bd[4][2];
	Bd[2][2]=Bd[5][2];

	//apply rotation matrix again
	for(int k=0;k<3;k++){
		for(int i=0;i<3;i++){
			Bd[k+3][i]=0;
		   for(int j=0;j<3;j++){
			   Bd[k+3][i]+=Rot3[i][j]*Bd[k][j];
			}
		}
	}

	// Update dB to rotated values
	dB[0]=Bd[3][0];
	dB[1]=Bd[3][1];
	dB[2]=Bd[3][2];
	dB[3]=Bd[4][0];
	dB[4]=Bd[4][1];
	dB[5]=Bd[4][2];
	dB[6]=Bd[5][0];
	dB[7]=Bd[5][1];
	dB[8]=Bd[5][2];

	}

void TEDMStaticB0GradZField::BField(double x, double y, double z, double t, double B[4][4]){

	if(ac==true && (t<on1 || t>off1)){
	return;
	}
	else{
		
	//point to be rotated into Bfield reference frame
	double t1[3]={x-edmB0xoff,y-edmB0yoff,z-edmB0zoff};
	double t2[3]={0};

	//rotate specified point to BField coordinate system
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			t2[i]+=Rot1[i][j]*t1[j];
		}
	}
	
	//compute Bfield compoenents
	double BF1[3]; 
	double BF2[3]={0};
              
	BF1[0] = -t2[0]/2*edmdB0z0dz;		// Bx
	BF1[1] = -t2[1]/2*edmdB0z0dz;		// By
	BF1[2] = edmB0z0 + edmdB0z0dz*t2[2];	// Bz
	

	//rotate Bfield components back to global coordinates
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			BF2[i]+=Rot2[i][j]*BF1[j];
		}
	}
	

	//Initialize a local instance of dB to be folded
	double dBScaled[9];
	for(int i = 0; i < 9; i++)
		dBScaled[i] = dB[i];
		
	//apply AC scaling if necessary
	if(ac==true){
		double scalar;
		scalar=sin((f*t+phase)*2*M_PI);
	for (int i = 0; i < 9; i++){
		dBScaled[i]*=scalar;
		if (i < 3)
			BF2[i] *= scalar;
		}
	}
	
	//Fold the field near its boundaries
	for (int i = 0; i < 3; i++){
		FieldSmthr(x, y, z, BF2, dBScaled, i);
	}
	
	//Update the simulation BField to include the EDM component
	B[0][0]+=BF2[0];
	B[1][0]+=BF2[1];
	B[2][0]+=BF2[2];

	//update the simulation BField gradient to include the EDM component
	B[0][1]+=dBScaled[0];
	B[1][1]+=dBScaled[1];
	B[2][1]+=dBScaled[2];
	B[0][2]+=dBScaled[3];
	B[1][2]+=dBScaled[4];
	B[2][2]+=dBScaled[5];
	B[0][3]+=dBScaled[6];
	B[1][3]+=dBScaled[7];
	B[2][3]+=dBScaled[8];
	}
}

void TEDMStaticB0GradZField::FieldSmthr(double x, double y, double z, double *Bxi, double *dBScaled, int xi){		
	
		// Fscale = P(x')*P(y')*P(z') in the boundary where P(xi') = SmthrStp(xi-xi_min / BoundaryWidth) for the lower boundary and SmthrStp(xi-xi_max / BoundaryWidth) for the upper boundary
		double compressionArray[6] = {1,1,1,0,0,0};
		double Fscale = 1; 
		double dBadd[3] = {0, 0, 0};
		
		// Throw an error if two edges to be scaled overlap, otherwise compute how far into each boundary (x,y,z) is
		try{
		CompressionFactor(x,y,z,compressionArray);
		}
		catch (const std::invalid_argument& e){
			std::cout << "max-min distance has to be at least twice the BoundaryWidth!" << "\n";
			exit(-1);
		}
			
		// compute the dBadd (d(Fscale)/dxi) term for all three directions
		for (int i = 0; i < 3; i++){
			if (compressionArray[i] != 1){
				Fscale *=SmthrStp(compressionArray[i]);
				if(compressionArray[i] == 0)
					dBadd[i] = 0;
				else if(compressionArray[i+3] == 1)
					dBadd[i] = -Bxi[xi]*SmthrStpDer(compressionArray[i])/(SmthrStp(compressionArray[i])*BoundaryWidth);
				else if(compressionArray[i+3] == 0)
					dBadd[i] = Bxi[xi]*SmthrStpDer(compressionArray[i])/(SmthrStp(compressionArray[i])*BoundaryWidth);

			}
		}

		// If one or more of the components was scaled, scale the Bfield its relevant directional components
		// Bxi' (Scaled) = Bxi (unscaled) * P(x')*P(y')*P(z')
		// dBxidxj' (Scaled) = P(x')*P(y')*P(z') * dBxixj (Unscaled) + Bxi * (d/dxj P(x')*P(y')*P(z'))
		// Note that for the supper boundary d/dxj P(xj') = -P'(xj')/BoundaryWidth and for the lower boundary d/dxj P(xj') = P'(xj')/BoundaryWidth
		if (Fscale != 1){
			Bxi[xi] *= Fscale; // scale field value
			int j = 0;
				for (int i = 0; i < 7; i = i + 3){
					dBScaled[xi + i] = dBScaled[xi + i]*Fscale + dBadd[j]*Fscale; // scale derivatives according to product rule
					j++;
			}
		}
	}
	
// Compute the percentage into the boundary widthy the coordinate is in the x,y,z directions
void TEDMStaticB0GradZField::CompressionFactor(double x, double y, double z, double *compFactors){
	
	// Do nothing if BoundaryWidth is set to zero
	if (BoundaryWidth != 0){
	
	// throw error if the min/max distances are insufficient (min > max or not twice BoundaryWidth apart)
	if ((xmax - xmin) < 2*BoundaryWidth || (ymax - ymin) < 2*BoundaryWidth || (zmax - zmin) < 2*BoundaryWidth){
		throw std::invalid_argument( "max - min has to be at least twice the BoundaryWidth!" );
	}
		
	// Scale x,y,z if they are within BoundaryWidth of max or min
	if ((x < xmax) && (x >= (xmax - BoundaryWidth))){
		compFactors[0] = (xmax - x)/BoundaryWidth;
		compFactors[3] = 1;
		}

	if ((x > xmin) && (x <= (xmin + BoundaryWidth)))
		compFactors[0] = (x - xmin)/BoundaryWidth;


	if ((y < ymax) && (y >= (ymax - BoundaryWidth))){
		compFactors[1] = (ymax - y)/BoundaryWidth;
		compFactors[4] = 1;
		}

	if ((y > ymin) && (y <= (ymin + BoundaryWidth)))
		compFactors[1] = (y - ymin)/BoundaryWidth;

		
	if ((z < zmax) && (z >= (zmax - BoundaryWidth))){
		compFactors[2] = (zmax - z)/BoundaryWidth;
		compFactors[5] = 1;
		}

	if ((z > zmin) && (z <= (zmin + BoundaryWidth)))
		compFactors[2] = (z - zmin)/BoundaryWidth;


	// Nullify the field outside of the min/max
	if (x >= xmax || x <= xmin)
		compFactors[0] = 0;
		
	if (y >= ymax || y <= ymin)
		compFactors[1] = 0;
		
	if (z >= zmax || z <= zmin)
		compFactors[2] = 0;
	}
}

double TEDMStaticB0GradZField::SmthrStp(double x) {
	return 6*pow(x, 5) - 15*pow(x, 4) + 10*pow(x, 3);
}

double TEDMStaticB0GradZField::SmthrStpDer(double x) {
	return 30*pow(x, 4) - 60*pow(x, 3) + 30*pow(x,2);
}

//TEDMStaticEField constructor
TEDMStaticEField::TEDMStaticEField (double aexMag, double aeyMag, double aezMag, std::string Escale): TField("1", Escale){
	exMag = aexMag;
	eyMag = aeyMag;
	ezMag = aezMag;
	} 

void TEDMStaticEField::EField (double x, double y, double z, double t, double &V, double Ei[3], double dEidxj[3][3]) {
	double Escale = EScaling(t);
	Ei[0] = exMag*Escale;
	Ei[1] = eyMag*Escale;
	Ei[2] = ezMag*Escale;

	V = Ei[2]*z*Escale;
	
}
