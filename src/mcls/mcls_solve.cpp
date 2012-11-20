#include "mcls_solve.h"

class Solution
{
	public :
		double z;
		int *x, *s, *r, *y;
		int t, i, cpt;
		
		Solution(const int, const int);
		
		void show();
		void write(const char*);
};

Solution::Solution(const int nbPeriods, const int nbProducts)
{
	cpt = 0;
	
	z = DBL_MAX;
	
	t = nbPeriods;
	i = nbProducts;
	
	int size = t * i;
	
	x = new int[size];
	s = new int[size];
	r = new int[size];
	y = new int[size];
}

/**
 *	Display solution in terminal.
 */
void Solution::show()
{
	int i = 0, j = 0, id = 0;
	
	std::cout << "Trouvé z = " << z << " en " << cpt << " itérations." << std::endl;
	
	for (i = 1; i <= t; i++) {
		for (j = 1; j <= i; j++) {
			id = (i - 1) * t + j;
			
			std::cout << 'x' << i << ',' << j << " = " << x[id];
			
			if (j == i) {
				std::cout << '\n';
			} else {
				std::cout << '\t';
			}
		}
		
		std::cout << '\n';
	}
	
	std::cout << '\n';
	
	for (i = 1; i <= t; i++) {
		for (j = 1; j <= i; j++) {
			id = (i - 1) * t + j;
			
			std::cout << 's' << i << ',' << j << " = " << s[id];
			
			if (j == i) {
				std::cout << '\n';
			} else {
				std::cout << '\t';
			}
		}
		
		std::cout << '\n';
	}
	
	std::cout << '\n';
	
	for (i = 1; i <= t; i++) {
		for (j = 1; j <= i; j++) {
			id = (i - 1) * t + j;
			
			std::cout << 'r' << i << ',' << j << " = " << r[id];
			
			if (j == i) {
				std::cout << '\n';
			} else {
				std::cout << '\t';
			}
		}
		
		std::cout << '\n';
	}
	
	std::cout << '\n';
	
	for (i = 1; i <= t; i++) {
		for (j = 1; j <= i; j++) {
			id = (i - 1) * t + j;
			
			std::cout << 'y' << i << ',' << j << " = " << y[id];
			
			if (j == i) {
				std::cout << '\n';
			} else {
				std::cout << '\t';
			}
		}
		
		std::cout << '\n';
	}
	
	std::cout << std::endl;
}

/**
 *	Write solution as a LaTeX array.
 */
void Solution::write(const char* tex)
{
	int i = 0, j = 0, id = 0;
	char texnl[32]; // empty line in latex array
	
	std::ofstream *q = new std::ofstream(tex);
	
	sprintf(texnl, "\\multicolumn{%d}{c}{}\\\\", t);
	
	*q << "Trouvé $z = " << z << "$ en " << cpt << " itérations." << std::endl;
	
	*q << "$$\\begin{array}{";
	
	for (i = 1; i <= t; i++) {
		*q << "c @{ = } c ";
	}
	
	*q << '}' << std::endl;
	
	for (i = 1; i <= t; i++) {
		for (j = 1; j <= i; j++) {
			id = (i - 1) * t + j;
			
			*q << "x_" << i << '^' << j << '&' << x[id];
			
			if (j == i) {
				*q << ' ';
			} else {
				*q << '&';
			}
		}
		
		*q << "\\\\\n";
	}
	
	*q << texnl;
	
	for (i = 1; i <= t; i++) {
		for (j = 1; j <= i; j++) {
			id = (i - 1) * t + j;
			
			*q << "s_" << i << '^' << j << '&' << s[id];
			
			if (j == i) {
				*q << ' ';
			} else {
				*q << '&';
			}
		}
		
		*q << "\\\\\n";
	}
	
	*q << texnl;
	
	
	for (i = 1; i <= t; i++) {
		for (j = 1; j <= i; j++) {
			id = (i - 1) * t + j;
			
			*q << "r_" << i << '^' << j << '&' << r[id];
			
			if (j == i) {
				*q << ' ';
			} else {
				*q << '&';
			}
		}
		
		*q << "\\\\\n";
	}
	
	*q << texnl;
	
	for (i = 1; i <= t; i++) {
		for (j = 1; j <= i; j++) {
			id = (i - 1) * t + j;
			
			*q << "y_" << i << '^' << j << '&' << y[id];
			
			if (j == i) {
				*q << ' ';
			} else {
				*q << '&';
			}
		}
		*q << "\\\\\n";
	}
	
	*q << "\\end{array}$$" << std::endl;
	
	q->close();
}

