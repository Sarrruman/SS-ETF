#pragma once
#include <list>
#include<string>
#include "relokacija.h"

class Instrukcija;

using namespace std;

class Sekcija {
public:
	string ime;
	list<char> niz_bajtova;
	int lc = 0;
	list<Instrukcija*> instrukcije;
	int broj_simbola = 0; // 0-ti je <UND>

	list<Relokacija*> relokacije;

	void dodaj_kod_int(int i); // funkcija koja od int-a pravi 4 bajta
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