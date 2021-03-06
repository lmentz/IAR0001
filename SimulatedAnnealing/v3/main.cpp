#include <bits/stdc++.h>

using namespace std;

typedef vector<int> vi;
typedef vector<vi> vvi;

double TEMP_INICIAL = 100;
double TEMP_FINAL = 5;
double NUM_ITERACOES = 250000;

struct Clausula {
    vector<int> variaveis;

    Clausula(){}
    Clausula(vector<int> &_variaveis){
        variaveis = _variaveis;
    }

	int satisfeito(vector<bool> &conf){
		int sol = 0;
		for(int v : variaveis){
			if(v < 0){
				sol = sol || (not conf[abs(v) - 1]);
			} else {
				sol = sol || conf[abs(v) - 1];
			}
		}
		return sol;
	}
};

struct Solucao {
    vector<bool> conf;
    int num_Sat;

    Solucao(){
        num_Sat = 0;
    }
};

random_device rd;
mt19937 engine{rd()};
uniform_real_distribution<double> dist{0.0, 1.0};

double CS0 (int iteracao){
    return TEMP_INICIAL - iteracao * (TEMP_INICIAL - TEMP_FINAL)/(double)NUM_ITERACOES;
}
double CS1 (int iteracao){
    return (TEMP_INICIAL * pow(TEMP_FINAL / TEMP_INICIAL, iteracao / NUM_ITERACOES));
}

// COM PROBLEMA:
double CS2 (int iteracao){
    double A = (TEMP_INICIAL - TEMP_FINAL) * (NUM_ITERACOES + 1) / NUM_ITERACOES;
    double B = TEMP_INICIAL - A;
    return  ((A / (double)(iteracao + 1)) + B);
}

// COM PROBLEMA:
double CS3 (int iteracao){
    double A = log(TEMP_INICIAL - TEMP_FINAL)/log(NUM_ITERACOES);
    return (TEMP_INICIAL - pow(iteracao, A));
}

// COM PROBLEMA:
double CS4 (int iteracao){
    return ((TEMP_INICIAL - TEMP_FINAL) / (1.0 + exp(3 * (iteracao - NUM_ITERACOES / 2.0))) + TEMP_FINAL);
}

double CS5 (int iteracao){
    return (0.5 * (TEMP_INICIAL - TEMP_FINAL) * (1 + cos((iteracao * acos(-1)) / NUM_ITERACOES)) + TEMP_FINAL);
}
double CS6 (int iteracao){
    return (0.5 * (TEMP_INICIAL - TEMP_FINAL) * (1 - tanh((10.0 * iteracao) / NUM_ITERACOES - 5.0)) + TEMP_FINAL);
}
double CS7 (int iteracao){
    return ((TEMP_INICIAL - TEMP_FINAL) / (cosh((10.0 * iteracao) / NUM_ITERACOES)) + TEMP_FINAL);
}
double CS8 (int iteracao){
    double A = (1 / NUM_ITERACOES) * (log(TEMP_INICIAL / TEMP_FINAL));
    return (TEMP_INICIAL * exp(-A * iteracao));
}
double CS9 (int iteracao){
    double A = (1 / pow(NUM_ITERACOES, 2)) * (log(TEMP_INICIAL / TEMP_FINAL));
    return (TEMP_INICIAL * exp(-A * pow(iteracao, 2)));
}

vector<bool> init(int num_X){
    vector<bool> vet;
    for(int i = 0; i < num_X; i++){
        double tmp = dist(engine);
        vet.push_back(tmp >= 0.5);
    }
    return vet;
}

int eval(vector<Clausula> &SAT, vector<bool> &conf){
	int res = 0;
	for(Clausula c : SAT){
		if(c.satisfeito(conf)){
			res++;
		}
	}
	return res;
}

vector<bool> nova_Conf(vector<bool> &conf){
	vector<bool> ret;
	for(int i = 0; i < conf.size(); i++){
		double randi = dist(engine);
		if(randi < 0.01){
			ret.push_back(!conf[i]);
		} else {
			ret.push_back(conf[i]);
		}
	}
	return ret;
}

