/**
 * \file zip_entry.cpp
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#include <boost/fusion/include/accumulate.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/stream.hpp>
#include <cppzip/v1/zip_entry.h>
#include <helper.h>
#include <local_file_header.h>
#include <zip_functions.h>

namespace cppzip
{
  inline namespace v1
  {
    struct ZipEntry::pimpl
    {
      pimpl(const LocalFileHeader& lf, size_t o, FileRead_fn fn)
        : m_local_file_header{lf}, m_offset{o}, m_read{std::move(fn)}, m_data{}
      {
      }

      pimpl(const LocalFileHeader& lf, const void* data, std::uint64_t length)
        : m_local_file_header{lf}, m_offset{}, m_read{}, m_data{}
      {
        if (length)
        {
          boost::iostreams::array_source arrs{reinterpret_cast<const char*>(data), length};
          boost::iostreams::filtering_istreambuf iin;
          boost::iostreams::zlib_params params{};
          params.noheader = true;
          iin.push(boost::iostreams::zlib_compressor{params});
          iin.push(arrs);
          m_data.assign(std::istreambuf_iterator<char>{&iin}, {});
          m_local_file_header.compressed_size = static_cast<uint32_t>(m_data.size());
        }
      }

      auto getEntryName() const -> std::string
      {
        return m_local_file_header.file_name;
      }

      auto getDate() const noexcept -> time_t
      {
        const uint16_t date = m_local_file_header.file_modification >> 16 & 0xFFFF;
        const uint16_t time = m_local_file_header.file_modification & 0xFFFF;
        return datetime_to_timestamp(date, time);
      }

      auto getCompressionMethod() const noexcept -> CompressionMethod
      {
        return static_cast<CompressionMethod>(m_local_file_header.compression_method);
      }

      auto getEncryptionMethod() const noexcept -> uint16_t
      {
        return 0;
      }

      auto getCompressedSize() const noexcept -> uint64_t
      {
        return m_local_file_header.compressed_size;
      }

      auto getUncompressedSize() const noexcept -> uint64_t
      {
        return m_local_file_header.uncompressed_size;
      }

      auto getCRC() const noexcept -> uint32_t
      {
        return m_local_file_header.crc32;
      }

      bool isDirectory() const noexcept
      {
        return !m_local_file_header.file_name.empty() && m_local_file_header.file_name.back() == '/';
      }

      bool isFile() const noexcept
      {
        return !isDirectory();
      }

      bool setComment(const std::string& str)
      {
        m_local_file_header.file_name = str;
        m_local_file_header.file_name_length = static_cast<uint16_t>(str.size());
        return true;
      }

      auto getComment() const -> std::string
      {
        return m_local_file_header.file_name;
      }

      auto readContent(std::ostream& ofOutput) const -> int64_t
      {
        if (m_data.empty() && m_local_file_header.uncompressed_size)
        {
          m_data.resize(m_local_file_header.compressed_size);
          const auto res = m_read(m_offset, std::ios::beg, m_data.data(), m_data.size());
          if (static_cast<size_t>(res) != m_data.size())
          {
            throw std::runtime_error("Could not read payload");
          }
        }
        if (!m_data.empty())
        {
          boost::iostreams::array_source arrs{reinterpret_cast<const char*>(m_data.data()), m_data.size()};
          boost::iostreams::filtering_istreambuf iin;
          boost::iostreams::zlib_params params{};
          params.noheader = true;
          iin.push(boost::iostreams::zlib_decompressor{params});
          iin.push(arrs);

          std::vector<uint8_t> data(std::istreambuf_iterator<char>{&iin}, {});
          auto crc = detail::getCrc32(data.data(), data.size());
          if (crc != m_local_file_header.crc32)
          {
            throw std::runtime_error("File is corrupt");
          }
          ofOutput.write(reinterpret_cast<const char*>(data.data()), data.size());
          return static_cast<int>(data.size());
        }
        return -1;
      }

      size_t writeEntry(std::ostream& ofOutput)
      {
        auto written = boost::fusion::accumulate(m_local_file_header, size_t(0), detail::WriteToStream(ofOutput));
        if (m_local_file_header.file_name_length)
        {
          ofOutput.write(m_local_file_header.file_name.data(), m_local_file_header.file_name.size());
          written += m_local_file_header.file_name.size();
        }
        if (m_local_file_header.extra_field_length)
        {
          ofOutput.write(reinterpret_cast<const char*>(m_local_file_header.extra_field.data()),
                         m_local_file_header.extra_field.size());
          written += m_local_file_header.extra_field.size();
        }
        if (!m_data.empty())
        {
          ofOutput.write(reinterpret_cast<const char*>(m_data.data()), m_data.size());
        }
        return written + m_data.size();
      }

      LocalFileHeader m_local_file_header;
      size_t m_offset;
      FileRead_fn m_read;
      mutable std::vector<uint8_t> m_data;
    };

    ZipEntry::ZipEntry(const LocalFileHeader& lf, size_t offset, FileRead_fn fn)
      : impl{std::make_unique<ZipEntry::pimpl>(lf, offset, std::move(fn))}
    {
    }
    ZipEntry::ZipEntry(const LocalFileHeader& lf, const void* data, std::uint64_t length)
      : impl{std::make_unique<ZipEntry::pimpl>(lf, data, length)}
    {
    }

    ZipEntry::~ZipEntry()
    {
    }

    auto ZipEntry::getEntryName() const -> std::string
    {
      return impl->getEntryName();
    }

    auto ZipEntry::getCompressionMethod() const noexcept -> CompressionMethod
    {
      return impl->getCompressionMethod();
    }

    auto ZipEntry::getEncryptionMethod() const noexcept -> uint16_t
    {
      return impl->getEncryptionMethod();
    }

    auto ZipEntry::getCompressedSize() const noexcept -> uint64_t
    {
      return impl->getCompressedSize();
    }

    auto ZipEntry::getUncompressedSize() const noexcept -> uint64_t
    {
      return impl->getUncompressedSize();
    }

    auto ZipEntry::getCRC() const noexcept -> uint32_t
    {
      return impl->getCRC();
    }

    bool ZipEntry::isDirectory() const noexcept
    {
      return impl->isDirectory();
    }

    bool ZipEntry::isFile() const noexcept
    {
      return impl->isFile();
    }

    bool ZipEntry::setComment(const std::string& str)
    {
      return impl->setComment(str);
    }

    auto ZipEntry::getComment() const -> std::string
    {
      return impl->getComment();
    }

    auto ZipEntry::readContent(std::ostream& ofOutput) const -> int64_t
    {
      return impl->readContent(ofOutput);
    }

    size_t ZipEntry::writeEntry(std::ostream& ofOutput)
    {
      return impl->writeEntry(ofOutput);
    }

    size_t ZipEntry::compressedSize() const
    {
      return impl->getCompressedSize();
    }

  } // namespace v1
} // namespace cppzip

