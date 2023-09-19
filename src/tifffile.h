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
#include <QVariant>
#include <QExplicitlySharedDataPointer>

class QByteArray;
class TiffIfdEntryPrivate;
class TiffIfdPrivate;
class TiffFilePrivate;

struct TiffParserOptions
{
    bool parserSubIfds{ true };
};

class TiffIfdEntry
{
public:
    enum Tag {
        T_SubFleType = 254,
        T_ImageWidth = 256,
        T_ImageLength = 257,
        T_Compression = 259,
        T_SubIfd = 330,
        T_Photoshop = 34377,
    };

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
        DT_SLong8,
        DT_Ifd8
    };

    TiffIfdEntry();
    TiffIfdEntry(const TiffIfdEntry &other);
    ~TiffIfdEntry();

    quint16 tag() const;
    QString tagName() const;
    quint16 type() const;
    QString typeName() const;
    quint64 count() const;
    QByteArray valueOrOffset() const;
    QVariantList values() const;
    QString valueDescription() const;
    bool isValid() const;

private:
    friend class TiffFilePrivate;
    QExplicitlySharedDataPointer<TiffIfdEntryPrivate> d;
};

class TiffIfd
{
public:
    TiffIfd();
    TiffIfd(const TiffIfd &other);
    ~TiffIfd();

    QVector<TiffIfdEntry> ifdEntries() const;
    QVector<TiffIfd> subIfds() const;
    qint64 nextIfdOffset() const;
    bool isValid() const;

private:
    friend class TiffFilePrivate;
    QExplicitlySharedDataPointer<TiffIfdPrivate> d;
};

class TiffFile
{
public:
    enum ByteOrder { LittleEndian, BigEndian };

    TiffFile(const QString &filePath, const TiffParserOptions &options);
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
    QVector<TiffIfd> ifds() const;

private:
    QScopedPointer<TiffFilePrivate> d;
};
