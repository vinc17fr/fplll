/* Copyright (C) 2005-2008 Damien Stehle.
   Copyright (C) 2007 David Cade.
   Copyright (C) 2011 Xavier Pujol.

   This file is part of fplll. fplll is free software: you
   can redistribute it and/or modify it under the terms of the GNU Lesser
   General Public License as published by the Free Software Foundation,
   either version 2.1 of the License, or (at your option) any later version.

   fplll is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with fplll. If not, see <http://www.gnu.org/licenses/>. */

#ifndef FPLLL_GSO_H
#define FPLLL_GSO_H

#include "nr/matrix.h"
#include "gsob.h"

FPLLL_BEGIN_NAMESPACE

/**
 * MatGSO provides an interface for performing elementary operations on a basis
 * and computing its Gram matrix and its Gram-Schmidt orthogonalization.
 * The Gram-Schmidt coefficients are computed on demand. The object keeps track
 * of which coefficients are valid after each row operation.
 */
template <class ZT, class FT> class MatGSO : public MatGSOb<ZT,FT>
{
public:
  using MatGSOb<ZT,FT>::d;
  using MatGSOb<ZT,FT>::n_known_rows;
  using MatGSOb<ZT,FT>::n_source_rows;
  using MatGSOb<ZT,FT>::u;
  using MatGSOb<ZT,FT>::enable_transform;
  using MatGSOb<ZT,FT>::cols_locked; // maybe scratch.
  using MatGSOb<ZT,FT>::enable_int_gram;
  using MatGSOb<ZT,FT>::gso_valid_cols;
  using MatGSOb<ZT,FT>::enable_inverse_transform;
  using MatGSOb<ZT,FT>::u_inv_t;
  using MatGSOb<ZT,FT>::sym_g;
  using MatGSOb<ZT,FT>::mu;
  using MatGSOb<ZT,FT>::r;
  using MatGSOb<ZT,FT>::ztmp1;
  using MatGSOb<ZT,FT>::ztmp2;
  using MatGSOb<ZT,FT>::row_op_force_long;
  using MatGSOb<ZT,FT>::alloc_dim;
  using MatGSOb<ZT,FT>::get_mu;
  using MatGSOb<ZT,FT>::get_r;
  using MatGSOb<ZT,FT>::gptr;  
  using MatGSOb<ZT,FT>::invalidate_gso_row;
  using MatGSOb<ZT,FT>::gf;
  using MatGSOb<ZT,FT>::bf;
  using MatGSOb<ZT,FT>::discover_all_rows;
  using MatGSOb<ZT,FT>::init_row_size;  
  using MatGSOb<ZT,FT>::enable_row_expo;
  using MatGSOb<ZT,FT>::row_expo;
  using MatGSOb<ZT,FT>::n_known_cols;
  using MatGSOb<ZT,FT>::tmp_col_expo;
//  using MatGSOb<ZT,FT>::create_row;
  using MatGSOb<ZT,FT>::remove_last_row;
  using MatGSOb<ZT,FT>::print_mu_r_g;
  using MatGSOb<ZT,FT>::update_gso;
  using MatGSOb<ZT,FT>::update_gso_row;
  using MatGSOb<ZT,FT>::row_addmul;
  using MatGSOb<ZT,FT>::symmetrize_g;
  //using MatGSOb<ZT,FT>::update_gso_row;
  //using MatGSOb<ZT,FT>::update_gso_row;
  /**
   * Constructor.
   * The precision of FT must be defined before creating an instance of the
   * class and must remain the same until the object is destroyed (or no longer
   * needed).
   * @param b
   *   The matrix on which row operations are performed. It must not be empty.
   * @param u
   *   If u is not empty, operations on b are also done on u
   *   (in this case both must have the same number of rows).
   *   If u is initially the identity matrix, multiplying transform by the
   *   initial basis gives the current basis.
   * @param u_inv_t
   *   Inverse transform (should be empty, which disables the computation, or
   *   initialized with identity matrix). It works only if u is not empty.
   * @param enable_int_gram
   *   If true, coefficients of the Gram matrix are computed with exact integer
   *   arithmetic (type ZT). Otherwise, they are computed in floating-point
   *   (type FT). Note that when exact arithmetic is used, all coefficients of
   *   the first n_known_rows are continuously updated, whereas in floating-point,
   *   they are computed only on-demand. This option cannot be enabled if
   *   enable_row_expo=true.
   * @param enable_row_expo
   *   If true, each row of b is normalized by a power of 2 before doing
   *   conversion to floating-point, which hopefully avoids some overflows.
   *   This option cannot be enabled if enable_int_gram=true and works only
   *   with FT=double and FT=long double. It is useless and MUST NOT be used
   *   for FT=dpe or FT=mpfr_t.
   * @param row_op_force_long
   *   Affects the behaviour of row_addmul(_we).
   *   See the documentation of row_addmul.
   */
  //~ MatGSO(Matrix<ZT>& b, Matrix<ZT>& u, Matrix<ZT>& u_inv_t, int flags);
  MatGSO(Matrix<ZT> &arg_b, Matrix<ZT> &arg_u, Matrix<ZT> &arg_uinv_t, int flags)
      : MatGSOb<ZT,FT>(arg_u,arg_uinv_t,flags) , b(arg_b)
  {
    FPLLL_DEBUG_CHECK(!(enable_int_gram && enable_row_expo));
    d = b.get_rows();
    if (enable_row_expo)
    {
      tmp_col_expo.resize(b.get_cols());
    }
    if (enable_int_gram) {
      gptr = &g;
    } 
    size_increased();
#ifdef DEBUG
    row_op_first = row_op_last = -1;
#endif
  }

  /**
   * Basis of the lattice
   */
//private:
  Matrix<ZT> &b;
//public:
  /**
   * Integer Gram matrix of the lattice
   */
  Matrix<ZT> g;

public:
  virtual inline long  get_max_exp_of_b();
  virtual inline bool b_row_is_zero(int i);
  virtual inline int get_cols_of_b();
  virtual inline int get_rows_of_b();
  virtual inline void negate_row_of_b(int i);

  virtual inline void create_rows(int n_new_rows);
  virtual inline void remove_last_rows(int n_removed_rows);

  virtual void move_row(int old_r, int new_r);

  /**
   * b[i] := b[i] + x * b[j].
   * After one or several calls to row_addmul, row_op_end must be called.
   * Special cases |x| &lt;= 1 and |x| &lt;= LONG_MAX are optimized.
   * x should be an integer.
   * If row_op_force_long=true, x is always converted to (2^expo * long) instead
   * of (2^expo * ZT), which is faster if ZT=mpz_t but might lead to a loss of
   * precision (in LLL, more Babai iterations are needed).
   */
  //virtual inline void row_addmul(int i, int j, const FT &x);

  /**
   * b[i] := b[i] + x * 2^expo_add * b[j].
   * After one or several calls to row_addmul_we, row_op_end must be called.
   * Special cases |x| &lt;= 1 and |x| &lt;= LONG_MAX are optimized.
   * x should be an integer.
   * If row_op_force_long=true, x is always converted to (2^expo * long) instead
   * of (2^expo * ZT), which is faster if ZT=mpz_t but might lead to a loss of
   * precision (in LLL, more Babai iterations are needed).
   */
  virtual void row_addmul_we(int i, int j, const FT &x, long expo_add);

  // b[i] += b[j] / b[i] -= b[j] (i > j)
  virtual void row_add(int i, int j);
  virtual void row_sub(int i, int j);

//  virtual inline void printparam(ostream &os);
  virtual inline FT &get_gram(FT &f, int i, int j);

private:


    /* Allocates matrices and arrays whose size depends on d (all but tmp_col_expo).
     When enable_int_gram=false, initializes bf. */
  virtual void size_increased();

  virtual void discover_row();

  /* Upates the i-th row of bf. It does not invalidate anything, so the caller
     must take into account that it might change row_expo. */
  virtual void update_bf(int i);
  /* Marks g(i, j) for all j <= i (but NOT for j > i) */
  virtual void invalidate_gram_row(int i);

  // b[i] <- b[i] + x * b[j] (i > j)
  virtual void row_addmul_si(int i, int j, long x);
  // b[i] <- b[i] + (2^expo * x) * b[j] (i > j)
  virtual void row_addmul_si_2exp(int i, int j, long x, long expo);
  virtual void row_addmul_2exp(int i, int j, const ZT &x, long expo);

public:
  // b[i] <-> b[j] (i < j)
  virtual void row_swap(int i, int j);
  //virtual void apply_transform(const Matrix<FT> &transform, int src_base, int target_base);
private:
//  virtual void symmetrize_g();

};

