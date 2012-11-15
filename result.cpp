#include "result.h"

Result::Result(int val, int* sol)
{
    val_ = val;
    x_ = sol;
}

int* Result::solution(){
    return x_;
}

int Result::val(){
    return val_;
}

void Result::set_val(int val){
	val_ = val;
}

void Result::set_sol(int* sol){
	delete[] x_;
	x_ = sol;
}