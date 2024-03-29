#include "hmm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>
#include <vector>
#include <string>
#include <assert.h>
using namespace std;

#define STATE_NUM 6
#define OBSERV_NUM 6
#define MAX_TIME 50
#define MODEL_NUM 5
#define SAMPLE_SIZE 12000

// int *forwarding(HMM hmm);

class MyClass{
    private:
        int time_length;
        int state_length;
    public:
        double alpha[MAX_TIME+2][STATE_NUM];
        double beta[MAX_TIME+2][STATE_NUM];
        double gamma[MAX_TIME+2][STATE_NUM];
        double e[MAX_TIME+2][STATE_NUM][STATE_NUM];
        int forward(HMM hmm, int o[MAX_TIME]);
        int backward(HMM hmm, int o[MAX_TIME]);
        int computeGamma();
        int computeEpsilon(HMM hmm, int o[MAX_TIME]);
        int display();
        MyClass(int,int);   
        MyClass();
};

class MyModel{
    private:
        int time_length;
        int state_length;
    public:
        double pi[STATE_NUM];
        double a[STATE_NUM][STATE_NUM];
        double b[OBSERV_NUM][STATE_NUM];       
        int BaumWelch(MyClass c[SAMPLE_SIZE] , int o[][MAX_TIME]);
        MyModel(){
            this->time_length = MAX_TIME;
            this->state_length = STATE_NUM;
            for(int i = 0; i < STATE_NUM; ++i)
                pi[i] = 0.0;
            for(int i = 0; i < STATE_NUM; ++i)
                for(int j = 0; j < STATE_NUM; ++j)
                    a[i][j] = 0.0;
            for(int i = 0; i < OBSERV_NUM; ++i)
                for(int j = 0; j < STATE_NUM; ++j)
                    b[i][j] = 0.0;
        }
         void display(){
            cout << "pi" << endl;
            for(int i = 0; i < STATE_NUM; ++i)
                cout << pi[i] << " ";
            cout << endl;
            cout << "a" << endl;
            for(int i = 0; i < STATE_NUM; ++i){
                for(int j = 0; j < STATE_NUM; ++j){
                     cout << a[i][j] << " ";
                }
                cout << endl;                   
            }
            cout << "b" << endl;
            for(int i = 0; i < OBSERV_NUM; ++i){
                for(int j = 0; j < STATE_NUM; ++j){
                    cout << b[i][j] << " ";
                }
                cout << endl;
            }
         }
};

int observation[SAMPLE_SIZE][MAX_TIME];
MyClass c[SAMPLE_SIZE];
void loadSeq(char*);
int char2int(char);
void save2HMM(string output_model, MyModel m);

// our training and final HMM
HMM hmm[1];

int main(int argc,char* argv[]){
    int iteration = atoi(argv[1]);
    string model_init(argv[2]);
    string output_model(argv[4]); 
   
    // ostringstream ss;
    // ss << iteration;
    // output_model =  ss.str() + output_model;
    
    HMM init_hmm;
	loadHMM( &init_hmm, argv[2]);
	// dumpHMM( stderr, &init_hmm);
    loadSeq(argv[3]);
    
    // compute the stuff
    for(int sid = 0; sid < SAMPLE_SIZE;  ++sid){
        c[sid].forward(init_hmm, observation[sid]);
        c[sid].backward(init_hmm, observation[sid]);
        c[sid].computeGamma();
        c[sid].computeEpsilon(init_hmm, observation[sid]);
    }
    // cout << "----Init----Forward-Backward------------\n";
    
    // compute the initial model
    MyModel m; 
    m.BaumWelch(c, observation);      
    // cout << "----Init------B-W------------------\n";
    save2HMM(output_model, m);

    // iterate 
    for(int i = 1; i < iteration; ++i){
        for(int sid = 0; sid < SAMPLE_SIZE;  ++sid){
            c[sid].forward(hmm[0], observation[sid]);
            c[sid].backward(hmm[0], observation[sid]);
            c[sid].computeGamma();
            c[sid].computeEpsilon(hmm[0], observation[sid]);
        }
        // cout << "----" << i + 1 <<"----Forward-Backward-----\n";
        MyModel m; 
        m.BaumWelch(c, observation);
        // cout << "----" << i + 1 <<"----B-W-----\n";
        // cout << i + 1 << endl;
        save2HMM(output_model, m);      
    }    

    
    // save2HMM(output_model, m);   
    
    dump_models( hmm, 1);
    // cout << "dump done" << endl;
    /* save done */
    
    return 0;
}

void save2HMM(string output_model, MyModel m){
    /* Save to HMM struct */
    // define model name 
    hmm[0].model_name = new char[20];
    strcpy(hmm[0].model_name, output_model.c_str());       
    
    hmm[0].state_num = STATE_NUM;
    hmm[0].observ_num = OBSERV_NUM;          
    for(int i = 0; i < STATE_NUM; ++i)
        hmm[0].initial[i] = m.pi[i];         
                    
    for(int i = 0; i < STATE_NUM; ++i)
        for(int j = 0; j < STATE_NUM; ++j)
            hmm[0].transition[i][j] = m.a[i][j];      
            
    for(int i = 0; i < OBSERV_NUM; ++i)
        for(int j = 0; j < STATE_NUM; ++j)
                hmm[0].observation[i][j] = m.b[i][j];    

}


