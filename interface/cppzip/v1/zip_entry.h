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
      // time_t getDate();

      /**
       * Returns the compression method.
       */
      std::uint16_t getCompressionMethod() const;

      /**
       * Returns the encryption method.
       */
      std::uint16_t getEncryptionMethod() const;

      /**
       * Returns the size of the file (not deflated).
       */
      std::uint64_t getCompressedSize() const;

      /**
       * Returns the size of the inflated file.
       */
      std::uint64_t getUncompressedSize() const;

      /**
       * Returns the CRC of the file.
       */
      auto getCRC() const noexcept -> uint32_t;

      /**
       * Returns true if the entry is a directory.
       */
      bool isDirectory() const;

      /**
       * Returns true if the entry is a file.
       */
      bool isFile() const;

      /**
       * Set the comment of the entry.
       */
      bool setComment(const std::string& str);

      /**
       * Get the comment of the entry.
       */
      std::string getComment() const;

      /**
       * Read the specified ZipEntry
       */
      int readContent(std::ostream& ofOutput) const;

    private:

      size_t writeEntry(std::ostream& ofOutput);
      size_t compressedSize() const;

      struct pimpl;
      std::unique_ptr<pimpl> impl;
    };
  } // namespace v1
} // namespace cppzip
#endif /* INTERFACE_CPPZIP_V1_ZIP_ENTRY_H */

