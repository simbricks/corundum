/*
 * Copyright 2024 Max Planck Institute for Software Systems, and
 * National University of Singapore
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <signal.h>

#include <iostream>

#include <simbricks/axi/axi_s.hpp>
#include <simbricks/axi/axi_subordinate.hh>
#include <simbricks/axi/axil_manager.hh>

#include "sims/external/corundum/obj_dir/Vmqnic_core_axi.h"
#include "verilated.h"

// #include <verilated_vcd_c.h>

extern "C" {
#include <simbricks/nicif/nicif.h>
}

#define CORUNDUM_VERILATOR_DEBUG 1

/* **************************************************************************
 * helpers
 * ************************************************************************** */

void reset_corundum(Vmqnic_core_axi &top_verilator_interface) {
  top_verilator_interface.rst = 1;
  top_verilator_interface.clk = 0;
  top_verilator_interface.eval();
  top_verilator_interface.clk = 1;
  top_verilator_interface.eval();
  top_verilator_interface.rst = 0;
  top_verilator_interface.clk = 0;
}

static volatile union SimbricksProtoPcieD2H *d2h_alloc(
    struct SimbricksNicIf &nicif, uint64_t cur_ts) {
  return SimbricksPcieIfD2HOutAlloc(&nicif.pcie, cur_ts);
}

/* **************************************************************************
 * AXI, AXIL and AXIS interface definitions required by this adapter
 * ************************************************************************** */

// handles DMA read requests
class CorundumAXISubordinateRead
    : public simbricks::AXISubordinateRead<
          4, 1, 16,
          /*num concurrently pending requests*/ 16> {
 public:
  explicit CorundumAXISubordinateRead(Vmqnic_core_axi &top_verilator_interface)
      : AXISubordinateRead(
            reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axi_araddr),
            &top_verilator_interface.m_axi_arid,
            top_verilator_interface.m_axi_arready,
            top_verilator_interface.m_axi_arvalid,
            top_verilator_interface.m_axi_arlen,
            top_verilator_interface.m_axi_arsize,
            top_verilator_interface.m_axi_arburst,
            reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axi_rdata),
            &top_verilator_interface.m_axi_rid,
            top_verilator_interface.m_axi_rready,
            top_verilator_interface.m_axi_rvalid,
            top_verilator_interface.m_axi_rlast) {
  }

 private:
  void do_read(const simbricks::AXIOperation &axi_op) final {
    // TODO
  }
};

// handles DMA write requests
class CorundumAXISubordinateWrite
    : public simbricks::AXISubordinateWrite<
          4, 1, 16,
          /*num concurrently pending requests*/ 16> {
 public:
  explicit CorundumAXISubordinateWrite(Vmqnic_core_axi &top_verilator_interface)
      : AXISubordinateWrite(
            reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axi_awaddr),
            &top_verilator_interface.m_axi_awid,
            top_verilator_interface.m_axi_awready,
            top_verilator_interface.m_axi_awvalid,
            top_verilator_interface.m_axi_awlen,
            top_verilator_interface.m_axi_awsize,
            top_verilator_interface.m_axi_awburst,
            reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axi_wdata),
            top_verilator_interface.m_axi_wready,
            top_verilator_interface.m_axi_wvalid,
            top_verilator_interface.m_axi_wstrb,
            top_verilator_interface.m_axi_wlast,
            &top_verilator_interface.m_axi_bid,
            top_verilator_interface.m_axi_bready,
            top_verilator_interface.m_axi_bvalid,
            top_verilator_interface.m_axi_bresp) {
  }

 private:
  void do_write(const simbricks::AXIOperation &axi_op) final {
    // TODO
  }
};

// handles host to device register reads / writes
class CorundumAXILManager : public simbricks::AXILManager<3, 4> {
 public:
  explicit CorundumAXILManager(Vmqnic_core_axi &top_verilator_interface)
      : AXILManager(reinterpret_cast<uint8_t *>(
                        &top_verilator_interface.s_axil_ctrl_araddr),
                    top_verilator_interface.s_axil_ctrl_arready,
                    top_verilator_interface.s_axil_ctrl_arvalid,
                    reinterpret_cast<uint8_t *>(
                        &top_verilator_interface.s_axil_ctrl_rdata),
                    top_verilator_interface.s_axil_ctrl_rready,
                    top_verilator_interface.s_axil_ctrl_rvalid,
                    top_verilator_interface.s_axil_ctrl_rresp,
                    reinterpret_cast<uint8_t *>(
                        &top_verilator_interface.s_axil_ctrl_awaddr),
                    top_verilator_interface.s_axil_ctrl_awready,
                    top_verilator_interface.s_axil_ctrl_awvalid,
                    reinterpret_cast<uint8_t *>(
                        &top_verilator_interface.s_axil_ctrl_wdata),
                    top_verilator_interface.s_axil_ctrl_wready,
                    top_verilator_interface.s_axil_ctrl_wvalid,
                    top_verilator_interface.s_axil_ctrl_wstrb,
                    top_verilator_interface.s_axil_ctrl_bready,
                    top_verilator_interface.s_axil_ctrl_bvalid,
                    top_verilator_interface.s_axil_ctrl_bresp) {
  }

