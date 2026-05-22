#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include <math.h>
#include <stdint.h>
#define ROUND 7
#define P_number 29 
#define K_number 1000
const int S[16] = { 0xc,0xa,0xd,0x3,0xe,0xb,0xf,0x7,0x8,0x9,0x1,0x5,0x0,0x2,0x4,0x6 };
const int P[16] = { 0xf,0xc,0xd,0xe,0xa,0x9,0x8,0xb,0x6,0x5,0x4,0x7,0x1,0x2,0x3,0x0 };
const int Q[16] = { 0xc,0xa,0xf,0x5,0xe,0x8,0x9,0x2,0xb,0x3,0x7,0x4,0x6,0x0,0x1,0xd };
const int RC3[32] = { 0x1,0x4,0x2,0x5,0x6,0x7,0x3,0x1,0x4,0x2,0x5,0x6,0x7,0x3,0x1,0x4,
				0x2,0x5,0x6,0x7,0x3,0x1,0x4,0x2,0x5,0x6,0x7,0x3,0x1,0x4,0x2,0x5 };
const int RC4[32] = { 0x1,0x8,0x4,0x2,0x9,0xc,0x6,0xb,0x5,0xa,0xd,0xe,0xf,0x7,0x3,0x1,
				0x8,0x4,0x2,0x9,0xc,0x6,0xb,0x5,0xa,0xd,0xe,0xf,0x7,0x3,0x1,0x8 };
//CRAFT

int TK[4][16];

void Initialize_key(int Key1[], int Key2[])
{

	int Tweak[16] = { 0 };
	int i = 0, j = 0;
	for (i = 0; i < 16; i++)
	{
		TK[0][i] = Key1[i] ^ Tweak[i];
		TK[1][i] = Key2[i] ^ Tweak[i];
		TK[2][i] = Key1[i] ^ Tweak[Q[i]];
		TK[3][i] = Key2[i] ^ Tweak[Q[i]];
	}

}

void Round(int r, int Stt[])
{

	int i = 0;
	for (i = 0; i < 4; i++)//MixColumn
	{
		Stt[i] ^= (Stt[i + 8] ^ Stt[i + 12]);
		Stt[i + 4] ^= Stt[i + 12];
	}

	int ind = r;


	Stt[4] ^= RC4[ind];//AddConstant
	Stt[5] ^= RC3[ind];

	for (i = 0; i < 16; i++)//AddTweakey
		Stt[i] ^= TK[ind % 4][i];

	if (r != 31)
	{
		int Temp[16];
		for (int i = 0; i < 16; i++)//Permutation
			Temp[P[i]] = Stt[i];

		for (i = 0; i < 16; i++)//SBox
			Stt[i] = S[Temp[i]];
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
	unsigned long long m = 0, c = 0, key1 = 0, key2 = 0;
	int P[16] = { 0 }, Key1[16] = { 0 }, Key2[16] = { 0 };
	long double bias, BIAS = 0, index;
	long double c2, C2 = 0, index_c2;
	srand((unsigned)time(NULL));
	for (k = 0; k < K_number; k++)/*key*/
	{
		for (int i = 0; i < 4; i++) {
			key1 = (key1 << 16) | (rand() & 0xFFFF);
			key2 = (key2 << 16) | (rand() & 0xFFFF);
		}
		for (j = 0; j < 16; j++)
		{
			Key1[j] = (key1 >> (60 - 4 * j)) & 0xf;
			Key2[j] = (key2 >> (60 - 4 * j)) & 0xf;
		}
		Initialize_key(Key1, Key2);
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
		bias = (double)num / data - 0.5;//bias
		index = log2(fabs(bias));
		c2 = (double)2 * num / data - 1;//cor=2*bias
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

	//-17.0956,6r
	/*alpha = 0x0005550000055005;
	beta  = 0x0005550000000500;
	linear(alpha, beta);*/
	//-25.5325,7r
	alpha = 0x0002220000022002;
	beta  = 0x2200000202200020;
	linear(alpha, beta);

	clock_t end = clock();
	double e = (double)(end - start) / CLOCKS_PER_SEC;
	printf("time: %.3f seconds.\n", e);
	return 0;
}