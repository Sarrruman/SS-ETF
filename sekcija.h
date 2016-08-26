#pragma once
#include <list>
#include<string>
#include "utilities.h"

using namespace std;

class Sekcija {
public:
	string ime;
	list<int> niz_bajtova;
	int lc = 0;
	list<Instrukcija*> instrukcije;
	int broj_simbola = 0; // 0-ti je <UND>

	Sekcija(string ime);
	~Sekcija();
};

class ListaSekcija {
public:
	list<Sekcija*> sekcije;

	Sekcija* get(string name);
	void resetujBrojace();
	~ListaSekcija();
};