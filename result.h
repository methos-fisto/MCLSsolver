#ifndef RESULT_H
#define RESULT_H

class Result
{
public:
    Result(int val, int* sol);
    int val();
    int* solution();
    void set_val(int val);
	void set_sol(int* sol);
private:
    int val_;
    int* x_;
    
};

#endif // RESULT_H
