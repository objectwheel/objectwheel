#include <zipper.h>
#include <miniz.h>
#include <filemanager.h>
#include <QDir>

void Zipper::compressDir(const QString& dir, const QString& outFilename, const QString& base)
{
	for (auto file : lsfile(dir)) {
		auto data = rdfile(dir + separator() + file);
		Q_ASSERT(mz_zip_add_mem_to_archive_file_in_place(outFilename.toStdString().c_str(), base.isEmpty() ?
														 file.toStdString().c_str() : (base + separator() + file).toStdString().c_str(),
														 data.constData(), data.size(), NULL, 0, MZ_BEST_COMPRESSION));
	}

	for (auto dr : lsdir(dir)) {
		Q_ASSERT(mz_zip_add_mem_to_archive_file_in_place(outFilename.toStdString().c_str(), base.isEmpty() ?
														 (dr + separator()).toStdString().c_str() :
														 (base + separator() + dr + separator()).toStdString().c_str(),
														 NULL, 0, NULL, 0, MZ_BEST_COMPRESSION));
		compressDir(dir + separator() + dr, outFilename, base.isEmpty() ? dr : base + separator() + dr);
	}
}

void Zipper::extractZip(const QByteArray& zipData, const QString& path)
{
	mz_zip_archive zip;
	memset(&zip, 0, sizeof(zip));
	Q_ASSERT(mz_zip_reader_init_mem(&zip, zipData.constData(), zipData.size(), 0));

	// Spin for dirs
	for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip); i++) {
		mz_zip_archive_file_stat file_stat;
		Q_ASSERT(mz_zip_reader_file_stat(&zip, i, &file_stat));
		if (mz_zip_reader_is_file_a_directory(&zip, i)) QDir(path).mkpath(file_stat.m_filename);
	}

	// Spin for data
	for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip); i++) {
		mz_zip_archive_file_stat file_stat;
		Q_ASSERT(mz_zip_reader_file_stat(&zip, i, &file_stat));
		if (!mz_zip_reader_is_file_a_directory(&zip, i)) {
			Q_ASSERT(mz_zip_reader_extract_to_file(&zip, i, QString(path+"/"+file_stat.m_filename).toStdString().c_str(), 0));
		}
	}

	// Close the archive, freeing any resources it was using
	mz_zip_reader_end(&zip);
}
