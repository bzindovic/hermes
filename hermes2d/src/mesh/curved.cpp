// This file is part of Hermes2D.
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

#include "curved.h"
#include <algorithm>
#include "global.h"
#include "shapeset/shapeset_h1_all.h"
#include "shapeset/shapeset_common.h"
#include "shapeset/precalc.h"
#include "mesh.h"
#include "quad_all.h"
#include "matrix.h"
#include "algebra/dense_matrix_operations.h"
#include "mesh/refmap.h"

using namespace Hermes::Algebra::DenseMatrixOperations;

namespace Hermes
{
  namespace Hermes2D
  {
    HERMES_API Quad1DStd g_quad_1d_std;
    HERMES_API Quad2DStd g_quad_2d_std;
    
    static H1ShapesetJacobi ref_map_shapeset;
    static PrecalcShapeset ref_map_pss_static(&ref_map_shapeset);

    CurvMapStatic::CurvMapStatic()
    {
      int order = ref_map_shapeset.get_max_order();

      this->edge_proj_matrix_size = order - 1;

      // Edges.
      this->edge_proj_matrix = new_matrix<double>(edge_proj_matrix_size, edge_proj_matrix_size);
      edge_p = malloc_with_check<double>(edge_proj_matrix_size);

      // Bubbles - triangles.
      this->tri_bubble_np = ref_map_shapeset.get_num_bubbles(order, HERMES_MODE_TRIANGLE);
      bubble_proj_matrix_tri = new_matrix<double>(tri_bubble_np, tri_bubble_np);
      bubble_tri_p = malloc_with_check<double>(tri_bubble_np);

      // Bubbles - quads.
      order = H2D_MAKE_QUAD_ORDER(order, order);
      this->quad_bubble_np = ref_map_shapeset.get_num_bubbles(order, HERMES_MODE_QUAD);
      bubble_proj_matrix_quad = new_matrix<double>(quad_bubble_np, quad_bubble_np);
      bubble_quad_p = malloc_with_check<double>(quad_bubble_np);

      this->precalculate_cholesky_projection_matrices_bubble();
      this->precalculate_cholesky_projection_matrix_edge();
    }

    CurvMapStatic::~CurvMapStatic()
    {
      free_with_check(edge_proj_matrix, true);
      free_with_check(bubble_proj_matrix_tri, true);
      free_with_check(bubble_proj_matrix_quad, true);
      free_with_check(edge_p);
      free_with_check(bubble_tri_p);
      free_with_check(bubble_quad_p);
    }

    double** CurvMapStatic::calculate_bubble_projection_matrix(int* indices, ElementMode2D mode)
    {
      int nb;
      double** mat;

      if (mode == HERMES_MODE_TRIANGLE)
      {
        mat = this->bubble_proj_matrix_tri;
        nb = this->tri_bubble_np;
      }
      else
      {
        mat = this->bubble_proj_matrix_quad;
        nb = this->quad_bubble_np;
      }

      for (int i = 0; i < nb; i++)
      {
        for (int j = i; j < nb; j++)
        {
          int ii = indices[i], ij = indices[j];
          int o = ref_map_shapeset.get_order(ii, mode) + ref_map_shapeset.get_order(ij, mode);
          o = std::max(H2D_GET_V_ORDER(o), H2D_GET_H_ORDER(o));

          ref_map_pss_static.set_active_shape(ii);
          ref_map_pss_static.set_quad_order(o, H2D_FN_VAL);
          double* fni = ref_map_pss_static.deep_copy_array();

          ref_map_pss_static.set_active_shape(ij);
          ref_map_pss_static.set_quad_order(o, H2D_FN_VAL);
          double* fnj = ref_map_pss_static.deep_copy_array();

          double3* pt = g_quad_2d_std.get_points(o, mode);
          double val = 0.0;
          for (int k = 0; k < g_quad_2d_std.get_num_points(o, mode); k++)
            val += pt[k][2] * (fni[k] * fnj[k]);

          free_with_check(fni);
          free_with_check(fnj);

          mat[i][j] = mat[j][i] = val;
        }
      }

      return mat;
    }

