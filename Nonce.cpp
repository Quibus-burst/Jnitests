#include "Nonce.h"
#include "shabal.h"
#include "mshabal.h"



namespace AVX1
{
	void work_i(const size_t local_num, const unsigned long long loc_addr, const unsigned long long local_startnonce, const unsigned long long local_nonces)
	{
		unsigned long long nonce;
		unsigned long long nonce1;
		unsigned long long nonce2;
		unsigned long long nonce3;
		unsigned long long nonce4;

		char *final = new char[32];
		char *gendata = new char[16 + PLOT_SIZE];
		char *final1 = new char[32];
		char *final2 = new char[32];
		char *final3 = new char[32];
		char *final4 = new char[32];
		char *gendata1 = new char[16 + PLOT_SIZE];
		char *gendata2 = new char[16 + PLOT_SIZE];
		char *gendata3 = new char[16 + PLOT_SIZE];
		char *gendata4 = new char[16 + PLOT_SIZE];
		
		size_t len;
		shabal_context *x = new shabal_context[sizeof(shabal_context)];
		mshabal_context *mx = new mshabal_context[sizeof(mshabal_context)];

		
		for (unsigned long long n = 0; n < local_nonces;)
		{
			if (n + 4 <= local_nonces)
			{
				char *xv = (char*)&loc_addr;

				for (size_t i = 0; i < 8; i++)
				{
					gendata1[PLOT_SIZE + i] = xv[7 - i];
					gendata2[PLOT_SIZE + i] = xv[7 - i];
					gendata3[PLOT_SIZE + i] = xv[7 - i];
					gendata4[PLOT_SIZE + i] = xv[7 - i];					
				}

				nonce1 = local_startnonce + n + 0;
				nonce2 = local_startnonce + n + 1;
				nonce3 = local_startnonce + n + 2;
				nonce4 = local_startnonce + n + 3;
				char *xv1 = (char*)&nonce1;
				char *xv2 = (char*)&nonce2;
				char *xv3 = (char*)&nonce3;
				char *xv4 = (char*)&nonce4;
				for (size_t i = 8; i < 16; i++)
				{
					gendata1[PLOT_SIZE + i] = xv1[15 - i];
					gendata2[PLOT_SIZE + i] = xv2[15 - i];
					gendata3[PLOT_SIZE + i] = xv3[15 - i];
					gendata4[PLOT_SIZE + i] = xv4[15 - i];
				}

				for (size_t i = PLOT_SIZE; i > 0; i -= HASH_SIZE)
				{
					avx1_mshabal_init(mx, 256);
					len = PLOT_SIZE + 16 - i;
					if (len > HASH_CAP)   len = HASH_CAP;
					avx1_mshabal(mx, &gendata1[i], &gendata2[i], &gendata3[i], &gendata4[i], len);
					avx1_mshabal_close(mx, 0, 0, 0, 0, 0, &gendata1[i - HASH_SIZE], &gendata2[i - HASH_SIZE], &gendata3[i - HASH_SIZE], &gendata4[i - HASH_SIZE]);
				}

				avx1_mshabal_init(mx, 256);
				avx1_mshabal(mx, gendata1, gendata2, gendata3, gendata4, 16 + PLOT_SIZE);
				avx1_mshabal_close(mx, 0, 0, 0, 0, 0, final1, final2, final3, final4);

				// XOR with final
				for (size_t i = 0; i < PLOT_SIZE; i++)
				{
					gendata1[i] ^= (final1[i % 32]);
					gendata2[i] ^= (final2[i % 32]);
					gendata3[i] ^= (final3[i % 32]);
					gendata4[i] ^= (final4[i % 32]);
				}

				// Sort them:
				for (size_t i = 0; i < HASH_CAP; i ++)
				{
					memmove(&cache[i][(n + 0 + local_num*local_nonces) * SCOOP_SIZE], &gendata1[i * SCOOP_SIZE], SCOOP_SIZE);
					memmove(&cache[i][(n + 1 + local_num*local_nonces) * SCOOP_SIZE], &gendata2[i * SCOOP_SIZE], SCOOP_SIZE);
					memmove(&cache[i][(n + 2 + local_num*local_nonces) * SCOOP_SIZE], &gendata3[i * SCOOP_SIZE], SCOOP_SIZE);
					memmove(&cache[i][(n + 3 + local_num*local_nonces) * SCOOP_SIZE], &gendata4[i * SCOOP_SIZE], SCOOP_SIZE);
				}

				n += 4;
			}
			else 
			{
				_mm256_zeroupper();
				char *xv = (char*)&loc_addr;

				for (size_t i = 0; i < 8; i++)
					gendata[PLOT_SIZE + i] = xv[7 - i];
				
				nonce = local_startnonce + n;
				xv = (char*)&(nonce);

				for (size_t i = 8; i < 16; i++)
					gendata[PLOT_SIZE + i] = xv[15 - i];
				
				for (size_t i = PLOT_SIZE; i > 0; i -= HASH_SIZE)
				{
					shabal_init(x, 256);

					len = PLOT_SIZE + 16 - i;
					if (len > HASH_CAP)   len = HASH_CAP;

					shabal(x, &gendata[i], len);
					shabal_close(x, 0, 0, &gendata[i - HASH_SIZE]);
				}

				shabal_init(x, 256);
				shabal(x, gendata, 16 + PLOT_SIZE);
				shabal_close(x, 0, 0, final);

				
				// XOR with final
				for (size_t i = 0; i < PLOT_SIZE; i++)			gendata[i] ^= (final[i % HASH_SIZE]);

				// Sort them:
				for (size_t i = 0; i < HASH_CAP; i++)		memmove(&cache[i][(n + local_num*local_nonces) * SCOOP_SIZE], &gendata[i * SCOOP_SIZE], SCOOP_SIZE); 

				n++;
			}
			worker_status[local_num] = n;
		}
		delete[] final;
		delete[] gendata;
		delete[] gendata1;
		delete[] gendata2;
		delete[] gendata3;
		delete[] gendata4;
		delete[] final1;
		delete[] final2;
		delete[] final3;
		delete[] final4;
		delete[] x;
		delete[] mx;

		return;
	}
}

