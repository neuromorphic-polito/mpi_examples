#include "nbody.h"


/**
 *
 * @param p
 * @return
 */
fix16_t engine_distance(vector_t p) {
  int d;
  fix16_t l = 0;

  for (d=0; d<N; ++d){
    l = fix16_sadd(l, fix16_smul(p[d], p[d]));
  }
  l = fix16_sqrt(l);

  return l;
}


/**
 *
 * @param env
 */
void engine_swap_f(environment_t *env){
  vector_t *temp;

  temp = env->f;
  env->f = env->f1;
  env->f1 = temp;
  return;
}