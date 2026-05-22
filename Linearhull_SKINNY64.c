#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include <math.h>
#include <stdint.h>
#define ROUND 6
#define P_number 29
#define K_number 1000
const int S[16] = { 0xc,0x6,0x9,0x0,0x1,0xa,0x2,0xb,0x3,0x8,0x5,0xd,0x4,0xe,0x7,0xf };
const int PK[16] = { 9,15,8,13,10,14,12,11,0,1,2,3,4,5,6,7 };
const int RC0[32] = { 0x1,0x3,0x7,0xf,0xf,0xe,0xd,0xb,0x7,0xf,0xe,0xc,0x9,0x3,0x7,0xe,
				0xd,0xa,0x5,0xb,0x6,0xc,0x8,0x0,0x1,0x2,0x5,0xb,0x7,0xe,0xc,0x8 };
const int RC1[32] = { 0x0,0x0,0x0,0x0,0x1,0x3,0x3,0x3,0x3,0x2,0x1,0x3,0x3,0x3,0x2,0x0,
				0x1,0x3,0x3,0x2,0x1,0x2,0x1,0x3,0x2,0x0,0x0,0x0,0x1,0x2,0x1,0x3 };
int TK[32][16];

void Initialize_key(int Key[], int r)
{
	int i = 0, j = 0;
	for (i = 0; i < 16; i++)
	{
		TK[0][i] = Key[i];
	}
	for (i = 1; i < r; i++)
	{
		for (j = 0; j < 16; j++)
		{
			TK[i][j] = TK[i - 1][PK[j]];
		}
	}

}

void Round(int r, int Stt[])
{
	int i = 0;
	for (i = 0; i < 16; i++)//SBox
	{
		Stt[i] = S[Stt[i]];
	}
	Stt[0] ^= RC0[r];//AddConstant
	Stt[4] ^= RC1[r];
	Stt[8] ^= 0x2;
	for (i = 0; i < 8; i++)   //Addtk
	{
		Stt[i] = Stt[i] ^ TK[r][i];
	}
	int Temp[16] = { 0 };    //shiftrow
	for (i = 0; i < 4; i++)
	{
		Temp[i] = Stt[i];
		Temp[i + 4] = Stt[((i + 3) % 4) + 4];
		Temp[i + 8] = Stt[((i + 2) % 4) + 8];
		Temp[i + 12] = Stt[((i + 1) % 4) + 12];
	}
	for (i = 0; i < 4; i++)//MixColumn
	{
		Stt[i] = Temp[i] ^ Temp[i + 8] ^ Temp[i + 12];
		Stt[i + 4] = Temp[i];
		Stt[i + 8] = Temp[i + 4] ^ Temp[i + 8];
		Stt[i + 12] = Temp[i] ^ Temp[i + 8];
	}
}

uint64_t dot_product(uint64_t a, uint64_t b)
{
	uint64_t c, i;
	a = a & b;
	b = 0;
	for (i = 0; i < 64; i++)
	{
		c = a & 1;
		a >>= 1;
		b = b ^ c;
	}
	return b;
}

void linear(uint64_t alpha, uint64_t beta)
{
	uint64_t k, i, num, data;
	int j, r = 0, zero;
	unsigned long long m = 0, c = 0, key = 0;
	int P[16] = { 0 }, Key[16] = { 0 };
	long double bias, BIAS = 0, index;
	long double c2, C2 = 0, index_c2;
	srand((unsigned)time(NULL));
	for (k = 0; k < K_number; k++)
	{
		for (int i = 0; i < 4; i++) {
			key = (key << 16) | (rand() & 0xFFFF);
		}
		for (j = 0; j < 16; j++)
		{
			Key[j] = (key >> (60 - 4 * j)) & 0xf;
		}
		Initialize_key(Key, ROUND);
		data = 1ULL << P_number;
		num = 0;
		for (i = 0; i < data; i++)
		{
			c = 0;
			for (int i = 0; i < 4; i++) {
				m = (m << 16) | (rand() & 0xFFFF);
			}
			for (j = 0; j < 16; j++)
			{
				P[j] = (m >> (60 - 4 * j)) & 0xf;
			}
			for (r = 0; r < ROUND; r++) 
			{
				Round(r, P);
			}
			for (j = 0; j < 16; j++)
			{
				c ^= (unsigned long long)(P[j]) << (60 - 4 * j);
			}
			zero = 0;
			zero = dot_product(m, alpha) ^ dot_product(c, beta);
			if (zero == 0)
			{
				num++;
			}
		}
		bias = (double)num / data - 0.5;
		index = log2(fabs(bias));
		c2 = (double)2* num / data - 1;//cor=2*bias
		c2 = c2 * c2;
		index_c2 = log2(fabs(c2));
		if (k % 100 == 0)
		{
			printf("num=%lld\n", num);
			printf("index_c2=%lf\n", index_c2);
		}
		BIAS += fabs(bias);
		C2 += fabs(c2);
	}
	BIAS = (long double)BIAS / K_number;
	C2 = (long double)C2 / K_number;
	printf("BIAS=%lf\n", BIAS);
	printf("INDEX_BIAS=%lf\n", log2(BIAS));
	printf("INDEX_C2=%lf\n", 2 * log2(BIAS) + 2);
	printf("C2=%lf\n", C2);
	printf("INDEX_C2=%lf\n", log2(C2));
}

int main()
{
	uint64_t alpha, beta;
	clock_t start = clock();

	//-18.27
	/*alpha = 0x0006060060000000;
	beta = 0x8080800080000880;
	linear(alpha, beta);*/
	//-25.1006
	alpha = 0x0002200002022000;
	beta = 0x0088088080808808;
	linear(alpha, beta);

	clock_t end = clock();
	double e = (double)(end - start) / CLOCKS_PER_SEC;
	printf("time: %.3f seconds.\n", e);
	return 0;
}


