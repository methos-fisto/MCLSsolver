#include "mcls_solve.h"

class Solution
{
	public :
		double z;
		int *x, *s, *r, *y;
		int cpt;
		
		Solution(const int);
};

Solution::Solution(const int size)
{
	cpt = 0;
	
	z = DBL_MAX;
	x = new int[size];
	s = new int[size];
	r = new int[size];
	y = new int[size];
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
	// 	3) product capacity
	glp_add_rows(pb, total * 2 + nbPeriods);
	
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
	
	// Product capacity limit
	for (t = 0; t < nbPeriods; t++) {
		id  = t + 1;
		id += total * 2;
		
		sprintf(name, "[3](%d)", t + 1);
	
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
	// 		- 4 variables ranging on the whole scale ;
	// 		- 3 based on t - 1, thus skipping the t = 0 period ;
	// 		- 1 skipping t = nbPeriods - 1 to ignore last period.
	int size = (4 + 4) * total - 4 * nbProducts + 1;
	
	ia = new int[size];    // lines
	ja = new int[size];    // columns
	ar = new double[size]; // coefs
	
	// ylp is the glp index of the period, and plp will stand for glp during
	// period t - 1.
	int ylp = 0, plp = 0;
	
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
		
			// [3] Capacity
			ia[id] = t + 1 + total * 2;
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
			
			}
			
			// - Set Yit
			
			++id;
		
			// [2] Activity
			ia[id] = glp + total * 1;
			ja[id] = glp + total * 3;
			ar[id] = (double) - capa[t];
			
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
				
				// - Set Ri(t-1)
				
				++id;
			
				// [1] Demands
				ia[id] = glp;
				ja[id] = plp + total * 2;
				ar[id] = -1.0;
			}
		}
	}
	
	/*
	for (i = 1; i <= id; i++) {
		sprintf(name, "ia[%d] : %d\tja[%d] : %d\tar[%d] : %f", i, ia[i], i, ja[i], i, ar[i]);
		std::cout << name << std::endl;
	}
	
	std::cout << "Total : " << total << " ; id = " << id << " ; size = " << size << std::endl;
	std::cout << "Variables : " << total * 4 << " ; Constraints : " << total * 3 + nbPeriods + nbProducts << std::endl;
	std::cout << std::endl;
	
	for (t = 0; t < nbPeriods; t++) {
		std::cout << 'C' << t + 1 << "   : " << capa[t] << '\t';
	}
	std::cout << std::endl;
	std::cout << std::endl;
	
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			std::cout << 'D' << t + 1 << ',' << i + 1 << " : " << demands[t][i] << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	*/
	
	glp_load_matrix(pb, id, ia, ja, ar);
	
	return pb;
}

