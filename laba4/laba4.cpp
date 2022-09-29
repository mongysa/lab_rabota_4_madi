#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>


using namespace std;


typedef double elemtype;
typedef elemtype** matrtype;


union error_checker_union
{
	int problema_otkrytia;
	struct {
		int nomer_stroki;
		int nomer_elementa;
		int place_error;
		string* posled;
	};
	int nepryamougolnaya;
	int* nomer_stroki_ukaz;
};


enum error_checker { 
	oshibka_otkrtia_case = -1, 
	good_case, 
	nevern_znachenie_case, 
	nepryamoug_case, 
	net_dannh_case, 
	massiv_ukazatel,
	massiv_stroki, 
	net_oshidki_case, 
	oshibka_file_case 
};


void zagruzka_matr(ifstream& file, matrtype matriza, int stroka, int stolbec) {
	file.seekg(0);
	for (int i = 0; i < stroka; i++) {
		for (int j = 0; j < stolbec; j++) {
			file >> matriza[i][j];
		}
	}
}


string mart_str_out(matrtype matriza, int stroka, int stolbec, unsigned short* shirina = NULL, string* razdelitel_strok = NULL, int* formatirovanye_flagov = NULL, char* prvo_mezhd_symb = NULL, unsigned short* tochnost = NULL) {
	stringstream ss;
	int bit_mask = 0;
	if (*formatirovanye_flagov & ios::adjustfield)
		bit_mask = bit_mask | ios::adjustfield;

	if (!shirina) {
		unsigned short t = 10;
		shirina = &t;
	}
	if (!razdelitel_strok) {
		string t = "\n";
		razdelitel_strok = &t;
	}
	if (!prvo_mezhd_symb) {
		char t = ' ';
		prvo_mezhd_symb = &t;
	}
	if (!tochnost) {
		unsigned short t = 5;
		tochnost = &t;
	}

	ss.setf(*formatirovanye_flagov, *formatirovanye_flagov | bit_mask);
	for (int i = 0; i < stroka; i++) {
		for (int j = 0; j < stolbec; j++) {
			ss << setw(*shirina) << setfill(*prvo_mezhd_symb) << setprecision(*tochnost) << matriza[i][j];
		}
		ss << *razdelitel_strok;
	}
	return ss.str();
}


int poisk_oshibok(ifstream& file, int& stroka, int& stolbec, error_checker_union* oshibka = NULL) {

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
				if (oshibka) {
					(*oshibka).nomer_stroki = stroka;
					(*oshibka).nomer_elementa = number;
					(*oshibka).place_error = place;
					*(*oshibka).posled = er;
					return nevern_znachenie_case;
				}
				else
					return oshibka_file_case;
			}
			number++;
		}

		if (stroka >= 1 && number != stolbec) {
			if (oshibka) {
				(*oshibka).nepryamougolnaya = stroka;
				return nepryamoug_case;
			}
			else
				return oshibka_file_case;
		}

		stroka++;
		stolbec = number;
		number = 0;
	}

	if (stroka == 0) {
		return net_dannh_case;
	}

	file.clear();
	return good_case;
}


void delete_matr(matrtype matriza, int stroka) {
	for (int i = 0; i < stroka; i++)
		delete[] matriza[i];
	delete[] matriza;
}

matrtype videl_pam(matrtype& matriza, int stroka, int stolbec, int* number = NULL) {
	matriza = NULL;
	int str_elem_peredacha = -1;

	try {
		matriza = new elemtype * [stroka];
		for (str_elem_peredacha = 0; str_elem_peredacha < stroka; str_elem_peredacha++)
			matriza[str_elem_peredacha] = new elemtype[stolbec];
	}
	catch (...)
	{
		delete_matr(matriza, str_elem_peredacha);
		if (number)
			*number = str_elem_peredacha;
	}

	return matriza;
}


bool obrabot_matr(matrtype& matriza, int stroka, int stolbec, const elemtype& summa_str_i_stlb) {
	int proizved = 1;

	for (int i = 0; i < stroka; i++) {
		for (int j = 0; j < stolbec; j++) {
			if (i + j+2== summa_str_i_stlb) {//тк начинается с нуля
				proizved *= matriza[i][j];
			}
		}
	}

	for (int i = 0; i < stroka; i++) {
		for (int j = 0; j < stolbec; j++) {
			if (stolbec-j==1) {
				matriza[i][j]=proizved;
			}
		}
	}

	return true;
}


