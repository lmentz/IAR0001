#include <bits/stdc++.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "Item.hpp"

using namespace std;

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

// Variáveis globais
// Gerador de numeros aleatorios
random_device g_rd;
mt19937 g_e(g_rd());
uniform_real_distribution<> g_dist(0, 1);

// Essa parte pode ser atualizada com argumentos passados pelo terminal
int NUM_FORMIGAS = 200, TAM_MAPA = 100, NUM_FORMIGAS_MORTAS = 5000;
int W_WINDOW = 600, H_WINDOW = W_WINDOW;
int NUM_COR;
int VISAO = 1;

// Tamanho da vizinhanca que a formiga enxerga
int ALCANCE = (2 * VISAO + 1) * (2 * VISAO + 1) - 1;
void updateAlcance(){ALCANCE = (2 * VISAO + 1) * (2 * VISAO + 1) - 1;}
void setVisao(int v){VISAO = v; updateAlcance();}
// Cores: White,
//        Black, Maroon, Green, Olive, Navy, Purple, Teal, Orange,
//        Gray, Red, Lime, Yellow, Blue, Fuchsia, Aqua, Brown.
// sf::Color cores[] = {{0xFFFFFFFF},
// 					 {0x000000FF}, {0x880000FF}, {0x008800FF}, {0x888800FF},
// 					 {0x000088FF}, {0x880088FF}, {0x008888FF}, {0xFF8800FF},
// 				 	 {0x888888FF}, {0xFF0000FF}, {0x00FF00FF}, {0xFFFF00FF},
// 				 	 {0x0000FFFF}, {0xFF00FFFF}, {0x0088FFFF}, {0xF4A460FF}};
sf::Color cores[] = {sf::Color(0xFFFFFFFF),
					 sf::Color(0x000000FF), sf::Color(0x880000FF),
					 sf::Color(0x008800FF), sf::Color(0xAA8800FF),
					 sf::Color(0x000088FF), sf::Color(0x880088FF),
					 sf::Color(0x008888FF), sf::Color(0xFF8800FF),
				 	 sf::Color(0x888888FF), sf::Color(0xFF0000FF),
					 sf::Color(0x00FF00FF), sf::Color(0xFFFF00FF),
				 	 sf::Color(0x0000FFFF), sf::Color(0xFF00FFFF),
					 sf::Color(0x0088FFFF), sf::Color(0xF4A460FF)};

vector<Item *> itens;
// sf::Color cores[] = {sf::Color(255,255,255,255)
//
// }
bool running;
mutex globmut;
// Fim variáveis globais

class Mapa {
	private:
		int n, m;
		vector<vector<Item *> > mapa;
		mutex **mmut;
		map<pair<int, int>, int> ff;
		double ALPHA, SIGMA;
	public:
		// Construtor
		Mapa(int n, int m){
			this -> n = n;
			this -> m = m;
			mapa.assign(n, vector<Item *>(m, NULL));
			mmut = new mutex*[n];
			for (int a=0;a<n;a++)
				mmut[a] = new mutex[m];
			SIGMA = 4;
			ALPHA = 2;
		}

		// Retorna o numero de linhas do mapa
		int getN(){
			return this -> n;
		}
		// Retorna o numero de colunas do mapa
		int getM(){
			return this -> m;
		}

		void setSigma(double sig){
			this -> SIGMA = sig;
		}
		void setAlpha(double alp){
			this -> ALPHA = alp;
		}

		// Preenche o mapa com formigas mortas em posicoes aleatorias
		void initMapa(vector<Item*> &itens){
			for(Item *&i : itens){
				int y = rand() % this -> n;
				int x = rand() % this -> m;
				while(this -> mapa[y][x]){
					y = rand() % this -> n;
					x = rand() % this -> m;
				}
				mapa[y][x] = i;
			}
		}

		// Bloqueia posição do mapa para controlar concorrência
		void lockPos(int i, int j){
			this -> mmut[i][j].lock();
		}

		// Desbloqueia posição do mapa
		void unlockPos(int i, int j){
			this -> mmut[i][j].unlock();
		}

		// Retorna o estado de uma celula do mapa
		Item * getPos(int i, int j){
			return this -> mapa[i][j];
		}
		// Seta um valor numa posicao especifica do mapa
		void setPos(int i, int j, Item * v){
			this -> mapa[i][j] = v;
		}

		// Para imprimir o mapa na main
		vector<vector<Item *> > &getMapa(){
			return mapa;
		}

		// Verifica se as coordenadas estão dentro do mapa
		int valid(int y, int x){
			return (y >= 0 && x >= 0 && y < this -> n && x < this -> m);
		}

