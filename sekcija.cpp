#include "sekcija.h"

Sekcija::Sekcija(string ime) : ime(ime)
{
}

Sekcija::~Sekcija()
{
	for (list<Instrukcija*>::iterator it = instrukcije.begin(); it != instrukcije.end(); ++it)
		delete *it;
}

Sekcija* ListaSekcija::get(string name)
{
	for (list<Sekcija*>::iterator it = sekcije.begin(); it != sekcije.end(); ++it) {
		if ((*it)->ime == name) return *it;
	}
	return nullptr;
}

void ListaSekcija::resetujBrojace()
{
	for (list<Sekcija*>::iterator it = sekcije.begin(); it != sekcije.end(); ++it) {
		(*it)->lc = 0;
	}
}

ListaSekcija::~ListaSekcija()
{
	for (list<Sekcija*>::iterator it = sekcije.begin(); it != sekcije.end(); ++it)
		delete *it;
}
