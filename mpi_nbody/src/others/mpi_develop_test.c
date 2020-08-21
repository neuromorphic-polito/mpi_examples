#include <sark.h>
#include <fix16.h>

#include "mpi.h"
#include "acp.h"

/// Prototypes
void fxp_test_run();
void ht_test_run(uint32_t n);
void acp_test_run();

void mpi_main(uint arg1, uint arg2);
void mpi_test_bcast(int mpi_rank, int mpi_size, int mpi_root);
void mpi_test_scatter(int mpi_rank, int mpi_size, int mpi_root);
void mpi_test_gather(int mpi_rank, int mpi_size, int mpi_root);
void mpi_test_allgather(int mpi_rank, int mpi_size);
void mpi_test_allgather_n(int mpi_rank, int mpi_size);
void mpi_test_sync();

/**
 * MAIN
 */
void c_main() {
  io_printf(IO_BUF, "[APP] Before MPI_Spinn\n");
  MPI_Spinn(mpi_main);
  io_printf(IO_BUF, "[APP] After MPI_Spinn\n");
  return;
}

/**
 *
 * @param arg1
 * @param arg2
 */
void mpi_main(uint arg1, uint arg2) {
  int mpi_rank, mpi_size, mpi_root;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  mpi_test_bcast(mpi_rank, mpi_size, 0);
  io_printf(IO_BUF, "[MPI-TEST] End bcast test\n");

//  mpi_test_scatter(mpi_rank, mpi_size, 0);
//  io_printf(IO_BUF, "[MPI-TEST] End scatter test\n");

//  mpi_test_gather(mpi_rank, mpi_size, 0);
//  io_printf(IO_BUF, "[MPI-TEST] End scatter test\n");

  mpi_test_allgather(mpi_rank, mpi_size);
  io_printf(IO_BUF, "[MPI-TEST] End allgather test\n");

  mpi_test_sync();
  io_printf(IO_BUF, "[MPI-TEST] End sync test\n");

  mpi_test_allgather_n(mpi_rank, mpi_size);
  io_printf(IO_BUF, "[MPI-TEST] End allgather_n test\n");

  MPI_Finalize();
  return;
}

void mpi_test_sync(){
  io_printf(IO_BUF, "[MPI-TEST] Start sync test\n");
  spin2_mc_wfs();
  return;
}

void mpi_test_bcast(int mpi_rank, int mpi_size, int mpi_root) {
  char buffer[10];

  if (mpi_rank == mpi_root) {
    sark_str_cpy(buffer, "H3ll0!");
    MPI_Bcast(buffer, sark_str_len(buffer), MPI_CHARACTER, mpi_root, MPI_COMM_WORLD);
    io_printf(IO_BUF, "[TEST-BCAST] Send %s\n", buffer);
  } else {
    MPI_Bcast(buffer, 10, MPI_CHARACTER, mpi_root, MPI_COMM_WORLD);
    io_printf(IO_BUF, "[TEST-BCAST] Received %s\n", buffer);
  }
  return;
}

void mpi_test_scatter(int mpi_rank, int mpi_size, int mpi_root) {
  int i;
  uint8_t *buffer8bit;

  buffer8bit = (uint8_t *) sark_alloc(mpi_size, sizeof(uint8_t));
  if (mpi_rank == mpi_root) {
    for (i = 0; i < mpi_size; i++) {
      buffer8bit[i] = i;
    }
  } else {
    for (i = 0; i < mpi_size; i++) {
      buffer8bit[i] = 0;
    }
  }

  MPI_Scatter(buffer8bit, 1, MPI_UINT8_T,
              buffer8bit, 1, MPI_UINT8_T,
              mpi_root, MPI_COMM_WORLD);

  if (mpi_rank != mpi_root) {
    io_printf(IO_BUF, "[APP] Scatter %d\n", buffer8bit[0]);
  }

  sark_free(buffer8bit);
  return;
}

void mpi_test_gather(int mpi_rank, int mpi_size, int mpi_root) {
  int i;
  uint8_t *buffer8bit;

  buffer8bit = (uint8_t *) sark_alloc(mpi_size, sizeof(uint8_t));
  if(buffer8bit == NULL){
    io_printf(IO_BUF, "ERR! no space");
    return;
  }

  if (mpi_rank != mpi_root) {
    buffer8bit[0] = mpi_rank;
  }

  MPI_Gather(buffer8bit, 1, MPI_UINT8_T,
             buffer8bit, 1, MPI_UINT8_T,
             mpi_root, MPI_COMM_WORLD);

  if (mpi_rank == mpi_root) {
    for (i = 0; i < mpi_size; i++) {
      io_printf(IO_BUF, "[APP] Gather %d - %d\n", i, buffer8bit[i]);
    }
  }

  sark_free(buffer8bit);
  return;
}

