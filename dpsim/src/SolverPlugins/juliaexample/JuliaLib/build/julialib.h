#include <dpsim/MNASolverDynInterface.h>

int example_init(struct dpsim_csr_matrix *matrix);
int example_decomp(struct dpsim_csr_matrix *matrix);
int example_solve(double *rhs_values,
				  double *lhs_values);
void example_log(const char *str);
void example_cleanup(void);