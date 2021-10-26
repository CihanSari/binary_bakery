#pragma once

#include <binary_bakery_lib/universal.h>

#include <optional>
#include <string>


namespace bb
{
   struct directory_path;
   struct file_path;

   struct config {
      std::string output_filename = "binary_bakery_payload.h";
      int indentation_size = 3;
      int max_columns = 100;
      compression_mode compression = compression_mode::none;
      bool prompt_for_key = true;
      bool recursive = false;
      bool parse_images = true;
      image_vertical_direction image_loading_direction = image_vertical_direction::bottom_to_top;
   };

   [[nodiscard]] auto get_cfg_from_dir(const directory_path& dir) -> std::optional<config>;
   [[nodiscard]] auto get_cfg_from_file(const file_path& file) -> std::optional<config>;

}