    void CurvMapStatic::precalculate_cholesky_projection_matrices_bubble()
    {
      // *** triangles ***
      // calculate projection matrix of maximum order
      {
        Element e;
        e.nvert = 3;
        ref_map_pss_static.set_active_element(&e);
        int* indices = ref_map_shapeset.get_bubble_indices(ref_map_shapeset.get_max_order(), HERMES_MODE_TRIANGLE);
        curvMapStatic.bubble_proj_matrix_tri = calculate_bubble_projection_matrix(indices, HERMES_MODE_TRIANGLE);

        // cholesky factorization of the matrix
        choldc(curvMapStatic.bubble_proj_matrix_tri, this->tri_bubble_np, curvMapStatic.bubble_tri_p);
      }

      // *** quads ***
      // calculate projection matrix of maximum order
      {
      Element e;
      e.nvert = 4;
      ref_map_pss_static.set_active_element(&e);
      int *indices = ref_map_shapeset.get_bubble_indices(H2D_MAKE_QUAD_ORDER(ref_map_shapeset.get_max_order(), ref_map_shapeset.get_max_order()), HERMES_MODE_QUAD);
      curvMapStatic.bubble_proj_matrix_quad = calculate_bubble_projection_matrix(indices, HERMES_MODE_QUAD);

      // cholesky factorization of the matrix
      choldc(curvMapStatic.bubble_proj_matrix_quad, this->quad_bubble_np, curvMapStatic.bubble_quad_p);
    }
    }

    void CurvMapStatic::precalculate_cholesky_projection_matrix_edge()
    {
      // calculate projection matrix of maximum order
      for (int i = 0; i < this->edge_proj_matrix_size; i++)
      {
        for (int j = i; j < this->edge_proj_matrix_size; j++)
        {
          int o = i + j + 4;
          double2* pt = g_quad_1d_std.get_points(o);
          double val = 0.0;
          for (int k = 0; k < g_quad_1d_std.get_num_points(o); k++)
          {
            double fi = 0;
            double fj = 0;
            double x = pt[k][0];
            switch (i + 2)
            {
            case 0:
              fi = l0(x);
              break;
            case 1:
              fi = l1(x);
              break;
            case 2:
              fi = l2(x);
              break;
            case 3:
              fi = l3(x);
              break;
            case 4:
              fi = l4(x);
              break;
            case 5:
              fi = l5(x);
              break;
            case 6:
              fi = l6(x);
              break;
            case 7:
              fi = l7(x);
              break;
            case 8:
              fi = l8(x);
              break;
            case 9:
              fi = l9(x);
              break;
            case 10:
              fi = l10(x);
              break;
            case 11:
              fi = l11(x);
              break;
            }
            switch (j + 2)
            {
            case 0:
              fj = l0(x);
              break;
            case 1:
              fj = l1(x);
              break;
            case 2:
              fj = l2(x);
              break;
            case 3:
              fj = l3(x);
              break;
            case 4:
              fj = l4(x);
              break;
            case 5:
              fj = l5(x);
              break;
            case 6:
              fj = l6(x);
              break;
            case 7:
              fj = l7(x);
              break;
            case 8:
              fj = l8(x);
              break;
            case 9:
              fj = l9(x);
              break;
            case 10:
              fj = l10(x);
              break;
            case 11:
              fj = l11(x);
              break;
            }
            val += pt[k][1] * (fi * fj);
          }
          this->edge_proj_matrix[i][j] = this->edge_proj_matrix[j][i] = val;
        }
      }

      // Cholesky factorization of the matrix
      choldc(this->edge_proj_matrix, this->edge_proj_matrix_size, this->edge_p);
    }

    CurvMapStatic curvMapStatic;

    Curve::Curve(CurvType type) : type(type)
    {
    }

    Curve::~Curve()
    {
    }

    Arc::Arc() : Curve(ArcType)
    {
      kv[0] = kv[1] = kv[2] = 0;
      kv[3] = kv[4] = kv[5] = 1;
    }

    Arc::Arc(double angle) : Curve(ArcType), angle(angle)
    {
      kv[0] = kv[1] = kv[2] = 0;
      kv[3] = kv[4] = kv[5] = 1;
    }

    Arc::Arc(const Arc* other) : Curve(ArcType)
    {
      this->angle = other->angle;

      memcpy(this->kv, other->kv, 6 * sizeof(double));
      memcpy(this->pt, other->pt, 3 * sizeof(double3));
    }

    Nurbs::Nurbs() : Curve(NurbsType)
    {
      pt = nullptr;
      kv = nullptr;
    };

    Nurbs::~Nurbs()
    {
      free_with_check(pt);
      free_with_check(kv);
    };

    Nurbs::Nurbs(const Nurbs* other) : Curve(NurbsType)
    {
      this->degree = other->degree;
      this->nk = other->nk;
      this->np = other->np;
      this->kv = malloc_with_check<double>(nk);
      this->pt = malloc_with_check<double3>(np);
    }

