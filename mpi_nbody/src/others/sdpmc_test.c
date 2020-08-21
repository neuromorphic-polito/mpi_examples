/**
 * Test for implement SDP over MC
 * An entire sdp can be send via MC packets instead with P2P via Monitor Processor
 */

#include <sark.h>
#include <spin1_api.h>

#include <stdint.h>
#include <stdbool.h>

// --- Test Values
#define DEBUG
#define BOARD_X_MAX 8
#define BOARD_Y_MAX 8
#define TEST_PKT_MAX 1024
#define THROTTLING_MICROSEC 5

// --- Bit operation Macro
#define MASK_SHIFT_R(v, m, s) (((v) & (m)) >> (s))
#define MASK_SHIFT_L(v, m, s) (((v) & (m)) << (s))

#define BIT_TOGGLE(v, b) ((v) ^= 1 << (b))
#define BIT_SET(v, b, x) ((v) ^= (-(x) ^ (v)) & (1 << (b)))
#define BIT_CHECK(v, b)  ((((v) >> (b)) & 1) == 1)

// --- Globals - Header v3
const uint32_t MC_X_MSK = 0xFF000000;
const uint32_t MC_X_SH = 24;
const uint32_t MC_Y_MSK = 0x00FF0000;
const uint32_t MC_Y_SH = 16;
const uint32_t MC_P_MSK = 0x0000F000;
const uint32_t MC_P_SH = 12;
const uint32_t MC_CMD_MSK = 0x00000C00;
const uint32_t MC_CMD_SH = 10;
const uint32_t MC_ID_MSK = 0x000003FF;
const uint32_t MC_ID_SH = 0;

const uint32_t W_MC_X_MSK = 0xFF;
const uint32_t W_MC_Y_MSK = 0xFF;
const uint32_t W_MC_P_MSK = 0x0F;
const uint32_t W_MC_CMD_MSK = 0x03;
const uint32_t W_MC_ID_MSK = 0x3FF;

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t p;
  uint8_t ctrl;
  uint16_t id;
  uint32_t payload;
} acp_mc_header_t;

const uint32_t ROUTE_LINK_E = 0;
const uint32_t ROUTE_LINK_NE = 1;
const uint32_t ROUTE_LINK_N = 2;
const uint32_t ROUTE_LINK_W = 3;
const uint32_t ROUTE_LINK_SW = 4;
const uint32_t ROUTE_LINK_S = 5;
const uint32_t ROUTE_ALL_AP = 0x1FFFE << 6;
const uint32_t ROUTE_MODIFICATORS[][2] = {
    {+1, 0}, {+1, +1}, {0, +1}, {-1, 0}, {-1, -1}, {0, -1}};

uint32_t core_x;
uint32_t core_y;
uint32_t core_p;

uint32_t app_id;
char *app_name;

acp_mc_header_t mc_packet;
bool pkt_received;

uint32_t *debug_mc_w;
uint32_t *debug_mc_we;
uint32_t *debug_mc_r;
uint32_t *debug_mc_re;

uint32_t buffer_send[TEST_PKT_MAX];
uint32_t buffer_recv[TEST_PKT_MAX];

// --- Functions
bool valid_chip(int32_t x, int32_t y) {
  return y <= (x + 3) &&
      y >= (x - 4) &&
      x >= 0 &&
      y >= 0 &&
      x < BOARD_X_MAX &&
      y < BOARD_Y_MAX;
}

void generate_safe_route(uint32_t *route, uint32_t direction) {
  int32_t x, y;

  x = core_x + ROUTE_MODIFICATORS[direction][0];
  y = core_y + ROUTE_MODIFICATORS[direction][1];

  if (valid_chip(x, y)) {
    BIT_SET(*route, direction, 1);
  }
  return;
}

