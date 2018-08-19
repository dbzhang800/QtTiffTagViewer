/****************************************************************************
** Copyright (c) 2018 Debao Zhang <hello@debao.me>
** All right reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#include "tifffile.h"
#include <QFile>
#include <QLoggingCategory>
#include <QtEndian>
#include <QSharedData>

Q_LOGGING_CATEGORY(tiffLog, "dbzhang800.tiffFile")

class TiffFileIfdEntryPrivate : public QSharedData
{
public:
    TiffFileIfdEntryPrivate() {}
    TiffFileIfdEntryPrivate(const TiffFileIfdEntryPrivate &other)
        : QSharedData(other)
        , tag(other.tag)
        , type(other.type)
        , count(other.count)
        , valueOrOffset(other.valueOrOffset)
    {
    }
    ~TiffFileIfdEntryPrivate() {}

    quint16 tag;
    quint16 type;
    quint64 count{ 0 };
    QByteArray valueOrOffset; // 12 bytes for tiff or 20 bytes for bigTiff
};

/*!
 * \class TiffFileIfdEntry
 */

TiffFileIfdEntry::TiffFileIfdEntry()
    : d(new TiffFileIfdEntryPrivate)
{
}

TiffFileIfdEntry::TiffFileIfdEntry(const TiffFileIfdEntry &other)
    : d(other.d)
{
}

TiffFileIfdEntry::~TiffFileIfdEntry()
{
}

quint16 TiffFileIfdEntry::tag() const
{
    return d->tag;
}

quint16 TiffFileIfdEntry::type() const
{
    return d->type;
}

quint64 TiffFileIfdEntry::count() const
{
    return d->count;
}

QByteArray TiffFileIfdEntry::valueOrOffset() const
{
    return d->valueOrOffset;
}

bool TiffFileIfdEntry::isValid() const
{
    return d->count;
}

class TiffFileIfdPrivate : public QSharedData
{
public:
    TiffFileIfdPrivate() {}
    TiffFileIfdPrivate(const TiffFileIfdPrivate &other)
        : QSharedData(other)
        , ifdEntries(other.ifdEntries)
        , subIfds(other.subIfds)
        , nextIfdOffset(other.nextIfdOffset)
    {
    }
    ~TiffFileIfdPrivate() {}

    QVector<TiffFileIfdEntry> ifdEntries;
    QVector<TiffFileIfd> subIfds;
    qint64 nextIfdOffset{ 0 };
};

/*!
 * \class TiffFileIfd
 */

TiffFileIfd::TiffFileIfd()
    : d(new TiffFileIfdPrivate)
{
}

TiffFileIfd::TiffFileIfd(const TiffFileIfd &other)
    : d(other.d)
{
}

TiffFileIfd::~TiffFileIfd()
{
}

QVector<TiffFileIfdEntry> TiffFileIfd::ifdEntries() const
{
    return d->ifdEntries;
}

QVector<TiffFileIfd> TiffFileIfd::subIfds() const
{
    return d->subIfds;
}

qint64 TiffFileIfd::nextIfdOffset() const
{
    return d->nextIfdOffset;
}

bool TiffFileIfd::isValid() const
{
    return !d->ifdEntries.isEmpty();
}

class TiffFilePrivate
{
public:
    TiffFilePrivate();
    void setError(const QString &errorString);
    bool readHeader();
    bool readIfd(qint64 offset, TiffFileIfd *parentIfd = nullptr);

    template <typename T>
    T getValueFromBytes(const char *bytes)
    {
        if (header.byteOrder == TiffFile::LittleEndian)
            return qFromLittleEndian<T>(reinterpret_cast<const uchar *>(bytes));
        return qFromBigEndian<T>(reinterpret_cast<const uchar *>(bytes));
    }

    template <typename T>
    T fixValueByteOrder(T value)
    {
        if (header.byteOrder == TiffFile::LittleEndian)
            return qFromLittleEndian<T>(value);
        return qFromBigEndian<T>(value);
    }

