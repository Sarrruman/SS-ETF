#include "utilities.h"
#include "sekcija.h"
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include "symtab.h"
#include <iomanip>

list<string> uvoz;
list <string> izvoz;

extern map<string, fp> hes_funkcija;
string tekuca_sekcija = "";
void dodaj_parametre(stringstream& sline, list<string>& lista);
void prvi_prolaz(ifstream& ifs, ListaSekcija* lista_sekcija, SymTab* symtab) {
	string line;
	while (getline(ifs, line)) {
		list<string> params;
		stringstream sline;
		sline << line;

		string op;
		sline >> op;

		if (!op.empty() && (op[op.size() - 1] == ':')) {
			if (symtab->exist(op)) throw new Error("Simbol " + op + "je vec definisan!!!");
			Sekcija* sekcija = lista_sekcija->get(tekuca_sekcija);
			if (sekcija == nullptr) throw new Error("Labela " + op + "se mora naci u sekciji!!!");

			Simbol* simbol = new Simbol();
			op.erase(op.size() - 1);
			simbol->ime = op;
			simbol->lokal = 'l'; // inicijalno su svi simboli lokalni
			simbol->offset = sekcija->lc;
			simbol->rbr = symtab->simboli.size(); // svaka sekcija ima inicijalno <UND> simbol
			simbol->sekcija = sekcija;
			symtab->simboli.push_back(simbol);

			string s;
			sline >> skipws >> s;
			op = s; // za slucaj da se u istoj liniji gde i labela nalazi instrukcija
		}
		if (!op.empty()) {
			string built_op;
			string extensions;

			if (op == ".char") {
				Instrukcija* ins = new Instrukcija();
				ins->ime = ".char";

				// ekstrahovati parametre
				dodaj_parametre(sline, ins->parametri);
				// dodati instrukciju u tekucu sekciju
				if (tekuca_sekcija == "") throw new Error("Instrukcija definisana izvan sekcije");
				Sekcija* sekcija = lista_sekcija->get(tekuca_sekcija);
				sekcija->instrukcije.push_back(ins);
				// azurirati location counter
				sekcija->lc += ins->parametri.size();
			}
			else if (op == ".word") {
				Instrukcija* ins = new Instrukcija();
				ins->ime = ".word";

				// ekstrahovati parametre
				dodaj_parametre(sline, ins->parametri);
				// dodati instrukciju u tekucu sekciju
				if (tekuca_sekcija == "") throw new Error("Instrukcija definisana izvan sekcije");
				Sekcija* sekcija = lista_sekcija->get(tekuca_sekcija);
				sekcija->instrukcije.push_back(ins);
				// azurirati location counter
				sekcija->lc += ins->parametri.size() * 2;
			}
			else if (op == ".long") {
				Instrukcija* ins = new Instrukcija();
				ins->ime = ".long";

				// ekstrahovati parametre
				while (sline.good())
				{
					string param;
					getline(sline, param, ',');

					// skloniti whitespaces
					param = param.substr(param.find_first_not_of(' '),
						param.find_last_not_of(' ') - param.find_first_not_of(' ') + 1);

					ins->parametri.push_back(param);
				}
				// dodati instrukciju u tekucu sekciju
				if (tekuca_sekcija == "") throw new Error("Instrukcija definisana izvan sekcije");
				Sekcija* sekcija = lista_sekcija->get(tekuca_sekcija);
				sekcija->instrukcije.push_back(ins);
				// azurirati location counter
				sekcija->lc += ins->parametri.size() * 4;
			}
			else if (op == ".align") {
				Instrukcija* ins = new Instrukcija();
				ins->ime = ".align";

				// ekstrahovati parametre
				dodaj_parametre(sline, ins->parametri);
				// dodati instrukciju u tekucu sekciju
				if (tekuca_sekcija == "") throw new Error("Instrukcija definisana izvan sekcije");
				Sekcija* sekcija = lista_sekcija->get(tekuca_sekcija);
				sekcija->instrukcije.push_back(ins);
				// azurirati location counter
				int br_param = ins->parametri.size();
				// moraju se prvo naci vrednosti parametara;
				int parametri[3];
				for (int i = 0; i < br_param; i++) {
					string par = ins->parametri.front();
					ins->parametri.pop_front();
					try {
						if (par[0] = 'x' && par[1] == '0') {
							parametri[i] = stoi(par, nullptr, 16);
						}
						else {
							parametri[i] = stoi(par, nullptr, 10);
						}
						ins->parametri.push_back(par);
					}
					catch (invalid_argument e) {
						throw new Error(".align - nepravilan argument");
					}

				}
				if (br_param <= 3) {
					int uvecanje = 0;
					if (sekcija->lc % parametri[0] != 0) uvecanje = parametri[0] - (sekcija->lc % parametri[0]);
					if (br_param == 3 && parametri[2] < uvecanje);
					else sekcija->lc += uvecanje;
				}
				else if (br_param > 3) { // greska 
					throw new Error("Instrukcija .char mora imati <= 3 parametra");
				}

			}
			else if (op == ".skip") {
				Instrukcija* ins = new Instrukcija();
				ins->ime = ".skip";

				// ekstrahovati parametre
				dodaj_parametre(sline, ins->parametri);
				// dodati instrukciju u tekucu sekciju
				if (tekuca_sekcija == "") throw new Error("Instrukcija definisana izvan sekcije");
				Sekcija* sekcija = lista_sekcija->get(tekuca_sekcija);
				sekcija->instrukcije.push_back(ins);
				// odrediti vrednost parametra
				string par = ins->parametri.front();
				int parametar;
				try {
					par[0] = 'x' && par[1] == '0' ? (parametar = stoi(par, nullptr, 16))
						: (parametar = stoi(par, nullptr, 10));
				}
				catch (invalid_argument e) {
					throw new Error(".skip - nepravilan argument");
				}
				sekcija->lc += parametar;
			}
			else if (op == ".public") {
				dodaj_parametre(sline, izvoz);
			}
			else if (op == ".extern") {
				dodaj_parametre(sline, uvoz);
			}
			else if (op == ".end") {
				return;
			}
			else if (op.substr(0, 5) == ".text" || op.substr(0, 5) == ".data") {
				// provera da li posle .text ili .data
				if (op.size() != 5 && op[5] != '.') throw new Error("Nazivi sekcije i podsekcije moraju biti odvojeni tackom");

				if (lista_sekcija->get(op) != nullptr) throw new Error(
					"Sekcija definisana dva puta!!!");

				Sekcija* sekcija = new Sekcija(op);
				lista_sekcija->sekcije.push_back(sekcija);
				tekuca_sekcija = op;
				// dodavanje imena sekcije u symtab
				Simbol* simbol = new Simbol();
				simbol->ime = op;
				simbol->lokal = 'l';
				simbol->offset = 0;
				simbol->sekcija = sekcija;
				simbol->rbr = symtab->simboli.size();
				symtab->simboli.push_back(simbol);
			}
			else { // grupa instrukcija koje se obradjuju samo u drugom prolazu
				bool found = false;
				for (int i = 0; i < op.size(); i++) {
					built_op += op[i];
					// ukoliko se radi o instrukciji ldc koja je u nazvu ldch
					// ili ldcl potrebno je dodati jos jedan karakter, a tako i za ostale ins. in
					if ((i != op.size() - 1) && op == "ldc" && (op[i + 1] == 'h' || op[i + 1] == 'l'))
						built_op += op[i];
					if ((i != op.size() - 1) && op == "in" && op[i + 1] == 't')
						built_op += op[i];
					if (hes_funkcija.find(built_op) != hes_funkcija.end()) {
						found = true;
						break;
					}
				}
				if (!found) throw new Error(op + "instrukcija ne postoji!!!\n");
				else {
					// potrebno je kreirati novu instancu klase Instrukcija koja je potrebna u drugom prolazu					
					Instrukcija* ins = new Instrukcija();
					ins->ime = built_op;
					ins->ekstenzije = op.substr(built_op.length());

					// ekstrahovati parametre
					dodaj_parametre(sline, ins->parametri);
					// dodati instrukciju u tekucu sekciju
					if (tekuca_sekcija == "") throw new Error("Instrukcija definisana izvan sekcije");
					Sekcija* sekcija = lista_sekcija->get(tekuca_sekcija);
					sekcija->instrukcije.push_back(ins);
					// azurirati location counter
					if (built_op == "ldc") sekcija->lc += 8; // prevodi se u 2 instrukcije (8 bajtova)
					else sekcija->lc += 4;
				}
			}
		}
	}
}

