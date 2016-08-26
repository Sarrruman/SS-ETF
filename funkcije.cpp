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
		registri.insert(pair<string, int>("r" + i, i));
		registri.insert(pair<string, int>("R" + i, i));
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
			ins.ekstenzije.erase(0);
		}
		if (ins.ekstenzije[0] == '_') {
			iter it = uslovi.find(ins.ekstenzije); // greska ako ne nadje --------->
			uslov = it->second;
		}
	}
	// konvertovanje prvog parametra u broj registra
	iter it = (registri.find(par[0]));
	if (it == registri.end()) { throw new Error("Prvi parametar arit. funkcije mora biti registar"); }
	if (par[0] == "psw" || par[0] == "PSW") throw new Error("PSW nije dozvoljen u arit. inst!!!");
	prvi = it->second;

	// konvertovanje drugog parametra
	if (param_u_broj(par[1], drugi) == -1) imm = 1;
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
	sekcija->niz_bajtova.push_back(rez);

	// azuriranje lc-a
	sekcija->lc += 4;
}

void bitska(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // and, or, not, test

}

void load_store(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) { // ldr, str

}

void call(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {

}

void in_out(Instrukcija& ins, Sekcija* sekcija, SymTab* symtab) {

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

//void long_direktiva(Instrukcija& instrukcija) {
//
//}
//
//void word_direktiva(Instrukcija& instrukcija) {
//
//}
//
//void char_direktiva(Instrukcija& instrukcija) {
//
//}
//
//void align_direktiva(Instrukcija& instrukcija) {
//	
//}
//
//void skip_direktiva(Instrukcija& instrukcija) {
//
//}

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
	/*hes_funkcija.insert(pair<string, fp>(".long", long_direktiva));
	hes_funkcija.insert(pair<string, fp>(".word", word_direktiva));
	hes_funkcija.insert(pair<string, fp>(".char", char_direktiva));
	hes_funkcija.insert(pair<string, fp>(".align", align_direktiva));
	hes_funkcija.insert(pair<string, fp>(".skip", skip_direktiva));*/
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
