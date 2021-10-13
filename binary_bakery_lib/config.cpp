#include "config.h"

#include <iostream>
#include <format>
#include <fstream>

#include <tomlplusplus/toml.hpp>

namespace
{

   using namespace bb;


   template<typename read_type = std::string, typename target_type, typename fun_type>
   auto set_value(
      target_type& target,
      const toml::table& tbl,
      const char* key,
      const fun_type& fun
   ) -> void
   {
      const std::optional<read_type> read_value = tbl[key].value<read_type>();
      if (read_value.has_value() == false)
      {
         return;
      }
      const std::optional<target_type> interpret_result = fun(read_value.value());
      if (interpret_result.has_value() == false)
      {
         std::cout << std::format("The config value \"{}\" couldn't be parsed. Skipping.\n", read_value.value());
         return;
      }
      target = interpret_result.value();
   }


   // For simple types, the reading type equals to type of the target
   template<typename target_type>
   auto set_value(
      target_type& target,
      const toml::table& tbl,
      const char* key
   ) -> void
   {
      using read_type = target_type;
      const auto opt_identity = [](const read_type & in) {
         return std::optional<read_type>{in};
      };
      set_value<read_type>(target, tbl, key, opt_identity);
   }


   [[nodiscard]] auto get_compression_mode(
      const std::string& value
   ) -> std::optional<compression_mode>
   {
      if (value == "none")
         return compression_mode::none;
      else if (value == "zstd")
         return compression_mode::zstd;
      else if (value == "lz4")
         return compression_mode::lz4;
      else
      {
         std::cout << std::format("compression_mode value \"{}\" not recognized. Using no compression.\n", value);
         return compression_mode::none;
      }
   }


   [[nodiscard]] auto get_image_write_direction(
      const std::string& value
   ) -> std::optional<image_vertical_direction>
   {
      if (value == "bottom_to_top")
         return image_vertical_direction::bottom_to_top;
      else if (value == "top_to_bottom")
         return image_vertical_direction::top_to_bottom;
      else
         return std::nullopt;
   }

   const std::string default_config_filename = "binary_bakery.toml";

} // namespace {}


auto bb::get_cfg_from_dir(
   const abs_directory_path& dir
) -> std::optional<config>
{
   const fs::path config_path = dir.get_path() / default_config_filename;
   if (fs::exists(config_path) == false)
   {
      return std::nullopt;
   }
   return get_cfg_from_file(abs_file_path{ dir.get_path() / default_config_filename });
}


auto bb::get_cfg_from_file(
   const abs_file_path& file
) -> std::optional<config>
{
   toml::table tbl;
   try
   {
      tbl = toml::parse_file(file.get_path().string());
   }
   catch (const toml::parse_error&)
   {
      const std::string msg = std::format("Couldn't parse file {}. Looking for other config.", file.get_path().string());
      std::cout << msg << std::endl;
      return std::nullopt;
   }

   config cfg;
   set_value(cfg.output_filename, tbl, "output_filename");
   set_value(cfg.max_columns, tbl, "max_columns");
   set_value(cfg.smart_mode, tbl, "smart_mode");
   set_value(cfg.compression, tbl, "compression_mode", get_compression_mode);
   set_value(cfg.prompt_for_key, tbl, "prompt_for_key");
   set_value(cfg.image_loading_direction, tbl, "image_loading_direction", get_image_write_direction);
   return cfg;
}
