/**
 * \file digital_signature.h
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#ifndef INTERFACE_CPPZIP_DIGITAL_SIGNATURE_H
#define INTERFACE_CPPZIP_DIGITAL_SIGNATURE_H

#include <boost/fusion/include/adapt_struct.hpp>
#include <string>

namespace cppzip
{
  struct DigitalSignature
  {
    uint32_t signature;
    uint16_t size;
    std::vector<uint8_t> data;
  };
  constexpr size_t digital_signature_size = 6;
  constexpr size_t digital_signature_signature = 0x05054b50;
} // namespace cppzip

BOOST_FUSION_ADAPT_STRUCT(cppzip::DigitalSignature,
                          signature,
						  size)

#endif /* INTERFACE_CPPZIP_DIGITAL_SIGNATURE_H */
