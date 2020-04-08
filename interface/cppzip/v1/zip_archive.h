/**
 * \file zip_archive.h
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#ifndef INTERFACE_CPPZIP_V1_ZIP_ARCHIVE_H
#define INTERFACE_CPPZIP_V1_ZIP_ARCHIVE_H

#include <boost/filesystem.hpp>
#include <memory>
#include <vector>

namespace cppzip
{
  inline namespace v1
  {
    class ZipEntry;

    /**
     * The ZipArchive which represents a zip file or a in memory zip file
     */
    class ZipArchive final
    {
    public:
      /**
       * The Open mode in which the underlying file of memory stream will be
       * opened
       */
      enum class OpenMode
      {
        ReadOnly,
        Write,
        New
      };

      ZipArchive();
      ZipArchive(boost::filesystem::path path, OpenMode mode);
      ZipArchive(const std::vector<uint8_t>& data, OpenMode mode);
      ~ZipArchive();
      ZipArchive(const ZipArchive&) = delete;
      ZipArchive(ZipArchive&&) = delete;
      ZipArchive& operator=(const ZipArchive&) = delete;
      ZipArchive& operator=(ZipArchive&&) = delete;

      /**
       * Return the path of the ZipArchive. Empty when in memory
       */
      auto getPath() const -> boost::filesystem::path;

      /**
       * Returns true if the ZipArchive is encrypted.
       */
      bool isEncrypted() const;

      /**
       * Set the comment of the archive.
       */
      void setComment(const std::string& comment);

      /**
       * Get the comment of the archive.
       */
      std::string getComment() const;

      /**
       * Returns the number of entries in this zip file (folders are included).
       */
      auto getNumberOfEntries() const noexcept -> int64_t;

      /**
       * Returns all the entries of the ZipArchive.
       */
      std::vector<std::shared_ptr<ZipEntry>> getEntries() const;

      /**
       * Return true if an entry with the specified name exists. If no such entry exists,
       * then false will be returned.
       */
      auto hasEntry(const std::string& zipEntryName) const noexcept -> bool;

      /**
       * Return the ZipEntry for the specified entry name. If no such entry exists,
       * then a null-ZiPEntry will be returned
       */
      std::shared_ptr<ZipEntry> getEntry(const std::string& name) const;

      /**
       * Renames the entry with the specified newName.
       */
      bool renameEntry(const std::string& entry, const std::string& newName) const;

      /**
       * Add the specified file in the archive with the given entry. If the entry already exists,
       * it will be replaced. This method returns true if the file has been added successfully.
       */
      auto addFile(const std::string& entryName, const boost::filesystem::path& file) const -> bool;

      /**
       * Add the given data to the specified entry name in the archive. If the entry already exists,
       * its content will be erased.
       */
      auto addData(const std::string& entryName, const void* data, uint64_t length) -> bool;

      /**
       * Add the specified entry to the ZipArchive. All the needed hierarchy will be created.
       * The entryName must be a directory.
       */
      bool addEntry(const std::string& entryName) const;

      void writeArchive(std::ostream& ofOutput);

    private:
      struct pimpl;
      std::unique_ptr<pimpl> impl;
    };
  } // namespace v1
} // namespace cppzip
#endif /* INTERFACE_CPPZIP_V1_ZIP_ARCHIVE_H */