void dodaj_parametre(stringstream& sline, list<string>& lista) {
	while (sline.good())
	{
		string param;
		getline(sline, param, ',');

		// skloniti whitespaces
		stringstream ss;
		ss << param;
		ss >> param;

		lista.push_back(param);
	}
}

// sluzi da se resetuju brojaci sekcija, da se lokalitet simbola azurira i da se izvrse provere ispravnosti
// i da se u tabelu simbola dodaju simboli koji se uvoze
void medjukorak(ListaSekcija* lista_sekcija, SymTab* symtab) {
	// resetovanje brojaca
	lista_sekcija->resetujBrojace();

	// provera da li su svi simboli koji se izvoze definisani, i azuriranje lokaliteta
	for (list<string>::iterator it = izvoz.begin(); it != izvoz.end(); ++it) {
		if (!symtab->exist(*it)) throw new Error("Simbol " + *it + " se izvozi a nije definisan!!!");
		else {
			(symtab->get(*it))->lokal = 'g';
		}
	}

	// dodavanje simbola koji se uvoze
	for (list<string>::iterator it = uvoz.begin(); it != uvoz.end(); ++it) {
		Simbol* simbol = new Simbol();
		simbol->ime = (*it);
		simbol->lokal = 'g';
		simbol->offset = 0;
		simbol->sekcija = nullptr;
		simbol->rbr = symtab->simboli.size();

		symtab->simboli.push_back(simbol);
	}
}

