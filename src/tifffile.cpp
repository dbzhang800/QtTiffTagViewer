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
#include <algorithm>

Q_LOGGING_CATEGORY(tiffLog, "dbzhang800.tiffFile")

const static char *dataTypeName[] = { nullptr,     "BYTE",  "ASCII",     "SHORT",  "LONG",
                                      "RATIONAL",  "SBYTE", "UNDEFINED", "SSHORT", "SLONG",
                                      "SRATIONAL", "FLOAT", "DOUBLE",    "IFD",    "LONG8",
                                      "SLONG8",    "IFD8" };

const static struct TagNameAndValue
{
    const char *tagName;
    int tagValue;
} tagNames[] = {
    { "SUBFILETYPE", 254 },
    { "OSUBFILETYPE", 255 },
    { "IMAGEWIDTH", 256 },
    { "IMAGELENGTH", 257 },
    { "BITSPERSAMPLE", 258 },
    { "COMPRESSION", 259 },
    { "PHOTOMETRIC", 262 },
    { "THRESHHOLDING", 263 },
    { "CELLWIDTH", 264 },
    { "CELLLENGTH", 265 },
    { "FILLORDER", 266 },
    { "DOCUMENTNAME", 269 },
    { "IMAGEDESCRIPTION", 270 },
    { "MAKE", 271 },
    { "MODEL", 272 },
    { "STRIPOFFSETS", 273 },
    { "ORIENTATION", 274 },
    { "SAMPLESPERPIXEL", 277 },
    { "ROWSPERSTRIP", 278 },
    { "STRIPBYTECOUNTS", 279 },
    { "MINSAMPLEVALUE", 280 },
    { "MAXSAMPLEVALUE", 281 },
    { "XRESOLUTION", 282 },
    { "YRESOLUTION", 283 },
    { "PLANARCONFIG", 284 },
    { "PAGENAME", 285 },
    { "XPOSITION", 286 },
    { "YPOSITION", 287 },
    { "FREEOFFSETS", 288 },
    { "FREEBYTECOUNTS", 289 },
    { "GRAYRESPONSEUNIT", 290 },
    { "GRAYRESPONSECURVE", 291 },
    { "GROUP3OPTIONS", 292 },
    { "T4OPTIONS", 292 },
    { "GROUP4OPTIONS", 293 },
    { "T6OPTIONS", 293 },
    { "RESOLUTIONUNIT", 296 },
    { "PAGENUMBER", 297 },
    { "COLORRESPONSEUNIT", 300 },
    { "TRANSFERFUNCTION", 301 },
    { "SOFTWARE", 305 },
    { "DATETIME", 306 },
    { "ARTIST", 315 },
    { "HOSTCOMPUTER", 316 },
    { "PREDICTOR", 317 },
    { "WHITEPOINT", 318 },
    { "PRIMARYCHROMATICITIES", 319 },
    { "COLORMAP", 320 },
    { "HALFTONEHINTS", 321 },
    { "TILEWIDTH", 322 },
    { "TILELENGTH", 323 },
    { "TILEOFFSETS", 324 },
    { "TILEBYTECOUNTS", 325 },
    { "BADFAXLINES", 326 },
    { "CLEANFAXDATA", 327 },
    { "CONSECUTIVEBADFAXLINES", 328 },
    { "SUBIFD", 330 },
    { "INKSET", 332 },
    { "INKNAMES", 333 },
    { "NUMBEROFINKS", 334 },
    { "DOTRANGE", 336 },
    { "TARGETPRINTER", 337 },
    { "EXTRASAMPLES", 338 },
    { "SAMPLEFORMAT", 339 },
    { "SMINSAMPLEVALUE", 340 },
    { "SMAXSAMPLEVALUE", 341 },
    { "CLIPPATH", 343 },
    { "XCLIPPATHUNITS", 344 },
    { "YCLIPPATHUNITS", 345 },
    { "INDEXED", 346 },
    { "JPEGTABLES", 347 },
    { "OPIPROXY", 351 },
    { "GLOBALPARAMETERSIFD", 400 },
    { "PROFILETYPE", 401 },
    { "FAXPROFILE", 402 },
    { "CODINGMETHODS", 403 },
    { "VERSIONYEAR", 404 },
    { "MODENUMBER", 405 },
    { "DECODE", 433 },
    { "IMAGEBASECOLOR", 434 },
    { "T82OPTIONS", 435 },
    { "JPEGPROC", 512 },
    { "JPEGIFOFFSET", 513 },
    { "JPEGIFBYTECOUNT", 514 },
    { "JPEGRESTARTINTERVAL", 515 },
    { "JPEGLOSSLESSPREDICTORS", 517 },
    { "JPEGPOINTTRANSFORM", 518 },
    { "JPEGQTABLES", 519 },
    { "JPEGDCTABLES", 520 },
    { "JPEGACTABLES", 521 },
    { "YCBCRCOEFFICIENTS", 529 },
    { "YCBCRSUBSAMPLING", 530 },
    { "YCBCRPOSITIONING", 531 },
    { "REFERENCEBLACKWHITE", 532 },
    { "STRIPROWCOUNTS", 559 },
    { "XMLPACKET", 700 },
    { "OPIIMAGEID", 32781 },
    { "REFPTS", 32953 },
    { "REGIONTACKPOINT", 32954 },
    { "REGIONWARPCORNERS", 32955 },
    { "REGIONAFFINE", 32956 },
    { "MATTEING", 32995 },
    { "DATATYPE", 32996 },
    { "IMAGEDEPTH", 32997 },
    { "TILEDEPTH", 32998 },
    { "PIXAR_IMAGEFULLWIDTH", 33300 },
    { "PIXAR_IMAGEFULLLENGTH", 33301 },
    { "PIXAR_TEXTUREFORMAT", 33302 },
    { "PIXAR_WRAPMODES", 33303 },
    { "PIXAR_FOVCOT", 33304 },
    { "PIXAR_MATRIX_WORLDTOSCREEN", 33305 },
    { "PIXAR_MATRIX_WORLDTOCAMERA", 33306 },
    { "WRITERSERIALNUMBER", 33405 },
    { "CFAREPEATPATTERNDIM", 33421 },
    { "CFAPATTERN", 33422 },
    { "COPYRIGHT", 33432 },
    { "RICHTIFFIPTC", 33723 },
    { "IT8SITE", 34016 },
    { "IT8COLORSEQUENCE", 34017 },
    { "IT8HEADER", 34018 },
    { "IT8RASTERPADDING", 34019 },
    { "IT8BITSPERRUNLENGTH", 34020 },
    { "IT8BITSPEREXTENDEDRUNLENGTH", 34021 },
    { "IT8COLORTABLE", 34022 },
    { "IT8IMAGECOLORINDICATOR", 34023 },
    { "IT8BKGCOLORINDICATOR", 34024 },
    { "IT8IMAGECOLORVALUE", 34025 },
    { "IT8BKGCOLORVALUE", 34026 },
    { "IT8PIXELINTENSITYRANGE", 34027 },
    { "IT8TRANSPARENCYINDICATOR", 34028 },
    { "IT8COLORCHARACTERIZATION", 34029 },
    { "IT8HCUSAGE", 34030 },
    { "IT8TRAPINDICATOR", 34031 },
    { "IT8CMYKEQUIVALENT", 34032 },
    { "FRAMECOUNT", 34232 },
    { "PHOTOSHOP", 34377 },
    { "EXIFIFD", 34665 },
    { "ICCPROFILE", 34675 },
    { "IMAGELAYER", 34732 },
    { "JBIGOPTIONS", 34750 },
    { "GPSIFD", 34853 },
    { "FAXRECVPARAMS", 34908 },
    { "FAXSUBADDRESS", 34909 },
    { "FAXRECVTIME", 34910 },
    { "FAXDCS", 34911 },
    { "STONITS", 37439 },
    { "FEDEX_EDR", 34929 },
    { "INTEROPERABILITYIFD", 40965 },
    { "DNGVERSION", 50706 },
    { "DNGBACKWARDVERSION", 50707 },
    { "UNIQUECAMERAMODEL", 50708 },
    { "LOCALIZEDCAMERAMODEL", 50709 },
    { "CFAPLANECOLOR", 50710 },
    { "CFALAYOUT", 50711 },
    { "LINEARIZATIONTABLE", 50712 },
    { "BLACKLEVELREPEATDIM", 50713 },
    { "BLACKLEVEL", 50714 },
    { "BLACKLEVELDELTAH", 50715 },
    { "BLACKLEVELDELTAV", 50716 },
    { "WHITELEVEL", 50717 },
    { "DEFAULTSCALE", 50718 },
    { "DEFAULTCROPORIGIN", 50719 },
    { "DEFAULTCROPSIZE", 50720 },
    { "COLORMATRIX1", 50721 },
    { "COLORMATRIX2", 50722 },
    { "CAMERACALIBRATION1", 50723 },
    { "CAMERACALIBRATION2", 50724 },
    { "REDUCTIONMATRIX1", 50725 },
    { "REDUCTIONMATRIX2", 50726 },
    { "ANALOGBALANCE", 50727 },
    { "ASSHOTNEUTRAL", 50728 },
    { "ASSHOTWHITEXY", 50729 },
    { "BASELINEEXPOSURE", 50730 },
    { "BASELINENOISE", 50731 },
    { "BASELINESHARPNESS", 50732 },
    { "BAYERGREENSPLIT", 50733 },
    { "LINEARRESPONSELIMIT", 50734 },
    { "CAMERASERIALNUMBER", 50735 },
    { "LENSINFO", 50736 },
    { "CHROMABLURRADIUS", 50737 },
    { "ANTIALIASSTRENGTH", 50738 },
    { "SHADOWSCALE", 50739 },
    { "DNGPRIVATEDATA", 50740 },
    { "MAKERNOTESAFETY", 50741 },
    { "CALIBRATIONILLUMINANT1", 50778 },
    { "CALIBRATIONILLUMINANT2", 50779 },
    { "BESTQUALITYSCALE", 50780 },
    { "RAWDATAUNIQUEID", 50781 },
    { "ORIGINALRAWFILENAME", 50827 },
    { "ORIGINALRAWFILEDATA", 50828 },
    { "ACTIVEAREA", 50829 },
    { "MASKEDAREAS", 50830 },
    { "ASSHOTICCPROFILE", 50831 },
    { "ASSHOTPREPROFILEMATRIX", 50832 },
    { "CURRENTICCPROFILE", 50833 },
    { "CURRENTPREPROFILEMATRIX", 50834 },
    { "DCSHUESHIFTVALUES", 65535 },
    { "FAXMODE", 65536 },
    { "JPEGQUALITY", 65537 },
    { "JPEGCOLORMODE", 65538 },
    { "JPEGTABLESMODE", 65539 },
    { "FAXFILLFUNC", 65540 },
    { "PIXARLOGDATAFMT", 65549 },
    { "DCSIMAGERTYPE", 65550 },
    { "DCSINTERPMODE", 65551 },
    { "DCSBALANCEARRAY", 65552 },
    { "DCSCORRECTMATRIX", 65553 },
    { "DCSGAMMA", 65554 },
    { "DCSTOESHOULDERPTS", 65555 },
    { "DCSCALIBRATIONFD", 65556 },
    { "ZIPQUALITY", 65557 },
    { "PIXARLOGQUALITY", 65558 },
    { "DCSCLIPRECTANGLE", 65559 },
    { "SGILOGDATAFMT", 65560 },
    { "SGILOGENCODE", 65561 },
    { "LZMAPRESET", 65562 },
    { "PERSAMPLE", 65563 },
};