void generate_broadcast_router_key(uint32_t x, uint32_t y) {
  uint32_t key, key_p;
  uint32_t mask, mask_p;
  uint32_t route, route_p;

  uint32_t rule;
  uint32_t p;

  key = 0 | MASK_SHIFT_L(x, W_MC_X_MSK, MC_X_SH) | MASK_SHIFT_L(y, W_MC_Y_MSK, MC_Y_SH);
  mask = MC_X_MSK | MC_Y_MSK;
  route = ROUTE_ALL_AP;

  if (core_x == x && core_y == y) {
    /// Generate Safe Neighbour routes
    generate_safe_route(&route, ROUTE_LINK_E);
    generate_safe_route(&route, ROUTE_LINK_NE);
    generate_safe_route(&route, ROUTE_LINK_N);
    generate_safe_route(&route, ROUTE_LINK_W);
    generate_safe_route(&route, ROUTE_LINK_SW);
    generate_safe_route(&route, ROUTE_LINK_S);

    /// Enable Processor check
    mask_p = mask | MC_P_MSK;

    /// Generate 16 rule, one for each Processors
    for (p = 1; p < 17; p++) {
      /// Add Processor to routing key
      key_p = key | MASK_SHIFT_L(p - 1, W_MC_P_MSK, MC_P_SH);

      /// Disable self connections
      route_p = route;
      BIT_TOGGLE(route_p, p + 6);

      /// Add rule
      rule = rtr_alloc(1);
      if (rule == 0)
        rt_error(RTE_ABORT);
      rtr_mc_set(rule, key_p, mask_p, route_p);
    }
  } else {
    if (core_x == x && core_y < y) {
      generate_safe_route(&route, ROUTE_LINK_SW);
      generate_safe_route(&route, ROUTE_LINK_S);
    } else if (core_x == x && core_y > y) {
      generate_safe_route(&route, ROUTE_LINK_NE);
      generate_safe_route(&route, ROUTE_LINK_N);
    } else if (core_y == y && core_x < x) {
      generate_safe_route(&route, ROUTE_LINK_N);
      generate_safe_route(&route, ROUTE_LINK_SW);
      generate_safe_route(&route, ROUTE_LINK_W);
    } else if (core_y == y && core_x > x) {
      generate_safe_route(&route, ROUTE_LINK_S);
      generate_safe_route(&route, ROUTE_LINK_NE);
      generate_safe_route(&route, ROUTE_LINK_E);
    } else if (core_x > x && core_y > y) {
      generate_safe_route(&route, ROUTE_LINK_NE);
    } else if (core_x > x && core_y < y) {
      generate_safe_route(&route, ROUTE_LINK_S);
    } else if (core_x < x && core_y < y) {
      generate_safe_route(&route, ROUTE_LINK_SW);
    } else if (core_x < x && core_y > y) {
      generate_safe_route(&route, ROUTE_LINK_N);
    }

    /// Generate 1 Rule
    rule = rtr_alloc(1);
    if (rule == 0)
      rt_error(RTE_ABORT);
    rtr_mc_set(rule, key, mask, route);
  }

  return;
}

void generate_broadcast_router_keys() {
  int32_t x, y;

  for (x = 0; x < BOARD_X_MAX; x++) {
    for (y = 0; y < BOARD_Y_MAX; y++) {
      if (valid_chip(x, y)) {
        generate_broadcast_router_key(x, y);
      }
    }
  }
  return;
}

/**
 *
 * @param h
 * @param key
 * @param data
 * @return
 */
bool acp_mc_header_write(acp_mc_header_t *h, uint *key, uint *data) {
  *key = 0;
  *key |= MASK_SHIFT_L(h->x, W_MC_X_MSK, MC_X_SH);
  *key |= MASK_SHIFT_L(h->y, W_MC_Y_MSK, MC_Y_SH);
  *key |= MASK_SHIFT_L(h->p - 1, W_MC_P_MSK, MC_P_SH);
  *key |= MASK_SHIFT_L(h->ctrl, W_MC_CMD_MSK, MC_CMD_SH);
  *key |= MASK_SHIFT_L(h->id, W_MC_ID_MSK, MC_ID_SH);
  *data = h->payload;
  return true;
}

/**
 *
 * @param h
 * @param key
 * @param data
 * @return
 */
bool acp_mc_header_read(acp_mc_header_t *h, uint key, uint data) {
  h->x = MASK_SHIFT_R(key, MC_X_MSK, MC_X_SH);
  h->y = MASK_SHIFT_R(key, MC_Y_MSK, MC_Y_SH);
  h->p = MASK_SHIFT_R(key, MC_P_MSK, MC_P_SH) + 1;
  h->ctrl = MASK_SHIFT_R(key, MC_CMD_MSK, MC_CMD_SH);
  h->id = MASK_SHIFT_R(key, MC_ID_MSK, MC_ID_SH);
  h->payload = data;
  return true;
}

/**
 * Send data with Broadcast and increment the id
 * @param pkt
 * @return
 */
bool mc_pkt_send(acp_mc_header_t *pkt, bool last){
  bool r;
  uint32_t mc_key, mc_payload;

  if(last){
    pkt->ctrl = 0b11;
  }
  else if(pkt->id == 0){
    pkt->ctrl = 0b00;
  }
  else{
    pkt->ctrl = 0b01;
  }

  acp_mc_header_write(pkt, &mc_key, &mc_payload);

  if(spin1_send_mc_packet(mc_key, mc_payload, 1)){
    pkt->id += 1;
    *debug_mc_w += 1;
    r = true;
  } else{
    *debug_mc_we += 1;
    r = false;
  }

  spin1_delay_us(THROTTLING_MICROSEC);
  return r;
}

