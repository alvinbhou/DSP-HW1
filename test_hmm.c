#include "hmm.h"
#include <math.h>
#include <iostream>
using namespace std;




int main()
{

	HMM hmms[5];
	load_models( "modellist.txt", hmms, 5);
	for(int i = 0; i < 5; ++i){
		for(int j = 0; j < 5; ++j){
			cout << hmms[4].initial[j] << " ";
		}
	}
	dump_models( hmms, 5);

	// HMM hmm_initial;
	// loadHMM( &hmm_initial, "model_init.txt" );
	// dumpHMM( stderr, &hmm_initial );
	// cout << hmm_initial.observ_num;

	

	printf("%f\n", log(1.5) ); // make sure the math library is included
	return 0;
}
