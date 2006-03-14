#define HALF_LN_OF_TWO	0.346573590279972654708616060729088
#define TPI		6.28318530717958647692528676655901

class FilterCoef
{

private:
	double	coef[5];

	void ComputeCoefs(int freq, int r, int t)
	{
	
	float omega =float (TPI*Kutoff(freq)/44100);
    float sn = (float)sin( omega);
    float cs = (float)cos( omega);
    float alpha;
        
	if( t<2)
    alpha =float(sn / Reonance( r *(freq+70)/(127.0f+70)));
    else
    alpha =float (sn * sinh( Bandwidth( r) * omega/sn));

        float a0, a1, a2, b0, b1, b2;

        switch( t)
		{
    
		case 0: // LP
                b0 =  (1 - cs)/2;
                b1 =   1 - cs;
                b2 =  (1 - cs)/2;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 1: // HP
                b0 =  (1 + cs)/2;
                b1 = -(1 + cs);
                b2 =  (1 + cs)/2;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 2: // BP
                b0 =   alpha;
                b1 =   0;
                b2 =  -alpha;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 3: // BR
                b0 =   1;
                b1 =  -2*cs;
                b2 =   1;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
		}

        coef[0] = b0/a0;
        coef[1] = b1/a0;
        coef[2] = b2/a0;
        coef[3] = -a1/a0;
        coef[4] = -a2/a0;
};
	

float Kutoff( int v)
{
        return float(pow( (v+5)/(127.0+5), 1.7)*13000+30);
};

float Reonance( float v)
{
        return float(pow( v/127.0, 4)*150+0.1);
};

float Bandwidth( int v)
{
        return float(pow( v/127.0, 4)*4+0.1);
};

public:

	float coefs[5][128][16][5];

	void MakeCoefs()
	{
		for(int r=0;r<5;r++)
		{
			for(int f=0;f<128;f++)
			{
				for(int q=0;q<16;q++)
				{
				ComputeCoefs(f,q*8,r);
				coefs[r][f][q][0]=(float)coef[0];
				coefs[r][f][q][1]=(float)coef[1];
				coefs[r][f][q][2]=(float)coef[2];
				coefs[r][f][q][3]=(float)coef[3];
				coefs[r][f][q][4]=(float)coef[4];
				}
			}
		}

	};

};
