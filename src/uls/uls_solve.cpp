#include "uls_solve.h"

void uls_parser(std::string fichier_arg, int*& demande, int*& cout_prod, int*& cout_stock,int*& cout_acti, int& nbPeriode){

	int i;
	std::ifstream* fichier_traite;
	fichier_traite = new std::ifstream(fichier_arg.c_str());
	*fichier_traite >> nbPeriode;
	cout_stock = new int[nbPeriode];
	cout_prod = new int[nbPeriode];
	demande = new int[nbPeriode];
	cout_acti = new int[nbPeriode];
	
	for(i=0; i< nbPeriode; i++){
		*fichier_traite >> demande[i];
	}
	for(i=0; i< nbPeriode; i++){
		*fichier_traite >> cout_prod[i];
	}
	for(i=0; i< nbPeriode; i++){
		*fichier_traite >> cout_stock[i];
	}
	for(i=0; i< nbPeriode; i++){
		*fichier_traite >> cout_acti[i];
	}
	delete fichier_traite;
}

void uls_recursif(Result*& solopti,glp_prob* probref, int* ia , int* ja ,double* ar, int tail_mat , int nbPeriode, int& appels){

	//Result* retour;
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; /* Paramtre de GLPK dans la rsolution d'un PL en variables continues afin de couper les affichages (dans lesquels on se noierait) */
	int i;
    glp_iocp parmip;
    glp_init_iocp(&parmip);
    parmip.msg_lev = GLP_MSG_OFF;
	//std::cout << "hola" << std::endl;

        glp_load_matrix(probref,tail_mat-1,ia,ja,ar); /* chargement de la matrice dans le problme */
		//std::cout << "charge" << std::endl;


        glp_simplex(probref,&parm);
        glp_intopt(probref,&parmip);
		//std::cout << "resolu" << std::endl;
		appels++;
		int* x = new int[nbPeriode];
		double* y = new double[nbPeriode];
		int plusfaible = nbPeriode+1;
		bool admissible = true;
		double z = glp_mip_obj_val(probref);
		if( z!=0 ){
		if(z < solopti->val())
		{	//std::cout << "interet" << std::endl;
			for(i=0;i<nbPeriode;i++) x[i] = glp_mip_col_val(probref,nbPeriode+i+1);
			for(i=0;i<nbPeriode;i++) y[i] = glp_mip_col_val(probref,2*nbPeriode+i+1);
			for(i=0;i< nbPeriode; i++){
				if((y[i] != 0 ) && (y[i] != 1)){
					admissible = false;
					if(plusfaible != nbPeriode+1){
						if(y[i] < y[plusfaible]){
						plusfaible= i;
						}
					}else{
						plusfaible= i;
					}
				}
			}
			if(admissible){
				//std::cout << "admis" << std::endl;
				solopti->set_val(z);
				solopti->set_sol(x);
			}else{
				//std::cout << "branch" << std::endl;
				glp_prob *prob_temp;
				//std::cout << "hola" << std::endl;
				prob_temp = glp_create_prob();
				//std::cout << "hola" << std::endl;
				glp_copy_prob(prob_temp, probref, GLP_OFF);
				//std::cout << "hola" << std::endl;
				int* temp_ia = new int[tail_mat+1];
				int* temp_ja = new int[tail_mat+1];
				double* temp_ar = new double[tail_mat+1];
				for(i = 0; i< tail_mat; i++){
					temp_ia[i]=ia[i];
					temp_ja[i]=ja[i];
					temp_ar[i]=ar[i];
				}
				//std::cout << "hola" << std::endl;
				temp_ia[tail_mat] = glp_add_rows(prob_temp, 1);;
				temp_ja[tail_mat] = 2*nbPeriode + plusfaible + 1;
				temp_ar[tail_mat] = 1.0f;
				//std::cout << "hola" << std::endl;
				glp_set_row_bnds(prob_temp, temp_ia[tail_mat], GLP_FX, 0.0f, 0.0f);
				//std::cout << "hola" << std::endl;
				uls_recursif(solopti,prob_temp, temp_ia , temp_ja , temp_ar, tail_mat + 1 , nbPeriode, appels);
				//std::cout << "redef" << std::endl;
				glp_set_row_bnds(prob_temp, temp_ia[tail_mat], GLP_FX, 1.0f, 1.0f);
				
				uls_recursif(solopti,prob_temp, temp_ia , temp_ja , temp_ar, tail_mat + 1 , nbPeriode, appels);
				//std::cout << "remonte" << std::endl;
				delete[] x;
			delete[] y;
			delete[] temp_ia;
			delete[] temp_ja;
			delete[] temp_ar;
			glp_delete_prob(prob_temp);
			}
			
		}else{
			//std::cout << "pas interet" << std::endl;
			delete[] x;
			delete[] y;
			//return solopti;
		}}else{
			delete[] x;
			delete[] y;
		}
		
		

}

