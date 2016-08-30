#include <string>
#include <map>
#include <iostream>
#include "utilities.h"
#include "sekcija.h"
#include "symtab.h"
#include <sstream>
#include "relokacija.h"

map<string, int> registri;
typedef map<string, int>::iterator iter;
void init_registri() {           // mora se pozvati u mainu pre pocetka drugog prolaza ----------------->
	for (int i = 0; i < 16; i++) {
		registri.insert(pair<string, int>("r" + to_string(i), i));
		registri.insert(pair<string, int>("R" + to_string(i), i));
	}
	registri.insert(pair<string, int>("pc", 16));
	registri.insert(pair<string, int>("PC", 16));
	registri.insert(pair<string, int>("lr", 17));
	registri.insert(pair<string, int>("LR", 17));
	registri.insert(pair<string, int>("sp", 18));
	registri.insert(pair<string, int>("SP", 18));
	registri.insert(pair<string, int>("psw", 19));
	registri.insert(pair<string, int>("PSW", 19));
}
int parsiraj_izraz(string izraz, string& op1, string& op2, char& znak);

map<string, int> uslovi;
void init_uslovi() { // mora se pozvati u mainu pre pocetka drugog prolaza ----------------->
	uslovi.insert(pair<string, int>("_eq", 0));
	uslovi.insert(pair<string, int>("_ne", 1));
	uslovi.insert(pair<string, int>("_gt", 2));
	uslovi.insert(pair<string, int>("_ge", 3));
	uslovi.insert(pair<string, int>("_lt", 4));
	uslovi.insert(pair<string, int>("_le", 5));
	uslovi.insert(pair<string, int>("_al", 7));
}

