#include "symtab.h"

bool SymTab::exist(string name)
{
	for (list<Simbol*>::iterator it = simboli.begin(); it != simboli.end(); ++it)
		if ((*it)->ime == name) return true;
	return false;
}

Simbol* SymTab::get(string name)
{
	for (list<Simbol*>::iterator it = simboli.begin(); it != simboli.end(); ++it)
		if ((*it)->ime == name) return (*it);
}

SymTab::SymTab()
{
	Simbol* sim = new Simbol();
	sim->ime = string("<UND>");
	sim->sekcija = nullptr;
	sim->rbr = sim->offset = 0;
	sim->lokal = 'l';

	simboli.push_back(sim); // nulti ulaz se ne koristi
}

SymTab::~SymTab()
{
	for (list<Simbol*>::iterator it = simboli.begin(); it != simboli.end(); ++it) delete *it;
}
