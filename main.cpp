#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "sekcija.h"
#include "utilities.h"
#include "symtab.h"

using namespace std;

extern map<string, fp> hes_funkcija;
void init_hes_funkcija();

int main(int argc, char* argv[]) {
	init_hes_funkcija();
	init_uslovi();
	init_registri();

	//if (argc < 2) {
	//	cout << "Nije prosledjen nijedan argument!!!";
	//	system("PAUSE");
	//	return 1;
	//}
	ifstream fin("ulaz.txt");
	ofstream fout("izlaz.txt");
	if (!fin.is_open()) { cout << "Nije pronadjen ulazni fajl" << endl; system("PAUSE"); return 1; }

	ListaSekcija* lista_sekcija = new ListaSekcija();
	SymTab* symtab = new SymTab();
	try {
		prvi_prolaz(fin, lista_sekcija, symtab);
		medjukorak(lista_sekcija, symtab);
		drugi_prolaz(lista_sekcija, symtab);
		ispis(fout, lista_sekcija, symtab);
	}
	catch (UserError* e) {
		cout << e->opis << endl;
		delete e;
		fin.close();
		fout.close();
		delete lista_sekcija;
		delete symtab;
		system("PAUSE");
		return 1;
	}
	fin.close();
	fout.close();
	delete lista_sekcija;
	delete symtab;

	cout << "Uspesno!!!" << endl;
	system("PAUSE");
	return 0;
}