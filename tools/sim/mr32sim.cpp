//--------------------------------------------------------------------------------------------------
// Copyright (c) 2018 Marcus Geelnard
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the
// authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial
// applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not claim that you wrote
//     the original software. If you use this software in a product, an acknowledgment in the
//     product documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be misrepresented as
//     being the original software.
//
//  3. This notice may not be removed or altered from any source distribution.
//--------------------------------------------------------------------------------------------------

#include "config.hpp"
#include "cpu_simple.hpp"
#include "ram.hpp"

#ifdef ENABLE_GUI
#include <glad/glad.h>
// Note: Keep this comment to convince clang-format to include glad.h before glfw3.h.
#include <GLFW/glfw3.h>
#include "gpu.hpp"
#endif

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>

namespace {
void read_bin_file(const char* file_name,
                   ram_t& ram,
                   const bool override_addr,
                   const uint32_t addr) {
  std::ifstream f(file_name, std::fstream::in | std::fstream::binary);
  if (f.bad()) {
    throw std::runtime_error("Unable to open the binary file.");
  }

  // Read the start address.
  uint32_t start_addr;
  if (!override_addr) {
    f.read(reinterpret_cast<char*>(&start_addr), 4);
    if (!f.good()) {
      throw std::runtime_error("Premature end of file.");
    }
  } else {
    start_addr = addr;
  }

  // Read blocks from the file into RAM.
  uint32_t current_addr = start_addr;
  uint32_t total_bytes_read = 0u;
  while (f.good()) {
    uint8_t byte;
    f.read(reinterpret_cast<char*>(&byte), 1);
    ram.store8(current_addr, byte);
    const uint32_t bytes_read = f ? 1 : static_cast<uint32_t>(f.gcount());
    total_bytes_read += bytes_read;
    current_addr += bytes_read;
  }

  f.close();
  if (config_t::instance().verbose()) {
    std::cout << "Read " << total_bytes_read << " bytes from " << file_name << " into RAM @ 0x"
              << std::hex << std::setw(8) << std::setfill('0') << start_addr << "\n";
    std::cout << std::resetiosflags(std::ios::hex);
  }
}

uint64_t str_to_uint64(const char* str) {
  return static_cast<uint64_t>(std::stoull(std::string(str), nullptr, 0));
}

int64_t str_to_int64(const char* str) {
  return static_cast<int64_t>(str_to_uint64(str));
}

uint32_t str_to_uint32(const char* str) {
  return static_cast<uint32_t>(str_to_uint64(str));
}

void print_help(const char* prg_name) {
  std::cout << "mr32sim - An MRISC32 CPU simulator\n";
  std::cout << "Usage: " << prg_name << " [options] bin-file\n";
  std::cout << "Options:\n";
  std::cout << "  -h, --help                       Display this information.\n";
  std::cout << "  -v, --verbose                    Print stats.\n";
  std::cout << "  -g, --gfx                        Enable graphics.\n";
  std::cout << "  -ga ADDR, --gfx-addr ADDR        Set framebuffer address.\n";
  std::cout << "  -gw WIDTH, --gfx-width WIDTH     Set framebuffer width.\n";
  std::cout << "  -gh HEIGHT, --gfx-height HEIGHT  Set framebuffer height.\n";
  std::cout << "  -gd DEPTH, --gfx-depth DEPTH     Set framebuffer depht.\n";
  std::cout << "  -t FILE, --trace FILE            Enable debug trace.\n";
  std::cout << "  -R N, --ram-size N               Set the RAM size (in bytes).\n";
  std::cout << "  -A ADDR, --addr ADDR             Set the program (ROM) start address.\n";
  std::cout << "  -c CYCLES, --cycles CYCLES       Maximum number of CPU cycles to simulate.\n";
  return;
}
}  // namespace