template <class ZT, class FT> inline long MatGSO<ZT,FT>::get_max_exp_of_b()
{
  return b.get_max_exp();
}

template <class ZT, class FT> inline bool MatGSO<ZT,FT>::b_row_is_zero(int i)
{
  return b[i].is_zero();
}
template <class ZT, class FT> inline int MatGSO<ZT,FT>::get_cols_of_b()
{
  return b.get_cols();
}

template <class ZT, class FT> inline int MatGSO<ZT,FT>::get_rows_of_b()
{
  return b.get_rows();
}

template <class ZT, class FT> inline void MatGSO<ZT,FT>::negate_row_of_b(int i)
{
     
      for (int j = 0; j < get_cols_of_b(); j++)
      {
        b[i][j].neg(b[i][j]);
      }
      if (enable_int_gram) {
        for (int j = 0; j < get_rows_of_b(); j++) {
          if (j < i) {
            //sym_g(i,j).neg(sym_g(i,j));
            g(i,j).neg(g(i,j));
          } else if (j > i) {
            g(j,i).neg(g(j,i));

          }
        } 
      }

}

/*
template <class ZT, class FT> inline FT &MatGSO<ZT, FT>::get_gram(FT &f, int i, int j)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && j >= 0 && j <= i && j < n_source_rows &&
                    !in_row_op_range(i));
  if (enable_int_gram)
    f.set_z(g(i, j));
  else
  {
    if (gf(i, j).is_nan())
    {
      dot_product(gf(i, j), bf[i], bf[j], n_known_cols);
    }
    f = gf(i, j);
  }
  return f;
}
*/

