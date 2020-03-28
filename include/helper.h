/**
 * \file helper.h
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#ifndef INTERFACE_CPPZIP_HELPER_H
#define INTERFACE_CPPZIP_HELPER_H

#include <boost/endian/conversion.hpp>
#include <boost/crc.hpp>
#include <ostream>

namespace cppzip
{
  namespace detail
  {
    struct WriteToStream
    {
      typedef size_t result_type;

      std::ostream& stream;
      WriteToStream(std::ostream& s) : stream{s}
      {
      }
      template<typename T>
      size_t operator()(size_t acc, const T& t) const
      {
        const size_t before = stream.tellp();
        T tmp = boost::endian::native_to_little(t);
        stream.write(reinterpret_cast<const char*>(&tmp), sizeof(T));
        return acc + stream.tellp() - before;
      }
    };

    inline uint32_t getCrc32(const uint8_t* data, size_t length)
    {
      boost::crc_32_type result;
      result.process_bytes(data, length);
      return result.checksum();
    }

  } // namespace detail
} // namespace cppzip

#endif /* INTERFACE_CPPZIP_HELPER_H */