    static double lambda_0(double x, double y)
    {
      return -0.5 * (x + y);
    }
    static double lambda_1(double x, double y)
    {
      return  0.5 * (x + 1);
    }
    static double lambda_2(double x, double y)
    {
      return  0.5 * (y + 1);
    }

    CurvMap::CurvMap() : ref_map_pss(PrecalcShapeset(&ref_map_shapeset))
    {
      coeffs = nullptr;
      ctm = nullptr;
      memset(curves, 0, sizeof(Curve*)* H2D_MAX_NUMBER_EDGES);
    }

    CurvMap::CurvMap(const CurvMap* cm) : ref_map_pss(PrecalcShapeset(&ref_map_shapeset))
    {
      this->nc = cm->nc;
      this->order = cm->order;
      /// \todo Find out if this is safe.
      this->ctm = cm->ctm;
      this->coeffs = malloc_with_check<double2>(nc, true);
      memcpy(coeffs, cm->coeffs, sizeof(double2)* nc);

      this->toplevel = cm->toplevel;
      if (this->toplevel)
      {
        for (int i = 0; i < 4; i++)
        {
          if (cm->curves[i])
          {
            if (cm->curves[i]->type == NurbsType)
              this->curves[i] = new Nurbs((Nurbs*)cm->curves[i]);
            else
              this->curves[i] = new Arc((Arc*)cm->curves[i]);
          }
          else
            this->curves[i] = nullptr;
        }
      }
      else
      {
        memset(curves, 0, sizeof(Curve*)* H2D_MAX_NUMBER_EDGES);
        this->parent = cm->parent;
        this->sub_idx = cm->sub_idx;
      }
    }

    CurvMap::~CurvMap()
    {
      this->free();
    }

    void CurvMap::free()
    {
      free_with_check(this->coeffs, true);

      if (toplevel)
      {
        for (int i = 0; i < 4; i++)
        if (curves[i])
        {
          delete curves[i];
          curves[i] = nullptr;
        }
      }
    }

    bool CurvMap::warning_issued = false;

    double CurvMap::nurbs_basis_fn(int i, int k, double t, double* knot)
    {
      if (k == 0)
      {
        return (t >= knot[i] && t <= knot[i + 1] && knot[i] < knot[i + 1]) ? 1.0 : 0.0;
      }
      else
      {
        double N1 = nurbs_basis_fn(i, k - 1, t, knot);
        double N2 = nurbs_basis_fn(i + 1, k - 1, t, knot);

        double result = 0.0;
        if (knot[i + k] != knot[i])
        {
          result += ((t - knot[i]) / (knot[i + k] - knot[i])) * N1;
        }
        if (knot[i + k + 1] != knot[i + 1])
        {
          result += ((knot[i + k + 1] - t) / (knot[i + k + 1] - knot[i + 1])) * N2;
        }
        return result;
      }
    }

