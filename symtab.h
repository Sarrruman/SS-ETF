#pragma once
#include "utilities.h"
#include "sekcija.h"

struct Simbol {
	string ime;
	Sekcija* sekcija;
	int rbr;
	int offset;
	char lokal;
};

class SymTab {
public:
	list<Simbol*> simboli;

	bool exist(string name);
	Simbol* get(string name);
	SymTab();
	~SymTab();
};