/**
 *	Parse a correctly formatted file to generate data for an instance. Basically
 *	loops feeding data into arrays.
 *	File is formatted as :

i = nb products
t = nb periods

int[i] backlogging cost
int[i] costs of opening to produce any item i
int[i] cost of storing a i
int[t] production capacity during period t

int[t][i] matrix of demands per product per period

 *	
 *	@param ifstream* file to read from
 *	@param int       number of products
 *	@param int       number of demands
 *	@param int*      cost of backlogging for product i
 *	@param int*      cost of opening for a period
 *	@param int*      cost of storing per period
 *	@param int*      capacities per period
 *	@param int**     demands
 */
void mcls_parse(
	std::ifstream* f
	, int& nbProducts, int& nbPeriods
	, int*& back, int*& open, int*& stock, int*& capa
	, int**& demands
) {
	// Parsing data
	int i = 0, j = 0;
	
	*f >> nbProducts;
	*f >> nbPeriods;
	
	// Declaring arrays
	back  = new int[nbProducts];
	open  = new int[nbPeriods];
	stock = new int[nbPeriods];
	capa  = new int[nbPeriods];
	
	// Demand per product per period
	demands = new int*[nbPeriods];
	
	for (i = 0; i < nbPeriods; i++) {
		demands[i] = new int[nbProducts];
	}
	
	// Assign
	for (i = 0; i < nbProducts; i++) {
		*f >> back[i];
	}
	
	for (i = 0; i < nbPeriods; i++) {
		*f >> open[i];
	}
	
	for (i = 0; i < nbPeriods; i++) {
		*f >> stock[i];
	}
	
	for (i = 0; i < nbPeriods; i++) {
		*f >> capa[i];
	}
	
	for (i = 0; i < nbPeriods; i++) {
		for (j = 0; j < nbProducts; j++) {
			*f >> demands[i][j];
		}
	}
}

/**
 *	Instanciate GLPK problem's data.
 *
 *	@param glp_prob*
 *	@param int       number of products
 *	@param int       number of periods
 *	@param int*      backlogging cost
 *	@param int*      cost of opening a line
 *	@param int*      cost of storing a product
 *	@param int*      capacity available
 *	@param int**     demands per product per period
 */
