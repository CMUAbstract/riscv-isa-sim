// See LICENSE for license details.

#include "disasm.h"
#include "sim.h"
#include "mmu.h"
#include <sys/mman.h>
#include <termios.h>
#include <map>
#include <iostream>
#include <climits>
#include <cinttypes>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
#include <sstream>

#include <common/decode.h>

DECLARE_TRAP(-1, interactive)

processor_t *sim_t::get_core(const std::string& i)
{
  char *ptr;
  unsigned long p = strtoul(i.c_str(), &ptr, 10);
  if (*ptr || p >= procs.size())
    throw trap_interactive();
  return get_core(p);
}

int sim_t::trigger(std::string s) {
  if(s.size() == 0) {
    set_procs_debug(true);
    step(1);
    return 0;
  }
  std::istringstream iss(s);
  std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
    std::istream_iterator<std::string>{}};
  std::string cmd = tokens[0];
  std::vector<std::string> args(tokens.begin() + 1, tokens.end());
  std::vector<std::string> disabled = {"run", "rs", "r"};
  try {
    if(exit_debug && inter_funcs.count(cmd) && 
        std::find(disabled.begin(), disabled.end(), cmd) == disabled.end()) {
      (this->*inter_funcs[cmd])(cmd, args);
    } else if(!exit_debug && inter_funcs.count(cmd)) {
      (this->*inter_funcs[cmd])(cmd, args);
    } else {
      fprintf(stderr, "Unknown command %s\n", cmd.c_str());
    }
  } catch(trap_t &t) {}
  ctrlc_pressed = false;
  return 0;
}

void sim_t::setup_interactive()
{
  inter_funcs["run"] = &sim_t::interactive_run_noisy;
  inter_funcs["r"] = inter_funcs["run"];
  inter_funcs["rs"] = &sim_t::interactive_run_silent;
  inter_funcs["step"] = &sim_t::interactive_step;
  inter_funcs["reverse"] = &sim_t::interactive_reverse;
  inter_funcs["reg"] = &sim_t::interactive_reg;
  inter_funcs["vreg"] = &sim_t::interactive_vreg;
  inter_funcs["freg"] = &sim_t::interactive_freg;
  inter_funcs["fregs"] = &sim_t::interactive_fregs;
  inter_funcs["fregd"] = &sim_t::interactive_fregd;
  inter_funcs["pc"] = &sim_t::interactive_pc;
  inter_funcs["read"] = &sim_t::interactive_read;
  inter_funcs["write"] = &sim_t::interactive_write;
  inter_funcs["str"] = &sim_t::interactive_str;
  inter_funcs["until"] = &sim_t::interactive_until;
  inter_funcs["while"] = &sim_t::interactive_until;
  inter_funcs["quit"] = &sim_t::interactive_quit;
  inter_funcs["q"] = inter_funcs["quit"];
  inter_funcs["help"] = &sim_t::interactive_help;
  inter_funcs["count"] = &sim_t::interactive_count;
  inter_funcs["reset"] = &sim_t::interactive_reset;
  inter_funcs["h"] = inter_funcs["help"];
}

void sim_t::interactive_help(const std::string& cmd, const std::vector<std::string>& args)
{
  std::cerr <<
    "Interactive commands:\n"
    "reg <core> [reg]                # Display [reg] (all if omitted) in <core>\n"
    "vreg <core> [reg]               # Display [vreg] (all if omitted) in <core>\n"
    "fregs <core> <reg>              # Display single precision <reg> in <core>\n"
    "fregd <core> <reg>              # Display double precision <reg> in <core>\n"
    "pc [core]                       # Show current PC in [core]\n"
    "read <hex addr>                 # Show contents of physical memory\n"
    "write <hex addr> <value>        # Write contents of physical memory\n"
    "str <hex addr>                  # Show NUL-terminated C string\n"
    "until reg <core> <reg> <val>    # Stop when <reg> in <core> hits <val>\n"
    "until pc <core> <val>           # Stop when PC in <core> hits <val>\n"
    "until mem <addr> <val>          # Stop when memory <addr> becomes <val>\n"
    "while reg <core> <reg> <val>    # Run while <reg> in <core> is <val>\n"
    "while pc <core> <val>           # Run while PC in <core> is <val>\n"
    "while mem <addr> <val>          # Run while memory <addr> is <val>\n"
    "run [count]                     # Resume noisy execution (until CTRL+C, or [count] insns)\n"
    "r [count]                         Alias for run\n"
    "rs [count]                      # Resume silent execution (until CTRL+C, or [count] insns)\n"
    "step [count]                    # Resume noisy execution (until [count] insns)\n"
    "reverse [count]                 # Reverse noisy execution (until [count] insns)\n"
    "count <core>                    # Print instruction count\n"
    "reset                           # Reset processes\n"
    "quit                            # End the simulation\n"
    "q                                 Alias for quit\n"
    "help                            # This screen!\n"
    "h                                 Alias for help\n"
    "Note: Hitting enter is the same as: run 1\n"
    << std::flush;
}

