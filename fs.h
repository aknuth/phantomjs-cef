#ifndef _FILE_H_
#define _FILE_H_

#include <QStringList>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QVariant>

class File : public QObject
{
    Q_OBJECT

public:
    // handle a textfile with given codec
    // if @p codec is null, the file is considered to be binary
    File(QFile* openfile);
    virtual ~File();

public slots:
	QString read(const QVariant& n = -1);
    bool write(const QString& data);

    void flush();
    void close();

    QString getEncoding() const;
    bool setEncoding(const QString& encoding);

private:
    bool _isUnbuffered() const;

    QFile* m_file;
    QTextStream* m_fileStream;
};

class FileSystem : public QObject
{
    Q_OBJECT

public:
    FileSystem();

public slots:
    // Attributes
    // 'size(path)' implemented in "filesystem-shim.js" using '_size(path)'
    int _size(const QString& path) const;
    QVariant lastModified(const QString& path) const;

    // Listing
    QStringList list(const QString& path) const;

    QString tmpPath() const;

    // Tests
    bool exists(const QString& path) const;
    bool isDirectory(const QString& path) const;
    bool isFile(const QString& path) const;
    bool isAbsolute(const QString& path) const;
    bool isExecutable(const QString& path) const;
    bool isReadable(const QString& path) const;
    bool isWritable(const QString& path) const;
    bool isLink(const QString& path) const;
    bool makeDirectory(const QString& path) const;
    bool copyRecursively(const QString &srcFilePath,const QString &tgtFilePath) const;
    bool remove(const QString& path) const;

};

#endif
