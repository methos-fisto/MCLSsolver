#include "main.h"

const std::string instance = "-instance";
const std::string type     = "-type";

/**
 *	Calls on correct solver depending on -type.
 *
 *	@param int
 *	@param char**
 *
 *	@return int 0 error, 1 success
 */
int main(int argc, char* argv[])
{
	if (argc < 5) {
		std::cout << "Ce programme s'appelle : ./solveLotSizing -type <T> -instance <fichier>" << std::endl;
		return 0;
	}
	
	int  i = 0;
	std::string fileName = "";
	
	void (*solve) (const std::string);
	
	for (i = 1; i < argc; i++) {		
		if (type.compare(argv[i]) == 0) {
			++i;
			
			if (strcmp(argv[i], "ULS") == 0 || strcmp(argv[i], "uls") == 0) {
				solve = &uls_solve;
			} else if (strcmp(argv[i], "CLS") == 0 || strcmp(argv[i], "cls") == 0) {
				solve = &cls_solve;
			} else if (strcmp(argv[i], "MCLS") == 0 || strcmp(argv[i], "mcls") == 0) {
				solve = &mcls_solve;
			} else {
				std::cout << "Type inconnu, il doit être dans {'ULS', 'CLS', 'MCLS'}." << std::endl;
				return 0;
			}
		} else if (instance.compare(argv[i]) == 0) {
			++i;
			
			fileName = argv[i];
		}
	}
	
	solve(fileName);
	
	return 1;
}
