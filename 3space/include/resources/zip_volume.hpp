#ifndef DARKSTARDTSCONVERTER_ZIP_VOLUME_HPP
#define DARKSTARDTSCONVERTER_ZIP_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include "archive_plugin.hpp"
#include "endian_arithmetic.hpp"

namespace studio::resources::zip
{
  struct zip_file_archive : studio::resources::archive_plugin
  {
    static bool is_supported(std::basic_istream<std::byte>& stream);

    bool stream_is_supported(std::basic_istream<std::byte>& stream) const override;
    std::vector<content_info> get_content_listing(std::basic_istream<std::byte>& stream, const listing_query& query) const override;
    void set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const override;
    void extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const override;
  };
}// namespace studio::resources::zip


#endif// DARKSTARDTSCONVERTER_ZIP_VOLUME_HPP