// prolazi kroz sve instrukcije i poziva njihove funkcije za obradu
void drugi_prolaz(ListaSekcija * lista_sekcija, SymTab * symtab) {
	list<Sekcija*> sekcije = lista_sekcija->sekcije;
	for (list<Sekcija*>::iterator sek = sekcije.begin(); sek != sekcije.end(); ++sek) {
		Sekcija* sekcija = *sek;
		for (list<Instrukcija*>::iterator ins = sekcija->instrukcije.begin(); ins != sekcija->instrukcije.end();
			++ins) {
			map<string, fp>::iterator it = hes_funkcija.find((*ins)->ime);
			fp funkcija = it->second;
			funkcija(*(*ins), sekcija, symtab);
		}
	}
}

void ispis(ofstream & ofs, ListaSekcija * lista_sekcija, SymTab * symtab)
{
	// Ispis tabele simbola
	ofs << "<------ TABELA SIMBOLA ----->" << endl << endl;
	ofs << left << setw(15) << "ime" << setw(15) << "sekcija" << setw(5) << "vr" << setw(15) << "vidljivost" <<
		setw(3) << "rbr" << endl << endl;
	for (list<Simbol*>::iterator it = symtab->simboli.begin(); it != symtab->simboli.end(); ++it) {
		Simbol s = **it;
		ofs << setw(15) << s.ime << setw(15) << (s.sekcija ? s.sekcija->ime : "-1") << setw(15) << setbase(16) << s.offset << setbase(10) <<
			setw(15) << s.lokal << setw(15) << s.rbr << endl;
	}
	// ispisivanje sekcija
	ofs << endl << endl << "<------ TABELA SEKCIJA ----->" << endl;
	list<Sekcija*> sekcije = lista_sekcija->sekcije;
	for (list<Sekcija*>::iterator sek = sekcije.begin(); sek != sekcije.end(); ++sek) {
		Sekcija* sekcija = *sek;
		ofs << endl << sekcija->ime << endl;

		int i = 0;
		for (list<char>::iterator kod = sekcija->niz_bajtova.begin(); kod != sekcija->niz_bajtova.end();
			++kod) {
			if (i % 4 == 0) ofs << endl << setw(3) << hex << i << ":  ";
			int pom = *kod;
			pom &= 0x000000FF;
			ofs << noshowbase << right << hex << setw(2) << setfill('0') << pom << " " << dec << setfill(' ') << left;
			i++;
		}
		// ispisivanje relokacija
		ofs << endl << endl;
		ofs << ".rel" + sekcija->ime << endl << endl;
		if (sekcija->relokacije.empty()) ofs << "Nema relokacija za sekciju " + sekcija->ime << endl << endl;
		else {
			ofs << left << setw(10) << "ofset" << setw(20) << "tip" << setw(3) << "rbr" << endl << endl;

			for (list<Relokacija*>::iterator it = sekcija->relokacije.begin(); it != sekcija->relokacije.end();
				++it) {
				Relokacija rel = **it;
				ofs << left << setw(10) << setbase(16) << rel.offset << setbase(10) << setw(20) << rel.tip
					<< setw(3) << rel.rbr << endl << endl;
			}
		}
		ofs << "--------------------" << endl;
	}
}

Error::Error(string opis)
{
	this->opis = opis;
}