void mcls_bnb(
	glp_prob *old, Solution *sol
	, const int &nbPeriods, const int &nbProducts
	, std::ofstream *p
) {
	glp_prob *pb = NULL;
	
	pb = glp_create_prob();
	
	glp_copy_prob(pb, old, GLP_ON);

	// - Solve problem
	glp_smcp parm;
	glp_init_smcp(&parm);
	parm.msg_lev = GLP_MSG_OFF;
	
	glp_iocp parmip;
	glp_init_iocp(&parmip);
	parmip.msg_lev = GLP_MSG_OFF;
	
	glp_simplex(pb, &parm);
	glp_intopt(pb, &parmip);
	
	/*
	if (sol->cpt == 0) {
		glp_write_lp(pb, NULL, "cplex0");
	}
	
	if (sol->cpt == 8) {
		glp_write_lp(pb, NULL, "cplex1");
	}
	*/
	
	int save = 0;
	save = ++sol->cpt;
	
	int total = nbProducts * nbPeriods;
	int i = 0, j = 0, id = 0, jd = 0;
	int min_i = 0, min_j = 0, num = 0;
	
	char name[10], var[10];
	
	int    *ind  = NULL, *ord = NULL;
	double *val  = NULL;
	
	// Problem data
	double z = glp_mip_obj_val(pb);
	double y = 0.0, min = DBL_MAX;
	
	bool admissible = true;
	
	*p << save << " [label=\"" << z << '"';
	
	// Admissibility, we check each Yit, and if one is not a boolean,
	// solution is not admissible. We then force the lowest Yit to 0 then 1
	// and run the algorithm again, making it a tree exploration.
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id = (i - 1) * nbPeriods + j + total * 3;
			y  = glp_mip_col_val(pb, id);
		
			sprintf(var, "%1.3f", y);
		
			std::cout << 'y' << i << ',' << j << " = " << var << ' ';
			
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
		std::cout << std::endl;
	}

	std::cout << (admissible ? 'A' : 'X') << " ? z = " << z << std::endl << std::endl;
	
	// Problem must be feasible : z > 0 (here epsilon), and have a lower --
	// relaxed -- bond inferior to our current optimum.
	if (z > DBL_MIN && z < sol->z) {
	
		// Problem has no admissible solution, we branch and bind it.
		if (!admissible) {
			*p << "];\n";
			
			num = glp_add_rows(pb, 1);
			// num = glp_get_num_rows(pb);
		
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
			
			std::cout << '(' << sol->cpt << ')' << " - Add " << name << std::endl;
			
			*p << '\t' << save << " -- " << sol->cpt + 1 << " [label=\"" << name << "\"];\n";
		
			glp_set_row_name(pb, num, name);
			glp_set_row_bnds(pb, num, GLP_FX, 0, 0);
		
			mcls_bnb(pb, sol, nbPeriods, nbProducts, p);
		
			// = 1
			sprintf(name, "Y%d,%d = 1", min_i, min_j);
		
			std::cout << '(' << sol->cpt << ')' << " - Add " << name << std::endl;
			
			*p << '\t' << save << " -- " << sol->cpt + 1 << " [label=\"" << name << "\"];\n";
		
			glp_set_row_name(pb, num, name);
			glp_set_row_bnds(pb, num, GLP_FX, 1, 1);
		
			mcls_bnb(pb, sol, nbPeriods, nbProducts, p);
			
			// - Delete generated data
			
			ord = new int[2];
			ord[1] = num;
		
			glp_del_rows(pb, 1, ord);
			
			sprintf(name, "Y%d,%d", min_i, min_j);
			std::cout << "Remove bond on " << name << std::endl;
		
			delete ind;
			delete val;
			delete ord;
		} else {
			sol->z = z;
			*p << "shape=rectangle, color=\"blue\", fontcolor=\"blue\"];\n";
			
			for (i = 1; i <= nbPeriods; i++) {
				for (j = 1; j <= nbProducts; j++) {
					id = (i - 1) * nbPeriods + j;
					jd = (i - 1) * nbPeriods + j;
					sol->x[jd] = glp_mip_col_val(pb, id);
				}
			}
	
			for (i = 1; i <= nbPeriods; i++) {
				for (j = 1; j <= nbProducts; j++) {
					id  = (i - 1) * nbPeriods + j;
					id += total;
					jd  = (i - 1) * nbPeriods + j;
					sol->s[jd] = glp_mip_col_val(pb, id);
				}
			}
	
			for (i = 1; i <= nbPeriods; i++) {
				for (j = 1; j <= nbProducts; j++) {
					id  = (i - 1) * nbPeriods + j;
					id += total * 2;
					jd  = (i - 1) * nbPeriods + j;
					sol->r[jd] = glp_mip_col_val(pb, id);
				}
			}
	
			for (i = 1; i <= nbPeriods; i++) {
				for (j = 1; j <= nbProducts; j++) {
					id  = (i - 1) * nbPeriods + j;
					id += total * 3;
					jd  = (i - 1) * nbPeriods + j;
					sol->y[jd] = glp_mip_col_val(pb, id);
				}
			}
		}
	} else {
		if (z < DBL_MIN) {
			std::cout << "Non-feasible !" << std::endl << std::endl;
			*p << ", shape=\"diamond\"];\n";
		} else {
			*p << "shape=rectangle, color=\"red\", fontcolor=\"red\"];\n";
		}
	}
	
	glp_delete_prob(pb);
}

/**
 *	Solves a capacity lot-sizing problem using a Branch & Bound approach.
 *
 *	@param string name of the data file
 */
void mcls_solve(const std::string fileName)
{
	std::ifstream *f = new std::ifstream(fileName.c_str());
	std::ofstream *p = new std::ofstream("ppa8.gv");
	
	if (!f->good()) {
		std::cout << "Fichier " << fileName << " non trouvé." << std::endl;
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
	
	Solution *sol = new Solution(nbProducts * nbPeriods);
	
	*p << "graph {\n";
	*p << "graph [rank=same];";
	*p << "node  [fixedsize=true, width=1.5, height=0.6];";
	
	mcls_bnb(pb, sol, nbProducts, nbPeriods, p);
	
	*p << "}";
	p->close();
	
	// system("dot -T png -o ppa40.png ppa4.gv");
	
	// - Display found solution
	
	/*
	int i = 0, j = 0, id = 0;
	
	std::cout << std::endl << "Trouvé : " << sol->z << " en : " << sol->cpt << " itérations." << std::endl;
	
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id = (i - 1) * nbPeriods + j;
			std::cout << 'X' << i << ',' << j << " : " << sol->x[id] << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id  = (i - 1) * nbPeriods + j;
			std::cout << 'S' << i << ',' << j << " : " << sol->s[id] << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id  = (i - 1) * nbPeriods + j;
			std::cout << 'R' << i << ',' << j << " : " << sol->r[id] << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id  = (i - 1) * nbPeriods + j;
			std::cout << 'Y' << i << ',' << j << " : " << sol->y[id] << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	*/
	// glp_write_lp(pb, NULL, "cplex");
	
	return;
}

int main()
{
	mcls_solve("ppa8.crap");
	
	return 1;
}