int zagryzka_is_fila(const string& file_name, matrtype& matriza, int& stroka, int& stolbec, error_checker_union* oshibka = NULL) {
	ifstream f1;
	f1.open(file_name);

	if (!f1.is_open()) {
		if (oshibka)
			(*oshibka).problema_otkrytia = errno;
		return oshibka_otkrtia_case;
	}

	stroka = 0;
	stolbec = 0;
	int err;
	err = poisk_oshibok(f1, stroka, stolbec, oshibka);
	if (err) {
		f1.close();
		return err;
	}

	videl_pam(matriza, stroka, stolbec, (*oshibka).nomer_stroki_ukaz);

	if (!matriza) {
		f1.close();
		if ((*oshibka).nomer_stroki_ukaz) {
			switch (*(*oshibka).nomer_stroki_ukaz) {
			case -1:
				return massiv_ukazatel;
			default:
				return massiv_stroki;
			}
		}
		return net_oshidki_case;
	}

	zagruzka_matr(f1, matriza, stroka, stolbec);
	f1.close();

	return good_case;
}


/*В матрице найти произведение тех её элементов, 
сумма номеров строки и столбца которых равна числу A. 
Заменить на полученное произведение последний столбец матрицы.*/


int main()
{
	system("chcp 1251");
	while (true)
	{
		string file_name;
		ifstream f1;

		cin.ignore(cin.rdbuf()->in_avail());
		cout << "Введите название файла, или \"*\" для выхода из программы." << "\n";
		getline(cin, file_name);
		if (file_name == "*")
			break;

		string er;
		int  ctroka, ctolbec, error;
		matrtype matriza;
		error_checker_union oshibka;
		oshibka.posled = &er;

		error = zagryzka_is_fila(file_name, matriza, ctroka, ctolbec, &oshibka);

		if (error) {
			switch (error)
			{
			case oshibka_otkrtia_case:
#pragma warning(suppress : 4996)
				cout << "Файл с названием \"" << file_name << "\" не может быть открыт: " << strerror(errno) << " \n";
				break;
			case nevern_znachenie_case:
				cout << "Неверное значение. Строка: " << oshibka.nomer_stroki + 1 << " Элемент: " << oshibka.nomer_elementa + 1 << " \"" << er << "\"" << " Абсолютная позиция первого символа некорректного значения: " << oshibka.place_error << '\n';
				break;
			case nepryamoug_case:
				cout << "Матрица не прямоугольная. Строка: " << oshibka.nepryamougolnaya + 1 << '\n';
				break;
			case net_dannh_case:
				cout << "Файл с названием \"" << file_name << "\" не содержит данных. " << '\n';
				break;
			case massiv_ukazatel:
				cout << "Bозникла ошибка при выделении памяти под массив указателей. " << '\n';
				break;
			case massiv_stroki:
				cout << "Возникла ошибка при выделении памяти под строку номер. " << *oshibka.nomer_stroki_ukaz + 1 << '\n';
				break;
			case net_oshidki_case:
				cout << "Возникла ошибка при выделении памяти. " << '\n';
				break;
			case oshibka_file_case:
				cout << "Файл с названием \"" << file_name << "\" содержит некорректные данные. " << '\n';
			}
			continue;
		}

		int format_flags = ios::dec | ios::right;
		unsigned short shirina = 10;
		string razdelit = "\n";
		char fill = ' ';
		unsigned short tochnost = 5;
		cout << mart_str_out(matriza, ctroka, ctolbec, &shirina, &razdelit, &format_flags);//сюда же можно fill и precision, но не является ли это дописыванием вывода в функции?
		elemtype summa_str_i_stlb;
		bool nevernoe_znacheniye = false;

		cout << "Введите значение суммы строк и столбцов или \"*\" для выхода в меню." << "\n";
		while (true)
		{
			cin.ignore(cin.rdbuf()->in_avail());
			if (cin.peek() == '*' && (cin.rdbuf()->in_avail()) == 2)
			{
				nevernoe_znacheniye = true;
				break;
			}
			cin >> summa_str_i_stlb;
			if ((cin.peek() != '\n') || ctroka + ctolbec < summa_str_i_stlb || summa_str_i_stlb<2)
			{
				cout << "Невозможно использовать данное значение, повторите ввод или введите \"*\" для выхода в меню. " << "\n";
				cin.clear();
				cin.ignore(cin.rdbuf()->in_avail());
				continue;
			}
			break;
		}
		if (nevernoe_znacheniye)
			continue;

		if (!obrabot_matr(matriza, ctroka, ctolbec, summa_str_i_stlb)) {
			cout << "Возникла ошибка при выделении памяти под массив.";
			delete_matr(matriza, ctroka);
			continue;
		}

		cout << mart_str_out(matriza, ctroka, ctolbec, &shirina, &razdelit, &format_flags);

		delete_matr(matriza, ctroka);
	}
}