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
#pragma once
#include <QScopedPointer>
#include <QVector>
#include <QExplicitlySharedDataPointer>

class QByteArray;
class TiffFileIfdEntryPrivate;
class TiffFileIfdPrivate;
class TiffFilePrivate;

class TiffFileIfdEntry
{
public:
    enum DataType {
        DT_Byte = 1,
        DT_Ascii,
        DT_Short,
        DT_Long,
        DT_Rational,
        DT_SByte,
        DT_Undefined,
        DT_SShort,
        DT_SLong,
        DT_SRational,
        DT_Float,
        DT_Double,
        DT_Ifd,
        DT_Long8,
        DT_Slong8,
        DT_Ifd8
    };

    TiffFileIfdEntry();
    TiffFileIfdEntry(const TiffFileIfdEntry &other);
    ~TiffFileIfdEntry();

    quint16 tag() const;
    quint16 type() const;
    quint64 count() const;
    QByteArray valueOrOffset() const;
    bool isValid() const;

private:
    friend class TiffFilePrivate;
    QExplicitlySharedDataPointer<TiffFileIfdEntryPrivate> d;
};

class TiffFileIfd
{
public:
    TiffFileIfd();
    TiffFileIfd(const TiffFileIfd &other);
    ~TiffFileIfd();

    QVector<TiffFileIfdEntry> ifdEntries() const;
    QVector<TiffFileIfd> subIfds() const;
    qint64 nextIfdOffset() const;
    bool isValid() const;

private:
    friend class TiffFilePrivate;
    QExplicitlySharedDataPointer<TiffFileIfdPrivate> d;
};

class TiffFile
{
public:
    enum ByteOrder { LittleEndian, BigEndian };

    TiffFile(const QString &filePath);
    ~TiffFile();

    QString errorString() const;
    bool hasError() const;

    // header information
    QByteArray headerBytes() const;
    bool isBigTiff() const;
    ByteOrder byteOrder() const;
    int version() const;
    qint64 ifd0Offset() const;

    // ifds
    QVector<TiffFileIfd> ifds() const;

private:
    QScopedPointer<TiffFilePrivate> d;
};
