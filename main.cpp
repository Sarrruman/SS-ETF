#include <iostream>
#include<fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include "sekcija.h"
#include <map>
#include <bitset>

using namespace std;

extern map<string, fp> hes_funkcija;
void init_hes_funkcija();

void test1();
void test2();

void main() {
	test2();

	system("PAUSE");
}

void test1() {
	init_hes_funkcija();
	ofstream fout("izlaz.txt");

	cout << "hello from here" << endl;
	cout << 10 << endl;
	cout << setw(10) << setbase(16) << showbase << 10 << setw(10) << 11 << 12 << endl;
	cout << 10 << endl;

	fout.close();

	string line = "d";
	while (!line.empty()) {
		list<string> params;
		stringstream sline;
		cout << "unesite liniju : " << endl;
		getline(cin, line);
		sline << line;

		string op;
		int n;
		sline >> op;

		if (op[op.size() - 1] == ':') {
			cout << "labela je: " << op.substr(0, op.size() - 1) << endl;
			sline >> op;
		}
		if (!op.empty()) {
			bool found = false;
			string built_op;
			string extensions;
			for (int i = 0; i < op.size(); i++) {
				built_op += op[i];
				if (hes_funkcija.find(built_op) != hes_funkcija.end()) {
					cout << built_op << endl;
					extensions = op.substr(built_op.length());
					extensions.empty() ? (cout << "nema ekstenzije" << endl) : (cout << extensions << endl);
					found = true;
					break;
				}
			}
			if (!found) {
				if (op == ".char") {
					cout << op;
				}
				else if (op == ".word") {
					cout << op;
				}
				else if (op == ".long") {
					cout << op;
				}
				else if (op == ".align") {
					cout << op;
				}
				else if (op == ".skip") {
					cout << op;
				}
				else if (op == ".public") {
					cout << op;
				}
				else if (op == ".extern") {
					cout << op;
				}
				else if (op == ".end") {
					cout << op;
				}
				else if (op.substr(0, 5) == ".text") {
					cout << op;
				}
				else if (op.substr(0, 5) == ".data") {
					cout << op;
				}
				else { // GRESKA !!!
					cout << "ne postoji" << endl;
				}

			}
			else { // if found
				   // extract parameters
				while (sline.good())
				{
					string param;
					getline(sline, param, ',');

					// skloniti whitespaces
					stringstream ss;
					ss << param;
					ss >> param;

					params.push_back(param);
				}
				params.empty() ? (cout << "Nema parametara" << endl) : (cout << "parametri su: " << endl);
				for (list<string>::iterator it = params.begin(); it != params.end(); ++it) {
					cout << dec << *it << (*it).size() << endl;
				}

			}
		}

	}
}

void test2() {
	bitset<32> bitovi;

	int op = 7;

	char prvi = 0, drugi = 0, treci = 0, cetvrti = 0;
	unsigned long ins = 0;
	cout << showbase << setbase(16);
	cout << setw(8) << setfill('0') << ins << endl;
	int uslov = 2;
	int s = 1;
	int prviop = 1;
	
	uslov <<= 29;
	ins |= uslov;
	s <<= 28;
	ins |= s;
	op <<= 24;
	ins |= op;

	drugi = -5;
	drugi << 27;
	drugi >> 27;
	ins |= drugi & 0x0000001f;
	cout << hex << showbase << ins << endl;

}