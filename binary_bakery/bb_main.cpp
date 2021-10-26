#include <chrono>
#include <span>

#include <binary_bakery_lib/config.h>
#include <binary_bakery_lib/file_tools.h>
#include <binary_bakery_lib/payload.h>

#if defined(_WIN32) || defined(WIN32)
#include <conio.h>
#endif

#include <fmt/format.h>

namespace bb {

   struct timer{
   private:
      std::chrono::time_point<std::chrono::high_resolution_clock> m_t0;
      std::string m_msg;

   public:
      explicit timer(const std::string& msg)
         : m_t0(std::chrono::high_resolution_clock::now())
         , m_msg(msg)
      {}
      ~timer()
      {
         const auto t1 = std::chrono::high_resolution_clock::now();
         const auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - m_t0).count();
         const double ms = us / 1000.0;
         fmt::print("{}: {}ms\n", m_msg, ms);
      }

      timer(const timer&) = delete;
      timer& operator=(const timer&) = delete;
      timer(timer&&) = delete;
      timer& operator=(timer&&) = delete;
   };


   // move this to config header?
   [[nodiscard]] auto get_config(
      const std::vector<file_path>& packing_files
   ) -> config
   {
      // First: Try to get the config from the directory of the first file
      if(packing_files.empty() == false)
      {
         const directory_path payload_dir{ packing_files[0].get_path().parent_path() };
         const std::optional<config> target_dir_cfg = get_cfg_from_dir(payload_dir);
         if (target_dir_cfg.has_value())
         {
            fmt::print("Using config from \"{}\".\n", payload_dir.get_path().string());
            return target_dir_cfg.value();
         }
      }

      // If that fails, try to get it from the working directory
      const directory_path working_dir{ fs::current_path() };
      const std::optional<config> working_dir_cfg = get_cfg_from_dir(working_dir);
      if (working_dir_cfg.has_value())
      {
         fmt::print("Using config from \"{}\".\n", working_dir.get_path().string());
         return working_dir_cfg.value();
      }


      // Lastly, use the default config
      config default_config;
      fmt::print("Using default config.\n");
      return default_config;
   }


   // This is a surprisingly difficult problem with no good universal solution.
   // For the moment, only do this on windows.
   auto wait_for_keypress() -> void
   {
#if defined(_WIN32) || defined(WIN32)
      fmt::print("\nPress any key to continue\n");
      [[maybe_unused]] const auto input_val = _getch();
#endif
   }

   // From the inputs, filter out all non-files and non-existing files. Also try to find a config
   // among the parameters
   struct input_files {
      std::vector<file_path> m_packing_files;
      std::optional<config> m_config;

      void addFile(std::filesystem::path const& filePath) {
        const file_path file{filePath};

        if (file.get_path().extension() == ".toml") {
          const std::optional<config> explicit_config = get_cfg_from_file(file);
          if (explicit_config.has_value()) {
            if (m_config.has_value()) {
              fmt::print(
                  "There was already a config among the input files. Ignoring "
                  "this one\n");
              return;
            }
            fmt::print("Using explicit config file \"{}\".\n",
                       file.get_path().string());
            m_config.emplace(explicit_config.value());
            return;
          }
        }

        auto constexpr skipExtensions = std::array{".h"};
        if (std::find(begin(skipExtensions), end(skipExtensions),
                      file.get_path().extension()) != end(skipExtensions)) {
          fmt::print("Skipping file due to its extension \"{}\".\n",
                     file.get_path().string());
          return;
        }
        m_packing_files.emplace_back(file);
      }

      void addDirectory(std::filesystem::path const& dirPath) {
        auto const dirIterator = std::filesystem::directory_iterator{dirPath};
        std::for_each(begin(dirIterator), end(dirIterator),
                      [this](std::filesystem::directory_entry const& entry) {
                        addPath(entry.path());
                      });
      }

      void addPath(std::filesystem::path const& argPath,
                   bool overrideRecursive = false) {
        if (is_directory(argPath) == true) {
          auto const isRecursive = [&overrideRecursive, &config = m_config]() {
            if (overrideRecursive) {
              return true;
            }
            if (config.has_value()) {
              return config->recursive;
            }
            return false;
          }();
          if (isRecursive) {
            addDirectory(argPath);
          } else {
            fmt::print(
                "Recursive parsing disabled. Path is ignored: {} -> "
                "skipping.\n",
                argPath.string());
          }
          return;
        }
        if (is_regular_file(argPath) == true) {
          addFile(argPath);
          return;
        }
        fmt::print("Path is not a file: {} -> skipping.\n", argPath.string());
      }
    };


   [[nodiscard]] auto get_input_files(
      std::span<char*> const arguments
   ) -> input_files
   {
      input_files result;
      result.m_packing_files.reserve(size(arguments));
      std::for_each(begin(arguments), end(arguments),
                [&result](char const* const arg) {
                  auto const argPath = std::filesystem::path{arg};
                  // Path must exist and must be a file
                  if (exists(argPath) == false) {
                    fmt::print("Path doesn't exist: {} -> skipping.\n", arg);
                    return;
                  }
                  result.addPath(argPath, true);
                });
      return result;
   }


   auto run(
      std::span<char*> const arguments
   ) -> int
   {
    // Only pass command line arguments, not executable path
    const input_files inputs = get_input_files(arguments.subspan(1));
      const config cfg = [&]() {
         if (inputs.m_config.has_value())
            return inputs.m_config.value();
         else
            return get_config(inputs.m_packing_files);
      }();

      std::vector<payload> payloads;
      payloads.reserve(inputs.m_packing_files.size());
      for (const file_path& file : inputs.m_packing_files)
      {
         payloads.emplace_back(get_payload(file, cfg));
      }

      {
         timer t("Time to write");
         const directory_path working_dir{ fs::current_path() };
         write_payloads_to_file(cfg, std::move(payloads), working_dir);
      }

      if (cfg.prompt_for_key)
         wait_for_keypress();
      return 0;
   }
}



auto main(
   int argc,
   char* argv[]
) -> int
{
  return bb::run(std::span{argv, static_cast<std::size_t>(argc)});
}
