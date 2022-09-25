//Доделать ошибку со сравнением элемента а и суммы строки и столбца
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

typedef double elemtype;
typedef elemtype** matrtype;

union errUnion
{
	int problem_otkr;
	struct {
		int nomer_str;
		int nomer_elem;
		int plase;
		string* posled;
		int oshib_sravn;
	};
	int nepryam;
	int* nomerstr;
};

enum myenum { 
	oshib_otkr = -1, 
	good, 
	false_zhach, 
	nepryam, 
	nodata, 
	mass_pointer,
	mass_rows, 
	unerror, 
	file_error 
};

bool mass_plus(int*& mass, int& size, int value) {
	int* newmass = new (nothrow) int[size + 1];
	if (!newmass)
		return false;
	for (int i = 0; i < size; i++) {
		newmass[i] = mass[i];
	}
	newmass[size] = value;
	size++;
	delete[] mass;
	mass = newmass;
	return true;
}

void zagruzka_matr(ifstream& file, matrtype matr, int str, int stolb) {
	file.seekg(0);
	for (int i = 0; i < str; i++) {
		for (int j = 0; j < stolb; j++) {
			file >> matr[i][j];
		}
	}
}

string mart_str_out(matrtype matr, int str, int stolb, unsigned short* shirina = NULL, string* razdelit_strok = NULL, int* format_flags = NULL, char* fill = NULL, unsigned short* tochnost = NULL) {
	stringstream ss;
	int bit_mask = 0;
	if (*format_flags & ios::adjustfield)
		bit_mask = bit_mask | ios::adjustfield;
	if (*format_flags & ios::basefield)
		bit_mask = bit_mask | ios::basefield;
	if (*format_flags & ios::floatfield)
		bit_mask = bit_mask | ios::floatfield;

	if (!shirina) {
		unsigned short t = 10;
		shirina = &t;
	}
	if (!razdelit_strok) {
		string t = "\n";
		razdelit_strok = &t;
	}
	if (!fill) {
		char t = ' ';
		fill = &t;
	}
	if (!tochnost) {
		unsigned short t = 5;
		tochnost = &t;
	}

	ss.setf(*format_flags, *format_flags | bit_mask);
	for (int i = 0; i < str; i++) {
		for (int j = 0; j < stolb; j++) {
			ss << setw(*shirina) << setfill(*fill) << setprecision(*tochnost) << matr[i][j];
		}
		ss << *razdelit_strok;
	}
	return ss.str();
}

int poisk_oshibok(ifstream& file, int& str, int& stolb, errUnion* oshib = NULL) {
	bool flag = false;
	int number = 0;
	int place = 0;
	int place2 = 0;
	elemtype d;
	string er, num;
	while (!(file >> ws).eof())
	{
		place2 = file.tellg();
		getline(file, num);
		istringstream ss(num);

		while (!(ss >> ws).eof())
		{
			place = place2 + ss.tellg();
			ss >> er;
			istringstream ss2(er);
			ss2 >> d;
			if (!ss2.eof() || ss2.fail())
			{
				if (oshib) {
					(*oshib).nomer_str = str;
					(*oshib).nomer_elem = number;
					(*oshib).plase = place;
					*(*oshib).posled = er;
					return false_zhach;
				}
				else
					return file_error;
			}
			number++;
		}

		if (str >= 1 && number != stolb) {
			if (oshib) {
				(*oshib).nepryam = str;
				return nepryam;
			}
			else
				return file_error;
		}
		str++;
		stolb = number;
		number = 0;
	}
	if (str == 0) {
		return nodata;
	}
	file.clear();
	return good;
}

void delete_matr(matrtype matr, int str) {
	for (int i = 0; i < str; i++)
		delete[] matr[i];
	delete[] matr;
}

matrtype videl_pam(matrtype& matr, int str, int stolb, int* number = NULL) {
	matr = NULL;
	int n = -1;
	try {
		matr = new elemtype * [str];
		for (n = 0; n < str; n++)
			matr[n] = new elemtype[stolb];
	}
	catch (...)
	{
		delete_matr(matr, n);
		if (number)
			*number = n;
	}
	return matr;
}

bool obrabot_matr(matrtype& matr, int str, int stolb, const elemtype& A) {

	int proizved = 1;

	for (int i = 0; i < str; i++) {
		for (int j = 0; j < stolb; j++) {
			if (i + j +2== A) {//тк начинается с нуля
				proizved *= matr[i][j];
			}
		}
	}

	for (int i = 0; i < str; i++) {
		for (int j = 0; j < stolb; j++) {
			if (stolb-j==1) {
				matr[i][j]=proizved;
			}
		}
	}

	return true;
}