void mpi_test_allgather(int mpi_rank, int mpi_size) {
  int i;
  uint8_t *buffer8bit;
  uint16_t *buffer16bit;
  uint32_t *buffer32bit;

  buffer8bit = (uint8_t *) sark_alloc(mpi_size, sizeof(uint8_t));
  buffer16bit = (uint16_t *) sark_alloc(mpi_size, sizeof(uint16_t));
  buffer32bit = (uint32_t *) sark_alloc(mpi_size, sizeof(uint32_t));

  // Init
  for (i = 0; i < mpi_size; i++) {
    buffer8bit[i] = 0;
    buffer16bit[i] = 0;
    buffer32bit[i] = 0;
  }
  buffer8bit[mpi_rank] = mpi_rank * 2;
  buffer16bit[mpi_rank] = mpi_rank * 3;
  buffer32bit[mpi_rank] = mpi_rank * 4;

  MPI_Allgather(&buffer8bit[mpi_rank], 1, MPI_UINT8_T,
                buffer8bit, 1, MPI_UINT8_T,
                MPI_COMM_WORLD);
//  for (i = 0; i < mpi_size; i++) {
//    io_printf(IO_BUF, "[APP] AllGather 8bit %d -> %d\n", i, buffer8bit[i]);
//  }

  MPI_Allgather(&buffer16bit[mpi_rank], 1, MPI_UINT16_T,
                buffer16bit, 1, MPI_UINT16_T,
                MPI_COMM_WORLD);
//  for (i = 0; i < mpi_size; i++) {
//    io_printf(IO_BUF, "[APP] AllGather 16bit %d -> %d\n", i, buffer16bit[i]);
//  }

  MPI_Allgather(&buffer32bit[mpi_rank], 1, MPI_UINT32_T,
                buffer32bit, 1, MPI_UINT32_T,
                MPI_COMM_WORLD);
//  for (i = 0; i < mpi_size; i++) {
//    io_printf(IO_BUF, "[APP] AllGather 32bit %d -> %d\n", i, buffer32bit[i]);
//  }

  sark_free(buffer8bit);
  sark_free(buffer16bit);
  sark_free(buffer32bit);

  return;
}

void mpi_test_allgather_n(int mpi_rank, int mpi_size) {
  int i, j, n;
  uint8_t *buffer8bit;
  uint16_t *buffer16bit;
  uint32_t *buffer32bit;

  n = 16;
  buffer8bit = (uint8_t *) sark_alloc(mpi_size * n, sizeof(uint8_t));
  if(buffer8bit == NULL){
    io_printf(IO_BUF, "[APP] GatherN alloc no memory SKIP\n");
    return;
  }

  buffer16bit = (uint16_t *) sark_alloc(mpi_size * n, sizeof(uint16_t));
  if(buffer16bit == NULL){
    io_printf(IO_BUF, "[APP] GatherN alloc no memory SKIP\n");
    sark_free(buffer8bit);
    return;
  }

  buffer32bit = (uint32_t *) sark_alloc(mpi_size * n, sizeof(uint32_t));
  if(buffer32bit == NULL){
    io_printf(IO_BUF, "[APP] GatherN alloc no memory SKIP\n");
    sark_free(buffer8bit);
    sark_free(buffer16bit);
    return;
  }

  // Init
  for (i = 0; i < mpi_size; i++) {
    for (j = 0; j < n; j++) {
      buffer8bit[n * i + j] = 0;
      buffer16bit[n * i + j] = 0;
      buffer32bit[n * i + j] = 0;
    }
  }

  for (j = 0; j < n; j++) {
    buffer8bit[n * mpi_rank + j] = mpi_rank * 2 + j;
    buffer16bit[n * mpi_rank + j] = mpi_rank * 3 + j;
    buffer32bit[n * mpi_rank + j] = mpi_rank * 4 + j;
  }

  MPI_Allgather(&buffer8bit[n * mpi_rank], n, MPI_UINT8_T,
                buffer8bit, n, MPI_UINT8_T,
                MPI_COMM_WORLD);
  for (i = 0; i < mpi_size; i++) {
    io_printf(IO_BUF, "[APP] AllGather 8bit %d -> ", i);
    for (j = n - 4; j < n; j++) {
      io_printf(IO_BUF, "%d ", buffer8bit[n * i + j]);
    }
    io_printf(IO_BUF, "\n");
  }

  MPI_Allgather(&buffer16bit[n * mpi_rank], n, MPI_UINT16_T,
                buffer16bit, n, MPI_UINT16_T,
                MPI_COMM_WORLD);
  for (i = 0; i < mpi_size; i++) {
    io_printf(IO_BUF, "[APP] AllGather 16bit %d -> ", i);
    for (j = n - 4; j < n; j++) {
      io_printf(IO_BUF, "%d ", buffer16bit[n * i + j]);
    }
    io_printf(IO_BUF, "\n");
  }

  MPI_Allgather(&buffer32bit[n * mpi_rank], n, MPI_UINT32_T,
                buffer32bit, n, MPI_UINT32_T,
                MPI_COMM_WORLD);
  for (i = 0; i < mpi_size; i++) {
    io_printf(IO_BUF, "[APP] AllGather 32bit %d -> ", i);
    for (j = n - 4; j < n; j++) {
      io_printf(IO_BUF, "%d ", buffer32bit[n * i + j]);
    }
    io_printf(IO_BUF, "\n");
  }

  sark_free(buffer8bit);
  sark_free(buffer16bit);
  sark_free(buffer32bit);

  return;
}

/**
 * TEST --- Fixed Point
 */
void fxp_test_run() {
  fix16_t a, b, c;

  a = F16C(0, 1);

  io_printf(IO_BUF, "a = %k (0x%08x)\n", a >> 1, a);
  a = -a;
  io_printf(IO_BUF, "a = %k (0x%08x)\n", a >> 1, a);
  a = -a;

  b = fix16_sqrt(a);
  io_printf(IO_BUF, "sqrt(a) = %k (0x%08x)\n", b >> 1, b);

  c = fix16_ssub(a, b);
  io_printf(IO_BUF, "a - sqrt(a) = %k (0x%08x)\n", c >> 1, c);

  return;
}

/**
 * TEST --- Hash Table
 * @param n
 */
void ht_test_run(uint32_t n) {
  if (spin2_ht_test(n))
    io_printf(IO_BUF, "[APP] Hash table test --- OK\n");
  else
    io_printf(IO_BUF, "[APP] Hash table test --- FAILED\n");
  return;
}

/**
 * TEST --- Application Command Protocol
 */
void acp_test_run() {
  acp_test();
  return;
}