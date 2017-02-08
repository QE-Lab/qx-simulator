/**
 * @file		gate.h
 * @author	Nader KHAMMASSI - nader.khammassi@gmail.com 
 * @date		02-10-15
 * @brief		
 */

#pragma once

#ifndef QX_GATE_H
#define QX_GATE_H
#define  _USE_MATH_DEFINES
#include <cmath> 
#include <map>

// #include <xpu.h>

#include <immintrin.h> // avx
#include <emmintrin.h> // sse

#include <core/hash_set.h>
#include <core/linalg.h>
#include <core/register.h>

#include <core/binary_counter.h>
#include <core/kronecker.h>

// #ifndef __BUILTIN_LINALG__
// #include <boost/numeric/ublas/matrix.hpp>
// #endif

#define SQRT_2   (1.4142135623730950488016887242096980785696718753769480731766797379f)
#define R_SQRT_2 (0.7071067811865475244008443621048490392848359376884740365883398690f)

#define __bit_test(x,pos) ((x) & (1<<(pos)))
#define __bit_set(x,pos) ((x) | (1<<(pos)))
#define __bit_flip(x,pos) ((x) ^ (1<<(pos)))
#define __bit_reset(x,pos) ((x) & ~(1<<(pos)))

#define __AVX__NO
#define __OP_PREFETCH__

//#define SQRT_2   (1.41421356237309504880f)
//#define R_SQRT_2 (0.70710678118654752440f)

namespace qx
{
   /**
    * types definition
    */
   typedef uint64_t                          basis_state_t;
   typedef std::map<basis_state_t,complex_t> quantum_state_t;
   typedef enum __gate_type_t
   {
      __identity_gate__,
      __hadamard_gate__,
      __pauli_x_gate__ ,
      __pauli_y_gate__ ,
      __pauli_z_gate__ ,
      __cnot_gate__    ,
      __toffoli_gate__ ,
      __swap_gate__    ,
      __phase_gate__   ,
      __rx_gate__      ,
      __ry_gate__      ,
      __rz_gate__      ,
      __cphase_gate__  ,
      __t_gate__       ,
      __tdag_gate__    ,
      __custom_gate__  ,
      __prepz_gate__   ,
      __measure_gate__ ,
      __measure_reg_gate__,
      __ctrl_phase_shift_gate__,
      __parallel_gate__,
      __display__,
      __display_binary__,
      __print_str__, 
      __bin_ctrl_gate__,
      __lookup_table__,
      __classical_not_gate__,
      __qft_gate__,
      __prepare_gate__
   } gate_type_t;


   /**
    * gates coeffecients
    */
   const complex_t cnot_c     [] __attribute__((aligned(64))) = { complex_t(1.0), complex_t(0.0), complex_t(0.0), complex_t(0.0), complex_t(0.0), complex_t(1.0), complex_t(0.0), complex_t(0.0), complex_t(0.0), complex_t(0.0), complex_t(0.0), complex_t(1.0), complex_t(0.0), complex_t(0.0), complex_t(1.0), complex_t(0.0) };  /* CNOT */
   const complex_t swap_c     [] __attribute__((aligned(64))) = { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 };  /* SWAP */
   const complex_t identity_c [] __attribute__((aligned(64))) = { complex_t(1.0), complex_t(0.0), complex_t(0.0), complex_t(1.0) };                                /* I */
   const complex_t pauli_x_c  [] __attribute__((aligned(64))) = { complex_t(0.0, 0.0) , complex_t(1.0, 0.0), complex_t(1.0, 0.0) , complex_t(0.0, 0.0) };          /* X */
   const complex_t pauli_y_c  [] __attribute__((aligned(64))) = { complex_t(0.0, 0.0) , complex_t(0.0,-1.0), complex_t(0.0, 1.0) , complex_t(0.0, 0.0) };          /* Y */
   const complex_t pauli_z_c  [] __attribute__((aligned(64))) = { complex_t(1.0, 0.0) , complex_t(0.0, 0.0), complex_t(0.0, 0.0) , complex_t(-1.0,0.0) };          /* Z */
   const complex_t phase_c    [] __attribute__((aligned(64))) = { complex_t(1.0, 0.0) , complex_t(0.0, 0.0), complex_t(0.0, 0.0) , complex_t(0.0, 1.0) };          /* S */
   const complex_t t_gate_c   [] __attribute__((aligned(64))) = { complex_t(1.0, 0.0) , complex_t(0.0, 0.0), complex_t(0.0, 0.0) , complex_t(cos(M_PI/4),sin(M_PI/4)) };         /* T */
   const complex_t tdag_gate_c[] __attribute__((aligned(64))) = { complex_t(1.0, 0.0) , complex_t(0.0, 0.0), complex_t(0.0, 0.0) , complex_t(cos(M_PI/4),-sin(M_PI/4)) };        /* T_dag */
   const complex_t hadamard_c [] __attribute__((aligned(64)))  = { R_SQRT_2,  R_SQRT_2, R_SQRT_2, -R_SQRT_2 };  /* H */

   #define __rc(r,c,s) (r*s+c)


   /**
    * \brief common abstract gate interface for
    *   all gates implementation.
    */
   class gate
   {
	 public:
	   
	   virtual int32_t                apply(qu_register& qureg) = 0;
	   virtual std::vector<uint32_t>  qubits() = 0;
	   virtual std::vector<uint32_t>  control_qubits() = 0;
	   virtual std::vector<uint32_t>  target_qubits()  = 0;
	   virtual gate_type_t            type() = 0;
	   virtual std::string            micro_code() { return "# unsupported operation : qubit out of range"; }
	   virtual void                   dump() = 0;
	   virtual                        ~gate() { };                

	   virtual void                   set_duration(uint64_t d) { duration = d; }
	   virtual uint64_t               get_duration() { return duration; }
	 
	 protected:

	   uint64_t                       duration;

   };


   /**
    * \brief rotation in the x-z plane with a given 
    *     angle theta (see "Large scale simulation of 
    *     error-prone quantum systems" p.39" [Niwa 2002])
    */
   inline cmatrix_t rotation(double theta)
   {
	 cmatrix_t r; // (2,2);
	 r(0,0) = complex_t(cos(theta),0); r(0,1) = complex_t(-sin(theta),0);
	 r(1,0) = complex_t(sin(theta),0); r(1,1) = complex_t(cos(theta),0);
	 return r;
   }

   /**
    * \brief phase shift for a given angle phi
    */
   inline cmatrix_t phase(double phi)
   {
	 cmatrix_t p; // (2,2);
	 p(0,0) = complex_t(1,0); p(0,1) = complex_t(0,0);
	 p(1,0) = complex_t(0,0); p(1,1) = complex_t(cos(phi),sin(phi));
	 return p;
   }

   /**
    * \brief generate noisy hadamard gate
    */
   cmatrix_t noisy_hadamard(double epsilon1=0, double epsilon2=0)
   {
#ifdef __BUILTIN_LINALG__
	 return mxm(rotation(M_PI/4 + epsilon1), phase(M_PI + epsilon2));
#else
	 cmatrix_t rz = rotation(M_PI/4 + epsilon1);
	 cmatrix_t p  = phase(M_PI + epsilon2);
	 return mxm(rz,p);
#endif 
   }


   /**
    * \brief build n x n matrix from an array
    */
   cmatrix_t build_matrix(const complex_t * c, uint32_t n)
   {
         // assert(n==2); 
	 // TO DO : remove the n parameter
	 cmatrix_t m; // (n,n);
	 for (int i=0; i<n; i++)
	    for (int j=0; j<n; j++) 
		  m(i,j) = c[i*n+j];
	 return m;
   }


   /**
    * sqg_apply
    */
   #ifdef QX_COMPACT_GATE_OP
 
   inline void sqg_apply(cmatrix_t & cm, uint32_t qubit, qu_register& qureg)
   {
	 uint32_t n  = qureg.size();

	 matrix_t m(2,row_t(2,0));
	 m[0][0] = cm(0,0); m[0][1] = cm(0,1);
	 m[1][0] = cm(1,0); m[1][1] = cm(1,1);

	 if (qubit == 0)
	 {
	    identity       id(1 << (n-1));
	    unitary_matrix um(cm.size1(),m);
	    kronecker      k(&id, &um);
	    cvector_t      r(qureg.get_data());
	    mulmv(k,qureg.get_data(),r);
	    qureg = r;
	 }
	 else if (qubit == n-1)
	 {
	    identity       id(1 << (n-1));
	    unitary_matrix um(cm.size1(),m);
	    kronecker      k(&um, &id);
	    cvector_t      r(qureg.get_data());
	    mulmv(k,qureg.get_data(),r);
	    qureg = r;
	 }
	 else
	 {
	    identity       id1(1 << (qubit));
	    identity       id2(1 << (n-qubit-1));
	    unitary_matrix um(cm.size1(),m);
	    kronecker      k(&id2, &um, &id1);
	    cvector_t      r(qureg.get_data());
	    mulmv(k,qureg.get_data(),r);
	    qureg = r;
	 }
   }

   /**
    * u on the kth qubit :
    * non-null value in each row of the kronocker matrix:
    *     for each row r :    
    *            c1 = r || 000100   // 1 at the n-k bit
    *            c2 = r || 000000
    */

   // #elif QX_SPARSE_MV_MUL
   #else // QX_SPARSE_MV_MUL
   
   uint32_t rw_process(int is, int ie, int s, uint32_t n, uint32_t qubit, const kronecker * m, cvector_t * v, cvector_t * res)
   {
      uint32_t k = n-qubit;
      // println("run : " << is << " .. " << ie);
      complex_t * pv = v->data();
      complex_t * pr = res->data();
      size_t nk = n-k;

      for (uint32_t r=is; r<ie; ++r)
      {
	 size_t bc = r;
	 size_t c1 = __bit_reset(bc,nk);
	 size_t c2 = __bit_set(bc,nk);
	 // complex_t s; // = 0;
	 pr[r] = pv[c1]*(m->get(r,c1)) + pv[c2]*(m->get(r,c2));
      }

      return 0;
   }
      