		// Retorna quantas formigas mortas tem na vizinhanca da formiga atual
		double getVizinhanca(int i, int j, Item *me){
			double cnt = 0;
			int numViz = 0;
			int flag = 1;
			for(int y = -VISAO; y <= VISAO; y++){
				for(int x = -VISAO; x <= VISAO; x++){
					int yy = y + i; yy %= n; if (yy < 0) yy += n;
					int xx = x + j; xx %= m; if (xx < 0) xx += m;
					if(this -> mapa[yy][xx] != NULL){
						numViz++;
						double aux = (1 - this -> mapa[yy][xx] -> calcDist(me) / this -> ALPHA);
						if(aux < 0) flag = 0;
						cnt += aux;
					}
				}
			}
			// cout << cnt << " " << numViz << endl;
			if(cnt < 0 or !flag) return 0.0;
			return cnt / pow(this -> SIGMA, 2);
		}

		// Seta um mapa dizendo onde tem formiga viva
		void setFF(vector<pair<int,pair<int,int> > > &pos){
			this -> ff.clear();
			for(pair<int,pair<int, int> > p : pos){
				if(p.first == true){
					ff[p.second] = 2;
				} else {
					ff[p.second] = 1;
				}
			}
		}

		// Printa o mapa em terminal
		void printMapa(){
			for(int i = 0; i < this -> n; i++){
				if(!i){
					for(int k = 0; k < this -> n + 2; k++) cout << "*";
					cout << endl;
				}
				for(int j = 0; j < this -> m; j++){
					if(!j) cout << "*";
					if(ff.count({i, j}) > 0){
						if(ff[{i,j}] == 2) cout << 3;
						else cout << 2;
					} else {
						if(mapa[i][j] == 0) cout << " ";
						else cout << mapa[i][j];
					}
				}
				cout << "*\n";
			}
			for(int k = 0; k < this -> n + 2; k++)
				cout << "*";
			cout << endl;
		}
};

Mapa *mapa;

class Formiga{
private:
	int x, y;
	Item *carry;
public:

	Formiga()
	{
		this->x = rand() % mapa -> getN();
		this->y = rand() % mapa -> getM();
		this->carry = NULL;
	}

	// Seta a posicao da formiga
	void setPos(int y, int x){
		this -> y = y;
		this -> x = x;
	}
	// Retorna a posicao da formiga
	pair<int,int> getPos(){
		return {this -> y, this -> x};
	}

	// Seta se esta carregando ou nao uma formiga
	void setCarry(Item *item){
		this -> carry = item;
	}
	// Retorna se a formiga esta ou nao carregando outra formiga
	Item * getCarry(){
		return this -> carry;
	}

	double getRandom();

	void validatePos();
	void move();
	void runStep();
};

double Formiga::getRandom(){
	return g_dist(g_e);
}

void Formiga::validatePos(){
	this->y %= mapa->getN(); if (this->y < 0) this->y += mapa->getN();
	this->x %= mapa->getM(); if (this->x < 0) this->x += mapa->getM();
}

void Formiga::move()
{
	// Mover-se em qualquer direção (se for para fora do ALCANCE
	//  apenas fica na mesma posição do quadro)
	int dir = int(this->getRandom() * 9);
	// 0,1,2,3,4,5,6,7,8 -> Cima,Direita,Baixo,Esquerda,CE,CD,BD,BE,Parado
	switch (dir) {
		case 0: this->setPos(y-1, x); break;
		case 1: this->setPos(y, x+1); break;
		case 2: this->setPos(y+1, x); break;
		case 3: this->setPos(y, x-1); break;
		case 4: this->setPos(y-1, x-1); break;
		case 5: this->setPos(y-1, x+1); break;
		case 6: this->setPos(y+1, x+1); break;
		case 7: this->setPos(y+1, x-1); break;
		case 8: break;
	}
	this->validatePos();
}

void Formiga::runStep()
{
	// Lógica para pegar
	// Se (não carrego) E (estou sobre formiga morta)
	if (!this->carry){
		mapa -> lockPos(y, x);
		if (mapa -> getPos(y, x) != NULL){
			// obter vizinhança para cálculo de probabilidade
			double viz = mapa -> getVizinhanca(y, x, mapa -> getPos(y, x));
			// cout << "viz: " << viz << endl;
			double prob;
			if(viz <= 1){
				prob = 1.0;
			} else {
				prob = 1.0 / pow(viz, 2);
			}
			// cout << "Pick: " << prob << endl;
			if (this -> getRandom() < prob) {
				setCarry(mapa -> getPos(y, x));
				mapa -> setPos(y, x, NULL);
			}
		}
		mapa -> unlockPos(y, x);
	}

	// Lógica para largar
	// Se (carrego) E (não estou sobre formiga morta)
	else if (this->carry){
		mapa -> lockPos(y, x);
		if (mapa -> getPos(y, x) == NULL){
			// obter vizinhança para cálculo de probabilidade
			double viz = mapa->getVizinhanca(y, x, this -> carry);
			// cout << "viz: " << viz << endl;
			double prob;
			if(viz > 1){
				prob = 1.0;
			} else {
				prob = pow(viz, 4);
			}
			// cout << "Release: " << prob << endl;
			if (this -> getRandom() < prob) {
				mapa -> setPos(y, x, this -> carry);
				setCarry(NULL);
			}
		}
		mapa -> unlockPos(y, x);
	}

	// Sempre se move após sua decisão, qualquer tenha sido.
	this->move();
}

