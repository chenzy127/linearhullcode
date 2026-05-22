#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include <math.h>
#include <stdint.h>
#define ROUND 4
#define P_number 28 /*指数*/
#define K_number 1000
const int S[16] = { 0xc,0xa,0xd,0x3,0xe,0xb,0xf,0x7,0x8,0x9,0x1,0x5,0x0,0x2,0x4,0x6 };
const int P[16] = { 0,10,5,15,14,4,11,1,9,3,12,6,7,13,2,8 };
const unsigned _int8 a[19][16] =
{
	0,0,0,1,0,1,0,1,1,0,1,1,0,0,1,1,
	0,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,
	1,0,1,0,0,1,0,0,0,0,1,1,0,1,0,1,
	0,1,1,0,0,0,1,0,0,0,0,1,0,0,1,1,
	0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,
	1,1,0,1,0,0,0,1,0,1,1,1,0,0,0,0,
	0,0,0,0,0,0,1,0,0,1,1,0,0,1,1,0,
	0,0,0,0,1,0,1,1,1,1,0,0,1,1,0,0,
	1,0,0,1,0,1,0,0,1,0,0,0,0,0,0,1,
	0,1,0,0,0,0,0,0,1,0,1,1,1,0,0,0,
	0,1,1,1,0,0,0,1,1,0,0,1,0,1,1,1,
	0,0,1,0,0,0,1,0,1,0,0,0,1,1,1,0,
	0,1,0,1,0,0,0,1,0,0,1,1,0,0,0,0,
	1,1,1,1,1,0,0,0,1,1,0,0,1,0,1,0,
	1,1,0,1,1,1,1,1,1,0,0,1,0,0,0,0,
	0,1,1,1,1,1,0,0,1,0,0,0,0,0,0,1,
	0,0,0,1,1,1,0,0,0,0,1,0,0,1,0,0,
	0,0,1,0,0,0,1,1,1,0,1,1,0,1,0,0,
	0,1,1,0,0,0,1,0,1,0,0,0,1,0,1,0,

};

int TK[ROUND][16];
void Initialize_key(int Key1[], int Key2[], int r)
{


	int i = 0, j = 0;
	for (i = 0; i < r; i++)
	{
		if ((i % 2) == 0)
		{
			for (j = 0; j < 16; j++)
				TK[i][j] = Key1[j] ^ a[i][j];
		}
		else
		{
			for (j = 0; j < 16; j++)
				TK[i][j] = Key2[j] ^ a[i][j];
		}
	}

}

void Round(int r, int Stt[])
{

	int i = 0;
	for (i = 0; i < 16; i++)//SBox
		Stt[i] = S[Stt[i]];
	int Temp[16] = { 0 };
	if (r != 15)
	{
		for (int i = 0; i < 16; i++)//Permutation
			Temp[i] = Stt[P[i]];

		for (i = 0; i < 16; i = i + 4)//MixColumn
		{
			Stt[i] = Temp[i + 1] ^ Temp[i + 2] ^ Temp[i + 3];
			Stt[i + 1] = Temp[i] ^ Temp[i + 2] ^ Temp[i + 3];
			Stt[i + 2] = Temp[i] ^ Temp[i + 1] ^ Temp[i + 3];
			Stt[i + 3] = Temp[i] ^ Temp[i + 1] ^ Temp[i + 2];
		}
		for (i = 0; i < 16; i++)//AddTweakey
			Stt[i] ^= TK[r][i];
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
	for (k = 0; k < K_number; k++)/*秘钥数据量*/
	{
		//key = ((unsigned long long)rand() << 32) | rand();
		for (int i = 0; i < 4; i++) {
			key1 = (key1 << 16) | (rand() & 0xFFFF);
			key2 = (key2 << 16) | (rand() & 0xFFFF);
		}
		//printf("%lld, %llx \n", k, key);
		for (j = 0; j < 16; j++)
		{
			Key1[j] = (key1 >> (60 - 4 * j)) & 0xf;
			Key2[j] = (key2 >> (60 - 4 * j)) & 0xf;
		}
		Initialize_key(Key1, Key2, ROUND);
		data = 1ULL << P_number;
		//printf("%llx\n",data);
		num = 0;
		for (i = 0; i < data; i++)/*明文数据量*/
		{
			c = 0;
			for (int i = 0; i < 4; i++) {
				m = (m << 16) | (rand() & 0xFFFF);
			}
			//printf("%d, %llx \n", i, m);
			for (j = 0; j < 16; j++)//分解成16个半字节块
			{
				P[j] = (m >> (60 - 4 * j)) & 0xf;
			}
			for (r = 0; r < ROUND; r++) //加密过程
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
		bias = (double)num / data - 0.5;//偏差
		index = log2(fabs(bias));
		c2 = (double)2 * num / data - 1;//相关系数=2*bias
		c2 = c2 * c2;
		index_c2 = log2(fabs(c2));
		if (k % 50 == 0)
		{
			printf("num=%lld\n", num);
			//printf("bias=%lf\n", bias);
			//printf("index=%lf\n", index);
			printf("index_c2=%lf\n", index_c2);
		}
		BIAS += fabs(bias);//取绝对值后求平均
		C2 += fabs(c2);
	}
	BIAS = (long double)BIAS / K_number;
	C2 = (long double)C2 / K_number;
	printf("BIAS=%lf\n", BIAS);
	printf("INDEX_BIAS=%lf\n", log2(BIAS));//取绝对值后求平均
	printf("INDEX_C2=%lf\n", 2 * log2(BIAS) + 2);//取绝对值后求平均
	printf("C2=%lf\n", C2);
	printf("INDEX_C2=%lf\n", log2(C2));
}


int main()
{
	uint64_t alpha, beta;
	clock_t start = clock();
	/*线性*/
	alpha = 0x0005000505000500;
	beta = 0x5550555050555055;
	linear(alpha, beta);


	clock_t end = clock();
	double e = (double)(end - start) / CLOCKS_PER_SEC;
	printf("time: %.3f seconds.\n", e);
	return 0;
}