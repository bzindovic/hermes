#define H2D_REPORT_WARN
#define H2D_REPORT_INFO
#define H2D_REPORT_VERBOSE
#define H2D_REPORT_FILE "application.log"
#include "hermes2d.h"

using Teuchos::RCP;
using Teuchos::rcp;

// This test makes sure that example 44-trilinos-coupled works correctly.

const int INIT_REF_NUM = 2;         // Number of initial uniform mesh refinements.
const int P_INIT = 2;               // Initial polynomial degree of all mesh elements.
const bool JFNK = true;             // true = jacobian-free method,
                                    // false = Newton
const int PRECOND = 2;              // Preconditioning by jacobian (1) (less GMRES iterations, more time to create precond)
                                    // or by approximation of jacobian (2) (less time for precond creation, more GMRES iters).
                                    // in case of jfnk,
                                    // default Ifpack proconditioner in case of Newton.
const double TAU = 0.05;            // Time step.
const double T_FINAL = 2*TAU + 1e-4;        // Time interval length.
const bool TRILINOS_OUTPUT = true;  // Display more details about nonlinear and linear solvers.

// Problem parameters.
const double Le    = 1.0;
const double alpha = 0.8;
const double beta  = 10.0;
const double kappa = 0.1;
const double x1    = 9.0;

// Boundary markers.
int bdy_left = 1;

// Boundary condition types.
BCType bc_types(int marker)
  { return (marker == bdy_left) ? BC_ESSENTIAL : BC_NATURAL; }

// Essential (Dirichlet) boundary condition values.
scalar essential_bc_values_t(int ess_bdy_marker, double x, double y)
  { return (ess_bdy_marker == bdy_left) ? 1.0 : 0; }

scalar essential_bc_values_c(int ess_bdy_marker, double x, double y)
  { return 0; }

// Initial conditions.
scalar temp_ic(double x, double y, scalar& dx, scalar& dy)
  { return (x <= x1) ? 1.0 : exp(x1 - x); }

scalar conc_ic(double x, double y, scalar& dx, scalar& dy)
  { return (x <= x1) ? 0.0 : 1.0 - exp(Le*(x1 - x)); }

// Weak forms. 
# include "forms.cpp"