// TODO those functions only needed here, right? delete in gsob?
/*
template <class ZT, class FT> inline const FT &MatGSO<ZT, FT>::get_mu_exp(int i, int j, long &expo)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && j >= 0 && j < i && j < gso_valid_cols[i] &&
                    !in_row_op_range(i));
  if (enable_row_expo)
    expo = row_expo[i] - row_expo[j];
  else
    expo = 0;
  return mu(i, j);
}

template <class ZT, class FT> inline const FT &MatGSO<ZT, FT>::get_mu_exp(int i, int j)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && j >= 0 && j < i && j < gso_valid_cols[i] &&
                    !in_row_op_range(i));
  return mu(i, j);
}

template <class ZT, class FT> inline FT &MatGSO<ZT, FT>::get_mu(FT &f, int i, int j)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && j >= 0 && j < i && j < gso_valid_cols[i] &&
                    !in_row_op_range(i));
  f = mu(i, j);
  if (enable_row_expo)
    f.mul_2si(f, row_expo[i] - row_expo[j]);
  return f;
}

template <class ZT, class FT> inline const FT &MatGSO<ZT, FT>::get_r_exp(int i, int j, long &expo)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && j >= 0 && j < gso_valid_cols[i] &&
                    !in_row_op_range(i));
  if (enable_row_expo)
    expo = row_expo[i] + row_expo[j];
  else
    expo = 0;
  return r(i, j);
}

template <class ZT, class FT> inline const FT &MatGSO<ZT, FT>::get_r_exp(int i, int j)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && j >= 0 && j < gso_valid_cols[i] &&
                    !in_row_op_range(i));
  return r(i, j);
}

template <class ZT, class FT> inline FT &MatGSO<ZT, FT>::get_r(FT &f, int i, int j)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && j >= 0 && j < gso_valid_cols[i] &&
                    !in_row_op_range(i));
  f = r(i, j);
  if (enable_row_expo)
    f.mul_2si(f, row_expo[i] + row_expo[j]);
  return f;
}

template <class ZT, class FT> inline void MatGSO<ZT, FT>::set_r(int i, int j, FT &f)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && gso_valid_cols[i] >= j && j >= 0 &&
                    j < n_source_rows);
  r(i, j) = f;
  if (gso_valid_cols[i] == j)
    gso_valid_cols[i]++;
}
*/
/*
template <class ZT, class FT> inline void MatGSO<ZT, FT>::row_addmul(int i, int j, const FT &x)
{
  row_addmul_we(i, j, x, 0);
}
*/
template <class ZT, class FT> inline FT &MatGSO<ZT, FT>::get_gram(FT &f, int i, int j)
{
  FPLLL_DEBUG_CHECK(i >= 0 && i < n_known_rows && j >= 0 && j <= i && j < n_source_rows &&
                    !in_row_op_range(i));
  if (enable_int_gram) 
  {
    f.set_z(g(i, j));
  }
  else 
  {
    if (gf(i, j).is_nan())
    {
      dot_product(gf(i, j), bf[i], bf[j], n_known_cols);
    }
    f = gf(i, j);
  }
  return f;
}

template <class ZT, class FT> inline void MatGSO<ZT, FT>::create_rows(int n_new_rows)
{
  FPLLL_DEBUG_CHECK(!cols_locked);
  int old_d = d;
  d += n_new_rows;
  b.set_rows(d);
  for (int i = old_d; i < d; i++)
  {
    for (int j = 0; j < b.get_cols(); j++)
    {
      b[i][j] = 0;
    }
  }
  if (enable_transform)
  {
    u.set_rows(d);
    for (int i = old_d; i < d; i++)
      for (int j = 0; j < u.get_cols(); j++)
        u[i][j]  = 0;
  }
  size_increased();
  if (n_known_rows == old_d)
    discover_all_rows();
}

template <class ZT, class FT> inline void MatGSO<ZT, FT>::remove_last_rows(int n_removed_rows)
{
  FPLLL_DEBUG_CHECK(!cols_locked && d >= n_removed_rows);
  d -= n_removed_rows;
  n_known_rows  = min(n_known_rows, d);
  n_source_rows = n_known_rows;
  b.set_rows(d);
  if (enable_transform)
    u.set_rows(d);
}


FPLLL_END_NAMESPACE

#endif