   void sparse_mulmv(uint32_t n, uint32_t qubit, const kronecker& m, cvector_t& v, cvector_t& res)
   {
      uint32_t k = n-qubit;
      uint32_t rows = (1 << n);
      uint32_t z = 0;

      // xpu::task rw_t(rw_process,0,0,0,n,qubit,&m,&v,&res);
      // xpu::parallel_for process(z,rows,1,&rw_t);

      // process.run();
      rw_process(0,rows,1,n,qubit,&m,&v,&res);

/*
      #pragma omp parallel for schedule(static)
      for (uint32_t r=0; r<rows; r++)
      {
	 binary_counter bc(n);
	 complex_t s = 0;
	 bc = r;
	 bc.unset(n-k);
	 uint32_t c1 = bc.value();
	 bc.set(n-k);
	 uint32_t c2 = bc.value();
	 //println("r=" << r << " : " << c1 << "," << c2);
	 //println("v=" << v[c1] << " x m=" << m.get(r,c1));
	 //println("v=" << v[c2] << " x m=" << m.get(r,c2));
	 s += v[c1]*(m.get(r,c1));
	 s += v[c2]*(m.get(r,c2));
	 //println("s=" << s);
	 res[r] = s;
      }
 */
   }

   void __apply_m(std::size_t start, std::size_t end, const std::size_t qubit, complex_t * state, const std::size_t stride0, const std::size_t stride1, const complex_t * matrix)
   {

#if 0
      __m128d m00 = matrix[0].xmm;
      __m128d m01 = matrix[1].xmm;
      __m128d m10 = matrix[2].xmm;
      __m128d m11 = matrix[3].xmm;
#endif

      complex_t m00 = matrix[0];
      complex_t m01 = matrix[1];
      complex_t m10 = matrix[2];
      complex_t m11 = matrix[3];


#ifdef USE_OPENMP
#pragma omp parallel for // shared(m00,m01,m10,m11)
#endif
      for(size_t offset = start; offset < end; offset += (1L << (qubit + 1L)))
	 for(size_t i = offset; i < offset + (1L << qubit); i++)
	 {
	    size_t i0 = i + stride0;
	    size_t i1 = i + stride1;

	    complex_t in0 = state[i0];
	    complex_t in1 = state[i1];
	    state[i0] = m00*in0+m01*in1;
	    state[i1] = m10*in0+m11*in1;
#if 0
	    __m128d in0 = state[i0].xmm;
	    __m128d in1 = state[i1].xmm;
	    state[i0].xmm = _mm_add_pd(xpu::_mm_mulc_pd(m00, in0), xpu::_mm_mulc_pd(m10, in1));
	    state[i1].xmm = _mm_add_pd(xpu::_mm_mulc_pd(m10, in1), xpu::_mm_mulc_pd(m11, in1));
#endif
	 }
   }
/*
#ifdef __SSE__
// #ifdef __FMA__
   void __apply_x(std::size_t start, std::size_t end, const std::size_t qubit, complex_t * state, const std::size_t stride0, const std::size_t stride1, const complex_t * matrix)
   {
#ifdef USE_OPENMP
#pragma omp parallel for // private(m00,r00,neg)    // BUG : ERRORS WHEN ACTIVATING PARALLELISM
#endif
      for(size_t offset = start; offset < end; offset += (1L << (qubit + 1L)))
	 for(size_t i = offset; i < offset + (1L << qubit); i++)
	 {
	    size_t i0 = i + stride0;
	    size_t i1 = i + stride1;

	    __m128d xin0 = state[i0].xmm; // _mm_load_pd((double*)&(state[i0].xmm));
	    // __m128d xin1 = state[i1].xmm; // _mm_load_pd((double*)&(state[i1].xmm));
	    
	    state[i0].xmm = state[i1].xmm;
	    state[i1].xmm = xin0;
	 }
   }
// #else
// #error "FMA not available !"
// #endif // FMA
#else
#error "SSE not available !"
#endif // SSE
*/

/*
#ifdef __SSE__
// #ifdef __FMA__
   void __apply_h(std::size_t start, std::size_t end, const std::size_t qubit, complex_t * state, const std::size_t stride0, const std::size_t stride1, const complex_t * matrix)
   {
      __m128d   m00 = matrix[0].xmm;
      __m128d   r00 = _mm_shuffle_pd(m00,m00,3);         // 1 cyc
      __m128d   neg = _mm_set1_pd(-0.0f);

#ifdef USE_OPENMP
#pragma omp parallel for // private(m00,r00,neg)    // BUG : ERRORS WHEN ACTIVATING PARALLELISM
#endif
      for(size_t offset = start; offset < end; offset += (1L << (qubit + 1L)))
	 for(size_t i = offset; i < offset + (1L << qubit); i++)
	 {
	    size_t i0 = i + stride0;
	    size_t i1 = i + stride1;

	    __m128d xin0 = state[i0].xmm; // _mm_load_pd((double*)&(state[i0].xmm));
	    __m128d xin1 = state[i1].xmm; // _mm_load_pd((double*)&(state[i1].xmm));

	    __m128d t2; // = _mm_shuffle_pd(m01,m01,3);     // 1 cyc
	    __m128d t1 = _mm_mul_pd(xin0,r00);               // 5 cyc
#ifdef __FMA__
	    __m128d xi0 = _mm_fmadd_pd (xin1,r00, t1);       // x2*t2+t1    // 5 cyc
#else 
	    __m128d xi0 = _mm_mul_pd(xin1,r00);
	    xi0         = _mm_add_pd(xi0,t1);       // x2*t2+t1    // 5 cyc
#endif // __FMA__
	    // t2 = _mm_shuffle_pd(m11,m11,3);              // 1 cyc
	    t2 = _mm_xor_pd(r00,neg);   // 1 cyc  (m11=-m00)
#ifdef __FMA__
	    __m128d xi1 = _mm_fmadd_pd (xin1, t2, t1);      // x2*t2+t1    // 5 cyc
#else
	    __m128d xi1 = _mm_mul_pd(xin1,t2);
	    xi1         = _mm_add_pd(xi1,t1);      // x2*t2+t1    // 5 cyc
#endif

	    state[i0].xmm = xi0; // _mm_store_pd((double*)(&state[i0].xmm),xi0);
	    state[i1].xmm = xi1; // _mm_store_pd((double*)(&state[i1].xmm),xi1);
	 }
   }
// #else
// #error "FMA not available !"
// #endif // FMA
#else
#error "SSE not available !"
#endif // SSE
*/
   uint32_t rw_process_ui(int is, int ie, int s, uint32_t n, uint32_t qubit, kronecker_ui m, cvector_t * v, cvector_t * res)
   {
/*
      uint32_t k = n-qubit;
      // println("run : " << is << " .. " << ie);
      complex_t * pv = v->data();
      complex_t * pr = res->data();
      size_t bc, c1, c2;
      size_t nk = n-k;

      for (uint32_t r=is; r<ie; ++r)
      {
	 bc = r;
	 c1 = __bit_reset(bc,nk);
	 c2 = __bit_set(bc,nk);
	 bc++;
#ifdef __OP_PREFETCH__
	 _mm_prefetch((void*)&pv[__bit_reset(bc,nk)],_MM_HINT_T0);
	 _mm_prefetch((void*)&pv[__bit_set(bc,nk)],_MM_HINT_T0);
#endif // __OP_PREFETCH__
#ifdef __AVX__ 
	 // cxc
	 xpu::_mm_cmul_add_pd(pv[c1], pv[c2], m.get(r,c1), m.get(r,c2),pr[r]);
	 // cxr
	 // pr[r].xmm = complex_t::_mm256_cr_mul_add_pd(pv[c1].xmm, m.get(r,c1).xmm, pv[c2].xmm, m.get(r,c2).xmm);
#elif __SSE__
	 // complex_t s; // = 0;
	 //pr[r] = pv[c1]*(m->get(r,c1)) + pv[c2]*(m->get(r,c2));
	 // --- cc mul add --- 
	 pr[r].xmm = _mm_add_pd((pv[c1]*(m.get(r,c1))).xmm, (pv[c2]*(m.get(r,c2))).xmm);
	 // --- cr mul add --- pr[r].xmm = _mm_add_pd(complex_t::mul_cr(pv[c1].xmm,m.get(r,c1).xmm), complex_t::mul_cr(pv[c2].xmm,m.get(r,c2).xmm));
	 // --- f. mul add ---
	 // pr[r].xmm = complex_t::_mm_cr_mul_add_pd(pv[c1].xmm, m.get(r,c1).xmm, pv[c2].xmm, m.get(r,c2).xmm);
#else
	 pr[r] = (pv[c1]*(m.get(r,c1))) + (pv[c2]*(m.get(r,c2)));
#endif
      }
*/

      return 0;
   }
 

   void sparse_mulmv(uint32_t n, uint32_t qubit, kronecker_ui m, cvector_t& v, cvector_t& res)
   {
      uint32_t k = n-qubit;
      uint32_t rows = (1 << n);
      uint32_t z = 0;

      rw_process_ui(z,rows,1,n,qubit,m,&v,&res);
   }

   uint32_t rw_process_iu(int is, int ie, int s, uint32_t n, uint32_t qubit, kronecker_iu m, cvector_t * v, cvector_t * res)
   {
/*
      uint32_t k = n-qubit;
      // println("run : " << is << " .. " << ie);
      complex_t * pv = v->data();
      complex_t * pr = res->data();
      size_t bc, c1, c2;
      size_t nk = n-k;

      for (uint32_t r=is; r<ie; ++r)
      {
	 bc = r;
	 c1 = __bit_reset(bc,nk);
	 c2 = __bit_set(bc,nk);
	 bc++;
#ifdef __OP_PREFETCH__
	 _mm_prefetch((void*)&pv[__bit_reset(bc,nk)],_MM_HINT_T0);
	 _mm_prefetch((void*)&pv[__bit_set(bc,nk)],_MM_HINT_T0);
#endif // __OP_PREFETCH__
#ifdef __AVX__ 
	 // cxc
	 xpu::_mm_cmul_add_pd(pv[c1], pv[c2], m.get(r,c1), m.get(r,c2),pr[r]);
	 // cxr
	 // pr[r].xmm = complex_t::_mm256_cr_mul_add_pd(pv[c1].xmm, m.get(r,c1).xmm, pv[c2].xmm, m.get(r,c2).xmm);
#elif __SSE__
	 // complex_t s; // = 0;
	 // pr[r] = pv[c1]*(m->get(r,c1)) + pv[c2]*(m->get(r,c2));
	 // --- cc mul add --- 
	 pr[r].xmm = _mm_add_pd((pv[c1]*(m.get(r,c1))).xmm, (pv[c2]*(m.get(r,c2))).xmm);
	 // --- cr mul add --- pr[r].xmm = _mm_add_pd(complex_t::mul_cr(pv[c1].xmm,m.get(r,c1).xmm), complex_t::mul_cr(pv[c2].xmm,m.get(r,c2).xmm));
	 // --- f. mul add ---
	 // pr[r].xmm = complex_t::_mm_cr_mul_add_pd(pv[c1].xmm, m.get(r,c1).xmm, pv[c2].xmm, m.get(r,c2).xmm);
#else
	 pr[r] = (pv[c1]*(m.get(r,c1))) + (pv[c2]*(m.get(r,c2)));
#endif
      }
*/

      return 0;
   }
 

