/**
 * \file zip_entry.h
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#ifndef INTERFACE_CPPZIP_V1_ZIP_ENTRY_H
#define INTERFACE_CPPZIP_V1_ZIP_ENTRY_H

#include <boost/filesystem.hpp>
#include <memory>

namespace cppzip
{
  struct LocalFileHeader;
  enum class CompressionMethod
  {
    no = 0,
    shrunk = 1,
    reduced_with_factor1 = 2,
    reduced_with_factor2 = 3,
    reduced_with_factor3 = 4,
    reduced_with_factor4 = 5,
    imploeded = 6,
    defalted = 8,
    enhanced_deflated = 9,
    pkware_compression_imploding = 10,
    bzip2 = 12,
    lzma = 14,
    ibm_compression = 16,
    ibm_terse = 18,
    ibm_lz77 = 19,
    jpeg_variant = 96,
    wav_pack = 97,
    ppm = 98,
    aex = 99
  };

  inline namespace v1
  {
    using FileRead_fn = std::function<long long(std::streamsize, std::ios::seekdir, uint8_t*, size_t)>;
    /**
     * The ZipEntry which represents an entry in a zip file
     */
    class ZipEntry
    {
      friend class ZipArchive;
      ZipEntry(const LocalFileHeader& lf, size_t offset, FileRead_fn fn);
      ZipEntry(const LocalFileHeader& lf, const void* data, std::uint64_t length);

    public:
      ~ZipEntry();
      ZipEntry(const ZipEntry&) = delete;
      ZipEntry(ZipEntry&&) = delete;
      ZipEntry& operator=(const ZipEntry&) = delete;
      ZipEntry& operator=(ZipEntry&&) = delete;

      /**
       * Returns the name of the entry.
       */
      auto getEntryName() const -> std::string;

      /**
       * Returns the timestamp of the entry.
       */
      auto getDate() const noexcept -> time_t;

      /**
       * Returns the compression method.
       */
      auto getCompressionMethod() const noexcept -> CompressionMethod;

      /**
       * Returns the encryption method.
       */
      auto getEncryptionMethod() const noexcept -> uint16_t;

      /**
       * Returns the size of the file (not deflated).
       */
      auto getCompressedSize() const noexcept -> uint64_t;

      /**
       * Returns the size of the inflated file.
       */
      auto getUncompressedSize() const noexcept -> uint64_t;

      /**
       * Returns the CRC of the file.
       */
      auto getCRC() const noexcept -> uint32_t;

      /**
       * Returns true if the entry is a directory.
       */
      bool isDirectory() const noexcept;

      /**
       * Returns true if the entry is a file.
       */
      bool isFile() const noexcept;

      /**
       * Set the comment of the entry.
       */
      bool setComment(const std::string& str);

      /**
       * Get the comment of the entry.
       */
      auto getComment() const -> std::string;

      /**
       * Read the specified ZipEntry
       */
      auto readContent(std::ostream& ofOutput) const -> int64_t;

    private:
      size_t writeEntry(std::ostream& ofOutput);
      size_t compressedSize() const;

      struct pimpl;
      std::unique_ptr<pimpl> impl;
    };
  } // namespace v1
} // namespace cppzip
#endif /* INTERFACE_CPPZIP_V1_ZIP_ENTRY_H */

