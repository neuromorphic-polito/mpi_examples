#include "spin1_api.h"
#include "spin2_api.h"
#include "acp.h"
#include "mpi.h"

#include "fed_alloc.h"

#include "fed.h"

//#define N_REF 5
//#define N_PAT 120


// --- Types
typedef struct match {
    uint32_t chunk;
    uint16_t pattern;
    uint16_t position;
    struct match *next;
} match_t;


typedef struct {
    match_t *head;
    match_t *tail;
    uint32_t elements;
} list_t;


void list_init(list_t *l) {
    l->head = NULL;
    l->tail = NULL;
    l->elements = 0;

    return;
}


void list_add(list_t *l, uint32_t chunk, uint16_t pattern, uint16_t position) {
    match_t *block = (match_t *) sark_xalloc(sv->sdram_heap, sizeof(match_t), 0, ALLOC_LOCK);
    block->chunk = chunk;
    block->pattern = pattern;
    block->position = position;
    block->next = NULL;

    if (l->elements == 0) {
        l->head = block;
        l->tail = block;
    } else {
        l->tail->next = block;
        l->tail = block;
    }
    l->elements += 1;

    return;
}


// --- Prototypes
void mpi_main(uint arg1, uint arg2);

void distributePatterns(uint8_t *pat_ram, int nWorkers, int nPatterns);

void distributeChunks(uint8_t *txt_ram, uint32_t nChunks);

fed_pattern *receivePatterns(uint16_t *nPatterns);

void receiveAndMatchChunks(fed_pattern *patterns, int processRank, uint16_t nPatterns);


// --- Global variables
list_t match_list;


// --- SpiNNaker Main
void c_main() {
    MPI_Spinn(mpi_main);
}


// --- SpinMPI Entry point
void mpi_main(uint arg1, uint arg2) {
    int mpi_rank, mpi_size;

    uint64_t time_start, time_end;
    uint64_t time_io, time_pat;

    uint32_t npat, nref;

    uint8_t *txt_ram;
    uint8_t *pat_ram;
    fed_pattern *workerPatterns;

    uint8_t temp;
    uint16_t nPatterns;

    // Start MPI
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    time_start = sv->clock_ms;
    if (mpi_rank == 0) {
        acp_me_create(1000, sizeof(uint8_t * ), NULL, NULL);
        acp_me_create(1001, sizeof(uint8_t), NULL, NULL);

        // TODO: Ottenere N_REF e N_PAT dall'host using 1002 and 1003
        acp_me_create(1002, sizeof(uint32_t), NULL, NULL);
        acp_me_create(1003, sizeof(uint32_t), NULL, NULL);

        acp_me_read(1002, (uint8_t * ) & nref, sizeof(uint32_t), ACP_CHANNEL_HOST, NULL, false);
        acp_me_read(1003, (uint8_t * ) & npat, sizeof(uint32_t), ACP_CHANNEL_HOST, NULL, false);

        txt_ram = (uint8_t *) sark_xalloc(sv->sdram_heap, nref * FED_TEXT_BUFFER_SIZE, 0, 0);
        if (txt_ram == NULL) {
            rt_error(RTE_ABORT);
        }
        io_printf(IO_BUF, "[FED] created FED text struct in SDRAM\n");
        io_printf(IO_BUF, "      %d Byte\n", nref * FED_TEXT_BUFFER_SIZE);

        pat_ram = (uint8_t *) sark_xalloc(sv->sdram_heap, npat * FED_PATTERN_BUFFER_SIZE * (mpi_size - 1), 0, 0);
        if (pat_ram == NULL) {
            rt_error(RTE_ABORT);
        }
        io_printf(IO_BUF, "[FED] created FED pattern struct in SDRAM\n");
        io_printf(IO_BUF, "      %d Byte\n", npat * FED_PATTERN_BUFFER_SIZE);


        /* ----------->>> */
        // Host to Board Communication: Text (aka Reference)
        // Put pointer of the SDRAM memory in 1000
        acp_me_update(1000, (uint8_t * ) & txt_ram, sizeof(uint8_t * ), ACP_CHANNEL_SELF, NULL, false);

        // Wait Host, not sync, wait_for_update always on with channel host
        acp_me_read(1001, &temp, sizeof(uint8_t), ACP_CHANNEL_HOST, NULL, false);
        /* <<<----------- */

        /* ----------->>> */
        // Host to Board Communication: Patterns (aka Query)
        // Put pointer of the SDRAM memory in 1000
        acp_me_update(1000, (uint8_t * ) & pat_ram, sizeof(uint8_t * ), ACP_CHANNEL_SELF, NULL, false);

        // Wait Host, not sync, wait_for_update always on on with channel host
        acp_me_read(1001, &temp, sizeof(uint8_t), ACP_CHANNEL_HOST, NULL, false);
        /* <<<----------- */

        io_printf(IO_BUF, "[FED] Waiting on barrier.\n");
        MPI_Barrier(MPI_COMM_WORLD);
        time_io = sv->clock_ms;
        io_printf(IO_BUF, "[FED] Host-board transfer time: %d ms\n", (time_io - time_start));

        distributePatterns(pat_ram, mpi_size - 1, npat);
        time_pat = sv->clock_ms;
        io_printf(IO_BUF, "[FED] Pattern scattering time: %d ms\n", (time_pat - time_io));

        io_printf(IO_BUF, "[FED] Starting FED\n");
        distributeChunks(txt_ram, nref);

        sark_xfree(sv->sdram_heap, txt_ram, 0);
        sark_xfree(sv->sdram_heap, pat_ram, 0);
    } else {
        io_printf(IO_BUF, "[FED] Waiting on barrier.\n");
        MPI_Barrier(MPI_COMM_WORLD);
        time_io = sv->clock_ms;
        io_printf(IO_BUF, "[FED] Host-board transfer time: %d ms\n", (time_io - time_start));

        workerPatterns = receivePatterns(&nPatterns);
        time_pat = sv->clock_ms;
        io_printf(IO_BUF, "[FED] Pattern scattering time: %d ms\n", (time_pat - time_io));

        io_printf(IO_BUF, "[FED] Starting FED\n");
        receiveAndMatchChunks(workerPatterns, mpi_rank, nPatterns);
    }
    time_end = sv->clock_ms;

    io_printf(IO_BUF, "[FED] FED time: %d ms\n", (time_end - time_pat));
    io_printf(IO_BUF, "[FED] FED + pattern time: %d ms\n", (time_end - time_io));
    io_printf(IO_BUF, "[FED] Total time: %d ms\n", (time_end - time_start));

    MPI_Finalize();
    return;
}


