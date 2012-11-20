#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <fstream>
#include <string>

#include "uls/uls_solve.h"
#include "cls/cls_solve.h"
#include "mcls/mcls_solve.h"

void cls_solve(const std::string);
void uls_solve(const std::string);
void mcls_solve(const std::string);

#endif