void interrupt(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	if (ins.parametri.size() != 1) throw new UserError("Instrukcija " + ins.ime + " prima 1 parametar, koji je broj");
	// pretvaranje parametra u broj
	int broj;
	if (param_u_broj(ins.parametri.front(), broj) == -1) throw new UserError("Instrukcija " + ins.ime + " broj");
	if (broj < 0 || broj > 15) throw new UserError("Prekid mora biti u opsegu [0, 15]");
	int rez = 0;
	int op = 0;
	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= broj << 20;
	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void aritmeticka(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // add, sub, mul, div, cmp
	// parsiranje parametara
	int imm = 0; // signalizira da je drugi parametar neposredna velicina
	if (ins.parametri.size() != 2) throw new UserError("Aritmeticka funkcija mora imati 2 parametra !!!");
	string par[2];
	par[0] = ins.parametri.front();
	par[1] = ins.parametri.back();
	int prvi = 0, drugi = 0;

	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new UserError("Prvi parametar arit. funkcije mora biti registar"); }
	if (par[0] == "psw" || par[0] == "PSW") throw new UserError("PSW nije dozvoljen u arit. inst!!!");
	prvi = it->second;

	// konvertovanje drugog parametra
	it = (registri.find(par[1]));
	if (it == registri.end()) imm = 1;
	else drugi = it->second;

	// provera da li se koriste pc, lr i sp ako ins. nije add ili sub
	if (ins.ime != "add" && ins.ime != "sub" && (prvi == 16 || prvi == 17 || prvi == 18 || !imm && (
		drugi == 16 || drugi == 17 || drugi == 18))) throw new UserError
		(" U aritmetickoj ins nisu dozvoljeni pc, lr ili sp ukoliko se ne radi o ins. add ili sub");
	// formiranje koda instrukcije
	int rez = 0; // niz bajtova koji ce se cuvati
	int op=0; // kod operacije
	if (ins.ime == "add") op = 1;
	else if (ins.ime == "sub") op = 2;
	else if (ins.ime == "mul") op = 3;
	else if (ins.ime == "div") op = 4;
	else if (ins.ime == "cmp") op = 5;

	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= prvi << 19;
	rez |= imm << 18;
	if (imm) {
		int pom;
		if (param_u_broj(par[1], pom) == -1) throw new 
			UserError("Arit. ins. mora za drugi par. imati ili broj ili registar");
		pom <<= 14;
		pom >>= 14;
		rez |= pom & 0x0003ffff;
	}
	else {
		iter it = (registri.find(par[1]));
		if (it == registri.end()) { throw new UserError("Drugi parametar arit. funkcije mora biti ili registar ili broj"); }
		if (par[1] == "psw" || par[1] == "PSW") throw new UserError("PSW nije dozvoljen u arit. inst!!!");
		drugi = it->second;
		rez |= drugi << 13;
	}

	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void bitska(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // and, or, not, test
	int imm = 0; // signalizira da je drugi parametar neposredna velicina
	if (ins.parametri.size() != 2) throw new UserError("Bitska funkcija mora imati 2 parametra !!!");
	string par[2];
	par[0] = ins.parametri.front();
	par[1] = ins.parametri.back();
	int prvi, drugi;

	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new UserError("Prvi parametar bitske funkcije mora biti registar"); }
	if (par[0] == "psw" || par[0] == "PSW" || par[0] == "pc" || par[0] == "PC" || par[0] == "lr" ||
		par[0] == "LR") throw new UserError("PSW, LR I PC nisu dozvoljeni u bitskoj inst!!!");
	prvi = it->second;

	// konvertovanje drugog parametra
	it = (registri.find(par[1]));
	if (it == registri.end()) { throw new UserError("Drugi parametar bitske funkcije mora biti registar"); }
	if (par[1] == "psw" || par[1] == "PSW" || par[1] == "pc" || par[1] == "PC" || par[1] == "lr" ||
		par[1] == "LR") throw new UserError("PSW, LR I PC nisu dozvoljeni u bitskoj inst!!!");
	drugi = it->second;

	// provera da li se radi o pc, lr ili psw
	if (prvi == 16 || prvi == 17 || prvi == 19 || drugi == 16 || drugi == 17 || drugi == 19) throw new
		UserError("U bitskoj instrukciji nisu dozvoljeni pc, lr i psw!!!");
	// formiranje koda instrukcije
	int rez = 0; // niz bajtova koji ce se cuvati
	int op; // kod operacije
	if (ins.ime == "and") op = 6;
	else if (ins.ime == "or") op = 7;
	else if (ins.ime == "not") op = 8;
	else if (ins.ime == "test") op = 9;

	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= prvi << 19;
	rez |= drugi << 14;
	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void load_store(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // ldr, str
	int br_param = ins.parametri.size();
	if (br_param < 1) throw new UserError(ins.ime + " funkcija mora imati bar 1 parametar !!!");
	string par[3];

	for (int i = 0; i < br_param; i++) {
		par[i] = ins.parametri.front();
		ins.parametri.pop_front();
		ins.parametri.push_back(par[i]);
	}
	int a = registri.find("pc")->second;
	int r; string lab = "";
	int imm = 0;
	// provera da li je postavljeno ia, ib, da ili db
	int f = 0;
	if (ins.ekstenzije[0] == 'i' && ins.ekstenzije[1] == 'a') {
		f = 2;
		ins.ekstenzije.erase(0, 2);
	}
	else if (ins.ekstenzije[0] == 'i' && ins.ekstenzije[1] == 'b') {
		f = 4;
		ins.ekstenzije.erase(0, 2);
	}
	else if (ins.ekstenzije[0] == 'd' && ins.ekstenzije[1] == 'a') {
		f = 3;
		ins.ekstenzije.erase(0, 2);
	}
	else if (ins.ekstenzije[0] == 'd' && ins.ekstenzije[1] == 'b') {
		f = 5;
		ins.ekstenzije.erase(0, 2);
	}
	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new UserError("Prvi parametar ldr/str funkcije mora biti registar"); }
	r = it->second;

	// konvertovanje drugog i treceg parametra ukoliko postoje
	if (br_param > 1) {
		// provera da li je labela tj. ldr r1, lab
		bool lab = false;
		Simbol* s = symtab->get(par[1]);
		if (s != nullptr) { // ukoliko je labela
			lab = true;
			if (br_param == 3 && lab) throw new UserError(
				"Ldr/str ins. sa 3 argumenta ne smeju za arg. imati labelu");
			// odredjivanje imm
			if (sekcija == s->sekcija) { // nema potrebe za relokacijama
				imm = s->offset - sekcija->lc - 4;
			}
			else { // potrebna relokacija
				imm = s->offset - 4; // bez -4 ako linker umesto adrese tekuce gleda adresu sledece ins.
				Relokacija* rel = new Relokacija();
				if (s->lokal = 'l')
					rel->rbr = symtab->get(sekcija->ime)->rbr;
				else
					rel->rbr = s->rbr;
				rel->offset = sekcija->lc;
				rel->tip = "R_PC_LAST10";
				sekcija->relokacije.push_back(rel);
			}
		}
		else { // drugi param. je registar ili neposredna velicina
			bool registar = true;
			it = (registri.find(par[1]));
			if (it == registri.end()) registar = false;
			else a = it->second;
			if (par[1] == "psw" || par[1] == "PSW") throw new UserError("Psw ne moze biti adresni reg");
			if (!registar) { // ukoliko je 2. neposrenda velicina
				if (param_u_broj(par[1], imm) == -1) throw new UserError(ins.ime +
					", nepravilna instrukcija");
				if (br_param == 3) throw new UserError("Nepravilna " + ins.ime + " instrukcija");
			}
			else { // ukoliko je 2. registar
				// citanje 3. operanda ukoliko postoji
				if (br_param == 3) {
					if (param_u_broj(par[2], imm) == -1) throw new UserError(ins.ime +
						" mora imati neposrednu velicinu za 3. argument");
				}
			}
		}
	}
	if (a == 16 && f != 0) throw new UserError(ins.ime +
		", ukoliko je adresni registar pc, ne sme biti ni inkrementa ni dekrementa");
	// formiranje koda instrukcije
	int rez = 0; // niz bajtova koji ce se cuvati
	int op = 10; // kod operacije
	int l_s = 0;
	if (ins.ime == "ldr") l_s = 1;

	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= a << 19;
	rez |= r << 14;
	rez |= f << 11;
	rez |= l_s << 10;

	imm <<= 22; imm >>= 22; imm &= 0x000003FF;
	rez |= imm;

	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void call(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	if (ins.parametri.size() > 2 || ins.parametri.size() == 0) throw new UserError(
		"Call instrukcija mora imati ili 1 ili 2 parametra");
	string par[2];
	int prvi, drugi;

	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	bool labela = false;
	if (ins.parametri.size() == 1) labela = true;

	int imm = 0; // neposredna velicina koja se u pisuje u instrukciju
	int dst = 0; // registar koji ce biti upisan u kod instrukcije

	if (labela) {
		// postavljanje pc-a kao odredista
		iter it = registri.find("pc");
		dst = it->second;

		// trazenje labele
		par[0] = ins.parametri.front();
		Simbol* s = symtab->get(par[0]);
		if (s == nullptr) throw new UserError(par[0] + " nije definisana!!!");
		int rbr_simbola;
		if (s->lokal == 'l') {
			imm = s->offset - 4; // bez -4 ukoliko linker gleda add. sledece ins.
			rbr_simbola = symtab->get(sekcija->ime)->rbr;
		}
		else {
			imm = -4;
			rbr_simbola = s->rbr;
		}
		if (s->sekcija != sekcija) {
			// pravljenje relokacionog zapisa
			Relokacija* rel = new Relokacija();
			rel->rbr = rbr_simbola;
			rel->tip = "R_PC_LAST19"; // linker treba da na polje imm doda adresu simbola i 
					// oduzme vrednost pc-a,
					// koja pokazuje na instrukciju posle ins. koju prepravlja (tj. na lokaciji offset +1)
			rel->offset = sekcija->lc;
			sekcija->relokacije.push_back(rel);
		}
		else { // Ukoliko su u istoj sekciji nije potrebna relokacija
			imm = s->offset - sekcija->lc - 4;
		}
	}
	else { // ukoliko je format call r1, 0x4
		par[0] = ins.parametri.front();
		par[1] = ins.parametri.back();
		// konvertovanje prvog parametra u broj registra
		iter it = (registri.find(par[0]));
		if (it == registri.end()) throw new UserError("Nperavilna instrukcija " + ins.ime);
		else prvi = it->second;
		// konvertovanje drugog parametra
		if (param_u_broj(par[1], drugi) == -1) throw new UserError("Nperavilna instrukcija " + ins.ime);

		imm = drugi;
		dst = prvi;
	}
	// formiranje koda instrukcije
	int rez = 0; // niz bajtova koji ce se cuvati
	int op = 12; // kod operacije
	imm <<= 13;
	imm >>= 13;
	imm &= 0x0007ffff;

	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= dst << 19;
	rez |= imm;

	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void in_out(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	if (ins.parametri.size() != 2) throw new UserError("In/out funkcija mora imati 2 parametra !!!");
	string par[2];
	par[0] = ins.parametri.front();
	par[1] = ins.parametri.back();
	int prvi, drugi;

	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new UserError("Prvi parametar in/out funkcije mora biti registar"); }
	prvi = it->second;

	// konvertovanje drugog parametra
	it = (registri.find(par[1]));
	if (it == registri.end()) { throw new UserError("Drugi parametar in/out funkcije mora biti registar"); }
	drugi = it->second;

	if ((prvi < 20 && prvi >15) || (drugi < 20 && drugi >15)) throw new
		UserError("In/out instrukcije ne prihvataju pc, sp, psw i lr registre kao argumente!!!");
	// formiranje koda instrukcije
	int rez = 0; // niz bajtova koji ce se cuvati
	int op = 13; // kod operacije
	int io = 0;
	if (ins.ime == "in") io = 1;

	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= prvi << 20;
	rez |= drugi << 16;
	rez |= io << 15;
	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void mov(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	int br_param = ins.parametri.size();
	if (br_param < 2) throw new UserError("Mov funkcija mora imati bar 2 parametra !!!");
	string par[3];

	for (int i = 0; i < br_param; i++) {
		par[i] = ins.parametri.front();
		ins.parametri.pop_front();
		ins.parametri.push_back(par[i]);
	}
	int prvi, drugi;
	int imm = 0;
	// provera da li je postavljeno siftovanje: mov<
	int shift = 0;
	if (ins.ekstenzije[0] == '<') {
		shift = 1;
		ins.ekstenzije.erase(0, 1);
	}
	else if (ins.ekstenzije[0] == '>') {
		ins.ekstenzije.erase(0, 1);
	}

	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new UserError("Prvi parametar mov funkcije mora biti registar"); }
	prvi = it->second;

	// konvertovanje drugog parametra
	it = (registri.find(par[1]));
	if (it == registri.end()) { throw new UserError("Drugi parametar mov funkcije mora biti registar"); }
	drugi = it->second;

	if (br_param == 3) {
		if (param_u_broj(par[2], imm) == -1) throw new UserError("Treci parametar mov ins. mora biti broj");
	}
	// formiranje koda instrukcije
	int rez = 0; // niz bajtova koji ce se cuvati
	int op = 14; // kod operacije

	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= prvi << 19;
	rez |= drugi << 14;

	imm <<= 27; imm >>= 27; imm &= 0x0000001f;
	rez |= imm << 9;
	rez |= shift << 8;
	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void shift(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // shl, shr
	// shift mora imati dva argumenta, prvi je registar i predstavlja i izvoriste i odrediste, a drugi broj
	if (ins.parametri.size() != 2) throw new UserError("Shift funkcija mora imati 2 parametra !!!");
	string par[2];
	par[0] = ins.parametri.front();
	par[1] = ins.parametri.back();
	int prvi, drugi;
	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new UserError("Prvi parametar shift funkcije mora biti registar"); }
	prvi = it->second;

	// odredjivanje pomeraja
	int imm;
	if (param_u_broj(par[1], imm) == -1) throw new UserError("Drugi param shift ins mora biti broj");
	// provera da li je pomeranje u levo lili desno
	int shift = 0;
	if (ins.ime == "shl") shift = 1;

	// formiranje koda
	int rez = 0; // niz bajtova koji ce se cuvati
	int op = 14; // kod operacije

	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= prvi << 19;
	rez |= prvi << 14;

	imm <<= 27; imm >>= 27; imm &= 0x0000001f;
	rez |= imm << 9;
	rez |= shift << 8;
	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void iret(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	if (ins.parametri.size() != 0) throw new UserError("Iret instrukcija ne sme imati parametre!!!");

	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// formiranje koda
	int rez = 0; // niz bajtova koji ce se cuvati
	int op = 14; // kod operacije
	int odrediste = registri.find("pc")->second;

	sign = 1;
	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= odrediste << 19;

	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void ldc_word(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // ldch, ldcl
	if (ins.parametri.size() != 2) throw new UserError(ins.ime + " funkcija mora imati 2 parametra !!!");
	string par[2];
	par[0] = ins.parametri.front();
	par[1] = ins.parametri.back();
	int prvi, drugi;

	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new UserError("Prvi parametar ldch/l funkcije mora biti registar"); }
	prvi = it->second;

	// konvertovanje drugog parametra
	if (param_u_broj(par[1], drugi) == -1) throw new UserError(ins.ime +
		", drugi operand mora biti neposredna velicina");
	// formiranje koda instrukcije
	int h_l = 0;
	if (ins.ime == "ldch") h_l = 1;
	int rez = 0; // niz bajtova koji ce se cuvati
	int op = 15; // kod operacije
	drugi <<= 16; drugi >>= 16; drugi &= 0x0000ffff;
	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= prvi << 20;
	rez |= h_l << 19;
	rez |= drugi;
	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}
void ldc_long(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // ldc
	if (ins.parametri.size() != 2) throw new UserError(
		"ldc instrukcija mora imati 2 parametra");
	string par[2];
	par[0] = ins.parametri.front();
	par[1] = ins.parametri.back();
	int prvi, drugi;

	// parsiranje ekstenzija
	int sign = 0;
	int uslov = 7; // bezuslovno podrazumevano
	if (!ins.ekstenzije.empty()) {
		if (ins.ekstenzije[0] == 's') {
			sign = 1;
			ins.ekstenzije.erase(0, 1);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije);
			if (it == uslovi.end()) throw new UserError("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) throw new UserError("Nperavilna instrukcija " + ins.ime);
	else prvi = it->second;

	bool labela = false;
	if (param_u_broj(par[1], drugi) == -1) labela = true;

	short ch = 0, cl = 0; // sluze za smestanje nizeg i viseg dela adrese

	if (labela) {
		// trazenje labele
		Simbol* s = symtab->get(par[1]);
		if (s == nullptr) throw new UserError(ins.ime + ", " + par[1] + " nije definisana!!!");

		// potrebne dve relokacije
		Relokacija* relh = new Relokacija();
		Relokacija* rell = new Relokacija();
		if (s->lokal == 'l') {
			int offset = s->offset;
			offset <<= 16; offset >>= 16; offset &= 0x0000ffff;
			cl = offset;
			ch = 0;

			rell->rbr = relh->rbr = symtab->get(s->sekcija->ime)->rbr;
		}
		else { // globalan simbol
			rell->rbr = relh->rbr = s->rbr;
		}
		relh->offset = sekcija->lc;
		rell->offset = sekcija->lc + 4;
		relh->tip = "R_LAST_16H";
		rell->tip = "R_LAST_16L";

		sekcija->relokacije.push_back(relh);
		sekcija->relokacije.push_back(rell);
	}
	else { // ukoliko je format ldc r1, 0x4
		// konvertovanje drugog parametra
		if (param_u_broj(par[1], drugi) == -1) throw new UserError("Nperavilna instrukcija " + ins.ime);
		// postavljanje ch i cl
		ch = (drugi >> 16) & 0x0000ffff;
		cl = drugi & 0x0000ffff;
	}
	// formiranje koda instrukcije ldch
	int h_l = 1;
	int rez = 0; // niz bajtova koji ce se cuvati
	int op = 15; // kod operacije
	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= prvi << 20;
	rez |= h_l << 19;
	int pom = 0; pom = ch; pom &= 0x0000ffff;
	rez |= pom;
	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;

	// formiranje koda instrukcije ldch
	h_l = 0;
	rez = 0; // niz bajtova koji ce se cuvati
	op = 15; // kod operacije
	rez |= uslov << 29;
	rez |= sign << 28;
	rez |= op << 24;
	rez |= prvi << 20;
	rez |= h_l << 19;
	pom = 0; pom = cl; pom &= 0x0000ffff;
	rez |= pom;
	// dodavanje instrukcije
	sekcija->dodaj_kod_int(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void long_direktiva(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	for (list<string>::iterator it = ins.parametri.begin(); it != ins.parametri.end(); ++it) {
		string op1 = "", op2 = "";
		char znak = 0;
		int pom = parsiraj_izraz(*it, op1, op2, znak);
		if (pom == -1) throw new UserError("Nepravilna direktiva .long");

		int kod = 0;
		if (pom == 1) { // U pitanju je samo jedan parametar
			if (param_u_broj(op1, kod) == -1) { // u pitanju je labela
				Simbol* s = symtab->get(op1);
				if (s == nullptr) throw new UserError("Nepravilna direktiva .long");
				// Pravi se zapis o relokaciji
				Relokacija* rel = new Relokacija();
				if (s->lokal == 'l') {
					kod = s->offset;
					rel->rbr = symtab->get(s->sekcija->ime)->rbr;
				}
				else { // globalna
					kod = 0;
					rel->rbr = s->rbr;
				}
				rel->offset = sekcija->lc;
				rel->tip = "R_32";
				sekcija->relokacije.push_back(rel);
			}
			else { // u pitanju je broj
			}
		}
		else { // u pitanju izraz
			// prvi operand izraza : 
			if (param_u_broj(op1, kod) == -1) { // u pitanju je labela
				Simbol* s = symtab->get(op1);
				if (s == nullptr) throw new UserError("Nepravilna direktiva .long");
				// Pravi se zapis o relokaciji
				Relokacija* rel = new Relokacija();
				if (s->lokal == 'l') {
					kod = s->offset;
					rel->rbr = symtab->get(s->sekcija->ime)->rbr;
				}
				else { // globalna
					kod = 0;
					rel->rbr = s->rbr;
				}
				rel->offset = sekcija->lc;
				rel->tip = "R_32";
				sekcija->relokacije.push_back(rel);
			}
			else { // u pitanju je broj
			}
			// drugi parametar
			int pom_kod;
			if (param_u_broj(op2, pom_kod) == -1) { // u pitanju je labela
				Simbol* s = symtab->get(op2);
				if (s == nullptr) throw new UserError("Nepravilna direktiva .long");
				// Pravi se zapis o relokaciji
				Relokacija* rel = new Relokacija();
				if (s->lokal == 'l') {
					if (znak == '+')
						kod += s->offset;
					else
						kod -= s->offset;
					rel->rbr = symtab->get(s->sekcija->ime)->rbr;
				}
				else { // globalna
					rel->rbr = s->rbr;
				}
				rel->offset = sekcija->lc;
				if (znak == '+')
					rel->tip = "R_32";
				else
					rel->tip = "R_SUB_32";
				sekcija->relokacije.push_back(rel);
			}
			else { // u pitanju je broj
				if (znak == '+')
					kod += pom_kod;
				else
					kod -= pom_kod;
			}
		}
		sekcija->dodaj_kod_int(kod);
		sekcija->lc += 4;
	}
}

void word_direktiva(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	for (list<string>::iterator it = ins.parametri.begin(); it != ins.parametri.end(); ++it) {
		int broj;
		if (param_u_broj(*it, broj) == -1) throw new UserError(ins.ime + " prihvata samo brojeve");
		if (broj > 0xFFFF) throw new UserError(ins.ime + " prihvata brojeve manje od 0xFFFF");
		short c = broj;
		char prvi = (c >> 8) & 0xFF;
		char drugi = c & 0xFF;
		sekcija->niz_bajtova.push_back(prvi);
		sekcija->niz_bajtova.push_back(drugi);
	}
	sekcija->lc += ins.parametri.size() * 2;
}

void char_direktiva(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	for (list<string>::iterator it = ins.parametri.begin(); it != ins.parametri.end(); ++it) {
		int broj;
		if (param_u_broj(*it, broj) == -1) throw new UserError(ins.ime + " prihvata samo brojeve");
		if (broj > 0xFF) throw new UserError(ins.ime + " prihvata brojeve manje od 0xFF");
		char c = broj;
		sekcija->niz_bajtova.push_back(c);
	}
	sekcija->lc += ins.parametri.size();
}

void align_direktiva(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // istestirati ----------->
	int br_param = ins.parametri.size();
	// moraju se prvo naci vrednosti parametara;
	int parametri[3];
	for (int i = 0; i < br_param; i++) {
		string par = ins.parametri.front();
		ins.parametri.pop_front();
		param_u_broj(par, parametri[i]);
		ins.parametri.push_back(par);
	}
	char za_popunjavanje = 0;
	int uvecanje = 0;
	if (sekcija->lc % parametri[0] != 0) uvecanje = parametri[0] - (sekcija->lc % parametri[0]);
	if (br_param == 3 && parametri[2] < uvecanje) uvecanje = 0;
	else if (br_param > 1) za_popunjavanje = parametri[1];
	for (int i = 0; i < uvecanje; i++) sekcija->niz_bajtova.push_back(za_popunjavanje);
	sekcija->lc += uvecanje;
}

void skip_direktiva(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	int param;
	if (ins.parametri.size() != 1) throw new UserError(ins.ime + " prihvata jedan argument, koji je broj");
	if (param_u_broj(ins.parametri.front(), param) == -1) throw new UserError(ins.ime +
		" prihvata jedan argument, koji je broj");
	for (int i = 0; i < param; i++) {
		sekcija->niz_bajtova.push_back(0);
	}
	sekcija->lc += param;
}

map<string, fp> hes_funkcija;

void init_hes_funkcija() {
	hes_funkcija.insert(pair<string, fp>("int", interrupt));
	hes_funkcija.insert(pair<string, fp>("add", aritmeticka));
	hes_funkcija.insert(pair<string, fp>("sub", aritmeticka));
	hes_funkcija.insert(pair<string, fp>("mul", aritmeticka));
	hes_funkcija.insert(pair<string, fp>("div", aritmeticka));
	hes_funkcija.insert(pair<string, fp>("cmp", aritmeticka));
	hes_funkcija.insert(pair<string, fp>("and", bitska));
	hes_funkcija.insert(pair<string, fp>("or", bitska));
	hes_funkcija.insert(pair<string, fp>("not", bitska));
	hes_funkcija.insert(pair<string, fp>("test", bitska));
	hes_funkcija.insert(pair<string, fp>("ldr", load_store));
	hes_funkcija.insert(pair<string, fp>("str", load_store));
	hes_funkcija.insert(pair<string, fp>("call", call));
	hes_funkcija.insert(pair<string, fp>("in", in_out));
	hes_funkcija.insert(pair<string, fp>("out", in_out));
	hes_funkcija.insert(pair<string, fp>("mov", mov));
	hes_funkcija.insert(pair<string, fp>("shr", shift));
	hes_funkcija.insert(pair<string, fp>("shl", shift));
	hes_funkcija.insert(pair<string, fp>("iret", iret));
	hes_funkcija.insert(pair<string, fp>("ldch", ldc_word));
	hes_funkcija.insert(pair<string, fp>("ldcl", ldc_word));
	hes_funkcija.insert(pair<string, fp>("ldc", ldc_long));
	hes_funkcija.insert(pair<string, fp>(".long", long_direktiva));
	hes_funkcija.insert(pair<string, fp>(".word", word_direktiva));
	hes_funkcija.insert(pair<string, fp>(".char", char_direktiva));
	hes_funkcija.insert(pair<string, fp>(".align", align_direktiva));
	hes_funkcija.insert(pair<string, fp>(".skip", skip_direktiva));
}

int param_u_broj(string param, int & broj)
{
	try {
		if (param[0] == '0' && param[1] == 'x') {
			broj = stoi(param, nullptr, 16);
		}
		else {
			broj = stoi(param, nullptr, 10);
		}
		return 0;
	}
	catch (invalid_argument e) {
		return -1;
	}
}

int parsiraj_izraz(string izraz, string& op1, string& op2, char& znak) { // vraca 0 ukoliko je prosledjeni 
	// string izraz , 1 ukoliko postoji samo jedan operand i -1 u slucaju greske
	int i = 0;
	while (i < izraz.size()) {
		if (izraz[i] == ' ' || izraz[i] == '+' || izraz[i] == '-') break;
		op1 = op1 + izraz[i];
		i++;
	}
	if (i == izraz.size()) return 1;
	while (izraz[i] == ' ') i++; // preskakanje razmaka
	if (izraz[i] != '+' && izraz[i] != '-') return -1;
	znak = izraz[i];
	i++;
	while (izraz[i] == ' ') i++; // preskakanje razmaka
	while (i < izraz.size()) {
		op2 += izraz[i];
		i++;
	}
	return 0;
}