glp_prob* mcls_create(
	 const int &nbProducts, const int &nbPeriods
	, int *&back, int *&open, int *&stock, int *&capa
	, int **&demands
) {
	char name[99];
	
	int i = 0, t = 0; // Loop counters
	int glp = 0, id = 0; // Glp counters
	int total = nbProducts * nbPeriods;
	
	glp_prob* pb;
	
	pb = glp_create_prob();
	
	glp_set_prob_name(pb, "CLS");
	glp_set_obj_dir(pb, GLP_MIN);
	
	// - Set variables
	// 4 spans of nbProducts * nbPeriods variables :
	//	1) production
	// 	2) stocks
	// 	3) backlogs
	// 	4) overture
	glp_add_cols(pb, total * 4);
	
	// Production
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			id = t * nbProducts + i + 1;
			
			sprintf(name, "x%d,%d", t + 1, i + 1);
			
			glp_set_col_name(pb, id, name);
			glp_set_col_bnds(pb, id, GLP_LO, 0, 0);
			glp_set_col_kind(pb, id, GLP_IV);
			// Cost is 1 for every period
			glp_set_obj_coef(pb, id, 0.0);
		}
	}
	
	// Stocks
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			id  = t * nbProducts + i + 1;
			id += total;
			
			sprintf(name, "s%d,%d", t + 1, i + 1);
			
			glp_set_col_name(pb, id, name);
			glp_set_col_bnds(pb, id, GLP_LO, 0, 0);
			glp_set_col_kind(pb, id, GLP_IV);
			// Stocking cost is the same for every period
			glp_set_obj_coef(pb, id, stock[i]);
		}
	}
	
	// Backlog
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			id  = t * nbProducts + i + 1;
			id += total * 2;
			
			sprintf(name, "r%d,%d", t + 1, i + 1);
			
			glp_set_col_name(pb, id, name);
			glp_set_col_bnds(pb, id, GLP_LO, 0, 0);
			glp_set_col_kind(pb, id, GLP_IV);
			// Cost is the same for every period
			glp_set_obj_coef(pb, id, back[i]);
		}
	}
	
	// Overture
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			id  = t * nbProducts + i + 1;
			id += total * 3;
		
			sprintf(name, "y%d,%d", t + 1, i + 1);
		
			glp_set_col_name(pb, id, name);
			glp_set_col_bnds(pb, id, GLP_DB, 0, 1);
			glp_set_col_kind(pb, id, GLP_CV);
			// Line overture cost is the same for every period
			glp_set_obj_coef(pb, id, open[i]);
		}
	}
	
	// - Constraints
	// Set constraints on :
	// 	1) demands satisfaction
	// 	2) line activity
	// 	3) valid inequality
	// 	4) product capacity
	glp_add_rows(pb, total * 3 + nbPeriods);
	
	// Demands
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			id = t * nbProducts + i + 1;
			
			sprintf(name, "[1](%d,%d)", t + 1, i + 1);
			
			glp_set_row_name(pb, id, name);
			// Demands are indexed on products first
			glp_set_row_bnds(pb, id, GLP_FX, demands[t][i], demands[t][i]);
		}
	}
	
	// Line activity
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			id  = t * nbProducts + i + 1;
			id += total * 1;
			
			sprintf(name, "[2](%d,%d)", t + 1, i + 1);
		
			glp_set_row_name(pb, id, name);
			glp_set_row_bnds(pb, id, GLP_UP, 0, 0);
		}
	}
	
	// Valid inequality
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			id  = t * nbProducts + i + 1;
			id += total * 2;
			
			sprintf(name, "[3](%d,%d)", t + 1, i + 1);
		
			glp_set_row_name(pb, id, name);
			glp_set_row_bnds(pb, id, GLP_LO, demands[t][i], 0);
		}
	}
	
	// Product capacity limit
	for (t = 0; t < nbPeriods; t++) {
		id  = t + 1;
		id += total * 2;
		
		sprintf(name, "[4](%d)", t + 1);
	
		glp_set_row_name(pb, id, name);
		glp_set_row_bnds(pb, id, GLP_UP, 0, capa[t]);
	}
	
	// - Problem's matrix
	
	// Basically, we will set each variable's coef one after the other,
	// traversing all indices. Per loop and variable, the ja -- column -- is 
	// constant, while we traverse constraints. The index of a variable or a
	// constraint is : glp + total * its_position.
	// glp is the index for glpk's arrays, id the indices of the sparse matrix.
	
	int *ia = NULL, *ja = NULL;
	double *ar = NULL;
	
	id = 0;
	
	// We have :
	// 		- 5 variables ranging on the whole scale ;
	// 		- 3 based on t - 1, thus skipping the t = 0 period ;
	// 		- 3 skipping t = nbPeriods - 1 to ignore last period.
	int size = 11 * total - 6 * nbProducts + 1;
	
	ia = new int[size];    // lines
	ja = new int[size];    // columns
	ar = new double[size]; // coefs
	
	// ylp is the glp index of the period, and plp will stand for glp during
	// period t - 1, tlp is our t' for M.
	int ylp = 0, plp = 0, tlp = 0;
	double M = 0.0;
	
	// Set coefs
	for (t = 0; t < nbPeriods; t++) {
		// Period index
		ylp = t * nbProducts + 1;
		
		for (i = 0; i < nbProducts; i++) {
			glp = ylp + i;
			
			// - Set Xit
			
			++id; // Index starts at 1
			
			// [1] Demands
			ia[id] = glp;
			ja[id] = glp;
			ar[id] = 1.0;
			
			++id;
			
			// [2] Activity
			ia[id] = glp + total * 1;
			ja[id] = glp;
			ar[id] = 1.0;
		
			// Set Sum_t(Xi)
			
			++id;
		
			// [4] Capacity
			ia[id] = t + 1 + total * 3;
			ja[id] = glp;
			ar[id] = 1.0;
			
			// No stock or backlog during last period
			if (t < nbPeriods - 1) {
				// - Set Sit
			
				++id;
			
				// [1] Demands
				ia[id] = glp;
				ja[id] = glp + total;
				ar[id] = -1.0;
			
				// - Set Rit
			
				++id;
			
				// [1] Demands
				ia[id] = glp;
				ja[id] = glp + total * 2;
				ar[id] = 1.0;
			
				++id;
				
				// [3] Valid
				ia[id] = glp + total * 2;
				ja[id] = glp + total * 2;
				ar[id] = 1.0;
			
			}
			
			// - Set Yit
			
			++id;
			
			// [2] Activity
			ia[id] = glp + total * 1;
			ja[id] = glp + total * 3;
			
			for (tlp = t; tlp < nbPeriods; tlp++) {
				M += demands[tlp][i];
			}
			
			if (M < capa[t]) {
				M = capa[t];
			}
			
			ar[id] = - M;
			
			++id;
			
			// [3] Valid
			ia[id] = glp + total * 2;
			ja[id] = glp + total * 3;
			ar[id] = demands[t][i];
			
			// Skip t = 0
			if (t > 0) {
				// Switch to (t-1)
				plp = glp - nbProducts;
				
				// - Set Si(t-1)
				
				++id;
				
				// [1] Demands
				ia[id] = glp;
				ja[id] = plp + total;
				ar[id] = 1.0;
				
				++id;
				
				// [3] Valid
				ia[id] = glp + total * 2;
				ja[id] = plp + total;
				ar[id] = 1.0;
				
				// - Set Ri(t-1)
				
				++id;
			
				// [1] Demands
				ia[id] = glp;
				ja[id] = plp + total * 2;
				ar[id] = -1.0;
			}
		}
	}
	
	glp_load_matrix(pb, id, ia, ja, ar);
	
	return pb;
}

