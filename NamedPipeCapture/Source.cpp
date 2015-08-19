#include "Writer.h"
#include <string>

void DoWrite1(std::string, const char*, size_t)
{
	std::cout << "Here!";

}

int main()
{
	std::string s;
	Writer w(s);
	char* c = nullptr;
	DoWrite(w, c, 0);

	char a;
	std::cin >> a;
}