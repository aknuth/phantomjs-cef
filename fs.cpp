#include "fs.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDateTime>

// File
// public:
File::File(QFile* openfile) :
	m_file(openfile),
    m_fileStream(0)
{
    m_fileStream = new QTextStream(m_file);
    m_fileStream->setCodec(QTextCodec::codecForName("utf-8"));
}

File::~File()
{
    this->close();
}

// public slots:
QString File::read(const QVariant& n)
{
    // Default to 1024 (used when n is "null")
    qint64 bytesToRead = 1024;

    // If parameter can be converted to a qint64, do so and use that value instead
    if (n.canConvert(QVariant::LongLong)) {
        bytesToRead = n.toLongLong();
    }

    const bool isReadAll = 0 > bytesToRead;

    if (!m_file->isReadable()) {
        qDebug() << "File::read - " << "Couldn't read:" << m_file->fileName();
        return QString();
    }
    if (m_file->isWritable()) {
        // make sure we write everything to disk before reading
        flush();
    }
    if (m_fileStream) {
        // text file
        QString ret;
        if (isReadAll) {
            // This code, for some reason, reads the whole file from 0 to EOF,
            // and then resets to the position the file was at prior to reading
            const qint64 pos = m_fileStream->pos();
            m_fileStream->seek(0);
            ret = m_fileStream->readAll();
            m_fileStream->seek(pos);
        } else {
            ret = m_fileStream->read(bytesToRead);
        }
        return ret;
    } else {
        // binary file
        QByteArray data;
        if (isReadAll) {
            // This code, for some reason, reads the whole file from 0 to EOF,
            // and then resets to the position the file was at prior to reading
            const qint64 pos = m_file->pos();
            m_file->seek(0);
            data = m_file->readAll();
            m_file->seek(pos);
        } else {
            data = m_file->read(bytesToRead);
        }
        QString ret(data.size(), ' ');
        for (int i = 0; i < data.size(); ++i) {
            ret[i] = data.at(i);
        }
        return ret;
    }
}


bool File::write(const QString& data)
{
    if (!m_file->isWritable()) {
        qDebug() << "File::write - " << "Couldn't write:" << m_file->fileName();
        return true;
    }
    if (m_fileStream) {
        // text file
        (*m_fileStream) << data;
        if (_isUnbuffered()) {
            m_fileStream->flush();
        }
        return true;
    } else {
        // binary file
        QByteArray bytes(data.size(), Qt::Uninitialized);
        for (int i = 0; i < data.size(); ++i) {
            bytes[i] = data.at(i).toLatin1();
        }
        return m_file->write(bytes);
    }
}

void File::flush()
{
    if (m_file) {
        if (m_fileStream) {
            // text file
            m_fileStream->flush();
        }
        // binary or text file
        m_file->flush();
    }
}

void File::close()
{
    flush();
    if (m_fileStream) {
        delete m_fileStream;
        m_fileStream = 0;
    }
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = NULL;
    }
    deleteLater();
}

bool File::setEncoding(const QString& encoding)
{
    if (encoding.isEmpty() || encoding.isNull()) {
        return false;
    }

    // "Binary" mode doesn't use/need text codecs
    if ((QTextStream*)NULL == m_fileStream) {
        // TODO: Should we switch to "text" mode?
        return false;
    }

    // Since there can be multiple names for the same codec (i.e., "utf8" and
    // "utf-8"), we need to get the codec in the system first and use its
    // canonical name
    QTextCodec* codec = QTextCodec::codecForName(encoding.toLatin1());
    if ((QTextCodec*)NULL == codec) {
        return false;
    }

    // Check whether encoding actually needs to be changed
    const QString encodingBeforeUpdate(m_fileStream->codec()->name());
    if (0 == encodingBeforeUpdate.compare(QString(codec->name()), Qt::CaseInsensitive)) {
        return false;
    }

    m_fileStream->setCodec(codec);

    // Return whether update was successful
    const QString encodingAfterUpdate(m_fileStream->codec()->name());
    return 0 != encodingBeforeUpdate.compare(encodingAfterUpdate, Qt::CaseInsensitive);
}