template <typename T>
static inline T getValueFromBytes(const char *bytes, TiffFile::ByteOrder byteOrder)
{
    if (byteOrder == TiffFile::LittleEndian)
        return qFromLittleEndian<T>(reinterpret_cast<const uchar *>(bytes));
    return qFromBigEndian<T>(reinterpret_cast<const uchar *>(bytes));
}

template <typename T>
static inline T fixValueByteOrder(T value, TiffFile::ByteOrder byteOrder)
{
    if (byteOrder == TiffFile::LittleEndian)
        return qFromLittleEndian<T>(value);
    return qFromBigEndian<T>(value);
}

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

    int typeSize()
    {
        switch (type) {
        case TiffFileIfdEntry::DT_Byte:
        case TiffFileIfdEntry::DT_SByte:
        case TiffFileIfdEntry::DT_Ascii:
        case TiffFileIfdEntry::DT_Undefined:
            return 1;
        case TiffFileIfdEntry::DT_Short:
        case TiffFileIfdEntry::DT_SShort:
            return 2;
        case TiffFileIfdEntry::DT_Long:
        case TiffFileIfdEntry::DT_SLong:
        case TiffFileIfdEntry::DT_Ifd:
        case TiffFileIfdEntry::DT_Float:
            return 4;
        default:
            return 8;
        }
    }

    void parserValues(const char *bytes, TiffFile::ByteOrder byteOrder);

    quint16 tag;
    quint16 type;
    quint64 count{ 0 };
    QByteArray valueOrOffset; // 12 bytes for tiff or 20 bytes for bigTiff
    QVariantList values;
};

