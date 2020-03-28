/**
 * \file end_of_central_directory_record.h
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#ifndef INTERFACE_CPPZIP_END_OF_CENTRAL_DIRECTORY_RECORD_H
#define INTERFACE_CPPZIP_END_OF_CENTRAL_DIRECTORY_RECORD_H

#include <boost/fusion/include/adapt_struct.hpp>
#include <string>

namespace cppzip
{
  struct EndOfCentralDirectoryRecord
  {
    uint32_t signature;
    uint16_t disk_number;
    uint16_t disk;
    uint16_t disk_entries;
    uint16_t total_entries;
    uint32_t central_directory_size;
    uint32_t offset;
    uint16_t comment;
    std::string zip_comment;
  };
  constexpr size_t end_of_central_directory_size = 22;
  constexpr int end_of_central_directory_signature = 0x06054b50;
} // namespace cppzip

BOOST_FUSION_ADAPT_STRUCT(cppzip::EndOfCentralDirectoryRecord,
                          signature,
                          disk_number,
                          disk,
                          disk_entries,
                          total_entries,
                          central_directory_size,
                          offset,
                          comment)

#endif /* INTERFACE_CPPZIP_END_OF_CENTRAL_DIRECTORY_RECORD_H */