QString File::getEncoding() const
{
    QString encoding;

    if ((QTextStream*)NULL != m_fileStream) {
        encoding = QString(m_fileStream->codec()->name());
    }

    return encoding;
}

// private:

bool File::_isUnbuffered() const
{
    return m_file->openMode() & QIODevice::Unbuffered;
}


// FileSystem
// public:
FileSystem::FileSystem()
{ }

// public slots:

// Attributes
int FileSystem::_size(const QString& path) const
{
    QFileInfo fi(path);
    if (fi.exists()) {
        return fi.size();
    }
    return -1;
}

QVariant FileSystem::lastModified(const QString& path) const
{
    QFileInfo fi(path);
    if (fi.exists()) {
        return QVariant(fi.lastModified());
    }
    return QVariant(QDateTime());
}

QStringList FileSystem::list(const QString& path) const
{
    return QDir(path).entryList();
}

bool FileSystem::exists(const QString& path) const
{
    return QFile::exists(path);
}

bool FileSystem::isDirectory(const QString& path) const
{
    return QFileInfo(path).isDir();
}

bool FileSystem::isFile(const QString& path) const
{
    return QFileInfo(path).isFile();
}

bool FileSystem::isAbsolute(const QString& path) const
{
    return QFileInfo(path).isAbsolute();
}

bool FileSystem::isExecutable(const QString& path) const
{
    return QFileInfo(path).isExecutable();
}

bool FileSystem::isLink(const QString& path) const
{
    return QFileInfo(path).isSymLink();
}

bool FileSystem::isReadable(const QString& path) const
{
    return QFileInfo(path).isReadable();
}

bool FileSystem::isWritable(const QString& path) const
{
    return QFileInfo(path).isWritable();
}
bool FileSystem::makeDirectory(const QString& path) const
{
    return QDir().mkdir(path);
}
bool FileSystem::copyRecursively(const QString &srcFilePath,
                            const QString &tgtFilePath) const
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
    	QDir sourceDir(srcFilePath);
    	QDir::Filters sourceDirFilter = QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files | QDir::NoSymLinks | QDir::Drives;

    	if (!FileSystem::exists(tgtFilePath) && !FileSystem::makeDirectory(tgtFilePath)) {
            return false;
        }

        foreach (QFileInfo entry, sourceDir.entryInfoList(sourceDirFilter, QDir::DirsFirst)) {
            if (entry.isDir()) {
                if (!FileSystem::copyRecursively(entry.absoluteFilePath(),
                		tgtFilePath + "/" + entry.fileName())) { //< directory: recursive call
                    return false;
                }
            } else {
                if (!FileSystem::copyRecursively(entry.absoluteFilePath(),
                		tgtFilePath + "/" + entry.fileName())) { //< file: copy
                    return false;
                }
            }
        }
    } else {
    	QFileInfo tgtFileInfo(tgtFilePath);
    	QFileInfo srcFileInfo(srcFilePath);
    	if (tgtFileInfo.isDir()){
    		qDebug() << tgtFilePath, + "/" + srcFileInfo.fileName();
    		if (!QFile::copy(srcFilePath, tgtFilePath + "/" + srcFileInfo.fileName()))
    		            return false;
    	} else {
            if (!QFile::copy(srcFilePath, tgtFilePath))
                return false;

    	}
    }
    return true;
}
bool FileSystem::remove(const QString& path) const{
	QFileInfo fileInfo(path);
	if (fileInfo.isDir()){
		QDir dir(path);
		if (!dir.removeRecursively())
			return false;
	}else {
		QFile f(path);
		if (!f.remove())
			return false;
	}
	return true;
}

QString FileSystem::tmpPath() const{
	return QDir::tempPath();
}
