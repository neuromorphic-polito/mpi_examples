#include "mpi.h"
#include "nbody.h"

#define ITERATIONS 10
#define PARTICLES 1920
#define TIMESTEP F16C(0, 5)

void test_memory();
void mpi_main(uint arg1, uint arg2);

void c_main() {
  MPI_Spinn(mpi_main);
}

void mpi_main(uint arg1, uint arg2) {
  environment_t env;
  int mpi_rank, mpi_size;
  uint32_t particle, i, d;
  uint64_t time_start, time_end;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  io_printf(IO_BUF, "[N-BODY] NBody for SpiNNaker - Rank %d \n", mpi_rank);

  engine_environment_init(&env, PARTICLES, mpi_size, mpi_rank, TIMESTEP);

  io_printf(IO_BUF, "[N-BODY] Node: %d/%d\n", mpi_rank, mpi_size - 1);
  io_printf(IO_BUF, "         Particles: %d:%d @ %d\n",
            env.chunk_particle_start, env.chunk_particle_end, env.chunk_size);

  if (mpi_rank == 0) {
    sark_srand(16);
    for (particle = 0; particle < PARTICLES; ++particle) {
      engine_particle_init(&env, particle);
    }
  }

  // Bcast space
  io_printf(IO_BUF, "[N-BODY] Bcast positions\n");
  MPI_Bcast(env.s, PARTICLES * N, MPI_UINT32_T, 0, MPI_COMM_WORLD);

  // Bcast mass
  io_printf(IO_BUF, "[N-BODY] Bcast mass\n");
  MPI_Bcast(env.m, PARTICLES, MPI_UINT32_T, 0, MPI_COMM_WORLD);

  // Test memory
  test_memory();

  // Simulation
  io_printf(IO_BUF, "[N-BODY] Simulation start\n", time_end - time_start);
  time_start = sv->clock_ms;
  for (i = 0; i < ITERATIONS; ++i) {

    // --- Velocity Verlet ---
    for (particle = env.chunk_particle_start;
         particle <= env.chunk_particle_end; ++particle) {
      engine_compute_force_particle(&env, particle);
    }

    for (particle = env.chunk_particle_start;
         particle <= env.chunk_particle_end; ++particle) {
      engine_update_velocity_particle(&env, particle);
    }

    engine_swap_f(&env);

    for (particle = env.chunk_particle_start;
         particle <= env.chunk_particle_end; ++particle) {
      engine_update_position_particle(&env, particle);
    }

    io_printf(IO_BUF, "[N-BODY] Allgather \n");
    MPI_Allgather(&env.s[env.chunk_particle_start],
                  env.chunk_size * N, MPI_UINT32_T,
                  env.s, env.chunk_size * N, MPI_UINT32_T,
                  MPI_COMM_WORLD);

  }
  time_end = sv->clock_ms;

  io_printf(IO_BUF, "[N-BODY] Simulation end: %d ms\n", time_end - time_start);

  MPI_Finalize();
}

void test_memory() {
  uint8_t *pointer;

  pointer = sark_alloc(1, sizeof(uint8_t));

  io_printf(IO_BUF, "[N-BODY] DTCM Memory Test\n");
  io_printf(IO_BUF, "         Pointer: %08x\n", pointer);
  io_printf(IO_BUF, "         Free: %d\n", DTCM_TOP - (int) pointer);
  io_printf(IO_BUF, "         Busy: %d\n", (int) pointer - DTCM_BASE);

  sark_free(pointer);

  return;
}