 private:
  void read_done(simbricks::AXILOperationR &axi_op) final {
    // TODO
  }

  void write_done(simbricks::AXILOperationW &axi_op) final {
    // TODO
  }
};

// handles network interfacing AXI stream interface
using AxiSFromNetworkT = simbricks::AXISManager<8, 32, 2048>;
using AxiSToNetworkT = simbricks::AXISSubordinate<8, 2048>;

/* **************************************************************************
 * H2D handling methods
 * ************************************************************************** */

void h2d_read(volatile struct SimbricksProtoPcieH2DRead &read, uint64_t cur_ts,
              CorundumAXILManager &mmio) {
#if CORUNDUM_VERILATOR_DEBUG
  sim_log::LogInfo("h2d_read ts=%lu bar=%d offset=%lu len=%lu\n", cur_ts,
                   static_cast<int>(read.bar),
                   static_cast<uint64_t>(read.offset),
                   static_cast<uint64_t>(read.len));
#endif

  if (read.bar != 0) {
    sim_log::LogError("write to unexpected bar=%d", static_cast<int>(read.bar));
    std::terminate();
  }

  mmio.issue_read(read.req_id, read.offset);
}

void h2d_write(struct SimbricksNicIf &nicif,
               volatile struct SimbricksProtoPcieH2DWrite &write,
               uint64_t cur_ts, bool posted, CorundumAXILManager &mmio) {
#if CORUNDUM_VERILATOR_DEBUG
  sim_log::LogInfo("h2d_write ts=%ul bar=%d offset=%ul len=%ul\n", cur_ts,
                   static_cast<int>(write.bar),
                   static_cast<uint64_t>(write.offset),
                   static_cast<uint64_t>(write.len));
#endif

  if (write.bar != 0) {
    sim_log::LogError("write to unexpected bar=%d", write.bar);
    std::terminate();
  }

  if (write.len > 8) {
    sim_log::LogError(
        "h2d_write() JPEG decoder register write needs to be 32 bits\n");
    std::terminate();
  }

  uint64_t data;
  std::memcpy(&data, const_cast<uint8_t *>(write.data), write.len);
  mmio.issue_write(write.req_id, write.offset, data, posted);

  if (!posted) {
    volatile union SimbricksProtoPcieD2H *msg = d2h_alloc(nicif, cur_ts);
    volatile struct SimbricksProtoPcieD2HWritecomp &writecomp = msg->writecomp;
    writecomp.req_id = write.req_id;

    SimbricksPcieIfD2HOutSend(&nicif.pcie, msg,
                              SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITECOMP);
  }
  return;
}

void h2d_readcomp(volatile struct SimbricksProtoPcieH2DReadcomp &readcomp,
                  uint64_t cur_ts, CorundumAXISubordinateRead &dma_read) {
  dma_read.read_done(readcomp.req_id, const_cast<uint8_t *>(readcomp.data));
}

void h2d_writecomp(volatile struct SimbricksProtoPcieH2DWritecomp &writecomp,
                   uint64_t cur_ts, CorundumAXISubordinateWrite &dma_write) {
  dma_write.write_done(writecomp.req_id);
}

