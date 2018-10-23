#include <zipper.h>
#include <miniz.h>
#include <filemanager.h>
#include <QDir>
#include <QDebug>

bool Zipper::compressDir(const QString& dir, const QString& outFilename, const QString& base)
{
    // Spin for data
    for (auto file : lsfile(dir)) {
        auto data = rdfile(dir + separator() + file);
        if (!mz_zip_add_mem_to_archive_file_in_place(outFilename.toStdString().c_str(), base.isEmpty() ?
                                                         file.toStdString().c_str() : (base + separator() + file).toStdString().c_str(),
                                                         data.constData(), data.size(), NULL, 0, MZ_BEST_COMPRESSION)) {
            qWarning("Zipper : Error occurred 0x01");
            return false;
        }
    }

    // Spin for dirs
    for (auto dr : lsdir(dir)) {
        #if defined(Q_OS_DARWIN)
        if (dr == "__MACOSX")
            continue;
        #endif
        if (!mz_zip_add_mem_to_archive_file_in_place(outFilename.toStdString().c_str(), base.isEmpty() ?
                                                         (dr + separator()).toStdString().c_str() :
                                                         (base + separator() + dr + separator()).toStdString().c_str(),
                                                         NULL, 0, NULL, 0, MZ_BEST_COMPRESSION)) {
            qWarning("Zipper : Error occurred 0x02");
            return false;
        }
        if (!compressDir(dir + separator() + dr, outFilename, base.isEmpty() ? dr : base + separator() + dr)) {
            return false;
        }
    }
    return true;
}

bool Zipper::extractZip(const QByteArray& zipData, const QString& path)
{
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_mem(&zip, zipData.constData(), zipData.size(), 0)) {
        qWarning("Zipper : Error occurred 0x03");
        return false;
    }

    // Spin for dirs
    bool existsChecked = false;
    for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip); i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
            qWarning("Zipper : Error occurred 0x04");
            return false;
        }
        if (mz_zip_reader_is_file_a_directory(&zip, i)) {
            if (!existsChecked && exists(path + separator() + file_stat.m_filename)) {
                qWarning() << "Extraction cancelled : Dir exists!";
                return false;
            }
            existsChecked = true;
            QDir(path).mkpath(file_stat.m_filename);
        }
    }

    // Spin for data
    for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip); i++) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
            qWarning("Zipper : Error occurred 0x05");
            return false;
        }
        if (!mz_zip_reader_is_file_a_directory(&zip, i)) {
            if (!mz_zip_reader_extract_to_file(&zip, i, QString(path + separator() + file_stat.m_filename).toStdString().c_str(), 0)) {
                qWarning("Zipper : Error occurred 0x06");
                return false;
            }
        }
    }

    // Close the archive, freeing any resources it was using
	mz_zip_reader_end(&zip);
    return true;
}
