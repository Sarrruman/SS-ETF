#pragma once
#include <list>
#include<string>
#include "utilities.h"

using namespace std;

class Sekcija;
class SymTab;

void init_hes_funkcija();

int param_u_broj(string param, int& broj); // vraca 0 ako je parametar broj, -1 ako je labela

struct Instrukcija {
	string ime, ekstenzije;
	list<string> parametri;
};

typedef void(*fp) (Instrukcija&, Sekcija*, SymTab*);

extern list<string> uvoz;
extern list <string> izvoz;

class Error {
public:
	string opis;
	Error(string opis);
};