void TiffFileIfdEntryPrivate::parserValues(const char *bytes, TiffFile::ByteOrder byteOrder)
{
    if (type == TiffFileIfdEntry::DT_Ascii) {
        int start = 0;
        for (int i = 0; i < count; ++i) {
            if (bytes[i] == '\0') {
                values.append(QString::fromLatin1(bytes + start, i - start + 1));
                start = i + 1;
            }
        }
        if (bytes[count - 1] != '\0') {
            qCDebug(tiffLog) << "ASCII value donesn't end with NUL";
            values.append(QString::fromLatin1(bytes + start, count - start));
        }
        return;
    }

    if (type == TiffFileIfdEntry::DT_Undefined) {
        values.append(QByteArray(bytes, count));
        return;
    }

    // To make things simple, save normal integer as qint32 or quint32 here.
    for (int i = 0; i < count; ++i) {
        QVariant variant;
        switch (type) {
        case TiffFileIfdEntry::DT_Byte:
            values.append(static_cast<quint32>(bytes[i]));
            break;
        case TiffFileIfdEntry::DT_SByte:
            values.append(static_cast<qint32>(bytes[i]));
            break;
        case TiffFileIfdEntry::DT_Short:
            values.append(
                static_cast<quint32>(getValueFromBytes<quint16>(bytes + i * 2, byteOrder)));
            break;
        case TiffFileIfdEntry::DT_SShort:
            values.append(static_cast<qint32>(getValueFromBytes<qint16>(bytes + i * 2, byteOrder)));
            break;
        case TiffFileIfdEntry::DT_Long:
        case TiffFileIfdEntry::DT_Ifd:
            values.append(getValueFromBytes<quint32>(bytes + i * 4, byteOrder));
            break;
        case TiffFileIfdEntry::DT_SLong:
            values.append(getValueFromBytes<qint32>(bytes + i * 4, byteOrder));
            break;
        case TiffFileIfdEntry::DT_Float:
            values.append(*(reinterpret_cast<const float *>(bytes + i * 4)));
            break;
        case TiffFileIfdEntry::DT_Double:
            values.append(*(reinterpret_cast<const double *>(bytes + i * 8)));
            break;
        case TiffFileIfdEntry::DT_Rational:
            values.append(getValueFromBytes<quint32>(bytes + i * 4, byteOrder));
            values.append(getValueFromBytes<quint32>(bytes + i * 4, byteOrder));
            break;
        case TiffFileIfdEntry::DT_SRational:
            values.append(getValueFromBytes<qint32>(bytes + i * 4, byteOrder));
            values.append(getValueFromBytes<qint32>(bytes + i * 4, byteOrder));
            break;
        case TiffFileIfdEntry::DT_Long8:
        case TiffFileIfdEntry::DT_Ifd8:
            values.append(getValueFromBytes<quint64>(bytes + i * 8, byteOrder));
            break;
        case TiffFileIfdEntry::DT_SLong8:
            values.append(getValueFromBytes<qint64>(bytes + i * 8, byteOrder));
            break;
        default:
            break;
        }
    }
}

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