/**
 *	Branch & bound recursive algorithm. Solves problem in parameter, then, if it
 *	is interesting and admissible, branch it, and call on subproblems.
 *
 *	@param glp_prob* problem to solve
 *	@param Solution* best solution holder
 *	@param int       nb periods we are working on
 *	@param int       nb products we are working on
 *	@param ofstream  file for graphviz generation
 */
void mcls_bnb(
	glp_prob *old, Solution *sol
	, const int &nbPeriods, const int &nbProducts
	, std::ofstream *p
) {
	// - Solve problem
	glp_smcp parm;
	glp_init_smcp(&parm);
	parm.msg_lev = GLP_MSG_OFF;
	
	glp_iocp parmip;
	glp_init_iocp(&parmip);
	parmip.msg_lev = GLP_MSG_OFF;
	
	glp_simplex(old, &parm);
	glp_intopt(old,  &parmip);
	
	int save = ++sol->cpt;
	
	int total = nbProducts * nbPeriods;
	int i = 0, j = 0, id = 0, jd = 0;
	int min_i = 0, min_j = 0, num = 0;
	
	char name[10];
	
	int    *ind  = NULL, *ord = NULL;
	double *val  = NULL;
	
	// Problem data
	double z = glp_mip_obj_val(old);
	double y = 0.0, min = DBL_MAX;
	
	bool admissible = true;
	
	// *p <<save << " [label=\"" << z << '"';
	
	// Problem must be feasible : z > 0 (here epsilon), and have a lower bound
	// -- relaxed -- inferior to our current optimum.
	if (z > DBL_MIN && z < sol->z) {
		// Admissibility, we check each Yit, and if one is not a boolean,
		// solution is not admissible. We then force the lowest Yit to 0 then 1
		// and run the algorithm again, making it a tree exploration.
	
		for (i = 1; i <= nbPeriods; i++) {
			for (j = 1; j <= nbProducts; j++) {
				id = (i - 1) * nbPeriods + j + total * 3;
				y  = glp_mip_col_val(old, id);
		
				// Use numeric bounds to account for approximation errors, hence, to
				// have an admissible solution, we need the ys to be :
				// 		- smaller than approximate zero
				// 		- between 1 - & and 1 + &
				if (y >= DBL_MIN && (y <= 1.0 - DBL_EPSILON || y >= 1.0 + DBL_EPSILON)) {
					admissible = false;
		
					if (y < min) {
						min   = y;
						min_i = i;
						min_j = j;
					}
				}
			}
		}
	
		// Problem has no admissible solution, we branch and bind it.
		if (!admissible) {
			glp_prob *pb = NULL;
	
			pb = glp_create_prob();
	
			glp_copy_prob(pb, old, GLP_ON);
			
			// *p << "];\n";
			
			num = glp_add_rows(pb, 1);
		
			// Build constraint row
			ind = new int[2];
			val = new double[2];
		
			ind[1] = (min_i - 1) * nbProducts + min_j + total * 3;
			val[1] = 1.0;
			// Call is : pb, row_number, ja, ar
			glp_set_mat_row(pb, num, 1, ind, val);
		
			// - Recursive calls
			
			// = 0		
			sprintf(name, "Y%d,%d = 0", min_i, min_j);
			
			// *p << '\t' << save << " -- " << cpt + 1 << " [label=\"" << name << "\"];\n";
		
			glp_set_row_name(pb, num, name);
			glp_set_row_bnds(pb, num, GLP_FX, 0, 0);
		
			mcls_bnb(pb, sol, nbPeriods, nbProducts, p);
		
			// = 1
			sprintf(name, "Y%d,%d = 1", min_i, min_j);
		
			// *p << '\t' << save << " -- " << cpt + 1 << " [label=\"" << name << "\"];\n";
		
			glp_set_row_name(pb, num, name);
			glp_set_row_bnds(pb, num, GLP_FX, 1, 1);
		
			mcls_bnb(pb, sol, nbPeriods, nbProducts, p);
			
			// - Delete generated data
			
			ord = new int[2];
			ord[1] = num;
		
			glp_del_rows(pb, 1, ord);
			
			sprintf(name, "Y%d,%d", min_i, min_j);
		
			delete ind;
			delete val;
			delete ord;
	
			glp_delete_prob(pb);
		} else {
			sol->z = z;
			// *p << ", shape=rectangle, color=\"blue\", fontcolor=\"blue\"];\n";
			
			for (i = 1; i <= nbPeriods; i++) {
				for (j = 1; j <= nbProducts; j++) {
					id = (i - 1) * nbPeriods + j;
					jd = (i - 1) * nbPeriods + j;
					sol->x[jd] = glp_mip_col_val(old, id);
				}
			}
	
			for (i = 1; i <= nbPeriods; i++) {
				for (j = 1; j <= nbProducts; j++) {
					id  = (i - 1) * nbPeriods + j;
					id += total;
					jd  = (i - 1) * nbPeriods + j;
					sol->s[jd] = glp_mip_col_val(old, id);
				}
			}
	
			for (i = 1; i <= nbPeriods; i++) {
				for (j = 1; j <= nbProducts; j++) {
					id  = (i - 1) * nbPeriods + j;
					id += total * 2;
					jd  = (i - 1) * nbPeriods + j;
					sol->r[jd] = glp_mip_col_val(old, id);
				}
			}
	
			for (i = 1; i <= nbPeriods; i++) {
				for (j = 1; j <= nbProducts; j++) {
					id  = (i - 1) * nbPeriods + j;
					id += total * 3;
					jd  = (i - 1) * nbPeriods + j;
					sol->y[jd] = glp_mip_col_val(old, id);
				}
			}
		}
	} else {
		if (z < DBL_MIN) {
			// *p << ", shape=\"diamond\", color=\"red\", fontcolor=\"red\"];\n";
		} else {
			// *p << ", shape=rectangle, color=\"forestgreen\", fontcolor=\"forestgreen\"];\n";
		}
	}
}