    template <typename T>
    T getValueFromFile()
    {
        T v{ 0 };
        file.read(reinterpret_cast<char *>(&v), sizeof(T));
        return fixValueByteOrder(v);
    }

    struct Header
    {
        QByteArray rawBytes;
        TiffFile::ByteOrder byteOrder{ TiffFile::LittleEndian };
        quint16 version{ 42 };
        qint64 ifd0Offset{ 0 };

        bool isBigTiff() const { return version == 43; }
    } header;

    QVector<TiffFileIfd> ifds;

    QFile file;
    QString errorString;
    bool hasError{ false };
};

TiffFilePrivate::TiffFilePrivate()
{
}

void TiffFilePrivate::setError(const QString &errorString)
{
    hasError = true;
    this->errorString = errorString;
}

bool TiffFilePrivate::readHeader()
{
    auto headerBytes = file.peek(8);
    if (headerBytes.size() != 8) {
        setError(QStringLiteral("Invalid tiff file"));
        return false;
    }

    // magic bytes
    auto magicBytes = headerBytes.left(2);
    if (magicBytes == QByteArray("II")) {
        header.byteOrder = TiffFile::LittleEndian;
    } else if (magicBytes == QByteArray("MM")) {
        header.byteOrder = TiffFile::BigEndian;
    } else {
        setError(QStringLiteral("Invalid tiff file"));
        return false;
    }

    // version
    header.version = getValueFromBytes<quint16>(headerBytes.data() + 2);
    if (!(header.version == 42 || header.version == 43)) {
        setError(QStringLiteral("Invalid tiff file: Unknown version"));
        return false;
    }
    header.rawBytes = file.read(header.isBigTiff() ? 16 : 8);

    // ifd0Offset
    if (!header.isBigTiff())
        header.ifd0Offset = getValueFromBytes<quint32>(header.rawBytes.data() + 4);
    else
        header.ifd0Offset = getValueFromBytes<qint64>(header.rawBytes.data() + 8);

    return true;
}