    void CurvMap::nurbs_edge(Element* e, Curve* curve, int edge, double t, double& x,
      double& y, double& n_x, double& n_y, double& t_x, double& t_y)
    {
      // Nurbs curves are parametrized from 0 to 1.
      t = (t + 1.0) / 2.0;

      // Start point A, end point B.
      double2 A, B;
      A[0] = e->vn[edge]->x;
      A[1] = e->vn[edge]->y;
      B[0] = e->vn[e->next_vert(edge)]->x;
      B[1] = e->vn[e->next_vert(edge)]->y;

      // Vector pointing from A to B.
      double2 v;
      v[0] = B[0] - A[0];
      v[1] = B[1] - A[1];
      double abs_v = sqrt(sqr(v[0]) + sqr(v[1]));

      // Straight line.
      if (!curve)
      {
        x = A[0] + t * v[0];
        y = A[1] + t * v[1];
        t_x = v[0] / abs_v;
        t_y = v[1] / abs_v;
        n_x = t_y;
        n_y = -t_x;
      }
      else
      {
        // Circular arc.
        if (curve->type == ArcType)
        {
          double3* cp = ((Arc*)curve)->pt;
          x = y = 0.0;
          double sum = 0.0;  // sum of basis fns and weights

          for (int i = 0; i < ((Arc*)curve)->np; i++)
          {
            double basis = nurbs_basis_fn(i, ((Arc*)curve)->degree, t, ((Arc*)curve)->kv);
            sum += cp[i][2] * basis;
            double x_i = cp[i][0];
            double y_i = cp[i][1];
            double w_i = cp[i][2];
            x += w_i * basis * x_i;
            y += w_i * basis * y_i;
          }

          x /= sum;
          y /= sum;

          // Normal and tangential vectors.
          // FIXME; This calculation is artificial and it assumes that
          // the NURBS is a circular arc. This should be done in the
          // same way for all NURBS.

          // End points, midpoint.
          double2 M;
          M[0] = (A[0] + B[0]) / 2.;
          M[1] = (A[1] + B[1]) / 2.;
          //printf("***** A = %g %g\n", A[0], A[1]);
          //printf("***** B = %g %g\n", B[0], B[1]);
          //printf("***** M = %g %g\n", M[0], M[1]);

          // Unit vector from M to center of circle S.
          double2 w;
          w[0] = -v[1] / abs_v;
          w[1] = v[0] / abs_v;

          // Distance L between M and center of circle S
          // can be calculated using a right-angle triangle
          // whose one angle is alpha/2.
          double alpha_rad = ((Arc*)curve)->angle * M_PI / 180.;
          double L = 0.5 * abs_v / tan(0.5 * alpha_rad);
          //printf("***** L = %g\n", L);

          // Center of circle.
          double2 S;
          S[0] = M[0] + w[0] * L;
          S[1] = M[1] + w[1] * L;
          //printf("***** S = %g %g\n", S[0], S[1]);

          // Calculation of radius and test.
          double2 SA, SB;
          SA[0] = A[0] - S[0];
          SA[1] = A[1] - S[1];
          SB[0] = B[0] - S[0];
          SB[1] = B[1] - S[1];
          double R = sqrt(sqr(SA[0]) + sqr(SA[1]));
          double R2 = sqrt(sqr(SB[0]) + sqr(SB[1]));
          if (std::abs(R - R2) > Hermes::HermesSqrtEpsilon)
            throw Hermes::Exceptions::Exception("Internal error in nurbs_edge() - bad radius R.");

          // Normal vectors to circular arc at edge end points A, B.
          double2 normal_A, normal_B;
          normal_A[0] = SA[0] / R;
          normal_A[1] = SA[1] / R;
          normal_B[0] = SB[0] / R;
          normal_B[1] = SB[1] / R;
          //printf("***** normal_A = %g %g\n", normal_A[0], normal_A[1]);
          //printf("***** normal_B = %g %g\n", normal_B[0], normal_B[1]);

          // Calculate rotational matrix that transforms AS_ref = (R, 0) -> SA
          // and SB_ref -> SB.
          double2 SB_ref;
          SB_ref[0] = R * cos(alpha_rad);
          SB_ref[1] = R * sin(alpha_rad);
          // First we need to invert the matrix[(R 0)^T, SB_ref^T].
          double m_11, m_12, m_21, m_22;
          m_11 = R;
          m_12 = SB_ref[0];
          m_21 = 0;
          m_22 = SB_ref[1];
          double m_det = m_11 * m_22 - m_12 * m_21;
          double inv_11, inv_12, inv_21, inv_22;
          inv_11 = m_22 / m_det;
          inv_12 = -m_12 / m_det;
          inv_21 = -m_21 / m_det;
          inv_22 = m_11 / m_det;
          double s_11, s_12, s_21, s_22;
          s_11 = SA[0];
          s_12 = SB[0];
          s_21 = SA[1];
          s_22 = SB[1];
          //Rotation matrix.
          double r_11, r_12, r_21, r_22;
          r_11 = s_11 * inv_11 + s_12 * inv_21;
          r_12 = s_11 * inv_12 + s_12 * inv_22;
          r_21 = s_21 * inv_11 + s_22 * inv_21;
          r_22 = s_21 * inv_12 + s_22 * inv_22;
          // The desired normal vector in reference coordinates.
          double n_x_ref = cos(alpha_rad * t);
          double n_y_ref = sin(alpha_rad * t);
          // Rotate it.
          n_x = r_11 * n_x_ref + r_12 * n_y_ref;
          n_y = r_21 * n_x_ref + r_22 * n_y_ref;

          /*
          // Calculate normal at point corresponding to the
          // position of parameter 't' between 0 and 1.
          n_x = normal_A[0] + t * (normal_B[0] - normal_A[0]);
          n_y = normal_A[1] + t * (normal_B[1] - normal_A[1]);
          double size_n = sqrt(sqr(n_x) + sqr(n_y));
          n_x /= size_n;
          n_y /= size_n;
          */

          // Calculate tangential vectors.
          t_x = -n_y;
          t_y = n_x;

          // Correcting sign so that the normal points outside
          // if the angle is negative.
          if (((Arc*)curve)->angle < 0)
          {
            n_x *= -1;
            n_y *= -1;
            t_x *= -1;
            t_y *= -1;
          }
        }
        // General NURBS.
        // FIXME - calculation of normal and tangential vectors needs to be added.
        else
        {
          double3* cp = ((Nurbs*)curve)->pt;
          x = y = 0.0;
          double sum = 0.0;  // sum of basis fns and weights

          for (int i = 0; i < ((Nurbs*)curve)->np; i++)
          {
            double basis = nurbs_basis_fn(i, ((Nurbs*)curve)->degree, t, ((Nurbs*)curve)->kv);
            sum += cp[i][2] * basis;
            double x_i = cp[i][0];
            double y_i = cp[i][1];
            double w_i = cp[i][2];
            x += w_i * basis * x_i;
            y += w_i * basis * y_i;
          }

          x /= sum;
          y /= sum;

          if (!warning_issued)
          {
            printf("FIXME: IMPLEMENT CALCULATION OF n_x, n_y, t_x, t_y in nurbs_edge() !!!\n");
            warning_issued = true;
          }
          n_x = 0;
          n_y = 0;
          t_x = 0;
          t_y = 0;
        }
      }
    }

