#include "sekcija.h"

void Sekcija::dodaj_kod_int(int i)
{
	char bajt = (i >> 24) & 0xFF;
	niz_bajtova.push_back(bajt);
	bajt = (i >> 16) & 0xFF;
	niz_bajtova.push_back(bajt);
	bajt = (i >> 8) & 0xFF;
	niz_bajtova.push_back(bajt);

	niz_bajtova.push_back(i);
}

Sekcija::Sekcija(string ime) : ime(ime)
{
}

Sekcija::~Sekcija()
{
	for (list<Instrukcija*>::iterator it = instrukcije.begin(); it != instrukcije.end(); ++it)
		delete *it;
	for (list<Relokacija*>::iterator it = relokacije.begin(); it != relokacije.end(); ++it)
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