bool TiffFilePrivate::readIfd(qint64 offset, TiffFileIfd *parentIfd)
{
    if (!file.seek(offset)) {
        setError(file.errorString());
        return false;
    }

    TiffFileIfd ifd;
    TiffFileIfdEntry deSubIfds;

    if (!header.isBigTiff()) {
        quint16 deCount = getValueFromFile<quint16>();
        for (int i = 0; i < deCount; ++i) {
            TiffFileIfdEntry ifdEntry;
            auto &dePrivate = ifdEntry.d;
            dePrivate->tag = getValueFromFile<quint16>();
            dePrivate->type = getValueFromFile<quint16>();
            dePrivate->count = getValueFromFile<quint32>();
            dePrivate->valueOrOffset = file.read(4);

            ifd.d->ifdEntries.append(ifdEntry);
            // subIfds tag
            if (dePrivate->tag == 330)
                deSubIfds = ifdEntry;
        }
        ifd.d->nextIfdOffset = getValueFromFile<quint32>();
    } else {
        quint64 deCount = getValueFromFile<quint64>();
        for (quint64 i = 0; i < deCount; ++i) {
            TiffFileIfdEntry ifdEntry;
            auto &dePrivate = ifdEntry.d;
            dePrivate->tag = getValueFromFile<quint16>();
            dePrivate->type = getValueFromFile<quint16>();
            dePrivate->count = getValueFromFile<quint64>();
            dePrivate->valueOrOffset = file.read(8);

            ifd.d->ifdEntries.append(ifdEntry);
            // subIfds tag
            if (dePrivate->tag == 330)
                deSubIfds = ifdEntry;
        }
        ifd.d->nextIfdOffset = getValueFromFile<qint64>();
    }

    if (!parentIfd) // IFD0
        ifds.append(ifd);
    else // subIfd
        parentIfd->d->subIfds.append(ifd);

    // get sub ifd offsets
    QVector<qint64> subIfdOffsets;
    if (deSubIfds.isValid()) {
        if (!header.isBigTiff()) {
            if (deSubIfds.type() == TiffFileIfdEntry::DT_Long
                || deSubIfds.type() == TiffFileIfdEntry::DT_Ifd) {
                if (deSubIfds.count() == 1) {
                    auto subIfdOffset = getValueFromBytes<qint32>(deSubIfds.valueOrOffset());
                    subIfdOffsets.append(subIfdOffset);
                } else {
                    auto valueOffset = getValueFromBytes<quint32>(deSubIfds.valueOrOffset());
                    file.seek(valueOffset);
                    for (int i = 0; i < deSubIfds.count(); ++i) {
                        auto subIfdOffset = getValueFromFile<qint32>();
                        subIfdOffsets.append(subIfdOffset);
                    }
                }
            } else {
                // Invalid data type
                qCWarning(tiffLog) << "Invalid data type for subIfds encountered";
            }
        } else {
            if (deSubIfds.type() == TiffFileIfdEntry::DT_Long
                || deSubIfds.type() == TiffFileIfdEntry::DT_Ifd) {
                QByteArray valueBytes;
                if (deSubIfds.count() <= 2) {
                    valueBytes = deSubIfds.valueOrOffset();
                } else {
                    auto valueOffset = getValueFromBytes<qint64>(deSubIfds.valueOrOffset());
                    file.seek(valueOffset);
                    valueBytes = file.read(deSubIfds.count() * 4);
                }
                const char *bytes = valueBytes.data();
                for (int i = 0; i < deSubIfds.count(); ++i) {
                    auto subIfdOffset = getValueFromBytes<qint32>(bytes);
                    subIfdOffsets.append(subIfdOffset);
                }
            } else if (deSubIfds.type() == TiffFileIfdEntry::DT_Long8
                       || deSubIfds.type() == TiffFileIfdEntry::DT_Ifd8) {
                if (deSubIfds.count() == 1) {
                    subIfdOffsets.append(getValueFromBytes<qint64>(deSubIfds.valueOrOffset()));
                } else {
                    auto valueOffset = getValueFromBytes<qint64>(deSubIfds.valueOrOffset());
                    file.seek(valueOffset);
                    for (int i = 0; i < deSubIfds.count(); ++i) {
                        auto subIfdOffset = getValueFromFile<qint64>();
                        subIfdOffsets.append(subIfdOffset);
                    }
                }
            } else {
                // Invalid data type
                qCWarning(tiffLog) << "Invalid data type for subIfds encountered";
            }
        }
    }

    foreach (auto subIfdOffset, subIfdOffsets) {
        // read sub ifds
        readIfd(subIfdOffset, &ifd);
    }

    if (ifd.nextIfdOffset() != 0) {
        // Read next ifd in the chain
        readIfd(ifd.nextIfdOffset(), parentIfd);
    }

    return true;
}

/*!
 * \class TiffFile
 */

/*!
 * Constructs the TiffFile object.
 */
TiffFile::TiffFile(const QString &filePath)
    : d(new TiffFilePrivate)
{
    d->file.setFileName(filePath);
    if (!d->file.open(QFile::ReadOnly)) {
        d->hasError = true;
        d->errorString = d->file.errorString();
    }

    if (!d->readHeader())
        return;

    d->readIfd(d->header.ifd0Offset);
}

TiffFile::~TiffFile()
{
}

QByteArray TiffFile::headerBytes() const
{
    return d->header.rawBytes;
}

bool TiffFile::isBigTiff() const
{
    return d->header.isBigTiff();
}

TiffFile::ByteOrder TiffFile::byteOrder() const
{
    return d->header.byteOrder;
}

int TiffFile::version() const
{
    return d->header.version;
}

qint64 TiffFile::ifd0Offset() const
{
    return d->header.ifd0Offset;
}

QVector<TiffFileIfd> TiffFile::ifds() const
{
    return d->ifds;
}

QString TiffFile::errorString() const
{
    return d->errorString;
}

bool TiffFile::hasError() const
{
    return d->hasError;
}