int main(const int argc, const char** argv) {
  // Parse command line options.
  // TODO(m): Add options for graphics (e.g. framebuffer size).
  const auto* bin_file = static_cast<const char*>(0);
  uint32_t bin_addr = 0u;
  int64_t max_cycles = -1;
  bool bin_addr_defined = false;
  try {
    for (int k = 1; k < argc; ++k) {
      if (argv[k][0] == '-') {
        if ((std::strcmp(argv[k], "--help") == 0) || (std::strcmp(argv[k], "-h") == 0) ||
            (std::strcmp(argv[k], "-?") == 0)) {
          print_help(argv[0]);
          exit(0);
        } else if ((std::strcmp(argv[k], "-v") == 0) || (std::strcmp(argv[k], "--verbose") == 0)) {
          config_t::instance().set_verbose(true);
        } else if ((std::strcmp(argv[k], "-g") == 0) || (std::strcmp(argv[k], "--gfx") == 0)) {
          config_t::instance().set_gfx_enabled(true);
        } else if ((std::strcmp(argv[k], "-ga") == 0) || (std::strcmp(argv[k], "--gfx-addr") == 0)) {
          if (k >= (argc - 1)) {
            std::cerr << "Missing option for " << argv[k] << "\n";
            print_help(argv[0]);
            exit(1);
          }
          config_t::instance().set_gfx_addr(str_to_uint32(argv[++k]));
        } else if ((std::strcmp(argv[k], "-gw") == 0) || (std::strcmp(argv[k], "--gfx-width") == 0)) {
          if (k >= (argc - 1)) {
            std::cerr << "Missing option for " << argv[k] << "\n";
            print_help(argv[0]);
            exit(1);
          }
          config_t::instance().set_gfx_width(str_to_uint32(argv[++k]));
        } else if ((std::strcmp(argv[k], "-gh") == 0) || (std::strcmp(argv[k], "--gfx-height") == 0)) {
          if (k >= (argc - 1)) {
            std::cerr << "Missing option for " << argv[k] << "\n";
            print_help(argv[0]);
            exit(1);
          }
          config_t::instance().set_gfx_height(str_to_uint32(argv[++k]));
        } else if ((std::strcmp(argv[k], "-gd") == 0) || (std::strcmp(argv[k], "--gfx-depth") == 0)) {
          if (k >= (argc - 1)) {
            std::cerr << "Missing option for " << argv[k] << "\n";
            print_help(argv[0]);
            exit(1);
          }
          config_t::instance().set_gfx_depth(str_to_uint32(argv[++k]));
        } else if ((std::strcmp(argv[k], "-t") == 0) || (std::strcmp(argv[k], "--trace") == 0)) {
          if (k >= (argc - 1)) {
            std::cerr << "Missing option for " << argv[k] << "\n";
            print_help(argv[0]);
            exit(1);
          }
          config_t::instance().set_trace_file_name(std::string(argv[++k]));
          config_t::instance().set_trace_enabled(true);
        } else if ((std::strcmp(argv[k], "-R") == 0) || (std::strcmp(argv[k], "--ram-size") == 0)) {
          if (k >= (argc - 1)) {
            std::cerr << "Missing option for " << argv[k] << "\n";
            print_help(argv[0]);
            exit(1);
          }
          config_t::instance().set_ram_size(str_to_uint64(argv[++k]));
        } else if ((std::strcmp(argv[k], "-A") == 0) || (std::strcmp(argv[k], "--addr") == 0)) {
          if (k >= (argc - 1)) {
            std::cerr << "Missing option for " << argv[k] << "\n";
            print_help(argv[0]);
            exit(1);
          }
          bin_addr = str_to_uint32(argv[++k]);
          bin_addr_defined = true;
        } else if ((std::strcmp(argv[k], "-c") == 0) || (std::strcmp(argv[k], "--cycles") == 0)) {
          if (k >= (argc - 1)) {
            std::cerr << "Missing option for " << argv[k] << "\n";
            print_help(argv[0]);
            exit(1);
          }
          max_cycles = str_to_int64(argv[++k]);
        } else {
          std::cerr << "Error: Unknown option: " << argv[k] << "\n";
          print_help(argv[0]);
          exit(1);
        }
      } else if (bin_file == static_cast<const char*>(0)) {
        bin_file = argv[k];
      } else {
        std::cerr << "Error: Only a single program file can be loaded.\n";
        print_help(argv[0]);
        exit(1);
      }
    }
  } catch (...) {
    std::cerr << "Error: Couldn't parse command line arguments.\n";
    print_help(argv[0]);
    exit(1);
  }
  if (bin_file == static_cast<const char*>(0)) {
    std::cerr << "Error: No program file specified.\n";
    print_help(argv[0]);
    std::exit(1);
  }

  try {
    // Initialize the RAM.
    ram_t ram(config_t::instance().ram_size());

    // Load the program file into RAM.
    read_bin_file(bin_file, ram, bin_addr_defined, bin_addr);

    // HACK: Populate MMIO memory with MC1 fields.
    const uint32_t MMIO_START = 0xc0000000u;
    if (config_t::instance().ram_size() >= (MMIO_START + 64)) {
      ram.store32(MMIO_START + 8, 70000000);     // CPUCLK
      ram.store32(MMIO_START + 12, 128 * 1024);  // VRAMSIZE
      ram.store32(MMIO_START + 20, 1920);        // VIDWIDTH
      ram.store32(MMIO_START + 24, 1080);        // VIDHEIGHT
      ram.store32(MMIO_START + 28, 60 * 65536);  // VIDFPS
      ram.store32(MMIO_START + 40, 4);           // SWITCHES
    }

    // Initialize the CPU.
    cpu_simple_t cpu(ram);

    if (config_t::instance().verbose()) {
      std::cout << "------------------------------------------------------------------------\n";
    }

    // Run the CPU in a separate thread.
    std::atomic_bool cpu_done(false);
    uint32_t cpu_exit_code = 0u;
    std::thread cpu_thread([&cpu_exit_code, &cpu, &cpu_done, max_cycles] {
      try {
        // Run until the program returns.
        cpu_exit_code = cpu.run(max_cycles);
      } catch (std::exception& e) {
        std::cerr << "Exception in CPU thread: " << e.what() << "\n";
        cpu_exit_code = 1u;
      }
      cpu_done = true;
    });

#ifdef ENABLE_GUI
    if (config_t::instance().gfx_enabled()) {
      try {
        // Initialize GLFW.
        if (glfwInit() != GLFW_TRUE) {
          throw std::runtime_error("Unable to initialize GLFW.");
        }

        // We want the display to be 24-bit RGB.
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, GLFW_DONT_CARE);
        glfwWindowHint(GLFW_DEPTH_BITS, GLFW_DONT_CARE);
        glfwWindowHint(GLFW_STENCIL_BITS, GLFW_DONT_CARE);

        // The GL context should support the 3.2 core profile (forward compatible).
        // This ensures that we get a modern GL context on macOS.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create a GLFW window.
        auto window_width = config_t::instance().gfx_width();
        auto window_height = config_t::instance().gfx_height();
        auto* window = glfwCreateWindow(static_cast<int>(window_width),
                                        static_cast<int>(window_height),
                                        "MRISC32 Simulator",
                                        nullptr,
                                        nullptr);
        if (window != nullptr) {
          glfwMakeContextCurrent(window);

          // Initialize GLAD.
          if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Unable to initialize GLAD.");
          }
          if (config_t::instance().verbose()) {
            std::cerr << "OpenGL version: " << GLVersion.major << "." << GLVersion.minor << "\n";
          }

          // Init the "GPU".
          gpu_t gpu(ram);

          // Enable vsync.
          glfwSwapInterval(1);

          // Main loop.
          bool simulation_finished = false;
          uint32_t frame_no = 0;
          while (!glfwWindowShouldClose(window)) {
            // Update the video mode.
            gpu.configure();
            if (window_width != gpu.width() || window_height != gpu.height()) {
              window_width = gpu.width();
              window_height = gpu.height();
              glfwSetWindowSize(
                  window, static_cast<int>(window_width), static_cast<int>(window_height));
            }

            // Update the frame number (MC1 compat).
            ram.store32(0xc0000020, frame_no);
            frame_no += 1u;

            // Get the actual window framebuffer size (note: this is important on systems that use
            // coordinate scaling, such as on macos with retina display).
            int actual_fb_width;
            int actual_fb_height;
            glfwGetFramebufferSize(window, &actual_fb_width, &actual_fb_height);

            // Paint the CPU RAM framebuffer contents to the window.
            gpu.paint(actual_fb_width, actual_fb_height);

            // Swap front/back buffers and poll window events.
            glfwSwapBuffers(window);
            glfwPollEvents();

            // Simulation finished?
            if (cpu_done && !simulation_finished) {
              glfwSetWindowTitle(window, "MRISC32 Simulator - Finished");
              simulation_finished = true;
            }

            // ESC pressed?
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
              glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
          }

          // Clean up GPU resources before we close the window.
          gpu.cleanup();

          // Close the window.
          glfwDestroyWindow(window);
          glfwTerminate();
        }
      } catch (std::exception& e) {
        std::cerr << "Graphics error: " << e.what() << "\n";
      }

      cpu.terminate();
    }
#endif  // ENABLE_GUI

    // Wait for the cpu thread to finish.
    cpu_thread.join();
    const int exit_code = static_cast<int>(cpu_exit_code);

    if (config_t::instance().verbose()) {
      // Show some stats.
      std::cout << "------------------------------------------------------------------------\n";
      std::cout << "Exit code: " << exit_code << "\n";
      cpu.dump_stats();
    }

    // Dump some RAM (we use the same range as the MC1 VRAM).
    cpu.dump_ram(0x40000000u, 0x40040000u, "/tmp/mrisc32_sim_vram.bin");

    std::exit(exit_code);
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    std::exit(1);
  }
}
