#pragma once

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;


namespace bb
{

   struct path_type {
   protected:
      fs::path m_path;

   public:
      explicit path_type(const fs::path& path);

      [[nodiscard]] auto get_path() const -> fs::path;
   };

   // Stores file path. Throws if file doesn't exist.
   struct file_path : path_type {
      using path_type::path_type;

      explicit file_path(const fs::path& path);
   };


   // Stores directory path. Throws if path doesn't exist.
   struct directory_path : path_type{
      using path_type::path_type;

      explicit directory_path(const fs::path& path);
   };

   [[nodiscard]] auto get_binary_file(const path_type& file) -> std::vector<uint8_t>;

}
