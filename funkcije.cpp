#include <string>
#include <map>
#include <iostream>
#include "utilities.h"
#include "sekcija.h"
#include "symtab.h"

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
			if (it == uslovi.end()) throw new Error("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	if (ins.parametri.size() != 1) throw new Error("Instrukcija " + ins.ime + " prima 1 parametar, koji je broj");
	// pretvaranje parametra u broj
	int broj;
	if (param_u_broj(ins.parametri.front, broj) == -1) throw new Error("Instrukcija " + ins.ime + " broj");
	if (broj < 0 || broj > 15) throw new Error("Prekid mora biti u opsegu [0, 15]");
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
	if (ins.parametri.size() != 2) throw new Error("Aritmeticka funkcija mora imati 2 parametra !!!");
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
			if (it == uslovi.end()) throw new Error("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new Error("Prvi parametar arit. funkcije mora biti registar"); }
	if (par[0] == "psw" || par[0] == "PSW") throw new Error("PSW nije dozvoljen u arit. inst!!!");
	prvi = it->second;

	// konvertovanje drugog parametra
	it = (registri.find(par[1]));
	if (it == registri.end()) imm = 1;
	else drugi = it->second;
	// formiranje koda instrukcije
	int rez = 0; // niz bajtova koji ce se cuvati
	int op; // kod operacije
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
		drugi << 14;
		drugi >> 14;
		rez |= drugi & 0x0003ffff;
	}
	else {
		iter it = (registri.find(par[1]));
		if (it == registri.end()) { throw new Error("Drugi parametar arit. funkcije mora biti ili registar ili broj"); }
		if (par[1] == "psw" || par[1] == "PSW") throw new Error("PSW nije dozvoljen u arit. inst!!!");
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
	if (ins.parametri.size() != 2) throw new Error("Bitska funkcija mora imati 2 parametra !!!");
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
			if (it == uslovi.end()) throw new Error("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new Error("Prvi parametar bitske funkcije mora biti registar"); }
	if (par[0] == "psw" || par[0] == "PSW" || par[0] == "pc" || par[0] == "PC" || par[0] == "lr" ||
		par[0] == "LR") throw new Error("PSW, LR I PC nisu dozvoljeni u bitskoj inst!!!");
	prvi = it->second;

	// konvertovanje drugog parametra
	iter it = (registri.find(par[1]));
	if (it == registri.end()) { throw new Error("Drugi parametar bitske funkcije mora biti registar"); }
	if (par[1] == "psw" || par[1] == "PSW" || par[1] == "pc" || par[1] == "PC" || par[1] == "lr" ||
		par[1] == "LR") throw new Error("PSW, LR I PC nisu dozvoljeni u bitskoj inst!!!");
	drugi = it->second;
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

}

void call(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {

}

void in_out(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	if (ins.parametri.size() != 2) throw new Error("In/out funkcija mora imati 2 parametra !!!");
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
			if (it == uslovi.end()) throw new Error("Nepravilna instrukcija " + ins.ime);
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new Error("Prvi parametar in/out funkcije mora biti registar"); }
	prvi = it->second;

	// konvertovanje drugog parametra
	iter it = (registri.find(par[1]));
	if (it == registri.end()) { throw new Error("Drugi parametar in/out funkcije mora biti registar"); }
	drugi = it->second;

	if ((prvi < 20 && prvi >15) || (drugi < 20 && drugi >15)) throw new
		Error("In/out instrukcije ne prihvataju pc, sp, psw i lr registre kao argumente!!!");
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

}

void shift(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // shl, shr

}

void iret(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {

}

void ldc_word(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // ldch, ldcl

}
void ldc_long(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // ldc

}

void long_direktiva(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {

}

void word_direktiva(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {
	for (list<string>::iterator it = ins.parametri.begin(); it != ins.parametri.end(); ++it) {
		int broj;
		if (param_u_broj(*it, broj) == -1) throw new Error(ins.ime + " prihvata samo brojeve");
		if (broj > 0xFFFF) throw new Error(ins.ime + " prihvata brojeve manje od 0xFFFF");
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
		if (param_u_broj(*it, broj) == -1) throw new Error(ins.ime + " prihvata samo brojeve");
		if (broj > 0xFF) throw new Error(ins.ime + " prihvata brojeve manje od 0xFF");
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
	if (ins.parametri.size() != 1) throw new Error(ins.ime + " prihvata jedan argument, koji je broj");
	if (param_u_broj(ins.parametri.front, param) == -1) throw new Error(ins.ime +
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
		if (param[0] = 'x' && param[1] == '0') {
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