    const double2 CurvMap::ref_vert[2][H2D_MAX_NUMBER_VERTICES] =
    {
      { { -1.0, -1.0 }, { 1.0, -1.0 }, { -1.0, 1.0 }, { 0.0, 0.0 } },
      { { -1.0, -1.0 }, { 1.0, -1.0 }, { 1.0, 1.0 }, { -1.0, 1.0 } }
    };

    void CurvMap::nurbs_edge_0(Element* e, Curve* curve, int edge, double t, double& x, double& y, double& n_x, double& n_y, double& t_x, double& t_y)
    {
      int va = edge;
      int vb = e->next_vert(edge);
      nurbs_edge(e, curve, edge, t, x, y, n_x, n_y, t_x, t_y);

      x -= 0.5 * ((1 - t) * (e->vn[va]->x) + (1 + t) * (e->vn[vb]->x));
      y -= 0.5 * ((1 - t) * (e->vn[va]->y) + (1 + t) * (e->vn[vb]->y));

      double k = 4.0 / ((1 - t) * (1 + t));
      x *= k;
      y *= k;
    }

    void CurvMap::calc_ref_map_tri(Element* e, Curve** curve, double xi_1, double xi_2, double& x, double& y)
    {
      double  fx, fy;
      x = y = 0.0;

      for (unsigned int j = 0; j < e->get_nvert(); j++)
      {
        int va = j;
        int vb = e->next_vert(j);
        double l_a = 0;
        double l_b = 0;
        switch (va)
        {
        case 0:
          l_a = lambda_0(xi_1, xi_2);
          break;
        case 1:
          l_a = lambda_1(xi_1, xi_2);
          break;
        case 2:
          l_a = lambda_2(xi_1, xi_2);
          break;
        }

        switch (vb)
        {
        case 0:
          l_b = lambda_0(xi_1, xi_2);
          break;
        case 1:
          l_b = lambda_1(xi_1, xi_2);
          break;
        case 2:
          l_b = lambda_2(xi_1, xi_2);
          break;
        }

        // vertex part
        x += e->vn[j]->x * l_a;
        y += e->vn[j]->y * l_a;

        if (!(((ref_vert[0][va][0] == xi_1) && (ref_vert[0][va][1] == xi_2)) ||
          ((ref_vert[0][vb][0] == xi_1) && (ref_vert[0][vb][1] == xi_2))))
        {
          // edge part
          double t = l_b - l_a;
          double n_x, n_y, t_x, t_y;
          nurbs_edge_0(e, curve[j], j, t, fx, fy, n_x, n_y, t_x, t_y);
          x += fx * l_a  * l_b;
          y += fy * l_a  * l_b;
        }
      }
    }