void sim_t::interactive_run_noisy(const std::string& cmd, const std::vector<std::string>& args)
{
  interactive_run(cmd,args,true);
}

void sim_t::interactive_run_silent(const std::string& cmd, const std::vector<std::string>& args)
{
  interactive_run(cmd,args,false);
}

void sim_t::interactive_run(const std::string& cmd, const std::vector<std::string>& args, bool noisy)
{
  size_t steps = args.size() ? atoll(args[0].c_str()) : -1;
  ctrlc_pressed = false;
  set_procs_debug(noisy);
  for (size_t i = 0; i < steps && !ctrlc_pressed && !done(); i++)
    step(1);
}

void sim_t::interactive_step(const std::string& cmd, const std::vector<std::string>& args) {
  std::vector<std::string> processed_args(args);
  if(!processed_args.size()) {
    processed_args.push_back("1");
  }
  interactive_run(cmd, processed_args, true);
}

void sim_t::interactive_reverse(const std::string& cmd, const std::vector<std::string>& args) {
  if(args.size() == 1) {
    reverse(atoll(args[0].c_str())); 
  } else if(args.size() == 2) {
    reverse(atoll(args[0].c_str()), atoll(args[1].c_str()));
  } else {
    reverse(1);
  }
}

void sim_t::interactive_quit(const std::string& cmd, const std::vector<std::string>& args)
{
  exit(0);
}

reg_t sim_t::get_pc(const std::vector<std::string>& args)
{
  if(args.size() != 1)
    throw trap_interactive();

  processor_t *p = get_core(args[0]);
  return p->get_state()->pc;
}

void sim_t::interactive_pc(const std::string& cmd, const std::vector<std::string>& args)
{
  fprintf(stderr, "0x%016" PRIx64 "\n", get_pc(args));
}

reg_t sim_t::get_reg(const std::vector<std::string>& args)
{
  if(args.size() != 2)
    throw trap_interactive();

  processor_t *p = get_core(args[0]);

  unsigned long r = std::find(xpr_name, xpr_name + NXPR, args[1]) - xpr_name;
  if (r == NXPR) {
    char *ptr;
    r = strtoul(args[1].c_str(), &ptr, 10);
    if (*ptr) {
      #define DECLARE_CSR(name, number) if (args[1] == #name) return p->get_csr(number);
      #include <common/encoding.h>              // generates if's for all csrs
      r = NXPR;                          // else case (csr name not found)
      #undef DECLARE_CSR
    }
  }

  if (r >= NXPR)
    throw trap_interactive();

  return p->get_state()->XPR[r];
}

freg_t sim_t::get_freg(const std::vector<std::string>& args)
{
  if(args.size() != 2)
    throw trap_interactive();

  processor_t *p = get_core(args[0]);
  int r = std::find(fpr_name, fpr_name + NFPR, args[1]) - fpr_name;
  if (r == NFPR)
    r = atoi(args[1].c_str());
  if (r >= NFPR)
    throw trap_interactive();

  return p->get_state()->FPR[r];
}

void sim_t::interactive_reg(const std::string& cmd, const std::vector<std::string>& args)
{
  if (args.size() == 1) {
    // Show all the regs!
    processor_t *p = get_core(args[0]);

    for (int r = 0; r < NXPR; ++r) {
      fprintf(stderr, "%-4s: 0x%016" PRIx64 "  ", xpr_name[r], p->get_state()->XPR[r]);
      if ((r + 1) % 4 == 0)
        fprintf(stderr, "\n");
    }
  } else
    fprintf(stderr, "0x%016" PRIx64 "\n", get_reg(args));
}

