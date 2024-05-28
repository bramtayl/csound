#include "lpred_public.h"
#include "fftlib.h"
#include "lpred.h"
#include "memalloc.h"

/** autocorrelation
    computes autocorr out-of-place using spectral or
    time-domain methods.
    r - output
    s - input
    size - input size
    buf - FFT buffer (if NULL, time-domain autocorr is used)
    N - FFT size (power-of-two >= size*2-1)
    returns r
*/
MYFLT *csoundAutoCorrelation(CSOUND *csound, MYFLT *r, MYFLT *s, int size,
                             MYFLT *buf, int N){
  if(buf != NULL) {
    int32_t i;
    MYFLT ai,ar;
    memset(buf, 0, sizeof(MYFLT)*N);
    memcpy(buf,s,sizeof(MYFLT)*size);
    csoundRealFFT(csound,buf,N);
    buf[0] *= buf[0];
    buf[1] *= buf[1];
    for(i = 2; i < N; i+=2) {
      ar = buf[i]; ai = buf[i+1];
      buf[i] = ar*ar + ai*ai; buf[i+1] = 0.;
    }
    csoundInverseRealFFT(csound, buf, N);
    memcpy(r,buf,sizeof(MYFLT)*size);
    return r;
  }
  else {
    MYFLT sum;
    int n,m,o;
    for(n=0; n < size; n++) {
      sum = FL(0.0);
      for(m=n,o=0; m < size; m++,o++)
        sum += s[o]*s[m];
      r[n] = sum;
    }
    return r;
  }
}

/** Set up linear prediction memory for
    autocorrelation size N and predictor order M
*/
void *csoundLPsetup(CSOUND *csound, int N, int M) {
  LPCparam *p = mcalloc(csound, sizeof(LPCparam));
  int fn = 0;

  if(N) {
    // allocate LP analysis memory if needed
    N = N < M+1 ? M+1 : N;
    p->r = mcalloc(csound, sizeof(MYFLT)*N);
    p->pk = mcalloc(csound, sizeof(MYFLT)*N);
    p->am = mcalloc(csound, sizeof(MYFLT)*N);
    p->E = mcalloc(csound, sizeof(MYFLT)*(M+1));
    p->k = mcalloc(csound, sizeof(MYFLT)*(M+1));
    p->b = mcalloc(csound, sizeof(MYFLT)*(M+1)*(M+1));
    for(fn=2; fn < N*2-1; fn*=2);
    p->ftbuf = mcalloc(csound, sizeof(MYFLT)*fn);
  }

  // otherwise just allocate coefficient/pole memory
  p->pl = mcalloc(csound, sizeof(MYCMPLX)*(M+1));
  p->cf = mcalloc(csound, sizeof(MYFLT)*(M+1));
  p->tmpmem = mcalloc(csound, sizeof(MYFLT)*(M+1));

  p->N = N;
  p->M = M;
  p->FN = fn;
  p->cps = 0;
  p->rms  = 0;
  return p;
}

/** Free linear prediction memory
 */
void csoundLPfree(CSOUND *csound, void *parm) {
  LPCparam *p = (LPCparam *) parm;
  mfree(csound, p->r);
  mfree(csound, p->b);
  mfree(csound, p->k);
  mfree(csound, p->E);
  mfree(csound, p);
}


/** Linear Prediction function
    output format: M+1 MYFLT array [E,c1,c2,...,cm]
    NB: c0 is always 1
*/
MYFLT *csoundLPred(CSOUND *csound, void *parm, MYFLT *x){

  LPCparam *p = (LPCparam *) parm;
  MYFLT *r = p->r;
  MYFLT *E = p->E;
  MYFLT *b = p->b;
  MYFLT *k = p->k;
  MYFLT s;
  int N = p->N;
  int M = p->M;
  int L = M+1;
  int m,i;

  r = csoundAutoCorrelation(csound,r,x,N,p->ftbuf,p->FN);
  MYFLT ro = r[0];
  p->rms = SQRT(ro/N);
  if (ro > FL(0.0)) {
    /* if signal power > 0 , do linear prediction */
    for(i=0;i<L;i++) r[i] /= ro;
    E[0] = r[0];
    b[M*L] = 1.;
    for(m=1;m<L;m++) {
      s = 0.;
      b[(m-1)*L] = 1.;
      for(i=0;i<m;i++)
        s += b[(m-1)*L+i]*r[m-i];
      k[m] = -(s)/E[m-1];
      b[m*L+m] = k[m];
      for(i=1;i<m;i++)
        b[m*L+i] = b[(m-1)*L+i] + k[m]*b[(m-1)*L+(m-i)];
      E[m] = (1 - k[m]*k[m])*E[m-1];
    }
    /* replace first coeff with squared error E*/
    b[M*L] = E[M];
  }
  /* return E + coeffs */
  return &b[M*L];
}

/** LP coeffs to Cepstrum
    takes an array c of N size
    and an array b of M+1 size with M all-pole coefficients
    and squared error E in place of coefficient 0 [E,c1,...,cM]
    returns N cepstrum coefficients
*/
MYFLT *csoundLPCeps(CSOUND *csound, MYFLT *c, MYFLT *b,
                    int N, int M){
  (void)(csound);
  int n,m;
  MYFLT s;
  c[0] = -LOG(b[0]);
  c[1] = b[1];
  for(n=2;n<N;n++){
    if(n > M)
      c[n] = 0;
    else {
      s = 0.;
      for(m=1;m<n;m++)
        s += (m/n)*c[m]*b[n-m];
      c[n] = b[n] - s;
    }
  }
  for(n=0;n<N;n++) c[n] *= -1;
  return c;
}

/** Cepstrum to LP coeffs
    takes an array c of N size
    and an array b of M+1 size
    returns M lp coefficients and squared error E in place of
    of coefficient 0 [E,c1,...,cM]
*/

MYFLT *csoundCepsLP(CSOUND *csound, MYFLT *b, MYFLT *c,
                    int M, int N){
  (void)(csound);
  (void)(N);
  int n,m;
  MYFLT s;
  b[0]  = 1;
  b[1] = -c[1];
  for(m=2;m<M+1;m++) {
    s = 0.;
    for(n=1;n<m;n++)
      s -= (m-n)*b[n]*c[m-n];
    b[m] = -c[m] + s/m;
  }
  b[0] = EXP(c[0]);
  return b;
}

MYFLT csoundLPrms(CSOUND *csound, void *parm){
  (void)(csound);
  LPCparam *p = (LPCparam *) parm;
  return p->rms;
}