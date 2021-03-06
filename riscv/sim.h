// See LICENSE for license details.

#ifndef _RISCV_SIM_H
#define _RISCV_SIM_H

#include "processor.h"
#include "devices.h"
#include "debug_module.h"
#include "simif.h"
#include <fesvr/htif.h>
#include <fesvr/context.h>
#include <io/io.h>
#include <vector>
#include <map>
#include <string>
#include <memory>

class mmu_t;
class remote_bitbang_t;

// this class encapsulates the processors and memory in a RISC-V machine.
class sim_t : public htif_t, public simif_t, public io::tinycon
{
public:
  sim_t(const char* isa, size_t _nprocs,  bool halted, reg_t start_pc,
        std::vector<std::pair<reg_t, mem_t*>> mems,
        const std::vector<std::string>& args, const std::vector<int> hartids,
        unsigned progsize, unsigned max_bus_master_bits, bool require_authentication);
  ~sim_t();

  // run the simulation to completion
  int run();
  void set_debug(bool value);
  void set_exit_debug(bool value);
  void set_trace(const char *tconfig, const char *outdir=nullptr);
  void set_intermittent(bool value);
  void set_maxvl(uint32_t vl);
  void set_segmented(uint32_t base, uint32_t size);
  void set_log(bool value);
  void set_histogram(bool value);
  void set_procs_debug(bool value);
  void set_remote_bitbang(remote_bitbang_t* remote_bitbang) {
    this->remote_bitbang = remote_bitbang;
  }
  const char* get_dts() { if (dts.empty()) reset(); return dts.c_str(); }
  processor_t* get_core(size_t i) { return procs.at(i); }
  unsigned nprocs() const { return procs.size(); }

  // Callback for processors to let the simulation know they were reset.
  void proc_reset(unsigned id);
  void hard_reset();

  // Sim Calls
  void mark(addr_t addr, size_t len, size_t tag);
  void unmark(addr_t addr, size_t len, size_t tag);
  void trace(void);
  void trace_roi(addr_t start_pc, addr_t end_pc=0);
  void stop_trace(void);
  void enable_intermittent(void);
  void disable_intermittent(void);

private:
  std::vector<std::pair<reg_t, mem_t*>> mems;
  mmu_t* debug_mmu;  // debug port into main memory
  std::vector<processor_t*> procs;
  reg_t start_pc;
  std::string dts;
  std::unique_ptr<rom_device_t> boot_rom;
  std::unique_ptr<clint_t> clint;
  bus_t bus;

  processor_t* get_core(const std::string& i);
  void step(size_t n); // step through simulation
  static const size_t INTERLEAVE = 5000;
  static const size_t INSNS_PER_RTC_TICK = 100; // 10 MHz clock for 1 BIPS core
  static const size_t CPU_HZ = 1000000000; // 1GHz CPU
  static const size_t INTERMITTENT_MAX = 350000;
  static const size_t INTERMITTENT_MIN = 150000;
  size_t current_step;
  size_t current_proc;
  bool debug;
  bool exit_debug;
  bool log;
  bool histogram_enabled; // provide a histogram of PCs
  bool intermittent;
  remote_bitbang_t* remote_bitbang;

  // Segmented memory support
  bool segmented = false;
  uint32_t segment_base = 0;
  uint32_t segment_size = 0;

  // memory-mapped I/O routines
  char* addr_to_mem(reg_t addr);
  bool mmio_load(reg_t addr, size_t len, uint8_t* bytes);
  bool mmio_store(reg_t addr, size_t len, const uint8_t* bytes);
  void make_dtb();

  // presents a prompt for introspection into the simulation
  typedef void (sim_t::*inter_func_t)(const std::string&, const std::vector<std::string>&);
  std::map<std::string, inter_func_t> inter_funcs;
  int trigger(std::string s);
  void setup_interactive();

  // functions that help implement interactive()
  void interactive_help(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_quit(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_run(const std::string& cmd, const std::vector<std::string>& args, bool noisy);
  void interactive_run_noisy(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_run_silent(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_step(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_reverse(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_reg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_vreg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_freg(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_fregs(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_fregd(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_pc(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_read(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_write(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_str(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_until(const std::string& cmd, const std::vector<std::string>& args);
  void interactive_count(const std::string &cmd, const std::vector<std::string>& args);
  void interactive_reset(const std::string &cmd, const std::vector<std::string>& args);
  reg_t get_reg(const std::vector<std::string>& args);
  freg_t get_freg(const std::vector<std::string>& args);
  reg_t get_mem(const std::vector<std::string>& args);
  reg_t get_pc(const std::vector<std::string>& args);
  void set_mem(const std::vector<std::string>& args);

  friend class processor_t;
  friend class mmu_t;
  friend class debug_module_t;

  // htif
  friend void sim_thread_main(void*);
  void main();

  context_t* host;
  context_t target;
  void reset();
  void reverse(size_t n);
  void reverse(size_t n, size_t p); // p is processor
  void idle();
  void read_chunk(addr_t taddr, size_t len, void* dst);
  void write_chunk(addr_t taddr, size_t len, const void* src);
  size_t chunk_align() { return 8; }
  size_t chunk_max_size() { return 8; }

public:
  // Initialize this after procs, because in debug_module_t::reset() we
  // enumerate processors, which segfaults if procs hasn't been initialized
  // yet.
  debug_module_t debug_module;
};

extern volatile bool ctrlc_pressed;

#endif
