/**
 * \file zip_functions.h
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#ifndef INTERFACE_CPPZIP_ZIP_V1_FUNCTIONS_H
#define INTERFACE_CPPZIP_ZIP_V1_FUNCTIONS_H

namespace cppzip
{
  inline namespace v1
  {
      inline time_t datetime_to_timestamp(uint16_t date, uint16_t time)
      {
        tm timeStruct;

        timeStruct.tm_year = ((date >> 9) & 0x7f) + 80;
        timeStruct.tm_mon = ((date >> 5) & 0x0f) - 1;
        timeStruct.tm_mday = ((date)&0x1f);

        timeStruct.tm_hour = ((time >> 11) & 0x1f);
        timeStruct.tm_min = ((time >> 5) & 0x3f);
        timeStruct.tm_sec = ((time << 1) & 0x3f);

        return mktime(&timeStruct);
      }

  }
} // namespace cppzip

#endif /* INTERFACE_CPPZIP_ZIP_V1_FUNCTIONS_H */