/**
 *	Solves a capacity lot-sizing problem using a Branch & Bound approach.
 *
 *	@param string file to load data from
 */
void mcls_solve(const std::string data)
{
	char gv[18]    = "res/mcls.gv";
	char tex[19]   = "res/mcls.tex";
	char png[19]   = "res/mcls.png";
	char cplex[21] = "res/mcls.cplex";

	std::ifstream *f = new std::ifstream(data.c_str());
	
	if (!f->good()) {
		std::cout << "Fichier " << data << " non trouvé." << std::endl;
		return;
	}
	
	int nbProducts = 0, nbPeriods = 0;
	int *back = NULL, *open = NULL, *stock = NULL, *capa = NULL;
	int **demands = NULL;
	
	// Generate data from file
	mcls_parse(f, nbProducts, nbPeriods, back, open, stock, capa, demands);
	
	delete f;
	
	glp_prob *pb = NULL;
	
	pb = mcls_create(nbProducts, nbPeriods, back, open, stock, capa, demands);
	
	std::ofstream *p = new std::ofstream(gv);
	Solution *sol    = new Solution(nbPeriods, nbProducts);
	
	// *p << "graph {\n";
	// *p << "graph [rank=same, splines=false];";
	// *p << "node  [fixedsize=true, width=1.5, height=0.6];";
	
	mcls_bnb(pb, sol, nbProducts, nbPeriods, p);
	
	// *p << '}';
	p->close();
	
	char call[50];
	
	sprintf(call, "dot -T png -o %s %s", png, gv);
	
	// system(call);
	
	// - Display found solution
	
	sol->show();
	sol->write(tex);
	
	glp_write_lp(pb, NULL, cplex);
}
