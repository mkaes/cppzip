/**
 * \file local_file_header.h
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#ifndef INTERFACE_CPPZIP_LOCAL_FILE_HEADER_H
#define INTERFACE_CPPZIP_LOCAL_FILE_HEADER_H

#include <boost/fusion/include/adapt_struct.hpp>
#include <string>

namespace cppzip
{
  struct LocalFileHeader
  {
    uint32_t signature;
    uint16_t version;
    uint16_t flags;
    uint16_t compression_method;
    uint32_t file_modification;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t file_name_length;
    uint16_t extra_field_length;
    std::string file_name;
    std::vector<uint8_t> extra_field;
    std::vector<uint8_t> compressed_file;
  };
  constexpr size_t local_file_header_size = 30;
  constexpr size_t local_file_header_signature = 0x4034b50;
} // namespace cppzip

BOOST_FUSION_ADAPT_STRUCT(cppzip::LocalFileHeader,
                          signature,
                          version,
                          flags,
                          compression_method,
                          file_modification,
                          crc32,
                          compressed_size,
                          uncompressed_size,
                          file_name_length,
                          extra_field_length)

#endif /* INTERFACE_CPPZIP_LOCAL_FILE_HEADER_H */
