#ifndef PROJECT_OGANESSON_NBODY_H
#define PROJECT_OGANESSON_NBODY_H

#include "fix16.h"

#define N 2
#define G F16C(6, 67408)
#define CONSTRAINT_DISTANCE F16C(2, 0)


typedef fix16_t vector_t[N];

typedef struct {
  uint32_t n;

  uint32_t chunk_size;
  uint32_t chunk_particle_start;
  uint32_t chunk_particle_end;

  vector_t *s;
  vector_t *v;
  vector_t *f;
  vector_t *f1;

//  bool *fixed_s;
//  bool *fixed_v;

  fix16_t *m;
  fix16_t dt;

} environment_t;

void engine_environment_init(environment_t *env,
                             uint32_t n, uint32_t chunk_number,
                             uint32_t chunk_id,
                             fix16_t dt);

void engine_particle_init(environment_t *env, uint32_t particle);
void engine_particle_center(environment_t *env, uint32_t particle);

void engine_compute_force_particle(environment_t *env, uint32_t particle);
void engine_update_position_particle(environment_t *env, uint32_t particle);
void engine_update_velocity_particle(environment_t *env, uint32_t particle);

fix16_t engine_distance(vector_t p);
void engine_swap_f(environment_t *env);


#endif //PROJECT_OGANESSON_NBODY_H