void distributePatterns(uint8_t *pat_ram, int nWorkers, int nPatterns) {
    uint8_t buffer[FED_PATTERN_BUFFER_SIZE];
    uint16_t patternsPerWorker;
    //int avgPatternsPerWorker;
    //int highLoadWorkers;
    //int targetWorker;
    int targetRank;
    int i;
/*
    avgPatternsPerWorker = nPatterns / nWorkers;
    highLoadWorkers = nPatterns % nWorkers;
*/
    io_printf(IO_BUF, "[FED] Sending patterns...\n");

    for (targetRank = 1; targetRank <= nWorkers; targetRank++) {
/*
        patternsPerWorker = avgPatternsPerWorker;
        if (targetWorker < highLoadWorkers) {
            patternsPerWorker = avgPatternsPerWorker + 1;
        }
*/
        patternsPerWorker = nPatterns;
        MPI_Send(&patternsPerWorker, 1, MPI_UINT16_T, targetRank, 0, MPI_COMM_WORLD);

        for (i = 0; i < patternsPerWorker; i++) {
            MPI_Send(pat_ram, FED_PATTERN_BUFFER_SIZE, MPI_UINT8_T, targetRank, 0, MPI_COMM_WORLD);
            pat_ram += FED_PATTERN_BUFFER_SIZE;
        }
    }

    return;
}