/**
 *
 * @param data
 * @param len
 * @return
 */
bool send_data_via_mc(uint32_t arg0, uint32_t arg1){
  uint32_t i;
  acp_mc_header_t mc_pkt;
  uint32_t mc_key, mc_payload;

  uint32_t *data = (uint32_t *)arg0;
  uint32_t len = arg1;

  io_printf(IO_BUF, "Start Send\n");

  mc_pkt.x = core_x;
  mc_pkt.y = core_y;
  mc_pkt.p = core_p;
  mc_pkt.ctrl = 0;
  mc_pkt.id = 0;

  for(i=0; i < len-1; i++){
    mc_pkt.payload = data[i];
    mc_pkt_send(&mc_pkt, false);
  }
  mc_pkt.payload = data[i];
  mc_pkt_send(&mc_pkt, true);

  io_printf(IO_BUF, "End Send\n");
  return true;
}

/**
 * This "proc" is called when a multicast packet is received. It sends
 * a message to the "tubogrid" containing the count that came in the
 * packet's payload. The background changes colour every 16 iterations.
 *
 * @param key
 * @param data
 */
void mc_recv(uint key, uint data) {
  acp_mc_header_t h;

  acp_mc_header_read(&h, key, data);

  if (h.id == h.payload) {
    *debug_mc_r += 1;
    buffer_recv[h.id] = h.payload;
  } else {
    *debug_mc_re += 1;
  }

  if (h.ctrl == 0b11) {
    pkt_received = true;
  }

  return;
}

void test_send(uint arg0, uint arg1){
  send_data_via_mc(&buffer_send, TEST_PKT_MAX);
  spin1_exit(0);
}

void test_recv(uint arg0, uint arg1){
  while(!pkt_received){ cpu_wfi(); }
  pkt_received = false;
  io_printf(IO_BUF, "End Recv\n");
  spin1_exit(0);
}

bool init(){
  int i=0;

  /// Initialize Core vars
  core_x = sark_chip_id() >> 8;
  core_y = sark_chip_id() & 0xFF;
  core_p = sark_core_id();

  /// Initialize App vars
  app_id = sark_app_id();
  app_name = (char *) &sark.vcpu->app_name;

  if (!valid_chip(core_x, core_y)) {
    io_printf(IO_BUF, "[%s] Out of scope, exit\n", app_name);
    return false;
  }

  /// Initialise User variables
  sark.vcpu->user0 = 0;
  sark.vcpu->user1 = 0;
  sark.vcpu->user2 = 0;
  sark.vcpu->user3 = 0;

  debug_mc_w = &sark.vcpu->user0;
  debug_mc_we = &sark.vcpu->user1;
  debug_mc_r = &sark.vcpu->user2;
  debug_mc_re = &sark.vcpu->user3;

  /// Initialize Routing Key
  mc_packet.x = core_x;
  mc_packet.y = core_y;
  mc_packet.p = core_p;
  mc_packet.ctrl = 0;
  mc_packet.id = 0;

  io_printf(IO_BUF, "[%s] Core %d@%d-%d (app_id %d)\n", app_name, core_p, core_x, core_y, app_id);

  if (core_p == 1) {
    io_printf(IO_BUF, "[%s] Generate routing key\n", app_name);
    generate_broadcast_router_keys();
  }

  /// Initialise buffers
  for (i=0; i<TEST_PKT_MAX; i++){
    buffer_send[i] = i;
    buffer_recv[i] = 0;
  }

  pkt_received = false;

  return true;
}


/**
 * The main program prints a message to the core's IO buffer and then
 * sets up a routing table entry so that packets which contain this
 * core's ID in the key field are routed to the next core in the ring.
 * Then it sets up callbacks for the elapsed timer and packet received
 * events. Core 1 (only) places an initial call to the timer proc on
 * the event queue. Finally, it starts event processing but requires a
 * SYNC0 signal before operation commences.
 */
void c_main(void) {
  uint rc;

  if(!init())
    return;

  /// MC with payload callback
  spin1_callback_on(MCPL_PACKET_RECEIVED, mc_recv, -1);

  /// TEST
  if (core_x == 0 && core_y == 0 && core_p == 1){
    spin1_schedule_callback(test_send, 0, 0, 1);
  }
  else{
    spin1_schedule_callback(test_recv, 0, 0, 1);
  }

  /// Start event processing but wait for SYNC0
  rc = spin1_start(SYNC_WAIT);

  /// Print RC if we stop...
  io_printf(IO_BUF, "[%s] Terminated - RC %d\n", app_name, rc);
}


