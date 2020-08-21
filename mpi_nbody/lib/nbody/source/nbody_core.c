#include "nbody.h"

/**
 *
 * @param env
 * @param particle
 */
void engine_compute_force_particle(environment_t *env, uint32_t particle){
  int i, d, chunk_particle;
  fix16_t distance, _f;
  vector_t r;

  chunk_particle = particle - env->chunk_particle_start;

  for(d=0; d<N; ++d) {
    env->f1[chunk_particle][d] = 0;
    r[d] = 0;
  }

  for (i=0; i < env->n; ++i){
    if (i == particle)
      continue;

    for(d=0; d<N; ++d){
      r[d] = fix16_ssub(env->s[particle][d], env->s[i][d]);
    }

    distance = engine_distance(r);
    if(distance < CONSTRAINT_DISTANCE) {
      distance = CONSTRAINT_DISTANCE;
    }

    // distance ^ 3
    distance = fix16_smul(distance, distance);
    distance = fix16_smul(distance, distance);

    // - (G * particle_m * i_m)  / (distance ^ 3)
    _f = fix16_smul(env->m[particle], env->m[i]);
    _f = fix16_smul(G, _f);
    _f = fix16_sdiv(_f, distance);
    _f = fix16_smul(F16C(-1, 0), _f);

    for(d=0; d<N; ++d){
      env->f1[chunk_particle][d] = fix16_sadd(
          env->f1[chunk_particle][d], fix16_smul(_f, r[d]));
    }
  }

  return;
}


/**
 *
 * @param env
 * @param particle
 */
void engine_update_position_particle(environment_t *env, uint32_t particle) {
  int d, chunk_particle;
  chunk_particle = particle - env->chunk_particle_start;
  fix16_t param1, param2, t, m, p2_t, x2_m;

//  if(env->fixed_s[particle])
//    return;

  t = env->dt;
  m = env->m[particle];
  p2_t = fix16_smul(t, t);
  x2_m = fix16_smul(m, F16C(2, 0));

  for (d = 0; d < N; d++) {
    param1 = fix16_smul(env->v[chunk_particle][d], t);
    param2 = fix16_smul(env->f[chunk_particle][d], p2_t);
    param2 = fix16_sdiv(param2, x2_m);

    env->s[particle][d] = fix16_sadd(env->s[particle][d], param1);
    env->s[particle][d] = fix16_sadd(env->s[particle][d], param2);
  }
  return;
}


/**
 *
 * @param env
 * @param particle
 */
void engine_update_velocity_particle(environment_t *env, uint32_t particle) {
  int d, chunk_particle;
  chunk_particle = particle - env->chunk_particle_start;
  fix16_t param1, f, t, m, x2_m;

//  if(env->fixed_v[particle])
//    return;

  t = env->dt;
  m = env->m[particle];
  x2_m = fix16_smul(m, F16C(2, 0));

  for (d = 0; d < N; d++) {
    f = fix16_sadd(env->f[chunk_particle][d], env->f1[chunk_particle][d]);
    param1 = fix16_smul(f, t);
    param1 = fix16_sdiv(param1, x2_m);

    env->v[chunk_particle][d] = fix16_sadd(env->v[chunk_particle][d], param1);
  }
  return;
}