vector<Formiga *> formigas;
void runFormigas() {
	while (running == true) {
		globmut.lock();
		#pragma omp parallel for
		for (int i = 0; i < formigas.size(); i++) {
			formigas[i] -> runStep();
		}
		globmut.unlock();
	}
}

// Descrição e análise do cenário:
// https://drive.google.com/open?id=18H2shg9uhS-mFW55CrwX-s6RQRUVXITYbvIdNhsiTkM
int main() {
	srand(time(NULL));

	cin >> TAM_MAPA;
	cin >> NUM_FORMIGAS_MORTAS;
	cin >> NUM_FORMIGAS;
	cin >> VISAO;
	double sig, alp;
	int numItems;
	cin >> numItems;
	cin >> sig >> alp;
	for(int i = 0; i < NUM_FORMIGAS_MORTAS; i++){
		double x;
		vector<double> tmp;
		for(int j = 0; j < numItems; j++){
			cin >> x;
			tmp.push_back(x);
		}
		int tipo;
		cin >> tipo;
		itens.push_back(new Item(cores[tipo], tmp));
	}
	setVisao(VISAO);
	mapa = new Mapa(TAM_MAPA, TAM_MAPA);
	mapa -> initMapa(itens);
	mapa -> setSigma(sig);
	mapa -> setAlpha(alp);

	// Cria as formigas
	for(int i = 0; i < NUM_FORMIGAS; i++){
		formigas.push_back(new Formiga());
	}

	// Cria a janela
	sf::RenderWindow window(sf::VideoMode(W_WINDOW, H_WINDOW), "Formigavel :D",
							sf::Style::Titlebar | sf::Style::Close);

	// Set o framerate para 24 (cinema carai)
	window.setFramerateLimit(1);
	window.setVerticalSyncEnabled(false);

	// Definições para desenho
	float D_W_SPACE = W_WINDOW / (float)TAM_MAPA;
	float D_H_SPACE = H_WINDOW / (float)TAM_MAPA;
	float D_RAD = (MIN(D_H_SPACE, D_W_SPACE) * 0.5);
	sf::RectangleShape dot(sf::Vector2f(D_W_SPACE, D_H_SPACE));
	sf::CircleShape form(D_RAD);
	dot.setOrigin(sf::Vector2f(0,0));
	form.setOrigin(sf::Vector2f(0,0));
	vector<vector<Item *> > _mapa;

	// Thread de processamento de formigas
	running = true;
	thread runThread (runFormigas);

	// Loop principal
	while(window.isOpen()){
		sf::Event event;
		while (window.pollEvent(event)){
			// Fecha a janela
			if (event.type == sf::Event::Closed){
				running = false;
				/*
				int cnt = 0;
				for(int i = 0; i < TAM_MAPA; i++)
					for(int j = 0; j < TAM_MAPA; j++)
						if(mapa -> getPos(i, j) > 0) cnt++;
				for(int i = 0; i < NUM_FORMIGAS; i++)
					if (formigas[i] -> getCarry() > 0) cnt++;
				cout << cnt << endl;
				*/
				window.close();
				continue;
			}
		}
		// Limpa fundo
		window.clear(sf::Color(40,40,40,255));

		// Desenha o grid
		//drawGrid(window, mapa, formigas);
		globmut.lock();
		_mapa = mapa -> getMapa();
		// Desenhando o mapa com as formigas mortas e os espaços livres
		for(int j = 0; j < TAM_MAPA; j++){
			for(int i = 0; i < TAM_MAPA; i++){
				dot.setPosition(D_W_SPACE * i, D_H_SPACE * j);
				if(_mapa[i][j] != NULL){
					dot.setFillColor(_mapa[i][j] -> getColor());
				} else dot.setFillColor(sf::Color::White);
				// if(_mapa[i][j] == 0){
					// dot.setFillColor(sf::Color(232,232,232));
				// } else {//if(_mapa[i][j] == 1){
					// dot.setFillColor
					// if(_mapa[i][j] == 1){
					// 	dot.setFillColor(cores[]);
					// } else {
					// 	dot.setFillColor(sf::Color(0,128,255));
					// }
				// }
				window.draw(dot);
			}
		}
		// Desenhando as formigas vivas (Green -> Carregando algo | Red -> Carregando nada)
		// for(int i = 0; i < NUM_FORMIGAS; i++){
		// 	pair<int,int> pos = formigas[i] -> getPos();
		// 	form.setPosition(D_W_SPACE * pos.first, D_H_SPACE * pos.second);
		// 	if(formigas[i] -> getCarry() != NULL){
		// 		form.setFillColor(sf::Color::Green);
		// 	} else {
		// 		form.setFillColor(sf::Color::Red);
		// 	}
		// 	window.draw(form);
		// }
		globmut.unlock();
		window.display();
		while(1){}
	}

	runThread.join();

	return 0;
}