int main(int argc, char* argv[])
{
  // Load the mesh.
  Mesh mesh;
  H2DReader mloader;
  mloader.load("domain.mesh", &mesh);

  // Perform initial mesh refinemets.
  for (int i=0; i < INIT_REF_NUM; i++)  mesh.refine_all_elements();

  // Create H1 spaces with default shapesets.
  H1Space* t_space = new H1Space(&mesh, bc_types, essential_bc_values_t, P_INIT);
  H1Space* c_space = new H1Space(&mesh, bc_types, essential_bc_values_c, P_INIT);
  int ndof = get_num_dofs(Tuple<Space *>(t_space, c_space));
  info("ndof = %d.", ndof);

  // Define initial conditions.
  Solution t_prev_time_1, c_prev_time_1, t_prev_time_2, 
           c_prev_time_2, t_iter, c_iter, t_prev_newton, c_prev_newton;
  t_prev_time_1.set_exact(&mesh, temp_ic);  
  c_prev_time_1.set_exact(&mesh, conc_ic);
  t_prev_time_2.set_exact(&mesh, temp_ic);  
  c_prev_time_2.set_exact(&mesh, conc_ic);
  t_iter.set_exact(&mesh, temp_ic);   
  c_iter.set_exact(&mesh, conc_ic);

  // Filters for the reaction rate omega and its derivatives.
  DXDYFilter omega(omega_fn, Tuple<MeshFunction*>(&t_prev_time_1, &c_prev_time_1));
  DXDYFilter omega_dt(omega_dt_fn, Tuple<MeshFunction*>(&t_prev_time_1, &c_prev_time_1));
  DXDYFilter omega_dc(omega_dc_fn, Tuple<MeshFunction*>(&t_prev_time_1, &c_prev_time_1));

  // Initialize visualization.
  ScalarView rview("Reaction rate", new WinGeom(0, 0, 800, 230));

  // Initialize weak formulation.
  WeakForm wf(2, JFNK ? true : false);
  if (!JFNK || (JFNK && PRECOND == 1))
  {
    wf.add_matrix_form(0, 0, callback(newton_bilinear_form_0_0), H2D_UNSYM, H2D_ANY, &omega_dt);
    wf.add_matrix_form_surf(0, 0, callback(newton_bilinear_form_0_0_surf), 3);
    wf.add_matrix_form(1, 1, callback(newton_bilinear_form_1_1), H2D_UNSYM, H2D_ANY, &omega_dc);
    wf.add_matrix_form(0, 1, callback(newton_bilinear_form_0_1), H2D_UNSYM, H2D_ANY, &omega_dc);
    wf.add_matrix_form(1, 0, callback(newton_bilinear_form_1_0), H2D_UNSYM, H2D_ANY, &omega_dt);
  }
  else if (PRECOND == 2)
  {
    wf.add_matrix_form(0, 0, callback(precond_0_0));
    wf.add_matrix_form(1, 1, callback(precond_1_1));
  }
  wf.add_vector_form(0, callback(newton_linear_form_0), H2D_ANY, 
                     Tuple<MeshFunction*>(&t_prev_time_1, &t_prev_time_2, &omega));
  wf.add_vector_form_surf(0, callback(newton_linear_form_0_surf), 3);
  wf.add_vector_form(1, callback(newton_linear_form_1), H2D_ANY, 
                     Tuple<MeshFunction*>(&c_prev_time_1, &c_prev_time_2, &omega));

  // Project the functions "t_iter" and "c_iter" on the FE space 
  // in order to obtain initial vector for NOX. 
  info("Projecting initial solutions on the FE meshes.");
  Vector* coeff_vec = new AVector(ndof);
  project_global(Tuple<Space *>(t_space, c_space), Tuple<int>(H2D_H1_NORM, H2D_H1_NORM), 
                 Tuple<MeshFunction*>(&t_prev_time_1, &c_prev_time_1), 
                 Tuple<Solution*>(&t_prev_time_1, &c_prev_time_1),
                 coeff_vec);

  // Initialize finite element problem.
  FeProblem fep(&wf, Tuple<Space*>(t_space, c_space));

  // Initialize NOX solver and preconditioner.
  NoxSolver solver(&fep);
  RCP<Precond> pc = rcp(new MlPrecond("sa"));
  if (PRECOND)
  {
    if (JFNK) solver.set_precond(pc);
    else solver.set_precond("Ifpack");
  }
  if (TRILINOS_OUTPUT)
    solver.set_output_flags(NOX::Utils::Error | NOX::Utils::OuterIteration |
                            NOX::Utils::OuterIterationStatusTest |
                            NOX::Utils::LinearSolverDetails);

  // Time stepping loop:
  double total_time = 0.0;
  for (int ts = 1; total_time <= T_FINAL; ts++)
  {
    info("---- Time step %d, t = %g s", ts, total_time + TAU);

    solver.set_init_sln(coeff_vec->get_c_array());
    bool solved = solver.solve();
    if (solved)
    {
      double* coeffs = solver.get_solution_vector();
      t_prev_newton.set_fe_solution(t_space, coeffs, ndof);
      c_prev_newton.set_fe_solution(c_space, coeffs, ndof);

      info("Number of nonlin iterations: %d (norm of residual: %g)",
          solver.get_num_iters(), solver.get_residual());
      info("Total number of iterations in linsolver: %d (achieved tolerance in the last step: %g)",
          solver.get_num_lin_iters(), solver.get_achieved_tol());

      // Update global time.
      total_time += TAU;

      // Saving solutions for the next time step.
      t_prev_time_2.copy(&t_prev_time_1);
      c_prev_time_2.copy(&c_prev_time_1);
      t_prev_time_1 = t_prev_newton;
      c_prev_time_1 = c_prev_newton;
    }
    else
      error("NOX failed.");

  }

  info("Coordinate (  0,   8) value = %lf", t_prev_time_1.get_pt_value(0.0, 8.0));
  info("Coordinate (  8,   8) value = %lf", t_prev_time_1.get_pt_value(8.0, 8.0));
  info("Coordinate ( 15,   8) value = %lf", t_prev_time_1.get_pt_value(15.0, 8.0));
  info("Coordinate ( 24,   8) value = %lf", t_prev_time_1.get_pt_value(24.0, 8.0));
  info("Coordinate ( 30,   8) value = %lf", t_prev_time_1.get_pt_value(30.0, 8.0));
  info("Coordinate ( 40,   8) value = %lf", t_prev_time_1.get_pt_value(40.0, 8.0));
  info("Coordinate ( 50,   8) value = %lf", t_prev_time_1.get_pt_value(50.0, 8.0));
  info("Coordinate ( 60,   8) value = %lf", t_prev_time_1.get_pt_value(60.0, 8.0));

  info("Coordinate (  0,   8) value = %lf", c_prev_time_1.get_pt_value(0.0, 8.0));
  info("Coordinate (  8,   8) value = %lf", c_prev_time_1.get_pt_value(8.0, 8.0));
  info("Coordinate ( 15,   8) value = %lf", c_prev_time_1.get_pt_value(15.0, 8.0));
  info("Coordinate ( 24,   8) value = %lf", c_prev_time_1.get_pt_value(24.0, 8.0));
  info("Coordinate ( 30,   8) value = %lf", c_prev_time_1.get_pt_value(30.0, 8.0));
  info("Coordinate ( 40,   8) value = %lf", c_prev_time_1.get_pt_value(40.0, 8.0));
  info("Coordinate ( 50,   8) value = %lf", c_prev_time_1.get_pt_value(50.0, 8.0));
  info("Coordinate ( 60,   8) value = %lf", c_prev_time_1.get_pt_value(60.0, 8.0));

#define ERROR_SUCCESS                                0
#define ERROR_FAILURE                               -1
  double coor_x[8] = {0.0, 8.0, 15.0, 24.0, 30.0, 40.0, 50.0, 60.0};
  double coor_y = 8.0;
  double t_value[8] = {1.000000, 1.006876, 0.013766, 0.000004, 0.000000, 0.000000, 0.000000, 0.000000};
  double c_value[8] = {0.000000, -0.006876, 0.986234, 0.999996, 1.000000, 1.000000, 1.000000, 1.000000};
  for (int i = 0; i < 8; i++)
  {
    if (!((t_value[i] - t_prev_time_1.get_pt_value(coor_x[i], coor_y)) < 1E-4) || 
        !((c_value[i] - c_prev_time_1.get_pt_value(coor_x[i], coor_y)) < 1E-4))
    {
      printf("Failure!\n");
      return ERROR_FAILURE;
    }
  }
  printf("Success!\n");
  return ERROR_SUCCESS;
}

