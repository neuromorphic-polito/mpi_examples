//
// Created by barchi on 3/20/19.
//
#include "spin1_api.h"
#include "spin2_api.h"
#include "acp.h"
#include "mpi.h"

#include "lib1.h"
#include "lib2.h"


// --- Prototypes ---
void test_memory();
void test_xmemory();
void test_barrier(int shift);
void test_swap();
void test_host(int mpi_rank, int mpi_comm_size);

void test_ucast(int mpi_rank, int mpi_comm_size, int byte);
void test_bcast(int root, int mpi_comm_size, int byte);

void mpi_main(uint arg1, uint arg2);


// --- Global Variables ---
uint8_t host_var=0;
uint8_t host_var_vec[10] = {0};


// --- Functions ---
void c_main() {
    MPI_Spinn(mpi_main);
}


void mpi_main(uint arg1, uint arg2) {
    int mpi_rank, mpi_size;
    uint8_t *memory;
    uint8_t temp;
    
    spin1_srand(100);
    
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    
    if (mpi_rank == 20){
        // Alloc Memory in SDRAM
        memory = (uint8_t *) sark_xalloc(sv->sdram_heap, 1024, 0, ALLOC_LOCK);
        
        // Create two memory entity
        acp_me_create(1000, sizeof(uint8_t *), NULL, NULL);
        acp_me_create(1001, sizeof(uint8_t), NULL, NULL);

        // Wait Host, with sync
        acp_me_update(1000, (uint8_t *)&memory, sizeof(uint8_t *), ACP_CHANNEL_SELF, NULL, false);
        
        // Wait Host, not sync but wait_for_update always on
        acp_me_read(1001, &temp, sizeof(uint8_t), ACP_CHANNEL_HOST, NULL, true);  
        
        // Wait Host, with sync
        acp_me_read(1001, &temp, sizeof(uint8_t), ACP_CHANNEL_HOST, NULL, true);  

        io_printf(IO_BUF, "[TEMPLATE] Rank %d - %s %s\n", mpi_rank, lib1_ping(), lib2_ping());
        io_printf(IO_BUF, "[TEMPLATE] Barrier 1\n");
        MPI_Barrier(MPI_COMM_WORLD);
    }
    else{
        MPI_Barrier(MPI_COMM_WORLD);
        io_printf(IO_BUF, "[TEMPLATE] Rank %d - %s %s\n", mpi_rank, lib1_ping(), lib2_ping());
        io_printf(IO_BUF, "[TEMPLATE] Barrier 1\n");
    }
    

/*
    if (mpi_rank == 20){ 
        spin1_delay_us(1000);
    }
    MPI_Barrier(MPI_COMM_WORLD);
*/

    test_memory();
    test_xmemory();

    MPI_Barrier(MPI_COMM_WORLD);
    test_barrier(0);
    
    MPI_Barrier(MPI_COMM_WORLD);
    test_barrier(1);
    
    MPI_Barrier(MPI_COMM_WORLD);
    test_barrier(2);
       
    MPI_Barrier(MPI_COMM_WORLD);
    test_swap();
    
    MPI_Barrier(MPI_COMM_WORLD);
    test_ucast(mpi_rank, mpi_size, 2000);

    MPI_Barrier(MPI_COMM_WORLD);
    test_bcast(mpi_rank, mpi_size, 2000);

    spin2_metrics_print();

    MPI_Finalize();
}


void test_swap() {
    uint8_t flag=1;
    uint8_t check=0;
    uint8_t *p_flag = &flag;

    io_printf(IO_BUF, "[TEMPLATE] Swap: Check %d, Flag %d\n", check, flag);
    check = spin2_swap_uint8(check, p_flag);
    io_printf(IO_BUF, "[TEMPLATE] Swap: Check %d, Flag %d\n", check, flag);
    return;
}


void test_barrier(int shift){
    uint64_t time_start, time_end;
    volatile int i;
    int sync;

    // shift 0 = 1000, 1 = 2000, 2 = 4000, 3 = 8000
    sync = 1000 << shift;

    time_start = sv->clock_ms;
    for (i=0; i<sync; i++){
        MPI_Barrier(MPI_COMM_WORLD);
    }
    time_end = sv->clock_ms;
    io_printf(IO_BUF, "[TEMPLATE] %d %d Sync: %d us\n", i, sync, (time_end - time_start)>>shift);

    return;
}


void test_memory() {

  io_printf(IO_BUF, "[TEMPLATE] DTCM Memory Test\n");
  io_printf(IO_BUF, "           Free: %d\n", ((heap_t *)sark_vec->heap_base)->free_bytes);
  
  return;
}