fed_pattern *receivePatterns(uint16_t *nPatterns) {
    uint8_t *buffer;
    fed_pattern *patterns;
    uint16_t i;
    uint16_t patternsPerWorker;

    MPI_Recv(&patternsPerWorker, 1, MPI_UINT16_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    io_printf(IO_BUF, "[FED] Receiving %d patterns\n", patternsPerWorker);

    patterns = sark_alloc(1, patternsPerWorker * sizeof(fed_pattern));
    if (patterns == NULL) {
        rt_error(RTE_ABORT);
    }

    for (i = 0; i < patternsPerWorker; i++) {
        patterns[i] = fed_pattern_alloc();
        buffer = fed_pattern_get_buffer(patterns[i], 0);
        MPI_Recv(buffer, FED_PATTERN_BUFFER_SIZE, MPI_UINT8_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    io_printf(IO_BUF, "[FED] Received %d patterns\n", patternsPerWorker);

    for (i = 0; i < patternsPerWorker; i++) {
        fed_pattern_expand_buffer0(patterns[i]);
    }

    *nPatterns = patternsPerWorker;

    return patterns;
}


void distributeChunks(uint8_t *txt_ram, uint32_t nChunks) {
    int i;

    io_printf(IO_BUF, "[FED] Sending %d chunks in broadcast\n", nChunks);
    MPI_Bcast(&nChunks, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);

    for (i = 0; i < nChunks; i++) {
        MPI_Bcast(txt_ram, FED_TEXT_BUFFER_SIZE, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        txt_ram += FED_TEXT_BUFFER_SIZE;
    }

    return;
}


void receiveAndMatchChunks(fed_pattern *patterns, int processRank, uint16_t nPatterns) {

    fed_text chunk;
    uint8_t *buffer_text;
    uint32_t chunk_id;
    uint8_t chunkSize;

    fed_pattern pattern;
    uint8_t *buffer_pattern;
    uint8_t *buffer_pattern_shifted;
    uint32_t pattern_id;
    uint8_t patternSize;

    uint32_t nChunks;
    uint16_t matchPosition;
    uint16_t matchOffset;

    int k, m;


    MPI_Bcast(&nChunks, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
    io_printf(IO_BUF, "[FED] Receiving %d chunks in broadcast\n", nChunks);

    chunk = fed_text_alloc();
    list_init(&match_list);

    for (chunk_id = 0; chunk_id < nChunks; chunk_id++) {

        buffer_text = fed_text_get_buffer(chunk);
        MPI_Bcast(buffer_text, FED_TEXT_BUFFER_SIZE, MPI_UINT8_T, 0, MPI_COMM_WORLD);
        chunkSize = fed_text_get_size(chunk);

        if (chunkSize == 0) {
            io_printf(IO_BUF, "[FED] Warning: chunck %d size 0!\n", chunk_id);
            continue;
        }

        for (pattern_id = 0; pattern_id < nPatterns; pattern_id++) {

            pattern = patterns[pattern_id];
            buffer_pattern = fed_pattern_get_buffer(pattern, 0);
            patternSize = fed_pattern_get_size(pattern, 0);

            if (patternSize == 0) {
                io_printf(IO_BUF, "[FED] Warning: pattern %d size 0!\n", pattern_id);
                continue;
            }

            matchPosition = 0;
            matchOffset = 0;
            //m = 0;

            while (fed_search(chunk, patterns[pattern_id], matchOffset, &matchPosition)) {
                list_add(&match_list, chunk_id, pattern_id, matchPosition);
                io_printf(IO_BUF, "[FED] MATCH: chunk %d, pattern %d, position %d\n", chunk_id, pattern_id, matchPosition);

                matchOffset = matchPosition + 1;
/*
                m++;

                if (m>1){
                    io_printf(IO_BUF, "[FED] MULTIPLE MATCH: chunk %d, pattern %d, position %d\n", chunk_id, pattern_id, matchPosition);
                    io_printf(IO_BUF, "[FED] MULTIPLE MATCH: pattern address %08x, buffer address %08x\n", pattern, buffer_pattern);
                                                
                    io_printf(IO_BUF, "[FED] Pattern (0)\n    ");
                    for (k=0; k<FED_PATTERN_BUFFER_SIZE; k++){
                        io_printf(IO_BUF, "%02x ", buffer_pattern[k]);
                    }
                    io_printf(IO_BUF, "\n");

                    buffer_pattern_shifted = fed_pattern_get_buffer(pattern, 1);
                    io_printf(IO_BUF, "[FED] Pattern (1)\n    ");
                    for (k=0; k<FED_PATTERN_BUFFER_SIZE; k++){
                        io_printf(IO_BUF, "%02x ", buffer_pattern_shifted[k]);
                    }
                    io_printf(IO_BUF, "\n");


                    io_printf(IO_BUF, "[FED] Text\n    ");
                    for (k=0; k<FED_TEXT_BUFFER_SIZE; k++){
                        io_printf(IO_BUF, "%02x ", buffer_text[k]);
                        if (k>0 && (k & 0x07) == 0) {
                            io_printf(IO_BUF, "\n    ");        
                        }
                    }
                    io_printf(IO_BUF, "\n");

                }
*/
            }
        }
    }

    return;
}