void poll_h2d(struct SimbricksNicIf &nicif, uint64_t cur_ts,
              CorundumAXISubordinateRead &dma_read,
              CorundumAXISubordinateWrite &dma_write,
              CorundumAXILManager &mmio) {
  volatile union SimbricksProtoPcieH2D *msg =
      SimbricksPcieIfH2DInPoll(&nicif.pcie, cur_ts);

  if (msg == NULL) {
#ifdef CORUNDUM_VERILATOR_DEBUG
    sim_log::LogWarn("poll_h2d msg NULL\n");
#endif
    return;
  }

  uint8_t type = SimbricksPcieIfH2DInType(&nicif.pcie, msg);
  switch (type) {
    case SIMBRICKS_PROTO_PCIE_H2D_MSG_READ:
      h2d_read(msg->read, cur_ts, mmio);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE:
      h2d_write(nicif, msg->write, cur_ts, false, mmio);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITE_POSTED:
      h2d_write(nicif, msg->write, cur_ts, true, mmio);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_READCOMP:
      h2d_readcomp(msg->readcomp, cur_ts, dma_read);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITECOMP:
      h2d_writecomp(msg->writecomp, cur_ts, dma_write);
      break;

    case SIMBRICKS_PROTO_PCIE_H2D_MSG_DEVCTRL:
    case SIMBRICKS_PROTO_MSG_TYPE_SYNC:
      break;

    case SIMBRICKS_PROTO_MSG_TYPE_TERMINATE:
      sim_log::LogError("poll_h2d: peer terminated\n");
      break;

    default:
      sim_log::LogError("poll_h2d: unsupported type=%d", type);
  }

  SimbricksPcieIfH2DInDone(&nicif.pcie, msg);
}

/* **************************************************************************
 * N2D handling methods
 * ************************************************************************** */

void n2d_recv(volatile struct SimbricksProtoNetMsgPacket &packet,
              uint64_t cur_ts, AxiSFromNetworkT &axis_from_network) {
  if (axis_from_network.full()) {
#ifdef CORUNDUM_VERILATOR_DEBUG
    sim_log::LogError("corundum verilator n2d_recv: dropping packet\n");
#endif
    return;
  }

  // TODO: FIXME
  const void *data = (const void *)packet.data;
  axis_from_network.read(static_cast<const uint8_t *>(data), packet.len);

#ifdef CORUNDUM_VERILATOR_DEBUG
  sim_log::LogInfo("%lu corundum verilator n2d_recv received packet\n", cur_ts);
#endif
}

void poll_n2d(struct SimbricksNicIf &nicif, uint64_t cur_ts,
              AxiSFromNetworkT &axis_from_network) {
  volatile union SimbricksProtoNetMsg *msg =
      SimbricksNetIfInPoll(&nicif.net, cur_ts);

  if (msg == NULL) {
#ifdef CORUNDUM_VERILATOR_DEBUG
    sim_log::LogWarn("poll_n2d msg NULL\n");
#endif
    return;
  }

  uint8_t type = SimbricksNetIfInType(&nicif.net, msg);
  switch (type) {
    case SIMBRICKS_PROTO_NET_MSG_PACKET:
      n2d_recv(msg->packet, cur_ts, axis_from_network);
      break;

    case SIMBRICKS_PROTO_MSG_TYPE_SYNC:
      break;

    default:
      sim_log::LogError("poll_n2d: unsupported type=%d\n", type);
  }

  SimbricksNetIfInDone(&nicif.net, msg);
}

/* **************************************************************************
 * signal handling
 * ************************************************************************** */

static uint64_t main_time = 0;
static volatile int exiting = 0;
static void sigint_handler(int dummy) {
  exiting = 1;
}
static void sigusr1_handler(int dummy) {
  sim_log::LogError("main_time = %lu\n", main_time);
}

/* **************************************************************************
 * main adapter driver
 * ************************************************************************** */