QString TiffFileIfdEntry::tagName() const
{
    auto it =
        std::find_if(std::begin(tagNames), std::end(tagNames),
                     [this](const TagNameAndValue &tagName) { return tagName.tagValue == d->tag; });
    if (it != std::end(tagNames))
        return QString::fromLatin1(it->tagName);

    return QStringLiteral("UnknownTag");
}

quint16 TiffFileIfdEntry::type() const
{
    return d->type;
}

QString TiffFileIfdEntry::typeName() const
{
    if (d->type > 0 && d->type < TiffFileIfdEntry::DT_Ifd8)
        return dataTypeName[d->type];

    return QString();
}

quint64 TiffFileIfdEntry::count() const
{
    return d->count;
}

QByteArray TiffFileIfdEntry::valueOrOffset() const
{
    return d->valueOrOffset;
}

QVariantList TiffFileIfdEntry::values() const
{
    return d->values;
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

    bool hasIfdEntry(quint16 tag);
    TiffFileIfdEntry ifdEntry(quint16 tag);

    QVector<TiffFileIfdEntry> ifdEntries;
    QVector<TiffFileIfd> subIfds;
    qint64 nextIfdOffset{ 0 };
};

bool TiffFileIfdPrivate::hasIfdEntry(quint16 tag)
{
    return ifdEntry(tag).isValid();
}

