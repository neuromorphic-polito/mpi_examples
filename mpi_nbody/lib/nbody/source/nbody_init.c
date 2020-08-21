#include "sark.h"
#include "nbody.h"


/**
 *
 * @param env
 * @param n
 * @param chunk_number
 * @param chunk_id
 * @param dt
 */
void engine_environment_init(environment_t *env,
                             uint32_t n, uint32_t chunk_number,
                             uint32_t chunk_id,
                             fix16_t dt) {
  int i, d;

  env->dt = dt;

  env->n = n;
  env->chunk_size = env->n / chunk_number;
  env->chunk_particle_start = env->chunk_size * chunk_id;
  env->chunk_particle_end = env->chunk_particle_start + env->chunk_size - 1;

  env->m = sark_alloc(env->n, sizeof(fix16_t));
  env->s = sark_alloc(env->n, sizeof(vector_t));

  env->v =  sark_alloc(env->chunk_size, sizeof(vector_t));
  env->f =  sark_alloc(env->chunk_size, sizeof(vector_t));
  env->f1 = sark_alloc(env->chunk_size, sizeof(vector_t));

  // N
  for(i=0; i < env->n; i++){
    for(d=0; d < N; d++){
      env->s[i][d] = 0;
    }
    env->m[i] = 0;
  }

  // N partial
  for(i=0; i < env->chunk_size; i++){
    for(d=0; d < N; d++){
      env->v[i][d] = 0;
      env->f[i][d] = 0;
      env->f1[i][d] = 0;
    }
  }
  return;
}

/**
 *
 * @param env
 * @param particle
 */
void engine_particle_init(environment_t *env, uint32_t particle) {
  int d;

  for(d=0; d < N; d++){
    env->s[particle][d] = F16C(sark_rand() % 200 - 100, 0);
  }
  env->m[particle] = F16C(sark_rand() % 10, 5);

  return;
}


/**
 *
 * @param env
 * @param particle
 */
void engine_particle_center(environment_t *env, uint32_t particle){
  int d;

  for(d=0; d < N; d++){
    env->s[particle][d] = 0;
  }
  env->m[particle] = F16C(200, 0);

  return;
}