int main(int argc, char const *argv[]) {
	string s, fpath;
    int num_X, num_Clausulas, resfriamento;
	ifstream entrada;
	if (argc <= 4){
		printf("4 argumentos: entrada CS# temp_inicial temp_final\n");
		return 1;
	}
	
	sscanf(argv[2], "%d", &resfriamento);
	sscanf(argv[3], "%lf", &TEMP_INICIAL);
	sscanf(argv[4], "%lf", &TEMP_FINAL);

	// Teste das fórmulas de temperatura:
	// for (int i = 0; i < NUM_ITERACOES; i++){
	// 	double temp;
	// 	switch (resfriamento){
	// 		case 0: temp = CS0(i); break;
	// 		case 1: temp = CS1(i); break;
	// 		case 2: temp = CS2(i); break;
	// 		case 3: temp = CS3(i); break;
	// 		case 4: temp = CS4(i); break;
	// 		case 5: temp = CS5(i); break;
	// 		case 6: temp = CS6(i); break;
	// 		case 7: temp = CS7(i); break;
	// 		case 8: temp = CS8(i); break;
	// 		case 9: temp = CS9(i); break;
	// 		default: temp = CS0(i); break;
	// 	}
	// 	printf("%d %6.4lf\n", i, temp);
	// }


// /*
	//
	// INICIO leitura da entrada
	entrada.open(argv[1]);
	if (!entrada.is_open()){
		printf("Erro ao abrir arquivo \"%s\"\n", argv[1]);
	}

	// cin >> s >> s;
	entrada >> s >> s;
	// cin >> num_X >> num_Clausulas;
	entrada >> num_X >> num_Clausulas;
	vector<Clausula> SAT;
    for(int i = 0; i < num_Clausulas; i++){
        vector<int> vet;
        for(int j = 0; j < 3; j++){
            int x;
			// cin >> x;
            entrada >> x;
            vet.push_back(x);
        }
		int t;
		// cin >> t; // ler o trailing zero
        entrada >> t; // ler o trailing zero
		SAT.push_back(Clausula(vet));
    }
	entrada.close();
	// FIM leitura da entrada
	//

    vector<bool> conf = init(num_X);
	int retT = eval(SAT, conf);

	double temp = TEMP_INICIAL;

    Solucao solucaoFinal;
	solucaoFinal.num_Sat = retT;
	solucaoFinal.conf = conf;

    for(int i = 1; i <= NUM_ITERACOES; i++){
		switch (resfriamento){
			case 0: temp = CS0(i); break;
			case 1: temp = CS1(i); break;
			case 2: temp = CS2(i); break;
			case 3: temp = CS3(i); break;
			case 4: temp = CS4(i); break;
			case 5: temp = CS5(i); break;
			case 6: temp = CS6(i); break;
			case 7: temp = CS7(i); break;
			case 8: temp = CS8(i); break;
			case 9: temp = CS9(i); break;
			default: temp = CS0(i); break;
		}
		// cerr << temp << endl;
		// temp = CS6(i);
		vector<bool> n_conf = nova_Conf(conf);
        int ret = eval(SAT, n_conf);
		if(ret > retT){
			conf = n_conf;
			retT = ret;
		} else {
			double delta = abs(ret - retT);
			double prob = exp(-delta / temp);
			cerr << temp << ": " << solucaoFinal.num_Sat << endl;
			double randi = dist(engine);
			if(randi <= prob){
				//cerr << oi++ << endl;
				conf = n_conf;
				retT = ret;
			}
		}
		if(retT > solucaoFinal.num_Sat){
			solucaoFinal.conf = conf;
			solucaoFinal.num_Sat = retT;
		}
		printf("%d %d\n", i, retT);
		//if(ret == num_Clausulas) {melhormelhor = i; break;}
    }

	cerr << "####################################################\n" << endl;
	cerr << "Melhor solucao encontrada: " << solucaoFinal.num_Sat << endl;
	for (int i = 0; i < solucaoFinal.conf.size(); i++){
		cerr << solucaoFinal.conf[i] ? "_":"1";
	} cerr << endl;
// */

    return 0;
}