int MyModel::BaumWelch(MyClass c[SAMPLE_SIZE], int o[][MAX_TIME]){  
    // init pi
    for(int i = 0; i < STATE_NUM; ++i){
        for(int s = 0; s < SAMPLE_SIZE; ++s){
            pi[i] +=  c[s].gamma[0][i];
        }
        pi[i] /= SAMPLE_SIZE;
    }
    // init a
    for(int i = 0; i < STATE_NUM; ++i){
        for(int j = 0; j < STATE_NUM; ++j){
            double numerator = 0.0, denominator = 0.0;
            for(int s = 0; s < SAMPLE_SIZE; ++s){
                for(int t = 0; t < MAX_TIME-1; ++t){
                     numerator += c[s].e[t][i][j];
                     denominator += c[s].gamma[t][i];
                }
            }
            assert(denominator!=0);
            a[i][j] = numerator / denominator;
        }
    }
    // init b
    for(int k = 0; k < OBSERV_NUM; ++k){
        for(int j = 0; j < STATE_NUM; ++j){
            double numerator = 0.0, denominator = 0.0;
            for(int s = 0; s < SAMPLE_SIZE; ++s){
                for(int t = 0; t < MAX_TIME; ++t){
                    if(o[s][t] == k)
                        numerator += c[s].gamma[t][j];
                    denominator += c[s].gamma[t][j];
                }
            }
            assert(denominator!=0);
            b[k][j] = numerator / denominator;
        }
    }
    
    

}




MyClass::MyClass(){
    this->time_length = MAX_TIME;
    this->state_length = STATE_NUM;
}

MyClass::MyClass(int t,int s){
    this->time_length = t;
    this->state_length = s;
}

int MyClass::forward(HMM hmm, int o[MAX_TIME]){
    // init
    for(int i = 0; i < this->state_length; ++i){
        // i is now state
        alpha[0][i] = hmm.initial[i] * hmm.observation[o[0]][i];
    }
    // induction
    for(int t = 1; t < this->time_length; ++t){
        for(int j = 0; j < this->state_length; ++j){
            // current state j
            double p = 0.0;
            for(int i = 0; i < this->state_length; ++i){
                p += alpha[t-1][i] * hmm.transition[i][j];
            }
            alpha[t][j] = p * hmm.observation[o[t]][j];
        }
    }
    /* compute P(O|lambda), optional */
    // double p = 0.0;
    // for(int i = 0; i < this->state_length; ++i){
    //     p += alpha[this->time_length-1][i];
    // }
    // cout << "Forward p " << p << endl;

}

int MyClass::backward(HMM hmm, int o[MAX_TIME]){
    // init
    for(int i = 0; i < this->state_length; ++i){
        // i is now state
        beta[this->time_length-1 ][i] = 1.0;
    }
    // induction
    for(int t = this->time_length-2; t >= 0 ; --t){
        for(int i = 0; i < this->state_length; ++i){
            // current state i
            double p = 0.0;
            for(int j = 0; j < this->state_length; ++j){
                p += hmm.transition[i][j] * hmm.observation[o[t+1]][j];
            }
            beta[t][i] = p;
        }
    }
    /* compute P(O|lambda), optional */
    // double p = 0.0;
    // for(int j = 0; j < this->state_length; ++j){
    //     p += hmm.initial[j] * hmm.observation[o[0]][j] * beta[0][j];
    // }
    //  cout << "Backward p " << p << endl;
}

int MyClass::computeGamma(){
    for (int t = 0; t < this->time_length; ++t){
        double p = 0.0;
        for(int i = 0; i < this->state_length; ++i){
            p += alpha[t][i] * beta[t][i];
            // cout << t << " " << i << " " << p << endl;
        }           
       
        assert(p != 0); 
        for (int i = 0; i < this->state_length; ++i)
            gamma[t][i] = alpha[t][i] * beta[t][i] / p;
    }
}

int MyClass::computeEpsilon(HMM hmm, int o[MAX_TIME]){
    for(int t = 0; t < this->time_length-1; ++t){
        double p = 0.0;
        for(int i = 0; i < this->state_length; ++i){
            for(int j = 0; j < this->state_length; ++j){
                p += alpha[t][i] * hmm.transition[i][j] * hmm.observation[o[t+1]][j] * beta[t+1][j];
            }
        }
        assert(p != 0);

        for(int i = 0; i < this->state_length; ++i){
            for(int j = 0; j < this->state_length; ++j){             
                e[t][i][j] = alpha[t][i] * hmm.transition[i][j] * hmm.observation[o[t+1]][j] * beta[t+1][j] / p;                
            }

        }
    }
}

int MyClass::display(){
    for(int t = 0; t < this->time_length; ++t){
        for(int i = 0; i < this->state_length; ++i){
            cout << t << " " << alpha[t][i] << endl;
            cout << t << " " << beta[t][i] << endl;
        }
        cout << endl;
    }
}

void loadSeq(char* file){
    ifstream inFile(file);
    string line;
    int t = 0;
    while(getline(inFile, line)){
        for(int i = 0; i < line.length(); ++i){        
            observation[t][i] = char2int(line[i]);
        }
        t++;
    }
    return;
}

int char2int(char s){ 
    int x = s - 65;
    return x;
}