   void sparse_mulmv(uint32_t n, uint32_t qubit, kronecker_iu m, cvector_t& v, cvector_t& res)
   {
      uint32_t k = n-qubit;
      uint32_t rows = (1 << n);
      uint32_t z = 0;

      rw_process_iu(z,rows,1,n,qubit,m,&v,&res);
   }
   
   // static xpu::core::os::mutex mtx;

   uint32_t rw_process_iui(int is, int ie, int s, uint32_t n, uint32_t qubit, kronecker_iui m, cvector_t * v, cvector_t * res)
   {
#if 0
      uint32_t k = n-qubit;
      // println("run : " << is << " .. " << ie);
      complex_t * pv = v->data();
      complex_t * pr = res->data();
      size_t bc, c1, c2;
      size_t nk = n-k;

      for (uint32_t r=is; r<ie; r++) //+=2)
      {
	 // 1st
	 bc = r;
	 c1 = __bit_reset(bc,nk);
	 c2 = __bit_set(bc,nk);
	 bc++;
#ifdef __OP_PREFETCH__
	 _mm_prefetch((void*)&pv[__bit_reset(bc,nk)],_MM_HINT_T0);
	 _mm_prefetch((void*)&pv[__bit_set(bc,nk)],_MM_HINT_T0);
#endif // __OP_PREFETCH__
#ifdef __AVX__ 
	 // mtx.lock();
	 // cxc : 
	 xpu::_mm_cmul_add_pd(pv[c1], pv[c2], m.get(r,c1), m.get(r,c2),pr[r]);
	 // cxr
	 // pr[r].xmm = complex_t::_mm256_cr_mul_add_pd(pv[c1].xmm, m.get(r,c1).xmm, pv[c2].xmm, m.get(r,c2).xmm);

	 /*
	 __m256d a; //_mm256_loadu2_m128d((double*)&pv[c1], (double*)&pv[c2]);
	 a  = _mm256_insertf128_pd(a,_mm_permute_pd(pv[c1].xmm,1), 0);
	 a  = _mm256_insertf128_pd(a,_mm_permute_pd(pv[c2].xmm,1), 1);
	 print("(r="<<r<<") : pr12: "); xpu::dump_m256d(a);
	 // __m256d b  = _mm256_set_m128d((m.get(r,c1)).xmm, (m.get(r,c2)).xmm);
	 __m256d b;
	 b  = _mm256_insertf128_pd(b,_mm_permute_pd(m.get(r,c1).xmm, 1), 1);
	 print("(r="<<r<<") : c1 : "); xpu::dump_m256d(b);
	 b  = _mm256_insertf128_pd(b,_mm_permute_pd(m.get(r,c2).xmm, 1), 0);
	 print("(r="<<r<<") : c2 : "); xpu::dump_m256d(b);
	 __m256d ab  = xpu::_mm256_cmul_pd(a,b);
	 print("(r="<<r<<") : mul: "); xpu::dump_m256d(ab);
	 __m256d abr = _mm256_permute2f128_pd(ab, ab, 1);
	 print("(r="<<r<<") : prm: "); xpu::dump_m256d(abr);
	 ab = _mm256_add_pd(ab,abr);
	 print("(r="<<r<<") : add: "); xpu::dump_m256d(ab);
	 pr[r].xmm = _mm256_extractf128_pd(ab,0);
	 print("(r="<<r<<") : res:"); xpu::dump_m128d(pr[r].xmm);
	 mtx.unlock();
	 */
#elif  __SSE__
	 /*
	 mtx.lock();
	 print("(r="<<r<<") : pr1: "); xpu::dump_m128d(pv[c1].xmm);
	 print("(r="<<r<<") : pr2: "); xpu::dump_m128d(pv[c2].xmm);
	 print("(r="<<r<<") : c1 : "); xpu::dump_m128d((m.get(r,c1)).xmm);
	 print("(r="<<r<<") : c2 : "); xpu::dump_m128d((m.get(r,c2)).xmm);
	 */
	 // --- cxc mul --- 
	 pr[r].xmm = _mm_add_pd((pv[c1]*(m.get(r,c1))).xmm, (pv[c2]*(m.get(r,c2))).xmm);
	 // --- cxr mul --- pr[r].xmm = _mm_add_pd(complex_t::mul_cr(pv[c1].xmm,m.get(r,c1).xmm), complex_t::mul_cr(pv[c2].xmm,m.get(r,c2).xmm));
	 // --- fus ma  ---
	 // pr[r].xmm = complex_t::_mm_cr_mul_add_pd(pv[c1].xmm, m.get(r,c1).xmm, pv[c2].xmm, m.get(r,c2).xmm);

         // pr[r].xmm = xpu::_mm128_mul_add_pc(pv[c1].xmm, pv[c2].xmm, m.get(r,c1).xmm, m.get(r,c2).xmm);
	 /*
	 print("(r="<<r<<") : res: "); xpu::dump_m128d(pr[r].xmm);
	 mtx.unlock();
	 */
#else
	 pr[r] = (pv[c1]*(m.get(r,c1))) + (pv[c2]*(m.get(r,c2)));
#endif
	 /*
	 // 2nd
	 c1 = __bit_reset(bc,n-k);
	 c2 = __bit_set(bc,n-k);
#ifdef __AVX__NO 
	 a  = _mm256_loadu2_m128d((double*)&pv[c1], (double*)&pv[c2]);
	 // __m256d b  = _mm256_set_m128d((m.get(r,c1)).xmm, (m.get(r,c2)).xmm);
	 b  = _mm256_insertf128_pd(b,(m.get(bc,c1)).xmm, 1);
	 b  = _mm256_insertf128_pd(b,(m.get(bc,c2)).xmm, 0);
	 ab  = xpu::_mm256_cmul_pd(a,b);
	 abr = _mm256_permute2f128_pd(ab, ab, 1);
	 ab = _mm256_add_pd(ab,abr);
	 pr[bc].xmm = _mm256_extractf128_pd(ab,0);
#elif  __SSE__
	 pr[bc].xmm = _mm_add_pd((pv[c1]*(m.get(bc,c1))).xmm, (pv[c2]*(m.get(bc,c2))).xmm);
#else
	 pr[bc] = (pv[c1]*(m.get(bc,c1))) + (pv[c2]*(m.get(bc,c2)));
#endif
	 */
      }
#endif

      return 0;
   }
 

   void sparse_mulmv(uint32_t n, uint32_t qubit, kronecker_iui m, cvector_t& v, cvector_t& res)
   {
      uint32_t k = n-qubit;
      uint32_t rows = (1 << n);
      uint32_t z = 0;

      rw_process_iui(z,rows,1,n,qubit,m,&v,&res);
   }



   inline void sqg_apply(cmatrix_t & cm, uint32_t qubit, qu_register& qureg)
   {
	 uint32_t     n  = qureg.size();
	 complex_t *  s  = qureg.get_data().data();
	 // cm.dump();
	 __apply_m(0, (1 << n), qubit, s, 0, (1 << qubit), cm.m);
	 return;

#if 0
	 if (qubit == 0)
	 {
	    /*
	    identity       id(1 << (n-1));
	    unitary_matrix um(cm.size1(),m);
	    kronecker      k(&id, &um);
	    // cvector_t      r(qureg.get_data());
	    cvector_t&     r = qureg.get_aux(); //r(qureg.get_data());
	    sparse_mulmv(n,qubit,k,qureg.get_data(),r);
	    //qureg = r;
	    qureg.get_data().swap(r);
	    */
	    // kronecker_ui k(cm,2,(1 << (n-1)));
	    // kronecker_ui k(hadamard_c,2,(1 << (n-1)));
	    kronecker_ui k(cm.m,2,(1 << (n-1)));
	    cvector_t&     r = qureg.get_aux(); 
	    sparse_mulmv(n,qubit,k,qureg.get_data(),r);
	    qureg.get_data().swap(r);
	 }
	 else if (qubit == n-1)
	 {
	    /*
	    identity       id(1 << (n-1));
	    unitary_matrix um(cm.size1(),m);
	    kronecker      k(&um, &id);
	    cvector_t&     r = qureg.get_aux(); //r(qureg.get_data());
	    sparse_mulmv(n,qubit,k,qureg.get_data(),r);
	    //qureg = r;
	    qureg.get_data().swap(r);
	    */
	    // kronecker_iu k(cm,2,(1 << (n-1)));
	    // kronecker_iu k(hadamard_c,2,(1 << (n-1)));
	    kronecker_iu k(cm.m,2,(1 << (n-1)));
	    cvector_t&     r = qureg.get_aux(); 
	    sparse_mulmv(n,qubit,k,qureg.get_data(),r);
	    qureg.get_data().swap(r);
	 }
	 else
	 {
	    /*
	    identity       id1(1 << (qubit));
	    identity       id2(1 << (n-qubit-1));
	    unitary_matrix um(cm.size1(),m);
	    kronecker      k(&id2, &um, &id1);
	    //cvector_t      r(qureg.get_data());
	    cvector_t&     r = qureg.get_aux(); //r(qureg.get_data());
	    sparse_mulmv(n,qubit,k,qureg.get_data(),r);
	    //qureg = r;
	    qureg.get_data().swap(r);
	    */
	    // kronecker_iui k(cm, 2, (1 << (n-qubit-1)), (1 << (qubit)));
	    // kronecker_iui k(hadamard_c, 2, (1 << (n-qubit-1)), (1 << (qubit)));
	    kronecker_iui k(cm.m, 2, (1 << (n-qubit-1)), (1 << (qubit)));
	    cvector_t&     r = qureg.get_aux(); 
	    sparse_mulmv(n,qubit,k,qureg.get_data(),r);
	    qureg.get_data().swap(r);
	 }
#endif
   }

#endif // remove naive tensor computation

/*
   #else

   inline void sqg_apply(cmatrix_t &m, uint32_t qubit, qu_register& qureg)
   {
	 uint32_t n  = qureg.size();
	 
	 if (qubit == 0)
	 {
	    cidentity_t i(1 << (n-1));
	    qureg = mxv(tensor(i,m), qureg.get_data());
	 }
	 else if (qubit == n-1)
	 {
	    cidentity_t i(1 << (n-1));
	    qureg = mxv(tensor(m,i), qureg.get_data());
	 }
	 else
	 {
	    cidentity_t i1(1 << (qubit));
	    cidentity_t i2(1 << (n-qubit-1));
	    qureg = mxv(tensor(tensor(i2,m),i1), qureg.get_data());
	 }
   }


   #endif // QX_COMPACT_GATE_OP

*/