TiffFileIfdEntry TiffFileIfdPrivate::ifdEntry(quint16 tag)
{
    auto it = std::find_if(ifdEntries.cbegin(), ifdEntries.cend(),
                           [tag](const TiffFileIfdEntry &de) { return tag == de.tag(); });
    if (it == ifdEntries.cend())
        return TiffFileIfdEntry();
    return *it;
}

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
    T getValueFromFile()
    {
        T v{ 0 };
        auto bytesRead = file.read(reinterpret_cast<char *>(&v), sizeof(T));
        if (bytesRead != sizeof(T))
            qCDebug(tiffLog) << "file read error.";
        return fixValueByteOrder(v, header.byteOrder);
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
    header.version = getValueFromBytes<quint16>(headerBytes.data() + 2, header.byteOrder);
    if (!(header.version == 42 || header.version == 43)) {
        setError(QStringLiteral("Invalid tiff file: Unknown version"));
        return false;
    }
    header.rawBytes = file.read(header.isBigTiff() ? 16 : 8);

    // ifd0Offset
    if (!header.isBigTiff())
        header.ifd0Offset =
            getValueFromBytes<quint32>(header.rawBytes.data() + 4, header.byteOrder);
    else
        header.ifd0Offset = getValueFromBytes<qint64>(header.rawBytes.data() + 8, header.byteOrder);

    return true;
}

bool TiffFilePrivate::readIfd(qint64 offset, TiffFileIfd *parentIfd)
{
    if (!file.seek(offset)) {
        setError(file.errorString());
        return false;
    }

    TiffFileIfd ifd;

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
        }
        ifd.d->nextIfdOffset = getValueFromFile<qint64>();
    }

    // parser data of ifdEntry
    foreach (auto de, ifd.ifdEntries()) {
        auto &dePrivate = de.d;

        auto valueBytesCount = dePrivate->count * dePrivate->typeSize();
        QByteArray valueBytes;
        if (!header.isBigTiff() && valueBytesCount > 4) {
            auto valueOffset = getValueFromBytes<quint32>(de.valueOrOffset(), header.byteOrder);
            if (!file.seek(valueOffset))
                qCDebug(tiffLog) << "Fail to seek pos: " << valueOffset;
            valueBytes = file.read(valueBytesCount);
        } else if (header.isBigTiff() && valueBytesCount > 8) {
            auto valueOffset = getValueFromBytes<quint64>(de.valueOrOffset(), header.byteOrder);
            if (!file.seek(valueOffset))
                qCDebug(tiffLog) << "Fail to seek pos: " << valueOffset;
            valueBytes = file.read(valueBytesCount);
        } else {
            valueBytes = dePrivate->valueOrOffset;
        }
        dePrivate->parserValues(valueBytes, header.byteOrder);
    }

    if (!parentIfd) // IFD0
        ifds.append(ifd);
    else // subIfd
        parentIfd->d->subIfds.append(ifd);

    // get sub ifd offsets
    // Fix me!!!
    // I couldn't parse ifds in photoshop generated Tiff files.
    if (!ifd.d->hasIfdEntry(TiffFileIfdEntry::T_Photoshop)) {
        TiffFileIfdEntry deSubIfd = ifd.d->ifdEntry(TiffFileIfdEntry::T_SubIfd);
        foreach (auto subIfdOffset, deSubIfd.values()) {
            // read sub ifds
            readIfd(subIfdOffset.toULongLong(), &ifd);
        }
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