void sim_t::interactive_vreg(const std::string& cmd, const std::vector<std::string>& args) {
  if (args.size() == 1) {
    processor_t *p = get_core(args[0]);

    for (int r = 0; r < NVECR; ++r) {
      fprintf(stderr, "%-8d     ", r);
    }
    fprintf(stderr, "\n");
    for (uint32_t l = 0; l < p->get_maxvl(); ++l) {
      for (int r = 0; r < NVECR; ++r) {
        fprintf(stderr, "0x%08" PRIx32 "   ", (int)p->get_state()->VPR.read(r, l));
      }
      fprintf(stderr, "\n");
    }
  } else if (args.size() == 2) {
    processor_t *p = get_core(args[0]);
    int r = atoi(args[1].c_str());
    if (r > NVECR) trap_interactive();
    for (uint32_t l = 0; l < p->get_maxvl(); ++l) {
      fprintf(stderr, "0x%08"  PRIx32 "   ", (int)p->get_state()->VPR.read(r, l));
      if(((l + 1) % 8) == 0) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
  }
}

union fpr
{
  freg_t r;
  float s;
  double d;
};

void sim_t::interactive_freg(const std::string& cmd, const std::vector<std::string>& args)
{
  freg_t r = get_freg(args);
  fprintf(stderr, "0x%016" PRIx64 "%016" PRIx64 "\n", r.v[1], r.v[0]);
}

void sim_t::interactive_fregs(const std::string& cmd, const std::vector<std::string>& args)
{
  fpr f;
  f.r = get_freg(args);
  fprintf(stderr, "%g\n", isBoxedF32(f.r) ? (double)f.s : NAN);
}

void sim_t::interactive_fregd(const std::string& cmd, const std::vector<std::string>& args)
{
  fpr f;
  f.r = get_freg(args);
  fprintf(stderr, "%g\n", isBoxedF64(f.r) ? f.d : NAN);
}

reg_t sim_t::get_mem(const std::vector<std::string>& args)
{
  if(args.size() != 1 && args.size() != 2)
    throw trap_interactive();

  std::string addr_str = args[0];
  mmu_t* mmu = debug_mmu;
  if(args.size() == 2)
  {
    processor_t *p = get_core(args[0]);
    mmu = p->get_mmu();
    addr_str = args[1];
  }

  reg_t addr = strtol(addr_str.c_str(),NULL,16), val;
  if(addr == LONG_MAX)
    addr = strtoul(addr_str.c_str(),NULL,16);

  switch(addr % 8)
  {
    case 0:
      val = mmu->load_uint64(addr);
      break;
    case 4:
      val = mmu->load_uint32(addr);
      break;
    case 2:
    case 6:
      val = mmu->load_uint16(addr);
      break;
    default:
      val = mmu->load_uint8(addr);
      break;
  }
  return val;
}

void sim_t::set_mem(const std::vector<std::string>& args)
{
  if(args.size() != 2 && args.size() != 3)
    throw trap_interactive();

  std::string addr_str = args[0];
  std::string val_str = args[1];
  mmu_t* mmu = debug_mmu;
  if(args.size() == 3)
  {
    processor_t *p = get_core(args[0]);
    mmu = p->get_mmu();
    addr_str = args[1];
    val_str = args[2];
  }

  reg_t addr = strtol(addr_str.c_str(),NULL,16);
  reg_t val = strtol(val_str.c_str(), NULL, 16);
  if(addr == LONG_MAX)
    addr = strtoul(addr_str.c_str(),NULL,16);

  fprintf(stderr, "0x%016" PRIx64 "\n", addr);
  switch(addr % 8)
  {
    case 0:
      mmu->store_uint64(addr, val);
      break;
    case 4:
      mmu->store_uint32(addr, val);
      break;
    case 2:
    case 6:
      mmu->store_uint16(addr, val);
      break;
    default:
      mmu->store_uint8(addr, val);
      break;
  }
}

void sim_t::interactive_read(const std::string& cmd, const std::vector<std::string>& args)
{
  fprintf(stderr, "0x%016" PRIx64 "\n", get_mem(args));
}

void sim_t::interactive_write(const std::string& cmd, const std::vector<std::string>& args) 
{
  set_mem(args);
}

void sim_t::interactive_str(const std::string& cmd, const std::vector<std::string>& args)
{
  if(args.size() != 1)
    throw trap_interactive();

  reg_t addr = strtol(args[0].c_str(),NULL,16);

  char ch;
  while((ch = debug_mmu->load_uint8(addr++)))
    putchar(ch);

  putchar('\n');
}

void sim_t::interactive_until(const std::string& cmd, const std::vector<std::string>& args)
{
  bool cmd_until = cmd == "until";

  if(args.size() < 3)
    return;

  reg_t val = strtol(args[args.size()-1].c_str(),NULL,16);
  if(val == LONG_MAX)
    val = strtoul(args[args.size()-1].c_str(),NULL,16);
  
  std::vector<std::string> args2;
  args2 = std::vector<std::string>(args.begin()+1,args.end()-1);

  auto func = args[0] == "reg" ? &sim_t::get_reg :
              args[0] == "pc"  ? &sim_t::get_pc :
              args[0] == "mem" ? &sim_t::get_mem :
              NULL;

  if (func == NULL)
    return;

  ctrlc_pressed = false;

  while (1)
  {
    try
    {
      reg_t current = (this->*func)(args2);

      if (cmd_until == (current == val))
        break;
      if (ctrlc_pressed)
        break;
    }
    catch (trap_t &t) {}

    set_procs_debug(false);
    step(1);
  }
}

void sim_t::interactive_count(const std::string& cmd, const std::vector<std::string>& args)
{
  std::string core = "0";
  if(args.size() == 1) {
    core = args[0];
  }
  processor_t *p = get_core(core);
  fprintf(stderr, "%lu\n", p->get_state()->minstret);
}

void sim_t::interactive_reset(const std::string& cmd, const std::vector<std::string>& args)
{
  hard_reset();
  fprintf(stderr, "Reset done\n");
}