   typedef enum    
   {
      __x180__, 
      __x90__ , 
      __y180__, 
      __y90__ , 
      __ym90__ 
   } elementary_operation_t;

   static const char * pulse_lt[][5] = 
   {
    { "  pulse 9,0,0", "  pulse 10,0,0", "  pulse 11,0,0", "  pulse 12,0,0", "  pulse 14,0,0" },
    { "  pulse 0,9,0", "  pulse 0,10,0", "  pulse 0,11,0", "  pulse 0,12,0", "  pulse 0,14,0" },
    { "  pulse 0,0,9", "  pulse 0,0,10", "  pulse 0,0,11", "  pulse 0,0,12", "  pulse 0,0,14" },
   };


   /**
    * \brief hadamard gate:
    *
    *               | 1   1|
    *     1/sqrt(2) |      |
    *               | 1  -1|
    */
   class hadamard : public gate
   {
	 private:

	   uint32_t   qubit;
	   cmatrix_t  m;

	 public:
	   
	   hadamard(uint32_t qubit) : qubit(qubit) //,m((complex_t*)hadamard_c)
	   {
		 m = build_matrix(hadamard_c,2);
	   }

	   int32_t apply(qu_register& qureg)
	   {
	         size_t qs = qureg.states();
		 complex_t * data = qureg.get_data().data();
		 // sqg_apply(m,qubit,qureg);
                 //__apply_h(0, qs, qubit, data, 0, (1 << qubit), hadamard_c);
                 __apply_m(0, qs, qubit, data, 0, (1 << qubit), hadamard_c);
                 //__apply_h_old(0, qs, qubit, data, 0, (1 << qubit), hadamard_c);

		 // qureg.set_binary(qubit,__state_unknown__);
		 qureg.set_measurement_prediction(qubit,__state_unknown__);
		 return 0;
	   }

	   std::string  micro_code()
	   {
	      /**
	        | wait 5
		| y90 q0  --> { pulse 12,0,0 }
	    	| wait 5
		| x180 q0 --> { pulse 9,0,0 }
		*/
	      if (qubit > 2) return "# unsupported operation : qubit out of range";
	      std::stringstream uc;
	      uc << pulse_lt[qubit][__y90__] << "\n";
	      uc << "  wait 4 \n";
	      uc << pulse_lt[qubit][__x180__] << "\n";
	      uc << "  wait 4 \n";
	      return uc.str();
	   }

	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
	   
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __hadamard_gate__; 
	   }