namespace SSE4
{
	void work_i(const size_t local_num, const unsigned long long loc_addr, const unsigned long long local_startnonce, const unsigned long long local_nonces)
	{
		unsigned long long nonce;
		unsigned long long nonce1;
		unsigned long long nonce2;
		unsigned long long nonce3;
		unsigned long long nonce4;

		char *final = new char[32];
		char *gendata = new char[16 + PLOT_SIZE];
		char *final1 = new char[32];
		char *final2 = new char[32];
		char *final3 = new char[32];
		char *final4 = new char[32];
		char *gendata1 = new char[16 + PLOT_SIZE];
		char *gendata2 = new char[16 + PLOT_SIZE];
		char *gendata3 = new char[16 + PLOT_SIZE];
		char *gendata4 = new char[16 + PLOT_SIZE];

		size_t len;
		shabal_context *x = new shabal_context[sizeof(shabal_context)];
		mshabal_context *mx = new mshabal_context[sizeof(mshabal_context)];

		for (unsigned long long n = 0; n < local_nonces;)
		{
			if (n + 4 <= local_nonces)
			{
				char *xv = (char*)&loc_addr;

				for (size_t i = 0; i < 8; i++) gendata1[PLOT_SIZE + i] = xv[7 - i];

				for (size_t i = PLOT_SIZE; i <= PLOT_SIZE + 7; ++i)
				{
					gendata2[i] = gendata1[i];
					gendata3[i] = gendata1[i];
					gendata4[i] = gendata1[i];
				}

				nonce1 = local_startnonce + n + 0;
				nonce2 = local_startnonce + n + 1;
				nonce3 = local_startnonce + n + 2;
				nonce4 = local_startnonce + n + 3;
				char *xv1 = (char*)&nonce1;
				char *xv2 = (char*)&nonce2;
				char *xv3 = (char*)&nonce3;
				char *xv4 = (char*)&nonce4;
				for (size_t i = 8; i < 16; i++)
				{
					gendata1[PLOT_SIZE + i] = xv1[15 - i];
					gendata2[PLOT_SIZE + i] = xv2[15 - i];
					gendata3[PLOT_SIZE + i] = xv3[15 - i];
					gendata4[PLOT_SIZE + i] = xv4[15 - i];
				}

				for (size_t i = PLOT_SIZE; i > 0; i -= HASH_SIZE)
				{
					sse4_mshabal_init(mx, 256);
					len = PLOT_SIZE + 16 - i;
					if (len > HASH_CAP)   len = HASH_CAP;
					sse4_mshabal(mx, &gendata1[i], &gendata2[i], &gendata3[i], &gendata4[i], len);
					sse4_mshabal_close(mx, 0, 0, 0, 0, 0, &gendata1[i - HASH_SIZE], &gendata2[i - HASH_SIZE], &gendata3[i - HASH_SIZE], &gendata4[i - HASH_SIZE]);
				}

				sse4_mshabal_init(mx, 256);
				sse4_mshabal(mx, gendata1, gendata2, gendata3, gendata4, 16 + PLOT_SIZE);
				sse4_mshabal_close(mx, 0, 0, 0, 0, 0, final1, final2, final3, final4);

				// XOR with final
				for (size_t i = 0; i < PLOT_SIZE; i++)
				{
					gendata1[i] ^= (final1[i % 32]);
					gendata2[i] ^= (final2[i % 32]);
					gendata3[i] ^= (final3[i % 32]);
					gendata4[i] ^= (final4[i % 32]);
				}

				// Sort them:
				for (size_t i = 0; i < HASH_CAP; i++)
				{
					memmove(&cache[i][(n + 0 + local_num*local_nonces) * SCOOP_SIZE], &gendata1[i * SCOOP_SIZE], SCOOP_SIZE);
					memmove(&cache[i][(n + 1 + local_num*local_nonces) * SCOOP_SIZE], &gendata2[i * SCOOP_SIZE], SCOOP_SIZE);
					memmove(&cache[i][(n + 2 + local_num*local_nonces) * SCOOP_SIZE], &gendata3[i * SCOOP_SIZE], SCOOP_SIZE);
					memmove(&cache[i][(n + 3 + local_num*local_nonces) * SCOOP_SIZE], &gendata4[i * SCOOP_SIZE], SCOOP_SIZE);
				}

				n += 4;
			}
			else
			{
				char *xv = (char*)&loc_addr;

				gendata[PLOT_SIZE] = xv[7]; gendata[PLOT_SIZE + 1] = xv[6]; gendata[PLOT_SIZE + 2] = xv[5]; gendata[PLOT_SIZE + 3] = xv[4];
				gendata[PLOT_SIZE + 4] = xv[3]; gendata[PLOT_SIZE + 5] = xv[2]; gendata[PLOT_SIZE + 6] = xv[1]; gendata[PLOT_SIZE + 7] = xv[0];

				nonce = local_startnonce + n;
				xv = (char*)&(nonce);

				gendata[PLOT_SIZE + 8] = xv[7]; gendata[PLOT_SIZE + 9] = xv[6]; gendata[PLOT_SIZE + 10] = xv[5]; gendata[PLOT_SIZE + 11] = xv[4];
				gendata[PLOT_SIZE + 12] = xv[3]; gendata[PLOT_SIZE + 13] = xv[2]; gendata[PLOT_SIZE + 14] = xv[1]; gendata[PLOT_SIZE + 15] = xv[0];

				for (size_t i = PLOT_SIZE; i > 0; i -= HASH_SIZE)
				{
					shabal_init(x, 256);

					len = PLOT_SIZE + 16 - i;
					if (len > HASH_CAP)   len = HASH_CAP;

					shabal(x, &gendata[i], len);
					shabal_close(x, 0, 0, &gendata[i - HASH_SIZE]);
				}

				shabal_init(x, 256);
				shabal(x, gendata, 16 + PLOT_SIZE);
				shabal_close(x, 0, 0, final);


				// XOR with final
				for (size_t i = 0; i < PLOT_SIZE; i++)			gendata[i] ^= (final[i % HASH_SIZE]);

				// Sort them:
				for (size_t i = 0; i < HASH_CAP; i++)		memmove(&cache[i][(n + local_num*local_nonces) * SCOOP_SIZE], &gendata[i * SCOOP_SIZE], SCOOP_SIZE);

				n++;
			}
			worker_status[local_num] = n;
		}
		delete[] final;
		delete[] gendata;
		delete[] gendata1;
		delete[] gendata2;
		delete[] gendata3;
		delete[] gendata4;
		delete[] final1;
		delete[] final2;
		delete[] final3;
		delete[] final4;
		delete[] x;
		delete[] mx;

		return;
	}
}