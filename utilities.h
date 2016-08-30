#pragma once
#include <list>
#include <string>
#include <exception>
#include "symtab.h"

class Sekcija;
class ListaSekcija;
using namespace std;

void init_hes_funkcija();
void init_uslovi();
void init_registri();
void dodaj_parametre(stringstream& sline, list<string>& lista);
void prvi_prolaz(ifstream& ifs, ListaSekcija* lista_sekcija, SymTab* symtab);
void medjukorak(ListaSekcija* lista_sekcija, SymTab* symtab);
void drugi_prolaz(ListaSekcija* lista_sekcija, SymTab* symtab);
void ispis(ofstream& ofs, ListaSekcija* lista_sekcija, SymTab* symtab);
int param_u_broj(string param, int& broj); // vraca 0 ako je parametar broj, -1 ako je labela

struct Instrukcija {
	string ime, ekstenzije;
	list<string> parametri;
};

typedef void(*fp) (Instrukcija&, Sekcija*, SymTab*);

extern list<string> uvoz;
extern list <string> izvoz;

class UserError : public exception {
public:
	string opis;
	UserError(string opis);
	virtual const char* what() const throw() 
	{
		return "error";
	}
};