	   void dump()
	   {
		 println("  [-] hadamard(q=" << qubit << ")");
	   }

   };
   
   void __swap(cvector_t& amp, size_t size, size_t bit, size_t trg, size_t ctrl, size_t offset=0)
   {
      // println("bit=" << bit);
      // println("ctrl=" << ctrl);
      complex_t * p = amp.data();
      for (size_t i=__bit_set(0,bit); i<(1<<size); i += (1 << (bit+1)))
	 for (size_t j=0; j<(1<<bit); j++)
	 {
	    size_t v = i+j+offset; 
	    /*
	    #ifdef __SSE__
	    __m128d x = _mm_load_pd((const double *)&p[v]);
	    __m128d y = _mm_load_pd((const double *)&p[__bit_reset(v,trg)]);
	    _mm_store_pd((double *)&p[__bit_reset(v,trg)],x);
	    _mm_store_pd((double *)&p[v],y);
	    #else
	    */
	    std::swap(amp[v], amp[__bit_reset(v,trg)]);
	    // println("swap("<<v<<","<<__bit_reset(v,trg)<<")");
	    // #endif
	 }
   }


   int cx_worker(int cs, int ce, int s, cvector_t * p_amp, size_t bit1, size_t bit2, size_t trg, size_t ctrl)
   {
      cvector_t &amp = * p_amp;
      // xpu::parallel_for fswp(__bit_set(0,b1), (1 << qn), (1 << (b1+1)), &t);
      size_t step=(1 << (bit1+1));
      size_t b   = cs;
      size_t e   = ce;
      size_t offset = __bit_set(0,bit1);

      //for (size_t i=__bit_set(0,bit1); i<(1<<size); i += (1 << (bit1+1)))
	 //__swap(amp,bit1,bit2,trg,ctrl,i);
      for (size_t i=b; i<e; i++)
	 __swap(amp,bit1,bit2,trg,ctrl,offset+(i*step));
      return 0;
   }

   /**
    * \brief controlled-not gate:
    *
    *    | 1  0  0  0 | 
    *    | 0  1  0  0 |
    *    | 0  0  0  1 |
    *    | 0  0  1  1 |  
    */
   class cnot : public gate
   {
	 private:

	   uint32_t control_qubit;
	   uint32_t target_qubit;

	   cmatrix_t  m;

	 public:

	   cnot(uint32_t ctrl_q, uint32_t target_q) : control_qubit(ctrl_q), 
	                                              target_qubit(target_q)
	   {
		 // m = build_matrix(cnot_c,4); // stack smaching
	   }
	   
	   // #define CG_HASH_SET
	   //#define CG_MATRIX
	   #ifndef CG_BC 
           #ifndef CG_MATRIX 
             #define CG_BC
           #endif
           #endif // CG_BC

	   int32_t apply(qu_register& qreg)
	   {
	         // println("cnot " << control_qubit << "," << target_qubit);
		 #ifdef CG_MATRIX
		 uint32_t sn = qreg.states();
		 uint32_t qn = qreg.size();
		 uint32_t cq = control_qubit;
		 uint32_t tq = target_qubit;

		 cmatrix_t i = cidentity_t(sn);
		 perm_t    p = perms(qn,cq,tq);
		 
		 // dump_matrix(i);

		 for (perm_t::iterator it = p.begin(); it != p.end(); it++)
		 {
		    i(it->first,it->second)  = 1;
		    i(it->second,it->first)  = 1;
		    i(it->first, it->first)  = 0;
		    i(it->second,it->second) = 0;
		 }

		 // dump_matrix(i);

	         qreg = mxv(i, qreg.get_data());

		 #elif defined(CG_BC)

		 uint32_t sn = qreg.states();
		 uint32_t qn = qreg.size();
		 uint32_t cq = control_qubit;
		 uint32_t tq = target_qubit;

		 cvector_t& amp = qreg.get_data();

		 // perms(qn,cq,tq,amp);
// #if 0
		 size_t b1 = std::max(cq,tq);
		 size_t b2 = std::min(cq,tq);


		 size_t steps = ((1 << qn)-(__bit_set(0,b1)))/(1 << (b1+1))+1;
		 /*
		 println("from=" << (__bit_set(0,b1)));
		 println("to=" << (1 << qn));
		 println("s=" << (1 << (b1+1)));
		 println("steps=" << steps);
		 */
		 if (qn<17) 
		    fast_cx(amp, qn, b1, b2, tq, cq);
		 else
		 {
		    #pragma omp parallel for
		    for (size_t i=0; i<steps; ++i)
		       cx_worker(i,i+1,1,&amp,b1,b2,(size_t)tq,(size_t)cq);
		 }
// #endif

		 #elif defined(CG_HASH_SET)
		  
		 uint32_t j = control_qubit+1;
		 uint32_t k = target_qubit+1;

		 uint32_t k2 = (1 << (k-1));
		 uint32_t j2 = (1 << (j-1));
		 
		 uint32_t r_size = qreg.states(); 

		 xpu::container::hash_set<uint32_t> swap_set;

		 // find swap pairs
		 for (uint32_t t = 0; t < r_size; t++) 
		 {
		    if ((t & j2) <= 0) 
			  continue;

		    if (swap_set.find(t-k2) == swap_set.end())
			  swap_set.insert(t);
		 }
		 
		 int32_t t2;
		 cvector_t& amp = qreg.get_data();
		 complex_t c1, c2;

		 for (xpu::container::hash_set<uint32_t>::iterator t = swap_set.begin(); t != swap_set.end(); ++t) 
		 {
		    int32_t _t = *t;
		    t2 = (_t + k2 < r_size) ? _t + k2 : _t - k2;
		    c1 = amp(_t);
		    c2 = amp(t2);
		    std::swap(c1, c2);
		    amp(_t) = c1;
		    amp(t2) = c2;
		 }
		 //qreg=amp;
		 
		 #endif // CG_HASH_SET

		 // if (qreg.get_binary(control_qubit) == __state_1__)
		 if (qreg.get_measurement_prediction(control_qubit) == __state_1__)
		    qreg.flip_binary(target_qubit);
		 //else if (qreg.get_binary(control_qubit) == __state_unknown__)
		 else if (qreg.get_measurement_prediction(control_qubit) == __state_unknown__)
		    qreg.set_measurement_prediction(target_qubit,__state_unknown__);
		    // qreg.set_binary(target_qubit,__state_unknown__);
		 
		 return 0;
	   }


	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(control_qubit);
		 r.push_back(target_qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(control_qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(target_qubit);
		 return r;
	   }


	   gate_type_t type()
	   {
	      return __cnot_gate__; 
	   }
	   
	   void dump()
	   {
		 println("  [-] cnot(ctrl_qubit=" << control_qubit << ", target_qubit=" << target_qubit << ")");
	   }

	   private:
#if 0
	   void __swap(cvector_t& amp, size_t size, size_t bit, size_t trg, size_t ctrl, size_t offset=0)
	   {
	      // println("bit=" << bit);
	      // println("ctrl=" << ctrl);
	      for (size_t i=__bit_set(0,bit); i<(1<<size); i += (1 << (bit+1)))
		 for (size_t j=0; j<(1<<bit); j++)
		 {
		    size_t v = i+j+offset; 
		    std::swap(amp[v], amp[__bit_reset(v,trg)]);
		    // println(" swap(" << std::bitset<16>(v) << "," << std::bitset<16>(__bit_reset(v,trg)) << ")");
		 }
	   }
#endif


	   void fast_cx(cvector_t& amp, size_t size, size_t bit1, size_t bit2, size_t trg, size_t ctrl)
	   {
	      /*
		 println("from=" << (__bit_set(0,bit1)));
		 println("to=" << (1 << size));
		 println("s=" << (1 << (bit1+1)));
	      */
	      for (size_t i=__bit_set(0,bit1); i<(1<<size); i += (1 << (bit1+1)))
		 __swap(amp,bit1,bit2,trg,ctrl,i);
	   }

   };


   template<typename T>
      void swap_if_greater(T& a, T& b)
      {
	 if (a > b)
	 {
	    T tmp(a);
	    a = b;
	    b = tmp;
	 }
      }

   template<typename T>
      void sort(T& a, T& b, T& c)
      {
	 swap_if_greater(a, b);
	 swap_if_greater(a, c);
	 swap_if_greater(b, c);
      }

   /**
    * \brief toffoli gate:
    *
    *    | 1  0  0  0 | 
    *    | 0  1  0  0 |
    *    | 0  0  0  1 |
    *    | 0  0  1  1 |  
    */
   class toffoli : public gate
   {
	 private:

	   uint32_t control_qubit_1;
	   uint32_t control_qubit_2;
	   uint32_t target_qubit;

	 public:

	   toffoli(uint32_t ctrl_q1, uint32_t ctrl_q2, uint32_t target_q) : control_qubit_1(ctrl_q1), 
	                                                                 control_qubit_2(ctrl_q2),
									 target_qubit(target_q)
	   {
	   }
	   

	   int32_t apply(qu_register& qreg)
	   {
		 uint32_t sn  = qreg.states();
		 uint32_t qn  = qreg.size();
		 uint32_t cq1 = control_qubit_1;
		 uint32_t cq2 = control_qubit_2;
		 uint32_t tq = target_qubit;

		 cvector_t& amp = qreg.get_data();

		 //println("\ntoffoli " << cq1 << "," << cq2 << "," << tq);
#if 1
		 size_t c1=cq1;
		 size_t c2=cq2;
		 size_t c3=tq;
		 size_t t=tq;
		 size_t size=qn;

		 sort(c1,c2,c3);
#ifdef USE_OPENMP
#pragma omp parallel for
#endif
		 for (size_t i=__bit_set(__bit_set(__bit_set(0,c1),c2),c3); i<(1<<size); i += (1 << (c3+1)))
		    for (size_t j=i; j<(i+(1<<c3)); j += (1 << (c2+1)))
		       for (size_t k=j; k<(j+(1<<c2)); k+=(1 << (c1+1)))
		           for (size_t l=k; l<(k+(1<<(c1))); l++)
		       {
			  std::swap(amp[__bit_set(l,t)],amp[__bit_reset(l,t)]);
			  // println("swap : " << __bit_set(l,t) << "," << __bit_reset(l,t));
		       }
#else
		 std::vector<uint32_t> done(sn, 0);
		 perm_t p = perms(qn,cq1,cq2,tq);
		 
		 uint32_t p1,p2;

		 for (perm_t::iterator it = p.begin(); it != p.end(); it++)
		 {
		    p1 = it->first;
		    p2 = it->second;
		    if (!(done[p1] || done[p2]))
		    //if (!(done[p1]))
		    {
			  // std::swap(amp(p1),amp(p2)); // ublas
			  std::swap(amp[p1],amp[p2]);
			  //println("swap : " << p1 << "," << p2);
			  done[p1] = 1;
			  done[p2] = 1;
		    }
		 }
#endif

		 if ((qreg.get_measurement_prediction(control_qubit_1) == __state_1__) && 
		     (qreg.get_measurement_prediction(control_qubit_2) == __state_1__) )
		 {
		    qreg.flip_binary(target_qubit);
		 }
		 else if ((qreg.get_measurement_prediction(control_qubit_1) == __state_unknown__) ||
		          (qreg.get_measurement_prediction(control_qubit_2) == __state_unknown__)  )
		 {
		    qreg.set_measurement_prediction(target_qubit,__state_unknown__);
		    // qreg.set_binary(target_qubit,__state_unknown__);
		 }
		 
		 return 0;
	   }


	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(control_qubit_1);
		 r.push_back(control_qubit_2);
		 r.push_back(target_qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(control_qubit_1);
		 r.push_back(control_qubit_2);
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(target_qubit);
		 return r;
	   }


	   gate_type_t type()
	   {
	      return __toffoli_gate__;
	   }

	   
	   void dump()
	   {
		 println("  [-] toffoli(ctrl_qubit_1=" << control_qubit_1 << ", ctrl_qubit_2=" << control_qubit_2 << ", target_qubit=" << target_qubit << ")");
	   }

   };


   int fliper(int cs, int ce, int s, uint32_t q, cvector_t * p_amp)
   {
      cvector_t &amp = * p_amp;
      for (int i=cs; i<ce; ++i)
      {
	    if (__bit_test(i,q))
	       std::swap(amp[i],amp[__bit_flip(i,q)]);
      }
      return 0;
   }

   #define __swap_xmm(x,y) { x = _mm_xor_pd(x,y); y = _mm_xor_pd(y,x); x = _mm_xor_pd(x,y); }

   void fast_flip(uint32_t q, uint32_t n, cvector_t& amp)
   {
      complex_t * x = amp.data();
#ifdef USE_OPENMP
#pragma omp parallel for 
#endif 
      for (size_t i=0; i<(1 << n); i+=(1 << (q+1)))
	 for (size_t j=i; j<(i+(1 << q)); j++) 
	    //__swap_xmm(x[j].xmm,x[__bit_flip(j,q)].xmm);
	    // std::swap(x[j].xmm,x[__bit_flip(j,q)].xmm);
	    std::swap(x[j],x[__bit_flip(j,q)]);
   }


   void flip(uint32_t q, uint32_t n, cvector_t& amp)
   {
      uint32_t nn = (1 << n);
      uint32_t p1, p2;
      std::bitset<MAX_QB_N> b;
      // perm_t res;

      b.reset();
      b.set(q);  

      uint32_t bc = b.to_ulong();

      while (bc < nn)
      {
	 b.set(q);  p1 = b.to_ulong();
	 b.flip(q); p2 = b.to_ulong();
	 if (p2<p1)
	    std::swap(amp[p1],amp[p2]);
	 b.flip(q);
	 b = inc(b);
	 b.set(q);
	 bc =  b.to_ulong();
      }
      //return res;
   }



   
   /**
    * \brief  pauli-x :
    *
    *    | 0  1 |
    *    | 1  0 | 
    *
    */
   class pauli_x : public gate
   {
	 private:

	   uint32_t                  qubit;
	   cmatrix_t  m;

	 public:

	   pauli_x(uint32_t q) : qubit(q)
	   {
		 m = build_matrix(pauli_x_c,2);
	   }

	   int32_t apply(qu_register& qreg)
	   {
// #define FAST_FLIP
#ifdef FAST_FLIP

	      uint32_t qn = qreg.size();
	      cvector_t& amp = qreg.get_data();
	      
	      // flip(qubit,qn,amp);
	      fast_flip(qubit,qn,amp);
	      /* 
	       xpu::task flip_t(fliper,0,0,0,qubit,&amp);
	       xpu::parallel_for parallel_flip(0,(1 << qn),1,&flip_t);
	       parallel_flip.run();
	      */
#else
	      uint32_t     n  = qreg.size();
	      complex_t *  s  = qreg.get_data().data();
	      // cm.dump();
	      __apply_m(0, (1 << n), qubit, s, 0, (1 << qubit), m.m);
	      // sqg_apply(m,qubit,qreg);
#endif // FAST_FLIP

		 qreg.flip_binary(qubit);
		 return 0;
	   }
	   
	   std::string  micro_code()
	   {
	      /**
	    	| wait 5
		| x180 q0 --> { pulse 9,0,0 }
		*/
	      if (qubit > 2) return "# unsupported operation : qubit out of range";
	      std::stringstream uc;
	      uc << pulse_lt[qubit][__x180__] << "\n";
	      uc << "  wait 4 \n";
	      return uc.str();
	   }


	   void dump()
	   {
		 println("  [-] pauli-x(qubit=" << qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __pauli_x_gate__; 
	   }


   };

   /**
    * \brief  pauli-y :
    *
    *    | 0 -i |
    *    | i  0 | 
    */
   class pauli_y : public gate
   {
	 private:

	   uint32_t                  qubit;
	   cmatrix_t  m;

	 public:

	   pauli_y(uint32_t qubit) : qubit(qubit)
	   {
		 m = build_matrix(pauli_y_c,2);
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 sqg_apply(m,qubit,qreg);
		 qreg.flip_binary(qubit);
		 return 0;
	   }
	   
	   std::string  micro_code()
	   {
	      /**
	    	| wait 5
		| x180 q0 --> { pulse 9,0,0 }
		*/
	      if (qubit > 2) return "# unsupported operation : qubit out of range";
	      std::stringstream uc;
	      uc << pulse_lt[qubit][__y180__] << "\n";
	      uc << "  wait 4 \n";
	      return uc.str();
	   }



	   void dump()
	   {
		 println("  [-] pauli-y(qubit=" << qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __pauli_y_gate__; 
	   }



   };

   /**
    * \brief  pauli-z :
    *
    *    | 1  0 |
    *    | 0 -1 | 
    */
   class pauli_z : public gate
   {
	 private:

	   uint32_t                  qubit;
	   cmatrix_t  m;

	 public:

	   pauli_z(uint32_t qubit) : qubit(qubit)
	   {
		 m = build_matrix(pauli_z_c,2);
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 sqg_apply(m,qubit,qreg);
		 return 0;
	   }
	   
	   std::string  micro_code()
	   {
	      /**
	    	| wait 5
		| x180 q0 --> { pulse 9,0,0 }
		*/
	      if (qubit > 2) return "# unsupported operation : qubit out of range";
	      std::stringstream uc;
	      uc << pulse_lt[qubit][__y180__] << "\n";
	      uc << "  wait 4 \n";
	      uc << pulse_lt[qubit][__x180__] << "\n";
	      uc << "  wait 4 \n";
	      return uc.str();
	   }



	   void dump()
	   {
		 println("  [-] pauli-z(qubit=" << qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __pauli_z_gate__; 
	   }
   };

   /**
    * \brief  phase :
    *     
    *    | 1  0 |
    *    | 0  i |
    */ 
   class phase_shift : public gate
   {
	 private:

	   uint32_t                  qubit;
	   cmatrix_t  m;

	 public:

	   phase_shift(uint32_t qubit) : qubit(qubit)
	   {
		 m = build_matrix(phase_c,2);
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 sqg_apply(m,qubit,qreg);
		 return 0;
	   }
	   
	   std::string  micro_code()
	   {
	      if (qubit > 2) return "# unsupported operation : qubit out of range";
	      std::stringstream uc;
	      uc << pulse_lt[qubit][__y90__] << "\n";
	      uc << "  wait 4 \n";
	      uc << pulse_lt[qubit][__x90__] << "\n";
	      uc << "  wait 4 \n";
	      uc << pulse_lt[qubit][__ym90__] << "\n";
	      uc << "  wait 4 \n";
	      return uc.str();
	   }

	   void dump()
	   {
		 println("  [-] phase(qubit=" << qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __phase_gate__; 
	   }
   };


   /**
    * \brief T gate
    */
   class t_gate : public gate
   {
	 private:

	   uint32_t   qubit;
	   cmatrix_t  m;

	 public:

	   t_gate(uint32_t qubit) : qubit(qubit)
	   {
		 m = build_matrix(t_gate_c,2);
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 sqg_apply(m,qubit,qreg);
		 return 0;
	   }

	   void dump()
	   {
		 println("  [-] t_gate(qubit=" << qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }


	   gate_type_t type()
	   {
	      return __t_gate__; 
	   }

   };


   /**
    * \brief T dag gate
    */
   class t_dag_gate : public gate
   {
	 private:

	   uint32_t   qubit;
	   cmatrix_t  m;

	 public:

	   t_dag_gate(uint32_t qubit) : qubit(qubit)
	   {
		 m = build_matrix(tdag_gate_c,2);
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 sqg_apply(m,qubit,qreg);
		 return 0;
	   }

	   void dump()
	   {
		 println("  [-] t_dag_gate(qubit=" << qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __tdag_gate__;
	   }

   };






   /**
    * \brief  rotation-x :
    */ 
   class rx : public gate
   {
	 private:

	   uint32_t   qubit;
	   double     angle;
	   cmatrix_t  m;

	 public:

	   rx(uint32_t qubit, double angle) : qubit(qubit), angle(angle)
	   {
		 // m.resize(2,2);
		 m(0,0) = cos(angle/2);      m(0,1) = complex_t(0,-sin(angle/2));
		 m(1,0) = complex_t(0,-sin(angle/2)); m(1,1) = cos(angle/2);
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 sqg_apply(m,qubit,qreg);
		 qreg.set_measurement_prediction(qubit,__state_unknown__);
		 // qreg.set_binary(qubit,__state_unknown__);
		 return 0;
	   }

	   void dump()
	   {
		 println("  [-] rx(qubit=" << qubit << ", angle=" << angle << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __rx_gate__; 
	   }

   };


   /**
    * \brief  rotation-y :
    */ 
   class ry : public gate
   {
	 private:

	   uint32_t   qubit;
	   double     angle;
	   cmatrix_t  m;

	 public:

	   ry(uint32_t qubit, double angle) : qubit(qubit), angle(angle)
	   {
		 // m.resize(2,2);
		 m(0,0) = cos(angle/2); m(0,1) = -sin(angle/2);
		 m(1,0) = sin(angle/2); m(1,1) = cos(angle/2);
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 sqg_apply(m,qubit,qreg);
		 qreg.set_measurement_prediction(qubit,__state_unknown__);
		 //qreg.set_binary(qubit,__state_unknown__);
		 return 0;
	   }

	   void dump()
	   {
		 println("  [-] ry(qubit=" << qubit << ", angle=" << angle << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }


	   gate_type_t type()
	   {
	      return __ry_gate__;
	   }
   };


   /**
    * \brief  rotation-z :
    */ 
   class rz : public gate
   {
	 private:

	   uint32_t   qubit;
	   double     angle;
	   cmatrix_t  m;

	 public:

	   rz(uint32_t qubit, double angle) : qubit(qubit), angle(angle)
	   {
		 // m.resize(2,2);
		 m(0,0) = complex_t(cos(-angle/2), sin(-angle/2));   m(0,1) = 0;
		 m(1,0) = 0;  m(1,1) =  complex_t(cos(angle/2), sin(angle/2));
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 sqg_apply(m,qubit,qreg);
		 qreg.set_measurement_prediction(qubit,__state_unknown__);
		 //qreg.set_binary(qubit,__state_unknown__);
		 return 0;
	   }

	   void dump()
	   {
		 println("  [-] rz(qubit=" << qubit << ", angle=" << angle << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __rz_gate__; 
	   }
   };

   void __shift(cvector_t& amp, size_t size, size_t bit, complex_t p, size_t offset=0)
   {
      // println("bit=" << bit);
      // println("ctrl=" << ctrl);
      complex_t * x = amp.data();
      for (size_t i=__bit_set(0,bit); i<(1<<size); i += (1 << (bit+1)))
	 for (size_t j=0; j<(1<<bit); j++)
	 {
	    size_t v = i+j+offset; 
	    // amp[v] *= p;
	    x[v] *= p;
	    // println(" swap(" << std::bitset<16>(v) << "," << std::bitset<16>(__bit_reset(v,trg)) << ")");
	 }
   }

   void __shift(complex_t * x, size_t size, size_t bit, complex_t p, size_t offset=0)
   {
      // println("bit=" << bit);
      // println("ctrl=" << ctrl);
      for (size_t i=__bit_set(0,bit); i<(1<<size); i += (1 << (bit+1)))
	 for (size_t j=0; j<(1<<bit); j++)
	 {
	    size_t v = i+j+offset; 
	    // amp[v] *= p;
	    x[v] *= p;
	    // println(" swap(" << std::bitset<16>(v) << "," << std::bitset<16>(__bit_reset(v,trg)) << ")");
	 }
   }




   int shift_worker(int cs, int ce, int s, cvector_t * p_amp, size_t bit1, size_t bit2, complex_t p)
   {
      cvector_t &amp = * p_amp;
      // xpu::parallel_for fswp(__bit_set(0,b1), (1 << qn), (1 << (b1+1)), &t);
      size_t step=(1 << (bit1+1));
      size_t b   = cs;
      size_t e   = ce;

      size_t offset = __bit_set(0,bit1);

      //for (size_t i=__bit_set(0,bit1); i<(1<<size); i += (1 << (bit1+1)))
	 //__swap(amp,bit1,bit2,trg,ctrl,i);
      for (size_t i=b; i<e; i++)
	 __shift(amp,bit1,bit2,p,offset+(i*step));
      return 0;
   }


   /**
    * \brief qft 
    */ 
   class qft : public gate
   {
	 private:

	   std::vector<uint32_t>     qubit;
	   cmatrix_t                 hm;

	 public:

	   qft(std::vector<uint32_t> qubit) : qubit(qubit)
	   {
		 hm = build_matrix(hadamard_c,2);
	   }

	   int32_t apply(qu_register& qreg)
	   { 
	   }

	   void dump()
	   {
		 print("  [-] qft(");
		 for (size_t i=0; i<(qubit.size()-1); ++i)
		    print("q" << qubit[i] << ","); 
		 println("q" << qubit[qubit.size()-1] << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 return qubit;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 return qubit;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 return qubit;
	   }

	   gate_type_t type()
	   {
	      return __qft_gate__; 
	   }
   };


   /**
    * \brief  controlled phase shift by (pi/(2^(ctrl-target)))
    *     
    */ 
   class ctrl_phase_shift : public gate
   {
	 private:

	   uint32_t                  ctrl_qubit;
	   uint32_t                  target_qubit;
	   complex_t                 z;

	 public:

	   ctrl_phase_shift(uint32_t ctrl_qubit, uint32_t target_qubit) : ctrl_qubit(ctrl_qubit), 
	                                                                  target_qubit(target_qubit), 
		      						          z(cos(M_PI/(1 << (ctrl_qubit - target_qubit))), 
								            sin(M_PI/(1 << (ctrl_qubit - target_qubit))))
	   {
	   }

	   int32_t apply(qu_register& qreg)
	   {
#if 0
	      uint32_t nn = (1 << qreg.size());
	      uint32_t p = 0;
	      std::bitset<MAX_QB_N> b;

	      b.reset();
	      b.set(ctrl_qubit);  
	      b.set(target_qubit);  

	      uint32_t   bc    = b.to_ulong();
	      cvector_t& state = qreg.get_data();

	      while (bc < nn)
	      {
		 b.set(ctrl_qubit);  
		 b.set(target_qubit);  
		 p = b.to_ulong();
		 state[p] *= z;
		 b = inc(b);
		 bc = b.to_ulong();
	      }
#endif
	      size_t b1 = std::max(ctrl_qubit,target_qubit);
	      size_t b2 = std::min(ctrl_qubit,target_qubit);
	      size_t qn = qreg.size();
	      size_t steps = ((1 << qn)-(__bit_set(0,b1)))/(1 << (b1+1))+1;
	      cvector_t& amp = qreg.get_data();

              #pragma omp parallel for
	      for (size_t i=0; i<steps; ++i)
	      {
		 // shift_worker(i,i+1,1,&amp,b1,b2,z);
		 size_t stride=(1 << (b1+1));
		 size_t offset = __bit_set(0,b1);
		 __shift(amp,b1,b2,z,offset+(i*stride));
	      }

	      return 0;
	   }

	   void dump()
	   {
		 println("  [-] ctrl_phase_shift(ctrl_qubit=" << ctrl_qubit << ", target_qubit: " << target_qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(ctrl_qubit);
		 r.push_back(target_qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(ctrl_qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(target_qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __ctrl_phase_shift_gate__; 
	   }
   };



   /**
    * \brief  swap :
    *
    *    | 1  0  0  0 | 
    *    | 0  0  1  0 |
    *    | 0  1  0  0 |
    *    | 0  0  0  1 |  
    */
   class swap : public gate
   {
	 private:
	   
	   uint32_t qubit1;
	   uint32_t qubit2;
	   
	   // cmatrix_t m;

	 public:

	   swap(uint32_t qubit1, uint32_t qubit2) : qubit1(qubit1), qubit2(qubit2)
	   {
		 // m =  build_matrix(swap_c,4);
	   }

	   int32_t apply(qu_register& qreg)
	   {
	      cnot(qubit1,qubit2).apply(qreg);
	      cnot(qubit2,qubit1).apply(qreg);
	      cnot(qubit1,qubit2).apply(qreg);
	      return 0;
	   }


	   void    dump()
	   {
		 println("  [-] swap(q1=" << qubit1 << ", q2=" << qubit2 << ")");
	   }

	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit1);
		 r.push_back(qubit2);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit1);
		 r.push_back(qubit2);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __swap_gate__; 
	   }

   };
   
   
   /**
    * \brief cphase
    */
   class cphase : public gate
   {
	 private:
	   
	   uint32_t ctrl_qubit;
	   uint32_t target_qubit;
	   
	 public:

	   cphase(uint32_t ctrl_qubit, uint32_t target_qubit) : ctrl_qubit(ctrl_qubit), target_qubit(target_qubit)
	   {
	   }

	   int32_t apply(qu_register& qreg)
	   {
	      hadamard(target_qubit).apply(qreg);
	      cnot(ctrl_qubit,target_qubit).apply(qreg);
	      hadamard(target_qubit).apply(qreg);
	      return 0;
	   }

	   void dump()
	   {
		 println("  [-] cphase(ctrl_qubit=" << ctrl_qubit << ", target_qubit=" << target_qubit << ")");
	   }

	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(ctrl_qubit);
		 r.push_back(target_qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(ctrl_qubit);
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(target_qubit);
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __cphase_gate__; 
	   }

   };

   /**
    * \brief  custom matrix gate
    *     
    */
   class custom : public gate
   {
	 private:

// #ifdef __BUILTIN_LINALG__
           std::vector<uint32_t> qubits;
// #else
// 	   ublas::vector<uint32_t>  qubits;
// #endif 
	   // cmatrix_t m;
	   qx::linalg::matrix<complex_t> m;

	 public:

// #ifdef __BUILTIN_LINALG__
	   custom(std::vector<uint32_t>  qubits, qx::linalg::matrix<complex_t> m) : qubits(qubits), m(m)
// #else
// 	   custom(ublas::vector<uint32_t>  qubits, cmatrix_t m) : qubits(qubits), m(m)
// #endif 
	   {
/*
		 uint32_t size = 1 << qubits.size();
		 if (size != m.size1() || size != m.size2())
		    println("[x] error: cutom gate : the matrix size do not match the number of qubits !");
		 // verify also that the matrix is unitary
// #ifdef __BUILTIN_LINALG__
		 // cmatrix_t ctr(m.size2(),m.size1());
		 qx::linalg::matrix<complex_t> ctr(m.size2(),m.size1());
		 for (uint32_t i=0; i<m.size2(); ++i)
		    for (uint32_t j=0; j<m.size1(); ++j)
		       ctr(i,j) = m(j,i).conj();
		 // cmatrix_t mxctr = mxm(m,ctr);
		 qx::linalg::matrix<complex_t> mxctr = mxm(m,ctr);
		 qx::linalg::identity_matrix<complex_t> id(m.size1());
// #else
// 		 cmatrix_t mxctr = mxm(m,ublas::trans(conj(m)));
// 		 ublas::identity_matrix<complex_t> id(m.size1());
// #endif

// #ifdef __BUILTIN_LINALG__
		 if (qx::linalg::equals(mxctr,id))
// #else
// 		 if (equals(mxctr,id))
// #endif
		    println("[x] error: custom gate : the specified matrix is not unitary !");
*/
	   }

	   int32_t apply(qu_register& qreg)
	   {
	      println("[x] custom matrix not supported !");
	      return 0;
	   }

	   void    dump()
	   {
		 println("  [-] custom matrix. ");
		 // println("  [-] custom(qubits=" << qubits << ", matrix=" << m << ")");
	   }

	   gate_type_t type()
	   {
	      return __custom_gate__; 
	   }
   };
  
   

   int p1_worker(int cs, int ce, int s, double * p1, uint32_t qubit, cvector_t * p_data)
   {
      cvector_t &data = * p_data;
      double local_p1 = 0;
      for (uint32_t i=cs; i<ce; ++i)
      {
	 i = __bit_set(i,qubit);
	 if (i<ce)
	    // local_p1 += data[i].norm(); //std::norm(data[i]);
	    local_p1 += std::norm(data[i]);
	 // if (__bit_test(i,qubit))
	    // local_p1 += std::norm(data[i]);
      }

      // println("l_p1 [" << cs << ".." << ce << "]: " << local_p1);
      *p1 += local_p1;
      return 0;
   }


   int zero_worker(int cs, int ce, int s, int m, double * length, uint32_t qubit, cvector_t * p_data)
   {
      cvector_t &data = * p_data;
      double local_length = 0;
      uint32_t       size = data.size(); 
      if (m)
      {
	 for (uint32_t i=cs; i<ce; ++i)
	 {
	    if (!__bit_test(i,qubit))
	       data[i] = 0;
	    local_length += std::norm(data[i]);
	    // local_length += data[i].norm(); //std::norm(data[i]);
	 }
      }
      else
      {
	 for (uint32_t i=cs; i<ce; ++i)
	 {
	    if (__bit_test(i,qubit))
	       data[i] = 0;
	    local_length += std::norm(data[i]);
	    // local_length += data[i].norm(); //std::norm(data[i]);
	 }
      }
      *length += local_length;
      return 0;
   }

   int renorm_worker(int cs, int ce, int s, double * length, cvector_t * p_data)
   {
      cvector_t &data = * p_data;
      double l = *length;
#ifdef __AVX__
      // println("avx");
      complex_t * vd = p_data->data();
      __m256d vl = _mm256_set1_pd(l);
      for (uint32_t i=cs; i<ce; i+=2)
      {
	 double * pvd = (double*)&vd[i];
	 __m256d va = _mm256_load_pd(pvd); 
	 __m256d vr = _mm256_div_pd(va, vl);
	 _mm256_store_pd(pvd,vr);
      }
#elif defined(__SSE__)
      // println("sse");
      complex_t * vd = p_data->data();
      __m128d vl = _mm_set1_pd(l);
      for (uint32_t i=cs; i<ce; ++i)
      {
	 double * pvd = (double*)&vd[i];
	 __m128d va = _mm_load_pd(pvd); 
	 __m128d vr = _mm_div_pd(va, vl);
	 _mm_store_pd(pvd,vr);
      }
#else
      for (uint32_t i=cs; i<ce; ++i)
	 data[i] /= l;
#endif // __SSE__
      return 0;
   }



   
   /**
    * measure
    */
   class measure : public gate
   {
	 private:

	   uint32_t  qubit;
	   bool      measure_all;
	   bool      disable_averaging;

	 public:

	   measure(uint32_t qubit, bool disable_averaging=false) : qubit(qubit), measure_all(false), disable_averaging(disable_averaging)
	   {
	   }
	   
	   measure() : qubit(0), measure_all(true)
	   {
	   }

	   int32_t apply(qu_register& qreg)
	   {
	      if (measure_all)
	      {
		 qreg.measure();
		 return 0;
	      }

	      double f = qreg.rand();
	      double p = 0;
	      int32_t value;
	      int size = qreg.size(); 
	      int n = (1 << size);
	      cvector_t& data = qreg.get_data();
	      double length = 0;
	      		 //#else
		 
		 int32_t k, l, m;
		 int32_t j = qubit;
		 double  fvalue;

		 std::bitset<MAX_QB_N> b;
		 b.reset();
		 b.set(qubit);
		 uint32_t bc = b.to_ulong();

		 while (bc < n)
		 {
		    bc =  b.to_ulong();
		    p += std::norm(data[bc]);
		    //p += data[bc].norm();
		    b = inc(b);
		    b.set(qubit);  
		    bc =  b.to_ulong();
		 }

		 if (f<p) value = 1;
		 else value = 0;

		 if (value)   // 1
		 {  // reset all states where the qubit is 0
		    for (uint32_t i=0; i<(1 << size); ++i)
		    {
		       if (!__bit_test(i,qubit))
			  data[i] = 0;
		    }
		 }
		 else
		 {
		    for (uint32_t i=0; i<(1 << size); ++i)
		    {
		       if (__bit_test(i,qubit))
			  data[i] = 0;
		    }
		 }

		 for (uint32_t k = 0; k < (1 << size); k++) 
		    length += std::norm(data[k]);
		    // length += data[k].norm(); //std::norm(data[k]);

		 length = std::sqrt(length);

		 for (uint32_t k = 0; k < (1 << size); k++) 
		    data[k] /= length;

		 // #endif // PARALLEL_MEASUREMENT

	      // println("  [>] measured value : " << value);

	      qreg.set_measurement_prediction(qubit,(value == 1 ? __state_1__ : __state_0__));
	      qreg.set_measurement(qubit,(value == 1 ? true : false));
	      //qreg.set_binary(qubit,(value == 1 ? __state_1__ : __state_0__));

	      if (!disable_averaging)
	      {
		 if (qreg.measurement_averaging_enabled)
		 {
		    if (value == 1)
		    {
		       // println("> exited_states++");
		       qreg.measurement_averaging[qubit].exited_states++;
		    }
		    else
		    {
		       // println("> ground_states++");
		       qreg.measurement_averaging[qubit].ground_states++;
		    }
		 }
	      }

	      return value;
	   }

	   void dump()
	   {
	      if (measure_all)
		 println("  [-] measure(register)");
	      else
		 println("  [-] measure(qubit=" << qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 if (!measure_all)
		    r.push_back(qubit);
		 else   // this is a dirty hack, itshould be fixed later (unknown qubit number !)
		 {
		    for (int32_t i=0; i<MAX_QB_N; ++i)
		       r.push_back(i);
		 }
		 return r;
	   }

	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 return qubits();
	   }

	   gate_type_t type()
	   {
	      if (measure_all)
		 return __measure_reg_gate__;
	      else
	         return __measure_gate__; 
	   }
   };


   /**
    * \brief   generic binary controlled gate
    */
   class bin_ctrl : public gate
   {
	 private:

	   uint32_t bit;
	   gate *   g;

	 public:

	   bin_ctrl(uint32_t bit, gate * g) : bit(bit), g(g)
	   {
	   }

	   int32_t apply(qu_register& qreg)
	   {
	      if (qreg.test(bit))
		 g->apply(qreg);
	      return 0;
	   }

	   gate * get_gate()
	   {
	      return g;
	   }

	   uint32_t get_bit()
	   {
	      return bit;
	   }

	   void dump()
	   {
		 print("  [-] bin_ctrl: \n bit=" << bit << " -> ");
		 g->dump();
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 return g->qubits();
	   }

	   std::vector<uint32_t>  control_qubits()
	   {
		 return g->control_qubits();
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 return g->target_qubits();
	   }

	   gate_type_t type()
	   {
	      return __bin_ctrl_gate__;
	   }


   };

   #define bin_ctrl_pauli_x(b,q) bin_ctrl(b,new pauli_x(q))
   #define bin_ctrl_pauli_y(b,q) bin_ctrl(b,new pauli_y(q))
   #define bin_ctrl_pauli_z(b,q) bin_ctrl(b,new pauli_z(q))



   /**
    * \brief classical binary not gate 
    */
   class classical_not : public gate
   {
	 private:

	   uint32_t bit;

	 public:

	   classical_not(uint32_t bit) : bit(bit)
	   {
	   }

	   int32_t apply(qu_register& qreg)
	   {
	      qreg.flip_measurement(bit);
	      return 0;
	   }

	   uint32_t get_bit()
	   {
	      return bit;
	   }

	   void dump()
	   {
		 // println("  [-] classical not gate: \n bit=" << bit);
		 println(" not " << bit);
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
	      return std::vector<uint32_t>();
	   }

	   std::vector<uint32_t>  control_qubits()
	   {
	      return std::vector<uint32_t>();
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
	      return std::vector<uint32_t>();
	   }

	   gate_type_t type()
	   {
	      return __classical_not_gate__;
	   }
   };






   /**
    * prepz  
    */
   class prepz : public gate
   {
	 private:

	   uint32_t  qubit;

	 public:

	   prepz(uint32_t qubit) : qubit(qubit)
	   {
	   }
	   
	   int32_t apply(qu_register& qreg)
	   {
	      measure(qubit,true).apply(qreg);
	      bin_ctrl_pauli_x(qubit,qubit).apply(qreg);
	      // bin_ctrl_pauli_z(qubit,qubit).apply(qreg);
	      qreg.set_measurement(qubit,false);
	      return 0; 
	   }

	   void dump()
	   {
	       println("  [-] prepz(qubit=" << qubit << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 r.push_back(qubit);
		 return r;
	   }

	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 return qubits();
	   }

	   gate_type_t type()
	   {
	      return __prepz_gate__;      
	   }

   };





   class lookup_gate_table : public gate 
   {
      private:

        std::vector<uint32_t>      ctrl_bits;
	std::map<uint32_t,gate *>  gates;   

      public:

        lookup_gate_table(uint32_t b0)
	{
	   ctrl_bits.push_back(b0);
	}


        lookup_gate_table(uint32_t b0, uint32_t b1)
	{
	   ctrl_bits.push_back(b0);
	   ctrl_bits.push_back(b1);
	}

        
	lookup_gate_table(uint32_t b0, uint32_t b1, uint32_t b2)
	{
	   ctrl_bits.push_back(b0);
	   ctrl_bits.push_back(b1);
	   ctrl_bits.push_back(b2);
	}


	lookup_gate_table(std::vector<uint32_t> ctrl_bits) : ctrl_bits(ctrl_bits)
	{
	}

	
	void add_gate(uint32_t cond, gate * g)
	{
	   assert(cond < (1<< ctrl_bits.size()));
	   gates[cond] = g;
	}

	
	int32_t apply(qu_register& qreg)
	{
	   uint32_t k = 0;

	   for (uint32_t i=0; i<ctrl_bits.size(); i++)
	   {
	      //println(qreg.get_binary(i));
	      if (qreg.test(ctrl_bits[i]))
		 k = k * 2 + 1;
	      else
		 k *= 2;
	   }

	   // println("[+] lookup table : cond = " << k);

	   std::map<uint32_t,gate*>::iterator it = gates.find(k);

	   if (it != gates.end())
	      (*it).second->apply(qreg);

	    return 0;
	}

	std::vector<uint32_t>  qubits()
	{
	   std::vector<uint32_t> r;
	   // to do
	   std::map<uint32_t,gate *>::iterator  ig;
	   for (ig=gates.begin(); ig!=gates.end(); ++ig)
	   {
	      std::vector<uint32_t> ri = ig->second->qubits();
	      r.insert(r.begin(), ri.begin(), ri.end());
	   }
	   return r;
	}


	std::vector<uint32_t>  control_qubits()
	{
	   std::vector<uint32_t> r;
	   // to do
	   std::map<uint32_t,gate *>::iterator  ig;
	   for (ig=gates.begin(); ig!=gates.end(); ++ig)
	   {
	      std::vector<uint32_t> ri = ig->second->control_qubits();
	      if (ri.size())
		 r.insert(r.begin(), ri.begin(), ri.end());
	   }
	   return r;
	}

	std::vector<uint32_t>  target_qubits()
	{
	   std::vector<uint32_t> r;
	   // to do
	   std::map<uint32_t,gate *>::iterator  ig;
	   for (ig=gates.begin(); ig!=gates.end(); ++ig)
	   {
	      std::vector<uint32_t> ri = ig->second->target_qubits();
	      if (ri.size())
		 r.insert(r.begin(), ri.begin(), ri.end());
	   }
	   return r;
	}


	void dump()
	{
	   println("  [-] lookup gate table : ");
	}

	gate_type_t type()
	{
	   return __lookup_table__;  
	}
   
   };

   /**
    * \brief display : debug utility
    *    display intermediate quantum states of a 
    *   quantum register whithin a circuit.
    */
   class display : public gate
   {
	 private:
	 
	   bool only_binary;

	 public:

	   display(bool only_binary=false) : only_binary(only_binary)
	   {
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 qreg.dump(only_binary);
		 return 0;
	   }

	   void dump()
	   {
		 println("  [-] display(only_binary=" << only_binary << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }

	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }

	   gate_type_t type()
	   {
	      if (only_binary)
		 return __display_binary__;
	      else
		 return __display__; 
	   }


   };



   /**
    * parallel gates
    */
   class parallel_gates : public gate
   {
	 public:
	   
	   parallel_gates()
	   {
	   }

	   int32_t apply(qu_register& qreg)
	   {
	      for (uint32_t i=0; i<gates.size(); i++)
		 gates[i]->apply(qreg);
	      return 0;
	   }

	   uint32_t add(gate * g)
	   {
		 gates.push_back(g);
		 return gates.size();
	   }

	   std::vector<gate *> get_gates()
	   {
	      return gates;
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 for (uint32_t i=0; i<gates.size(); i++)
		 {
		    std::vector<uint32_t> q = gates[i]->qubits();
		    r.insert(r.end(),q.begin(),q.end());	
		 }
		 return r;
	   }
	   
	   
	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 for (uint32_t i=0; i<gates.size(); i++)
		 {
		    std::vector<uint32_t> q = gates[i]->control_qubits();
		    r.insert(r.end(),q.begin(),q.end());	
		 }
		 return r;
	   }
	   
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 for (uint32_t i=0; i<gates.size(); i++)
		 {
		    std::vector<uint32_t> q = gates[i]->target_qubits();
		    r.insert(r.end(),q.begin(),q.end());	
		 }
		 return r;
	   }


	   void dump()
	   {
		 println("  [-] parallel_gates (" << gates.size() << " gates) : ");
		 for (uint32_t i=0; i<gates.size(); i++)
		    gates[i]->dump();
	   }


	   gate_type_t type()
	   {
	      return __parallel_gate__; 
	   }
	

	 private:

	   std::vector<gate *> gates; // list of the parallel gates

   };


   /**
    * prepare the qubits into an arbitrary quantum state
    */
   class prepare : public gate
   {
	 private:

	   quantum_state_t * state;

	 public:

	   prepare(quantum_state_t * state) : state(state)
	   {
	   }
	  

	   int32_t apply(qu_register& qreg)
	   {
	      qreg.reset();
	      cvector_t&  q = qreg.get_data();
	      double      norm = 0;

	      for (quantum_state_t::iterator i=state->begin(); i != state->end(); ++i)
	      {
		 basis_state_t bs = (*i).first;
		 complex_t     c  = (*i).second;
		 // println("bs=" << bs << ", a=" << c);
		 q[bs] = c;
		 norm += std::norm(c);
		 // norm += c.norm(); //std::norm(c);
	      }
	      
	      if (std::fabs(norm-1) > QUBIT_ERROR_THRESHOLD)
	      {
		 println("[!] warning : the loaded quantum state is not normalized (norm = " << norm << ") !");
		 println("[!] renormalizing the quantum state...");
		 qreg.normalize();
		 println("[!] quantum state renormalized successfully.");
	      }

	      for (size_t qi=0; qi<qreg.size(); ++qi)
	      {
		 qreg.set_measurement_prediction(qi,__state_unknown__);
		 //qreg.set_binary(qi,__state_unknown__);
	      }
	      return 0;
	   }

	   void dump()
	   {
		 println("  [-] prepare (quantum_state=" << state << ")");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
	      std::vector<uint32_t> r;
	      // this is a dirty hack, itshould be fixed later (unknown qubit number !)
	      for (int32_t i=0; i<MAX_QB_N; ++i)
		 r.push_back(i);
	      return r;
	   }

	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 return qubits();
	   }

	   gate_type_t type()
	   {
	      return __prepare_gate__; 
	   }
   };

   /**
    * \brief print  : debug utility
    *     print arbitrary string
    */
   class print_str : public gate
   {
	 private:
	 
	   std::string str; 

	 public:

	   print_str(std::string& s) : str(s)
	   {
	   }

	   int32_t apply(qu_register& qreg)
	   {
		 println(str);
		 return 0;
	   }

	   void dump()
	   {
		 println(" print " << str << "\"");
	   }
	   
	   std::vector<uint32_t>  qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }

	   std::vector<uint32_t>  control_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }
 
	   std::vector<uint32_t>  target_qubits()
	   {
		 std::vector<uint32_t> r;
		 return r;
	   }

	   gate_type_t type()
	   {
	      return __print_str__;   
	   }

   };





}

#endif // QX_GATE_H

