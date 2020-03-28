/**
 * \file central_directory_file_header.h
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#ifndef INTERFACE_CPPZIP_CENTRAL_DIRECTORY_FILE_HEADER_H
#define INTERFACE_CPPZIP_CENTRAL_DIRECTORY_FILE_HEADER_H

#include <boost/fusion/include/adapt_struct.hpp>
#include <string>
#include <vector>

namespace cppzip
{
  struct CentralDirectoryFileHeader
  {
    uint32_t signature;
    uint16_t version;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression;
    uint32_t file_modification;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    uint16_t file_comment_lenght;
    uint16_t disk_start;
    uint16_t internal_attributes;
    uint32_t external_attributes;
    uint32_t offset_of_local_header;
    std::string file_name;
    std::vector<uint8_t> extra_field;
    std::string file_comment;
  };
  constexpr size_t central_directory_file_header_size = 46;
  constexpr size_t central_directory_file_header_signature = 0x2014b50;
} // namespace cppzip

BOOST_FUSION_ADAPT_STRUCT(cppzip::CentralDirectoryFileHeader,
                          signature,
                          version,
                          version_needed,
                          flags,
                          compression,
                          file_modification,
                          crc32,
                          compressed_size,
                          uncompressed_size,
                          file_name_length,
                          extra_field_length,
                          file_comment_lenght,
                          disk_start,
                          internal_attributes,
                          external_attributes,
                          offset_of_local_header)

#endif /* INTERFACE_CPPZIP_CENTRAL_DIRECTORY_FILE_HEADER_H */