    void CurvMap::calc_ref_map_quad(Element* e, Curve** curve, double xi_1, double xi_2,
      double& x, double& y)
    {
      double ex[H2D_MAX_NUMBER_EDGES], ey[H2D_MAX_NUMBER_EDGES];

      double n_x, n_y, t_x, t_y;
      nurbs_edge(e, curve[0], 0, xi_1, ex[0], ey[0], n_x, n_y, t_x, t_y);
      nurbs_edge(e, curve[1], 1, xi_2, ex[1], ey[1], n_x, n_y, t_x, t_y);
      nurbs_edge(e, curve[2], 2, -xi_1, ex[2], ey[2], n_x, n_y, t_x, t_y);
      nurbs_edge(e, curve[3], 3, -xi_2, ex[3], ey[3], n_x, n_y, t_x, t_y);

      x = (1 - xi_2) / 2.0 * ex[0] + (1 + xi_1) / 2.0 * ex[1] +
        (1 + xi_2) / 2.0 * ex[2] + (1 - xi_1) / 2.0 * ex[3] -
        (1 - xi_1)*(1 - xi_2) / 4.0 * e->vn[0]->x - (1 + xi_1)*(1 - xi_2) / 4.0 * e->vn[1]->x -
        (1 + xi_1)*(1 + xi_2) / 4.0 * e->vn[2]->x - (1 - xi_1)*(1 + xi_2) / 4.0 * e->vn[3]->x;

      y = (1 - xi_2) / 2.0 * ey[0] + (1 + xi_1) / 2.0 * ey[1] +
        (1 + xi_2) / 2.0 * ey[2] + (1 - xi_1) / 2.0 * ey[3] -
        (1 - xi_1)*(1 - xi_2) / 4.0 * e->vn[0]->y - (1 + xi_1)*(1 - xi_2) / 4.0 * e->vn[1]->y -
        (1 + xi_1)*(1 + xi_2) / 4.0 * e->vn[2]->y - (1 - xi_1)*(1 + xi_2) / 4.0 * e->vn[3]->y;
    }

    void CurvMap::calc_ref_map(Element* e, Curve** curve, double xi_1, double xi_2, double2& f)
    {
      if (e->get_mode() == HERMES_MODE_QUAD)
        calc_ref_map_quad(e, curve, xi_1, xi_2, f[0], f[1]);
      else
        calc_ref_map_tri(e, curve, xi_1, xi_2, f[0], f[1]);
    }

    void CurvMap::edge_coord(Element* e, int edge, double t, double2& x, double2& v) const
    {
      int mode = e->get_mode();
      double2 a, b;
      a[0] = ctm->m[0] * ref_vert[mode][edge][0] + ctm->t[0];
      a[1] = ctm->m[1] * ref_vert[mode][edge][1] + ctm->t[1];
      b[0] = ctm->m[0] * ref_vert[mode][e->next_vert(edge)][0] + ctm->t[0];
      b[1] = ctm->m[1] * ref_vert[mode][e->next_vert(edge)][1] + ctm->t[1];

      for (int i = 0; i < 2; i++)
      {
        v[i] = b[i] - a[i];
        x[i] = a[i] + (t + 1.0) / 2.0 * v[i];
      }
      double lenght = sqrt(v[0] * v[0] + v[1] * v[1]);
      v[0] /= lenght; v[1] /= lenght;
    }

    void CurvMap::calc_edge_projection(Element* e, int edge, Curve** nurbs, int order, double2* proj) const
    {
      int i, j, k;
      int mo1 = g_quad_1d_std.get_max_order();
      int np = g_quad_1d_std.get_num_points(mo1);
      int ne = order - 1;
      int mode = e->get_mode();

      assert(np <= 15 && ne <= 10);
      double2 fn[15];
      double rhside[2][10];
      memset(fn, 0, sizeof(double2)* np);
      memset(rhside[0], 0, sizeof(double)* ne);
      memset(rhside[1], 0, sizeof(double)* ne);

      double a_1, a_2, b_1, b_2;
      a_1 = ctm->m[0] * ref_vert[mode][edge][0] + ctm->t[0];
      a_2 = ctm->m[1] * ref_vert[mode][edge][1] + ctm->t[1];
      b_1 = ctm->m[0] * ref_vert[mode][e->next_vert(edge)][0] + ctm->t[0];
      b_2 = ctm->m[1] * ref_vert[mode][e->next_vert(edge)][1] + ctm->t[1];

      // values of nonpolynomial function in two vertices
      double2 fa, fb;
      calc_ref_map(e, nurbs, a_1, a_2, fa);
      calc_ref_map(e, nurbs, b_1, b_2, fb);

      double2* pt = g_quad_1d_std.get_points(mo1);
      for (j = 0; j < np; j++) // over all integration points
      {
        double2 x, v;
        double t = pt[j][0];
        edge_coord(e, edge, t, x, v);
        calc_ref_map(e, nurbs, x[0], x[1], fn[j]);

        for (k = 0; k < 2; k++)
          fn[j][k] = fn[j][k] - (fa[k] + (t + 1) / 2.0 * (fb[k] - fa[k]));
      }

      double2* result = proj + e->get_nvert() + edge * (order - 1);
      for (k = 0; k < 2; k++)
      {
        for (i = 0; i < ne; i++)
        {
          for (j = 0; j < np; j++)
          {
            double t = pt[j][0];
            double fi = 0;
            switch (i + 2)
            {
            case 0:
              fi = l0(t);
              break;
            case 1:
              fi = l1(t);
              break;
            case 2:
              fi = l2(t);
              break;
            case 3:
              fi = l3(t);
              break;
            case 4:
              fi = l4(t);
              break;
            case 5:
              fi = l5(t);
              break;
            case 6:
              fi = l6(t);
              break;
            case 7:
              fi = l7(t);
              break;
            case 8:
              fi = l8(t);
              break;
            case 9:
              fi = l9(t);
              break;
            case 10:
              fi = l10(t);
              break;
            case 11:
              fi = l11(t);
              break;
            }
            rhside[k][i] += pt[j][1] * (fi * fn[j][k]);
          }
        }
        // solve
        cholsl(curvMapStatic.edge_proj_matrix, ne, curvMapStatic.edge_p, rhside[k], rhside[k]);
        for (i = 0; i < ne; i++)
          result[i][k] = rhside[k][i];
      }
    }

