#include "polytope.h"

Body_T Polytope_T = {
	.print = Polytope_print,
        .free = Polytope_free,
        .inside = Polytope_inside_ref,
        .intersect = Polytope_intersect_ref,
        .intersectCoord = Polytope_intersectCoord_ref,
	.cacheAlloc = Polytope_cacheAlloc_ref,
	.cacheReset = Polytope_cacheReset_ref,
	.cacheUpdateCoord = Polytope_cacheUpdateCoord_ref,
        .shallowCutOracle = Polytope_shallowCutOracle_ref,
	.transform = Polytope_transform_ref,
        .boundingSphere = Polytope_bounding_ref
};

Polytope* Polytope_new(int n, int m) {
   Polytope* o = (Polytope*) malloc(sizeof(Polytope));
   o->n = n;
   o->m = m;
   o->line = ceil_cache(n,sizeof(FT)); // make sure next is also 32 alligned
   int line_m = ceil_cache(m,sizeof(FT));
   int size_A = o->line*m;
   o->A = (FT*)(aligned_alloc(32, (size_A+line_m)*sizeof(FT))); // align this to 32
   o->b = o->A + o->line*m;
   for(int i=0;i<size_A+line_m;i++) {o->A[i]=0;}
   return o;
}


void Polytope_free(const void* o) {
   Polytope* p = (Polytope*)o;
   free(p->A);
   free(p);
}


void Polytope_print(const void* o) {
   const Polytope* p = (Polytope*)o;
   printf("Polytope: n=%d, m=%d\n",p->n,p->m);
   for(int i=0; i<p->m; i++) {
      for(int j=0; j<p->n; j++) {
         printf(" %.3f",Polytope_get_a(p,i,j));
      }
      printf(" | %.3f\n",Polytope_get_b(p,i));
   }
}

bool Polytope_inside_ref(const void* o, const FT* v) {
   const Polytope* p = (Polytope*)o;
   for(int i=0; i<p->n; i++) {
      FT sum = 0;
      for(int x=0; x<p->n; x++) { sum+= v[x] * Polytope_get_a(p, i, x);}
      if(sum > Polytope_get_b(p, i)) {return false;}
   }
   return true; // passed all inequalities
}


void Polytope_intersect_ref(const void* o, const FT* x, const FT* d, FT* t0, FT* t1) {
   const Polytope* p = (Polytope*)o;
   const int n = p->n;
   const int m = p->m;
   
   FT t00 = -FT_MAX;// tmp variables for t0, t1
   FT t11 = FT_MAX;

   for(int i=0; i<m; i++) {
      // check orientation of plane vs direction of line:
      //   if orthogonal (d*ai = 0), then no intersection
      //   if <0, then same direction -> t0
      //   if >0, then opp  direction -> t1
      const FT* ai = Polytope_get_Ai(p,i);
      const FT b = Polytope_get_b(p, i);
      const FT dai = dotProduct(d,ai,n);
      // Note: if base-vector: could just pick i'th entry!
      
      //printf("dai: %f %f\n",dai,FT_EPS);

      if(dai <= FT_EPS && -dai <= FT_EPS) {continue;} // orthogonal

      // find intersections y of line with all planes:
      //   y = x + d*t
      //   ai*y = b
      //   
      //   t = (b - ai*x)/(d*ai)
      
      FT t = (b - dotProduct(ai,x,n)) / dai;
      //printf("t: %f\n",t);
      
      if(dai < 0.0) {
         t00 = (t00>t)?t00:t; // max
      } else {
         t11 = (t11<t)?t11:t; // min
      }
   }
   
   // return:
   *t0 = t00;
   *t1 = t11;
}

void Polytope_intersectCoord_ref(const void* o, const FT* x, const int d, FT* t0, FT* t1, void* cache) {
   const Polytope* p = (Polytope*)o;
   const int n = p->n;
   const int m = p->m;
   FT* Aix = (FT*)cache;
   
   FT t00 = -FT_MAX;// tmp variables for t0, t1
   FT t11 = FT_MAX;

   for(int i=0; i<m; i++) {
      const FT* ai = Polytope_get_Ai(p,i);
      const FT b = Polytope_get_b(p, i);
      const FT dai = ai[d]; // dot product with unit vector dim d
      
      if(dai <= FT_EPS && -dai <= FT_EPS) {continue;} // orthogonal
      
      const FT aix = dotProduct(ai,x,n);
      assert(aix == Aix[i] && "Cache must be accurate!");
      FT t = (b - aix) / dai;
      
      if(dai < 0.0) {
         t00 = (t00>t)?t00:t; // max
      } else {
         t11 = (t11<t)?t11:t; // min
      }
   }
   
   // return:
   *t0 = t00;
   *t1 = t11;
}