int zagryzka_is_fila(const string& name, matrtype& matr, int& str, int& stolb, errUnion* oshib = NULL) {
	ifstream f1;
	f1.open(name);

	if (!f1.is_open()) {
		if (oshib)
			(*oshib).problem_otkr = errno;
		return oshib_otkr;
	}

	str = 0;
	stolb = 0;
	int err;
	err = poisk_oshibok(f1, str, stolb, oshib);

	if (err) {
		f1.close();
		return err;
	}

	videl_pam(matr, str, stolb, (*oshib).nomerstr);
	if (!matr) {
		f1.close();
		if ((*oshib).nomerstr) {
			switch (*(*oshib).nomerstr) {
			case -1:
				return mass_pointer;
			default:
				return mass_rows;
			}
		}
		return unerror;
	}
	zagruzka_matr(f1, matr, str, stolb);
	f1.close();
	return good;
}

/*В матрице найти произведение тех её элементов, 
сумма номеров строки и столбца которых равна числу A. 
Заменить на полученное произведение последний столбец матрицы.*/

int main()
{
	system("chcp 1251");
	while (true)
	{
		string name;
		ifstream f1;
		cin.ignore(cin.rdbuf()->in_avail());
		cout << "Введите название файла, или \"*\" для выхода из программы." << "\n";
		getline(cin, name);
		if (name == "*")
			break;

		string er;
		int  ctr, ctolb, error;
		matrtype matr;
		errUnion oshib;
		oshib.posled = &er;
		error = zagryzka_is_fila(name, matr, ctr, ctolb, &oshib);
		if (error) {
			switch (error)
			{
			case oshib_otkr:
#pragma warning(suppress : 4996)
				cout << "Файл с названием \"" << name << "\" не может быть открыт: " << strerror(errno) << " \n";
				break;
			case false_zhach:
				cout << "Неверное значение. Строка: " << oshib.nomer_str + 1 << " Элемент: " << oshib.nomer_elem + 1 << " \"" << er << "\"" << " Абсолютная позиция первого символа некорректного значения: " << oshib.plase << '\n';
				break;
			case nepryam:
				cout << "Матрица не прямоугольная. Строка: " << oshib.nepryam + 1 << '\n';
				break;
			case nodata:
				cout << "Файл с названием \"" << name << "\" не содержит данных. " << '\n';
				break;
			case mass_pointer:
				cout << "Bозникла ошибка при выделении памяти под массив указателей. " << '\n';
				break;
			case mass_rows:
				cout << "Возникла ошибка при выделении памяти под строку номер. " << *oshib.nomerstr + 1 << '\n';
				break;
			case unerror:
				cout << "Возникла ошибка при выделении памяти. " << '\n';
				break;
			case file_error:
				cout << "Файл с названием \"" << name << "\" содержит некорректные данные. " << '\n';
			}
			continue;
		}
		int format_flags = ios::dec | ios::right;
		unsigned short shirina = 10;
		string razdelit = "\n";
		char fill = ' ';
		unsigned short tochnost = 5;
		cout << mart_str_out(matr, ctr, ctolb, &shirina, &razdelit, &format_flags/*&fill,&tochnost*/);
		elemtype A;
		bool flag = false;
		cout << "Введите значение суммы строк и столбцов или \"*\" для выхода в меню." << "\n";
		while (true)
		{
			cin.ignore(cin.rdbuf()->in_avail());
			if (cin.peek() == '*' && (cin.rdbuf()->in_avail()) == 2)
			{
				flag = true;
				break;
			}
			cin >> A;
			if ((cin.peek() != '\n') || A == 0 || ctr + ctolb < A)
			{
				cout << "Невозможно использовать данное значение, повторите ввод или введите \"*\" для выхода в меню. " << "\n";
				cin.clear();
				cin.ignore(cin.rdbuf()->in_avail());
				continue;
			}
			break;
		}
		if (flag)
			continue;

		if (!obrabot_matr(matr, ctr, ctolb, A)) {
			cout << "Возникла ошибка при выделении памяти под массив.";
			delete_matr(matr, ctr);
			continue;
		}

		cout << mart_str_out(matr, ctr, ctolb, &shirina, &razdelit, &format_flags);

		delete_matr(matr, ctr);
	}
}