int main(int argc, char *argv[]) {
  // declarations
  Vmqnic_core_axi top_verilator_interface;
  // VerilatedVcdC trace;
  AxiSFromNetworkT axis_from_network{
      top_verilator_interface.s_axis_rx_tvalid,
      top_verilator_interface.s_axis_rx_tready,
      reinterpret_cast<uint8_t *>(&top_verilator_interface.s_axis_rx_tdata),
      &top_verilator_interface.s_axis_rx_tkeep,
      top_verilator_interface.s_axis_rx_tlast,
      reinterpret_cast<uint8_t *>(&top_verilator_interface.s_axis_rx_tuser)};

  AxiSToNetworkT axis_to_network{
      top_verilator_interface.m_axis_tx_tvalid,
      top_verilator_interface.m_axis_tx_tready,
      reinterpret_cast<uint8_t *>(&top_verilator_interface.m_axis_tx_tdata),
      &top_verilator_interface.m_axis_tx_tkeep,
      top_verilator_interface.m_axis_tx_tlast,
      &top_verilator_interface.m_axis_tx_tuser};

  CorundumAXISubordinateRead dma_read{top_verilator_interface};
  CorundumAXISubordinateWrite dma_write{top_verilator_interface};
  CorundumAXILManager mmio{top_verilator_interface};

  struct SimbricksBaseIfParams netParams;
  struct SimbricksBaseIfParams pcieParams;
  struct SimbricksNicIf nicif;
  struct SimbricksProtoPcieDevIntro di;
  uint64_t clock_period = 4 * 1000ULL;  // 4ns -> 250MHz

  // argument parsing and initialization
  if (argc < 4 || argc > 10) {
    sim_log::LogError(
        "Usage: corundum_simbricks_adapter PCI-SOCKET ETH-SOCKET "
        "SHM [SYNC-MODE] [START-TICK] [SYNC-PERIOD] [PCI-LATENCY] "
        "[ETH-LATENCY] [CLOCK-FREQ-MHZ]\n");
    return EXIT_FAILURE;
  }
  pcieParams.sock_path = argv[1];
  netParams.sock_path = argv[2];

  memset(&di, 0, sizeof(di));
  di.bars[0].len = 1 << 24;
  di.bars[0].flags = SIMBRICKS_PROTO_PCIE_BAR_64;
  di.pci_vendor_id = 0x5543;
  di.pci_device_id = 0x1001;
  di.pci_class = 0x02;
  di.pci_subclass = 0x00;
  di.pci_revision = 0x00;
  di.pci_msi_nvecs = 32;

  if (SimbricksNicIfInit(&nicif, argv[3], &netParams, &pcieParams, &di)) {
    sim_log::LogError("corundum simbricks adapter SimbricksNicIfInit failed\n");
    return EXIT_FAILURE;
  }
  if (argc >= 6) {
    main_time = strtoull(argv[5], NULL, 0);
  }
  if (argc >= 7) {
    uint64_t sync_interval = strtoull(argv[6], NULL, 0) * 1000ULL;
    netParams.sync_interval = sync_interval;
    pcieParams.sync_interval = sync_interval;
  }
  if (argc >= 8) {
    pcieParams.link_latency = strtoull(argv[7], NULL, 0) * 1000ULL;
  }
  if (argc >= 9) {
    netParams.link_latency = strtoull(argv[8], NULL, 0) * 1000ULL;
  }
  if (argc >= 10) {
    clock_period = 1000000ULL / strtoull(argv[9], NULL, 0);
  }

  bool sync_pci = SimbricksBaseIfSyncEnabled(&nicif.pcie.base);
  bool sync_eth = SimbricksBaseIfSyncEnabled(&nicif.net.base);
#ifdef CORUNDUM_VERILATOR_DEBUG
  sim_log::LogInfo("sync_pci=%d sync_eth=%d\n", sync_pci, sync_eth);
#endif

  signal(SIGINT, sigint_handler);
  signal(SIGUSR1, sigusr1_handler);

  // TODO: allow for verilator tracing
  // Verilated::traceEverOn(true);
  // top_verilator_interface.trace(&trace, 0);

  reset_corundum(top_verilator_interface);

  // main simulation loop
  while (not exiting) {
    bool done = false;
    do {
      if (SimbricksPcieIfD2HOutSync(&nicif.pcie, main_time) < 0) {
        sim_log::LogWarn("SimbricksPcieIfD2HOutSync failed (t=%lu)\n",
                         main_time);
        done = true;
      }
      if (SimbricksNetIfOutSync(&nicif.net, main_time) < 0) {
        sim_log::LogWarn("SimbricksNetIfOutSync failed (t=%lu)\n", main_time);
        done = true;
      }
    } while (not done);

    do {
      poll_h2d(nicif, main_time, dma_read, dma_write, mmio);
      poll_n2d(nicif, main_time, axis_from_network);
    } while (
        not exiting and
        ((sync_pci and
          SimbricksPcieIfH2DInTimestamp(&nicif.pcie) <= main_time) or
         (sync_eth and SimbricksNetIfInTimestamp(&nicif.net) <= main_time)));

    /* falling edge */
    top_verilator_interface.clk = 0;
    main_time += clock_period / 2;

    top_verilator_interface.eval();

    // evaluate on rising edge
    dma_read.step(main_time);
    dma_write.step(main_time);
    mmio.step(main_time);
    axis_from_network.step();
    axis_to_network.step();
    top_verilator_interface.clk = 1;
    main_time += clock_period / 2;

    top_verilator_interface.eval();

    // finalize updates
    dma_read.step_apply();
    dma_write.step_apply();
    mmio.step_apply();
  }

  top_verilator_interface.final();

#ifdef CORUNDUM_VERILATOR_DEBUG
  sim_log::LogInfo("corundum simbricks adapter finished\n");
#endif
  return EXIT_SUCCESS;
}