void Polytope_intersectCoord_cached_ref(const void* o, const FT* x, const int d, FT* t0, FT* t1, void* cache) {
   const Polytope* p = (Polytope*)o;
   const int n = p->n;
   const int m = p->m;
   FT* Aix = (FT*)cache;
   
   FT t00 = -FT_MAX;// tmp variables for t0, t1
   FT t11 = FT_MAX;

   for(int i=0; i<m; i++) {
      const FT* ai = Polytope_get_Ai(p,i);
      const FT b = Polytope_get_b(p, i);
      const FT dai = ai[d]; // dot product with unit vector dim d
      
      if(dai <= FT_EPS && -dai <= FT_EPS) {continue;} // orthogonal
      
      const FT aix = Aix[i];
      FT t = (b - aix) / dai;
      
      if(dai < 0.0) {
         t00 = (t00>t)?t00:t; // max
      } else {
         t11 = (t11<t)?t11:t; // min
      }
   }
   
   // return:
   *t0 = t00;
   *t1 = t11;
}



int  Polytope_cacheAlloc_ref(const void* o) {
   const Polytope* p = (Polytope*)o;
   // allocate one FT per inequality: store dot product: dot(ai,x)
   return p->m * sizeof(FT);
}
void Polytope_cacheReset_ref(const void* o, const FT* x, void* cache) {
   const Polytope* p = (Polytope*)o;
   FT* c = (FT*)cache;
   const int n = p->n;
   const int m = p->m;
   for(int i=0; i<m; i++) {
      const FT* ai = Polytope_get_Ai(p,i);
      c[i] = dotProduct(ai,x,n);
   }
}

void Polytope_cacheUpdateCoord_ref(const void* o, const int d, const FT dx, void* cache) {
   const Polytope* p = (Polytope*)o;
   const int m = p->m;
   FT* c = (FT*)cache;
   for(int i=0; i<m; i++) {
      c[i] += dx * Polytope_get_a(p,i,d);
   } 
}

bool Polytope_shallowCutOracle_ref(const void* o, const Ellipsoid* e, FT* v, FT* c) {
   const Polytope* p = (Polytope*)o;
   const int n = p->n;
   const int m = p->m;
   
   FT Ax[m];

   // check if center of Ellisoid e = (X,x) is in Polytope:
   // for all i, check if:
   //    Ai * x <= bi
   
   int i0 = prng_get_random_int_in_range(0,m-1);// just an experiment to see if it helps balance things
   for(int ii=0;ii<m;ii++) {
      int i = (ii+i0) % m;
      FT* Ai = Polytope_get_Ai(p,i);
      FT bi = Polytope_get_b(p,i);
      FT* x = e->a;
      Ax[i] = dotProduct(Ai,x,n);
      if(Ax[i] > bi) { // found one -> return (Ai, bi)
         for(int j=0;j<n;j++) {v[j] = Ai[j];}
	 *c = bi;
         return true;
      }
   }
   

   // check if inner Ellipsoid e = ( (2n)^-2 * T.inverse(), x) is in Polytope:
   // for all i, check if:
   //   AiT * T * Ai <= (bi - AiT * x)^2 * (2n)^2
   const FT twon2 = 4.0*n*n;
   int i1 = prng_get_random_int_in_range(0,m-1);//ballance experiment
   for(int ii=0;ii<m;ii++) {
      int i = (ii+i1) % m;
      FT* Ai = Polytope_get_Ai(p,i);
      FT bi = Polytope_get_b(p,i);
      
      FT AitTAi = 0; // could be useful to cache...
      for(int j=0;j<n;j++) {
         FT* Tj = Ellipsoid_get_Ti(e,j);
	 FT TjAi = dotProduct(Tj,Ai,n);
         AitTAi += Ai[j] * TjAi;
      }
      
      FT diff = bi - Ax[i];
      if(AitTAi > diff*diff*twon2) { // found one -> return (Ai, bi)
         for(int j=0;j<n;j++) {v[j] = Ai[j];}
	 *c = bi;
         return true;
      }
   }
   
   // no half-plane violated inner ellipse
   return false;
}

void Polytope_transform_ref(const void* o_in, void* o_out, const Matrix* L, FT* a, FT beta) {
   const Polytope* p_in = (Polytope*)o_in;
   Polytope* p_out = (Polytope*)o_out;
   const int n = p_in->n;
   const int m = p_in->m;
   // computation according to explanation in preprocess_ref
   
   // b' = b - A * a
   // b'' = b' / beta
   FT beta_r = 1.0 / beta; 
   for (int i = 0; i < m; i++){
      FT* Ai = Polytope_get_Ai(p_in,i);
      FT bi = Polytope_get_b(p_in, i);
      FT distance = bi - dotProduct(Ai, a, n);
      Polytope_set_b(p_out, i, beta_r * distance);
   }
   
   // A'' = A' = A * L
   for (int i = 0; i < m; i++){
      for (int j = 0; j < n; j++){
         FT sum = 0;
         for (int k = 0; k < n; k++){
            sum += Polytope_get_a(p_in, i, k) * Matrix_get(L, k, j);
         }
         Polytope_set_a(p_out, i, j, sum);
      }
   }
}

