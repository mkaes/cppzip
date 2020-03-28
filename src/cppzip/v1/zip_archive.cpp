/**
 * \file zip_archive.cpp
 */
//		Copyright Michael Kaes 2017.
//		Distributed under rhe MIT License.
//		(See accompanying file LICENSE)

#include <boost/fusion/include/accumulate.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <central_directory_file_header.h>
#include <cppzip/v1/zip_archive.h>
#include <cppzip/v1/zip_entry.h>
#include <end_of_central_directory_record.h>
#include <helper.h>
#include <local_file_header.h>

namespace cppzip
{
  inline namespace v1
  {
    namespace
    {
      constexpr uint16_t VERSION = 20;

      constexpr uint16_t MakeFlags()
      {
        return 0;
      }

      constexpr uint16_t internalAttr()
      {
        return 0;
      }

      constexpr uint32_t externalAttr()
      {
        return 0;
      }

      struct TimeVal
      {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
      };

      uint32_t dostime(int year, int month, int day, int hour, int minute, int second)
      {
        if (year < 1980 || year > 2107 || month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 ||
            minute < 0 || minute > 59 || second < 0 || second > 59)
        {
          return 0;
        }
        return (((uint32_t)year - 1980) << 25) | (((uint32_t)month) << 21) | (((uint32_t)day) << 16) |
               (((uint32_t)hour) << 11) | (((uint32_t)minute) << 5) | (((uint32_t)second) >> 1);
      }

      TimeVal dosSplit(uint32_t dostime)
      {
        int t_sec = (((int)dostime << 1) & 0x3E);
        int t_min = (((int)dostime >> 5) & 0x3F);
        int t_hour = (((int)dostime >> 11) & 0x1F);
        int t_day = (((int)dostime >> 16) & 0x1F);
        int t_mon = (((int)dostime >> 21) & 0x0F) - 1;
        int t_year = (((int)dostime >> 25) & 0x7F) + 80;
        return TimeVal{t_year, t_mon, t_day, t_hour, t_min, t_sec};
      }

      struct EntryCompare
      {
        const std::string& entry;
        EntryCompare(const std::string& e) : entry{e}
        {
        }

        bool operator()(const std::shared_ptr<ZipEntry> e)
        {
          return entry == e->getName();
        }
      };

      struct FileAccess
      {
        FileAccess(boost::filesystem::path p, ZipArchive::OpenMode mode)
          : m_file(p.string(),
                   mode != ZipArchive::OpenMode::ReadOnly ? (std::ios::in | std::ios::out | std::ios::binary)
                                                          : (std::ios::in | std::ios::binary))
        {
          if (!m_file)
          {
            throw std::runtime_error("Could not load open file");
          }
        }

        std::streamsize read(std::streamsize pos, std::ios::seekdir sd, uint8_t* b, size_t l) const
        {
          m_file.seekg(pos, sd);
          m_file.read(reinterpret_cast<char*>(b), l);
          return m_file.gcount();
        }
        mutable std::fstream m_file;
      };

      struct MemoryAccess
      {
        MemoryAccess(const std::vector<uint8_t>& d, ZipArchive::OpenMode mode) : m_data(d), m_offset{}
        {
          if (mode == ZipArchive::OpenMode::Write)
          {
            throw std::runtime_error("Memoryzip cannot write");
          }
        }

        std::streamsize read(std::streamsize pos, std::ios::seekdir sd, uint8_t* b, size_t l) const
        {
          switch (sd)
          {
          case std::ios::cur:
            m_offset += pos;
            break;
          case std::ios::beg:
            m_offset = pos;
            break;
          case std::ios::end:
            m_offset = m_data.size() + pos;
            break;
          default:
            break;
          }
          if (m_offset > m_data.size())
          {
            throw std::runtime_error("Out of bounds");
          }
          memcpy(b, m_data.data() + m_offset, l);
          return l;
        }
        const std::vector<uint8_t> m_data;
        mutable size_t m_offset;
      };

      struct ReadFromArray
      {
        const uint8_t* buffer;
        ReadFromArray(const uint8_t* a) : buffer(a)
        {
        }
        template<typename T>
        void operator()(T& t)
        {
          T tmp;
          memcpy(&tmp, buffer, sizeof(T));
          t = boost::endian::little_to_native(tmp);
          buffer += sizeof(T);
        }
      };
    } // namespace

    struct ZipArchive::pimpl
    {

      pimpl() : m_end_of_central_directory_record{end_of_central_directory_signature, {}, {}, {}, {}, {}, {}, {}, {}}
      {
      }
      pimpl(boost::filesystem::path path, OpenMode mode)
        : m_path{std::move(path)}
        , m_read([access = std::make_shared<FileAccess>(m_path, mode)](
                     std::streamsize p, std::ios::seekdir o, uint8_t* b, size_t l) { return access->read(p, o, b, l); })
      {
        init_end_of_central_directory();
        init_central_directory();
        load_entries();
      }

      pimpl(const std::vector<uint8_t>& data, OpenMode mode)
        : m_read([access = std::make_shared<MemoryAccess>(data, mode)](
                     std::streamsize p, std::ios::seekdir o, uint8_t* b, size_t l) { return access->read(p, o, b, l); })
      {
      }

      void init_end_of_central_directory()
      {
        uint8_t rec[end_of_central_directory_size];
        std::streamsize seek_pos = -std::streamsize(end_of_central_directory_size);
        while (const auto res = m_read(seek_pos, std::ios::end, rec, end_of_central_directory_size))
        {
          boost::fusion::for_each(m_end_of_central_directory_record, ReadFromArray(rec));
          if (res < static_cast<long long>(end_of_central_directory_size) ||
              m_end_of_central_directory_record.signature == end_of_central_directory_signature)
          {
            break;
          }
          --seek_pos;
        }
        if (m_end_of_central_directory_record.signature != end_of_central_directory_signature)
        {
          throw std::runtime_error("Could not load end of central directory");
        }
        if (m_end_of_central_directory_record.disk_number != 0 || m_end_of_central_directory_record.disk != 0)
        {
          throw std::runtime_error("Multi file zip not implemented");
        }
        if (const std::streamsize new_pos = seek_pos + end_of_central_directory_size)
        {
          m_end_of_central_directory_record.zip_comment.resize((-new_pos) + 1);
          const auto res =
              m_read(new_pos, std::ios::end, (uint8_t*)&m_end_of_central_directory_record.zip_comment[0], -new_pos);
          if (res != -new_pos)
          {
            throw std::runtime_error("Multi file zip not implemented");
          }
        }
      }

      void init_central_directory()
      {
        std::vector<uint8_t> central_directory(m_end_of_central_directory_record.central_directory_size);
        const auto res = m_read(m_end_of_central_directory_record.offset, std::ios::beg, central_directory.data(),
                                central_directory.size());
        if (static_cast<size_t>(res) != central_directory.size())
        {
          throw std::runtime_error("Could not load central directory");
        }

        const uint8_t* pos = central_directory.data();
        for (auto i = 0; i < m_end_of_central_directory_record.total_entries; ++i)
        {
          CentralDirectoryFileHeader central_directory_file_header;
          boost::fusion::for_each(central_directory_file_header, ReadFromArray(pos));
          if (central_directory_file_header.signature != central_directory_file_header_signature)
          {
            throw std::runtime_error("Wrong central directory signature");
          }
          pos += central_directory_file_header_size;
          if (central_directory_file_header.file_name_length)
          {
            central_directory_file_header.file_name.assign(reinterpret_cast<const char*>(pos),
                                                           central_directory_file_header.file_name_length);
            pos += central_directory_file_header.file_name_length;
          }
          if (central_directory_file_header.extra_field_length)
          {
            central_directory_file_header.extra_field.assign(pos,
                                                             pos + central_directory_file_header.extra_field_length);
            pos += central_directory_file_header.extra_field_length;
          }
          if (central_directory_file_header.file_comment_lenght)
          {
            central_directory_file_header.file_comment.assign(reinterpret_cast<const char*>(pos),
                                                              central_directory_file_header.file_comment_lenght);
            pos += central_directory_file_header.file_comment_lenght;
          }
          m_central_directory_file_headers.push_back(central_directory_file_header);
        }
        if (pos != central_directory.data() + central_directory.size())
        {
          throw std::runtime_error("Central Directory contains more data");
        }
      }

      void load_entries()
      {
        for (const auto& file_header : m_central_directory_file_headers)
        {
          std::vector<uint8_t> loc(local_file_header_size);
          const auto res = m_read(file_header.offset_of_local_header, std::ios::beg, loc.data(), loc.size());
          if (static_cast<size_t>(res) != loc.size())
          {
            throw std::runtime_error("Could not local file header");
          }
          LocalFileHeader local_file_header;
          boost::fusion::for_each(local_file_header, ReadFromArray(loc.data()));
          if (local_file_header.file_name_length)
          {
            local_file_header.file_name.resize(local_file_header.file_name_length);
            const auto res =
                m_read(0, std::ios::cur, (uint8_t*)&local_file_header.file_name[0], local_file_header.file_name_length);
            if (res != local_file_header.file_name_length)
            {
              throw std::runtime_error("Could not read file name");
            }
          }
          if (local_file_header.extra_field_length)
          {
            std::vector<uint8_t> ex(local_file_header.extra_field_length);
            const auto res = m_read(0, std::ios::cur, ex.data(), ex.size());
            if (static_cast<size_t>(res) != ex.size())
            {
              throw std::runtime_error("Could not read extra data");
            }
          }
          const size_t datapos = file_header.offset_of_local_header + local_file_header_size +
                                 local_file_header.file_name_length + local_file_header.extra_field_length;
          m_entries.push_back(std::shared_ptr<ZipEntry>(new ZipEntry(local_file_header, datapos, m_read)));
        }
      }

      std::string getPath() const
      {
        return m_path.string();
      }

      bool isEncrypted() const
      {
        return false;
      }

      void setComment(const std::string& comment)
      {
        m_end_of_central_directory_record.zip_comment = comment;
        m_end_of_central_directory_record.comment = static_cast<uint16_t>(comment.size());
      }

      std::string getComment() const
      {
        return m_end_of_central_directory_record.zip_comment;
      }

      std::int64_t getNumberOfEntries() const
      {
        return m_end_of_central_directory_record.total_entries;
      }

      std::vector<std::shared_ptr<ZipEntry>> getEntries() const
      {
        return m_entries;
      }

      bool hasEntry(const std::string& name) const
      {
        return std::find_if(std::begin(m_entries), std::end(m_entries), EntryCompare{name}) != m_entries.end();
      }

      std::shared_ptr<ZipEntry> getEntry(const std::string& name) const
      {
        const auto iter = std::find_if(std::begin(m_entries), std::end(m_entries), EntryCompare(name));
        if (iter != std::end(m_entries))
        {
          return *iter;
        }
        return {};
      }

      bool renameEntry(const std::string& entry, const std::string& newName) const
      {
        return false;
      }

      bool addFile(const std::string& entryName, const std::string& file) const
      {
        return false;
      }

      bool addData(const std::string& entryName, const void* data, std::uint64_t length)
      {
        boost::filesystem::path path{entryName};
        if (path.is_absolute())
        {
          throw std::runtime_error("Cannot add absolute path");
        }
        boost::filesystem::path fullpath{};
        for (const auto& p : path)
        {
          fullpath /= p;
          if (fullpath == path)
          {
            break;
          }
          fullpath.append("/");
          if (!hasEntry(fullpath.string()))
          {
            newEntry(fullpath.string(), nullptr, 0);
          }
        }

        newEntry(fullpath.string(), data, length);
        return true;
      }

      bool addEntry(const std::string& entryName) const
      {
        return false;
      }

      void newEntry(const std::string& name, const void* data, std::uint64_t length)
      {
        const auto crc32 = detail::getCrc32(reinterpret_cast<const uint8_t*>(data), length);
        const uint16_t compressionMode = data ? 8 : 0;
        const auto val = dostime(2020, 3, 26, 16, 31, 11);
        LocalFileHeader h{local_file_header_signature,
                          VERSION,
                          MakeFlags(),
                          compressionMode,
                          val,
                          crc32,
                          0,
                          static_cast<uint32_t>(length),
                          static_cast<uint16_t>(name.size()),
                          0,
                          name,
                          {},
						  {}};
        m_entries.push_back(std::shared_ptr<ZipEntry>(new ZipEntry(h, data, length)));

        CentralDirectoryFileHeader cf{central_directory_file_header_signature,
                                      VERSION,
                                      VERSION,
                                      MakeFlags(),
                                      compressionMode,
                                      val,
                                      crc32,
                                      static_cast<uint32_t>(m_entries.back()->compressedSize()),
                                      static_cast<uint32_t>(length),
                                      static_cast<uint16_t>(name.size()),
                                      0,
                                      0,
                                      0,
                                      internalAttr(),
                                      externalAttr(),
                                      0,
                                      name,
                                      {},
                                      {}};

        m_central_directory_file_headers.push_back(cf);
        m_end_of_central_directory_record.total_entries++;
        m_end_of_central_directory_record.disk_entries++;
      }

      void writeArchive(std::ostream& ofOutput)
      {
        size_t written = 0;
        std::vector<size_t> offsets(1);
        for (const auto& e : m_entries)
        {
          written += e->writeEntry(ofOutput);
          offsets.push_back(written);
        }
        auto iter = offsets.begin();
        const auto cdoffset = written;
        written = 0;
        for (auto& h : m_central_directory_file_headers)
        {
          h.offset_of_local_header = *iter++;
          written = boost::fusion::accumulate(h, written, detail::WriteToStream(ofOutput));
          if (h.file_name_length)
          {
            ofOutput.write(h.file_name.data(), h.file_name.size());
            written += h.file_name.size();
          }
          if (h.extra_field_length)
          {
            ofOutput.write(reinterpret_cast<const char*>(h.extra_field.data()), h.extra_field.size());
            written += h.extra_field.size();
          }
          if (h.file_comment_lenght)
          {
            ofOutput.write(h.file_comment.data(), h.file_comment.size());
            written += h.file_comment.size();
          }
        }
        m_end_of_central_directory_record.offset = cdoffset;
        m_end_of_central_directory_record.central_directory_size = written;
        written =
            boost::fusion::accumulate(m_end_of_central_directory_record, written, detail::WriteToStream(ofOutput));
        if (!m_end_of_central_directory_record.zip_comment.empty())
        {
          ofOutput.write(m_end_of_central_directory_record.zip_comment.c_str(),
                         m_end_of_central_directory_record.zip_comment.size());
        }
      }

      boost::filesystem::path m_path;
      std::function<long long(std::streamsize, std::ios::seekdir, uint8_t*, size_t)> m_read;
      EndOfCentralDirectoryRecord m_end_of_central_directory_record;
      std::vector<CentralDirectoryFileHeader> m_central_directory_file_headers;
      std::vector<std::shared_ptr<ZipEntry>> m_entries;
    };

    ZipArchive::ZipArchive() : impl{std::make_unique<ZipArchive::pimpl>()}
    {
    }

    ZipArchive::ZipArchive(boost::filesystem::path path, OpenMode mode)
      : impl{std::make_unique<ZipArchive::pimpl>(std::move(path), mode)}
    {
    }

    ZipArchive::ZipArchive(const std::vector<uint8_t>& data, OpenMode mode)
      : impl{std::make_unique<ZipArchive::pimpl>(data, mode)}
    {
    }

    ZipArchive::~ZipArchive() = default;

    std::string ZipArchive::getPath() const
    {
      return impl->getPath();
    }

    bool ZipArchive::isEncrypted() const
    {
      return impl->isEncrypted();
    }

    void ZipArchive::setComment(const std::string& comment)
    {
      impl->setComment(comment);
    }

    std::string ZipArchive::getComment() const
    {
      return impl->getComment();
    }

    std::int64_t ZipArchive::getNumberOfEntries() const
    {
      return impl->getNumberOfEntries();
    }

    std::vector<std::shared_ptr<ZipEntry>> ZipArchive::getEntries() const
    {
      return impl->getEntries();
    }

    bool ZipArchive::hasEntry(const std::string& name) const
    {
      return impl->hasEntry(name);
    }

    std::shared_ptr<ZipEntry> ZipArchive::getEntry(const std::string& name) const
    {
      return impl->getEntry(name);
    }

    bool ZipArchive::renameEntry(const std::string& entry, const std::string& newName) const
    {
      return impl->renameEntry(entry, newName);
    }

    bool ZipArchive::addFile(const std::string& entryName, const std::string& file) const
    {
      return impl->addFile(entryName, file);
    }

    bool ZipArchive::addData(const std::string& entryName, const void* data, std::uint64_t length)
    {
      return impl->addData(entryName, data, length);
    }

    bool ZipArchive::addEntry(const std::string& entryName) const
    {
      return impl->addEntry(entryName);
    }

    void ZipArchive::writeArchive(std::ostream& ofOutput)
    {
      return impl->writeArchive(ofOutput);
    }
  } // namespace v1
} // namespace cppzip