    void CurvMap::old_projection(Element* e, int order, double2* proj, double* old[2])
    {
      int mo2 = g_quad_2d_std.get_max_order(e->get_mode());
      int np = g_quad_2d_std.get_num_points(mo2, e->get_mode());

      for (unsigned int k = 0; k < e->get_nvert(); k++) // loop over vertices
      {
        // vertex basis functions in all integration points
        int index_v = ref_map_shapeset.get_vertex_index(k, e->get_mode());
        ref_map_pss.set_active_shape(index_v);
        ref_map_pss.set_quad_order(mo2, H2D_FN_VAL_0);
        const double* vd = ref_map_pss.get_fn_values();

        for (int m = 0; m < 2; m++)   // part 0 or 1
        for (int j = 0; j < np; j++)
          old[m][j] += proj[k][m] * vd[j];

        for (int ii = 0; ii < order - 1; ii++)
        {
          // edge basis functions in all integration points
          int index_e = ref_map_shapeset.get_edge_index(k, 0, ii + 2, e->get_mode());
          ref_map_pss.set_active_shape(index_e);
          ref_map_pss.set_quad_order(mo2, H2D_FN_VAL_0);
          const double* ed = ref_map_pss.get_fn_values();

          for (int m = 0; m < 2; m++)  //part 0 or 1
          for (int j = 0; j < np; j++)
            old[m][j] += proj[e->get_nvert() + k * (order - 1) + ii][m] * ed[j];
        }
      }
    }

    void CurvMap::calc_bubble_projection(Element* e, Curve** curve, int order, double2* proj)
    {
      ref_map_pss.set_active_element(e);

      int i, j, k;
      int mo2 = g_quad_2d_std.get_max_order(e->get_mode());
      int np = g_quad_2d_std.get_num_points(mo2, e->get_mode());
      int qo = e->is_quad() ? H2D_MAKE_QUAD_ORDER(order, order) : order;
      int nb = ref_map_shapeset.get_num_bubbles(qo, e->get_mode());

      double2* fn = new double2[np];
      memset(fn, 0, np * sizeof(double2));

      double* rhside[2];
      double* old[2];
      for (i = 0; i < 2; i++)
      {
        rhside[i] = new double[nb];
        old[i] = new double[np];
        memset(rhside[i], 0, sizeof(double)* nb);
        memset(old[i], 0, sizeof(double)* np);
      }

      // compute known part of projection (vertex and edge part)
      old_projection(e, order, proj, old);

      // fn values of both components of nonpolynomial function
      double3* pt = g_quad_2d_std.get_points(mo2, e->get_mode());
      for (j = 0; j < np; j++)  // over all integration points
      {
        double2 a;
        a[0] = ctm->m[0] * pt[j][0] + ctm->t[0];
        a[1] = ctm->m[1] * pt[j][1] + ctm->t[1];
        calc_ref_map(e, curve, a[0], a[1], fn[j]);
      }

      double2* result = proj + e->get_nvert() + e->get_nvert() * (order - 1);
      for (k = 0; k < 2; k++)
      {
        for (i = 0; i < nb; i++) // loop over bubble basis functions
        {
          // bubble basis functions in all integration points
          int index_i = ref_map_shapeset.get_bubble_indices(qo, e->get_mode())[i];
          ref_map_pss.set_active_shape(index_i);
          ref_map_pss.set_quad_order(mo2, H2D_FN_VAL_0);
          const double *bfn = ref_map_pss.get_fn_values();

          for (j = 0; j < np; j++) // over all integration points
            rhside[k][i] += pt[j][2] * (bfn[j] * (fn[j][k] - old[k][j]));
        }

        // solve
        if (e->get_mode() == HERMES_MODE_TRIANGLE)
          cholsl(curvMapStatic.bubble_proj_matrix_tri, nb, curvMapStatic.bubble_tri_p, rhside[k], rhside[k]);
        else
          cholsl(curvMapStatic.bubble_proj_matrix_quad, nb, curvMapStatic.bubble_quad_p, rhside[k], rhside[k]);

        for (i = 0; i < nb; i++)
          result[i][k] = rhside[k][i];
      }

      for (i = 0; i < 2; i++)
      {
        delete[] rhside[i];
        delete[] old[i];
      }
      delete[] fn;
    }