void test_xmemory() {
  
  io_printf(IO_BUF, "[TEMPLATE] SDRAM Memory Test\n");
  io_printf(IO_BUF, "           Free:       %d\n", sv->sdram_heap->free_bytes);
  io_printf(IO_BUF, "           sdram_bufs: 0x%08x\n", sv->sdram_bufs);
  io_printf(IO_BUF, "           sdram_base: 0x%08x\n", sv->sdram_base);
  io_printf(IO_BUF, "           sdram_heap: 0x%08x\n", sv->sdram_heap);
  io_printf(IO_BUF, "           sdram_sys:  0x%08x\n", sv->sdram_sys);
  io_printf(IO_BUF, "           sys_heap:   0x%08x\n", sv->sys_heap);
  
  return;
}


void test_host(int mpi_rank, int mpi_comm_size){
    int i;
    uint16_t len_temp;
    uint8_t val_temp;

    if (mpi_rank == mpi_comm_size-2){
        for (i=0; i<10; i++){
            len_temp = sizeof(val_temp);
            acp_me_read(1000, &val_temp, len_temp, ACP_CHANNEL_HOST, NULL, false);
            io_printf(IO_BUF, "[TEMPLATE] Host %d (%d Byte)\n", val_temp, len_temp);
        }
    }

    return;
}


void test_ucast(int mpi_rank, int mpi_comm_size, int byte) {
    int i, errors, rank_from, rank_to;
    uint8_t *buffer_send = sark_alloc(byte, sizeof(uint8_t));
    uint8_t *buffer_recv = sark_alloc(byte, sizeof(uint8_t));

    if (buffer_send==NULL){
        io_printf(IO_BUF, "[TEMPLATE] No more memory!\n");
        rt_error(RTE_ABORT);
    }

    if (buffer_recv==NULL){
        io_printf(IO_BUF, "[TEMPLATE] No more memory!\n");
        rt_error(RTE_ABORT);
    }

    for (i=0; i<byte; i++){
        buffer_send[i] = (uint8_t)(spin1_rand()+i & 0x000000FF); //(uint8_t)((i<<1) & 0x000000FF);
        buffer_recv[i] = 0;
    }

    rank_from = 1; 
    rank_to = mpi_comm_size-1;

    if (mpi_rank == rank_from){
        MPI_Send(buffer_send, byte, MPI_UINT8_T, rank_to, 0, MPI_COMM_WORLD);

    } else if (mpi_rank == rank_to){
        MPI_Recv(buffer_recv, byte, MPI_UINT8_T, rank_from, 0, MPI_COMM_WORLD, NULL);
        
        for (i=0, errors=0; i<byte; i++){
            if (buffer_recv[i] != buffer_send[i] ){
                io_printf(IO_BUF, "[TEMPLATE] (%d) expected %d obtained %d\n", 
                    i, buffer_send[i], buffer_recv[i]);
                errors ++;
            }
        }
        io_printf(IO_BUF, "[TEMPLATE] test_ucast %d Bytes: %d errors\n", byte, errors);
    }

    sark_free(buffer_recv);
    sark_free(buffer_send);

    return; 
}


void test_bcast(int mpi_rank, int mpi_comm_size, int byte) {
    int i, errors;
    uint8_t *buffer_send = sark_alloc(byte, sizeof(uint8_t));
    uint8_t *buffer_recv = sark_alloc(byte, sizeof(uint8_t));

     if (buffer_send==NULL){
        io_printf(IO_BUF, "[TEMPLATE] No more memory!\n");
        rt_error(RTE_ABORT);
    }

    if (buffer_recv==NULL){
        io_printf(IO_BUF, "[TEMPLATE] No more memory!\n");
        rt_error(RTE_ABORT);
    }

    for (i=0; i<byte; i++){
        buffer_send[i] = (uint8_t)(spin1_rand()+i & 0x000000FF);
        buffer_recv[i] = 0;
    }

    if (mpi_rank == 0){
        MPI_Bcast(buffer_send, byte, MPI_UINT8_T, 0, MPI_COMM_WORLD);
    } else {
        MPI_Bcast(buffer_recv, byte, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        for (i=0, errors=0; i<byte; i++){
            if (buffer_recv[i] != buffer_send[i] ){
                if(mpi_rank==mpi_comm_size-1){
                    io_printf(IO_BUF, "[TEMPLATE] (%d) expected %d obtained %d\n", 
                        i, buffer_send[i], buffer_recv[i]);
                }
                errors ++;
            }
        }
        io_printf(IO_BUF, "[TEMPLATE] test_bcast %d Bytes: %d errors\n", byte, errors);
    }

    sark_free(buffer_recv);
    sark_free(buffer_send);

    return; 
}