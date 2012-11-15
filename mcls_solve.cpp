#include "mcls_solve.h"

class Solution
{
	public :
		int z;
		int *x, *y;
};

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
			// Cost is nil for every period
			glp_set_obj_coef(pb, id, 0);
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
	
	// Product capacity limit
	for (t = 0; t < nbPeriods; t++) {
		for (i = 0; i < nbProducts; i++) {
			id  = t * nbProducts + i + 1;
			id += total * 1;
			
			sprintf(name, "[2](%d,%d)", t + 1, i + 1);
		
			glp_set_row_name(pb, id, name);
			glp_set_row_bnds(pb, id, GLP_UP, 0, 0);
		}
	}
	
	// Line activity
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
	
	int *ia, *ja;
	double *ar;
	
	id = 0;
	
	// We have 8 constraints ranging on the full spectrum, while 3 -- with (t-1)
	// -- skip one loop on nbPeriods.
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
	*/
	
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
	
	glp_load_matrix(pb, id, ia, ja, ar);
	
	return pb;
}

void mcls_bnb(glp_prob *pb, Solution &sol, const int &nbProducts, const int &nbPeriods)
{
	glp_smcp parm;
	glp_init_smcp(&parm);
	// parm.msg_lev = GLP_MSG_OFF;
	
	glp_iocp parmip;
	glp_init_iocp(&parmip);
	// parmip.msg_lev = GLP_MSG_OFF;
	
	glp_simplex(pb, &parm);
	glp_intopt(pb, &parmip);
	
	int i = 0, j = 0, id = 0;
	int total = nbProducts * nbPeriods;
	double z = glp_mip_obj_val(pb);
	
	std::cout << "Optimal : " << z << std::endl;
	
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id = (i - 1) * nbPeriods + j;
			std::cout << 'X' << i << ',' << j << " : " << glp_mip_col_val(pb, id) << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id  = (i - 1) * nbPeriods + j;
			id += total;
			std::cout << 'S' << i << ',' << j << " : " << glp_mip_col_val(pb, id) << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id  = (i - 1) * nbPeriods + j;
			id += total * 2;
			std::cout << 'R' << i << ',' << j << " : " << glp_mip_col_val(pb, id) << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	
	for (i = 1; i <= nbPeriods; i++) {
		for (j = 1; j <= nbProducts; j++) {
			id  = (i - 1) * nbPeriods + j;
			id += total * 3;
			std::cout << 'Y' << i << ',' << j << " : " << glp_mip_col_val(pb, id) << '\t';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	
	glp_write_mps(pb, GLP_MPS_FILE, NULL, "mps");
	glp_write_lp(pb, NULL, "cplex");
}

/**
 *	Solves a capacity lot-sizing problem using a Branch & Bound approach.
 *
 *	@param string name of the data file
 */
void mcls_solve(const std::string fileName)
{
	std::ifstream *f;
	
	f = new std::ifstream(fileName.c_str());
	
	if (!f->good()) {
		std::cout << "Fichier " << fileName << " non trouvé." << std::endl;
		return;
	}
	
	int nbProducts, nbPeriods;
	int *back, *open, *stock, *capa;
	int **demands;
	
	// Generate data from file
	mcls_parse(f, nbProducts, nbPeriods, back, open, stock, capa, demands);
	
	delete f;
	
	glp_prob *pb = NULL;
	
	pb = mcls_create(nbProducts, nbPeriods, back, open, stock, capa, demands);
	
	Solution sol;
	
	mcls_bnb(pb, sol, nbProducts, nbPeriods);
	
	return;
}

int main()
{
	mcls_solve("ppa.crap");
	
	return 1;
}