void uls_solve(const std::string fichier_arg){

	int* demande;
int* 	cout_prod;
int*  cout_stock;
int*  cout_acti;
	int nbPeriode;
	int j;
	int appels = 0;
	 // si un nom de fichier pass en argument
        uls_parser(fichier_arg,demande, cout_prod, cout_stock, cout_acti,nbPeriode);  // on le rcupere
    
	int* M = new int[nbPeriode];
	for(j=0;j<nbPeriode;j++){
		M[j] = 0;
	}
	for(j=0;j<nbPeriode;j++){
		for(int i = 0; i <= j; i++){
			M[i] = M[i]+demande[j];
		}
	}
	//for(j=0;j<nbPeriode;j++){
		//std::cout << M[j] << std::endl;
	//}
	/*for(j=0;j<nbPeriode;j++){
	std::cout << demande[j] << " ";
	}
	std::cout << std::endl;
	for(j=0;j<nbPeriode;j++){
	std::cout << cout_prod[j] << " ";
	}
	std::cout << std::endl;
	for(j=0;j<nbPeriode;j++){
	std::cout << cout_stock[j] << " ";
	}
	std::cout << std::endl;
	for(j=0;j<nbPeriode;j++){
	std::cout << cout_acti[j] << " ";
	}
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;*/
	////////////////////////
	//     code GLPK      //
	////////////////////////
	glp_prob *prob; /* dclaration du pointeur sur le problme */
    prob = glp_create_prob(); /* allocation mmoire pour le problme */
    glp_set_prob_name(prob, "ULS"); /* affectation d'un nom */
    glp_set_obj_dir(prob, GLP_MIN); /* Il s'agit d'un problme de minimisation */
    //Result* resultat;
    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; /* Paramtre de GLPK dans la rsolution d'un PL en variables continues afin de couper les affichages (dans lesquels on se noierait) */

    glp_iocp parmip;
    glp_init_iocp(&parmip);
    parmip.msg_lev = GLP_MSG_OFF;
	
	
	glp_add_rows(prob, 3*nbPeriode+1);
	for(j=1;j<=nbPeriode;j++){// s x y
	glp_set_row_bnds(prob, j, GLP_UP, 0.0, 0.0);
	}
	for(j=nbPeriode+1;j<=2*nbPeriode;j++){// s x y
	glp_set_row_bnds(prob, j, GLP_FX, demande[j-nbPeriode-1], demande[j-nbPeriode-1]);
	}
	for(j=2*nbPeriode+1;j<=3*nbPeriode;j++){// s x y
	glp_set_row_bnds(prob, j, GLP_LO, demande[j-2*nbPeriode-1], 1000.0);
	}
	glp_set_row_bnds(prob, 3*nbPeriode+1, GLP_FX, 1.0, 1.0);
	glp_add_cols(prob, 3*nbPeriode);
	for(j=1;j<=2*nbPeriode;j++){// s x y
	glp_set_col_bnds(prob, j, GLP_LO, 0.0, 1000.0);
	}
	for(j=2*nbPeriode+1;j<=3*nbPeriode;j++){
	glp_set_col_bnds(prob, j, GLP_DB, 0.0, 1.0);
	}
	
	for(j=1;j<=nbPeriode;j++){// s x y
	glp_set_obj_coef(prob, j, (double) cout_stock[j-1] );
	glp_set_obj_coef(prob, j+nbPeriode, (double) cout_prod[j-1] );
	glp_set_obj_coef(prob, j+2*nbPeriode, (double) cout_acti[j-1] );
	}
	
	int* ia = new int[7*nbPeriode];
	int* ja = new int[7*nbPeriode];
	double* ar = new double[7*nbPeriode];
	
	int compteur =0 ;
	for(int i = 1; i<= nbPeriode; i++){//x premiere contrainte
		compteur++;
		ia[compteur] = i;
		ja[compteur] = nbPeriode+i;
		ar[compteur] = 1.0f;
	}
	for(int i = 1; i<= nbPeriode; i++){//y premiere contrainte
		compteur++;
		ia[compteur] = i;
		ja[compteur] = 2*nbPeriode+i;
		ar[compteur] = - ((double) M[i-1]);
	}
	for(int i = 1; i<= nbPeriode; i++){//x deuxieme contrainte
		compteur++;
		ia[compteur] = i+nbPeriode;
		ja[compteur] = nbPeriode+i;
		ar[compteur] = 1.0f;
	}
	for(int i = 1; i<= nbPeriode; i++){//s deuxieme contrainte
		compteur++;
		ia[compteur] = i+nbPeriode;
		ja[compteur] = i;
		ar[compteur] = -1.0f;
	}
	for(int i = 2; i<= nbPeriode; i++){//s-1 deuxieme contrainte
		compteur++;
		ia[compteur] = i+nbPeriode;
		ja[compteur] = i-1;
		ar[compteur] = 1.0f;
	}
	for(int i = 1; i<= nbPeriode; i++){//y troisieme contrainte
		compteur++;
		ia[compteur] = i+2*nbPeriode;
		ja[compteur] = 2*nbPeriode+i;
		ar[compteur] = (double) demande[i-1];
	}
	for(int i = 2; i<= nbPeriode; i++){//s troisieme contrainte
		compteur++;
		ia[compteur] = i+2*nbPeriode;
		ja[compteur] = i-1;
		ar[compteur] = 1.0f;
	}
	compteur++;
	ia[compteur] = 3*nbPeriode+1;
	ja[compteur] = 2*nbPeriode+1;
	ar[compteur] = 1.0f;
	//std::cout << compteur << " : " << ia[compteur] << " " << ja[compteur] << " : " << ar[compteur] << std::endl;
	//for(compteur = 1; compteur < 7*nbPeriode ; compteur++){
//		std::cout << compteur << " : " << ia[compteur] << " " << ja[compteur] << " : " << ar[compteur] << std::endl;
	//}
	
	Result* solopti = new Result(12000, new int[4]);
	//std::cout << "hola" << std::endl;
	uls_recursif(solopti, prob , ia , ja , ar, 7*nbPeriode , nbPeriode, appels);
	std::cout << "Nombre de résolutions : " << appels << std::endl;
	std::cout <<"valeur optimale : " << solopti->val() << std::endl << std::endl << "Plan de Production : " <<std::endl;
	int* x = solopti->solution();
	for(int i = 0 ; i < nbPeriode; i++){
		std::cout << "x[" << i +1 << "]= " << x[i] << std::endl;
	}
	
}