    void CurvMap::update_refmap_coeffs(Element* e)
    {
      ref_map_pss.set_quad_2d(&g_quad_2d_std);
      ref_map_pss.set_active_element(e);

      // allocate projection coefficients
      int nv = e->get_nvert();
      int ne = order - 1;
      int qo = e->is_quad() ? H2D_MAKE_QUAD_ORDER(order, order) : order;
      int nb = ref_map_shapeset.get_num_bubbles(qo, e->get_mode());
      nc = nv + nv*ne + nb;
      this->coeffs = realloc_with_check<double2>(this->coeffs, nc);

      // WARNING: do not change the format of the array 'coeffs'. If it changes,
      // RefMap::set_active_element() has to be changed too.
      Curve** curves;
      if (toplevel == false)
      {
        ref_map_pss.set_active_element(e);
        ref_map_pss.set_transform(this->sub_idx);
        curves = parent->cm->curves;
      }
      else
      {
        ref_map_pss.reset_transform();
        curves = e->cm->curves;
      }
      ctm = ref_map_pss.get_ctm();

      // calculation of new_ projection coefficients
      // vertex part
      for (unsigned int i = 0; i < e->get_nvert(); i++)
      {
        coeffs[i][0] = e->vn[i]->x;
        coeffs[i][1] = e->vn[i]->y;
      }

      if (e->cm->toplevel == false)
        e = e->cm->parent;

      // edge part
      for (int edge = 0; edge < e->get_nvert(); edge++)
        calc_edge_projection(e, edge, curves, order, coeffs);

      //bubble part
      calc_bubble_projection(e, curves, order, coeffs);

      RefMap::set_element_iro_cache(e);
    }

    void CurvMap::get_mid_edge_points(Element* e, double2* pt, int n)
    {
      Curve** curves = this->curves;
      Transformable tran;
      tran.set_active_element(e);

      if (toplevel == false)
      {
        tran.set_transform(this->sub_idx);
        e = e->cm->parent;
        curves = e->cm->curves;
      }

      ctm = tran.get_ctm();
      double xi_1, xi_2;
      for (int i = 0; i < n; i++)
      {
        xi_1 = ctm->m[0] * pt[i][0] + ctm->t[0];
        xi_2 = ctm->m[1] * pt[i][1] + ctm->t[1];
        calc_ref_map(e, curves, xi_1, xi_2, pt[i]);
      }
    }

    CurvMap* CurvMap::create_son_curv_map(Element* e, int son)
    {
      // if the top three bits of part are nonzero, we would overflow
      // -- make the element non-curvilinear
      if (e->cm->sub_idx & 0xe000000000000000ULL)
        return nullptr;

      // if the parent element is already almost straight-edged,
      // the son will be even more straight-edged
      if (e->iro_cache == 0)
        return nullptr;

      CurvMap* cm = new CurvMap;
      if (e->cm->toplevel == false)
      {
        cm->parent = e->cm->parent;
        cm->sub_idx = (e->cm->sub_idx << 3) + son + 1;
      }
      else
      {
        cm->parent = e;
        cm->sub_idx = (son + 1);
      }

      cm->toplevel = false;
      cm->order = 4;

      return cm;
    }
  }
}
