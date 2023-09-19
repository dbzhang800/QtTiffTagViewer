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

const static char *g_dataTypeName[] = { nullptr,     "BYTE",  "ASCII",     "SHORT",  "LONG",
                                        "RATIONAL",  "SBYTE", "UNDEFINED", "SSHORT", "SLONG",
                                        "SRATIONAL", "FLOAT", "DOUBLE",    "IFD",    "LONG8",
                                        "SLONG8",    "IFD8" };

const static QMap<int, QByteArray> g_tagNames = {
    { 254, "SUBFILETYPE" },
    { 255, "OSUBFILETYPE" },
    { 256, "IMAGEWIDTH" },
    { 257, "IMAGELENGTH" },
    { 258, "BITSPERSAMPLE" },
    { 259, "COMPRESSION" },
    { 262, "PHOTOMETRIC" },
    { 263, "THRESHHOLDING" },
    { 264, "CELLWIDTH" },
    { 265, "CELLLENGTH" },
    { 266, "FILLORDER" },
    { 269, "DOCUMENTNAME" },
    { 270, "IMAGEDESCRIPTION" },
    { 271, "MAKE" },
    { 272, "MODEL" },
    { 273, "STRIPOFFSETS" },
    { 274, "ORIENTATION" },
    { 277, "SAMPLESPERPIXEL" },
    { 278, "ROWSPERSTRIP" },
    { 279, "STRIPBYTECOUNTS" },
    { 280, "MINSAMPLEVALUE" },
    { 281, "MAXSAMPLEVALUE" },
    { 282, "XRESOLUTION" },
    { 283, "YRESOLUTION" },
    { 284, "PLANARCONFIG" },
    { 285, "PAGENAME" },
    { 286, "XPOSITION" },
    { 287, "YPOSITION" },
    { 288, "FREEOFFSETS" },
    { 289, "FREEBYTECOUNTS" },
    { 290, "GRAYRESPONSEUNIT" },
    { 291, "GRAYRESPONSECURVE" },
    { 292, "GROUP3OPTIONS" },
    { 292, "T4OPTIONS" },
    { 293, "GROUP4OPTIONS" },
    { 293, "T6OPTIONS" },
    { 296, "RESOLUTIONUNIT" },
    { 297, "PAGENUMBER" },
    { 300, "COLORRESPONSEUNIT" },
    { 301, "TRANSFERFUNCTION" },
    { 305, "SOFTWARE" },
    { 306, "DATETIME" },
    { 315, "ARTIST" },
    { 316, "HOSTCOMPUTER" },
    { 317, "PREDICTOR" },
    { 318, "WHITEPOINT" },
    { 319, "PRIMARYCHROMATICITIES" },
    { 320, "COLORMAP" },
    { 321, "HALFTONEHINTS" },
    { 322, "TILEWIDTH" },
    { 323, "TILELENGTH" },
    { 324, "TILEOFFSETS" },
    { 325, "TILEBYTECOUNTS" },
    { 326, "BADFAXLINES" },
    { 327, "CLEANFAXDATA" },
    { 328, "CONSECUTIVEBADFAXLINES" },
    { 330, "SUBIFD" },
    { 332, "INKSET" },
    { 333, "INKNAMES" },
    { 334, "NUMBEROFINKS" },
    { 336, "DOTRANGE" },
    { 337, "TARGETPRINTER" },
    { 338, "EXTRASAMPLES" },
    { 339, "SAMPLEFORMAT" },
    { 340, "SMINSAMPLEVALUE" },
    { 341, "SMAXSAMPLEVALUE" },
    { 343, "CLIPPATH" },
    { 344, "XCLIPPATHUNITS" },
    { 345, "YCLIPPATHUNITS" },
    { 346, "INDEXED" },
    { 347, "JPEGTABLES" },
    { 351, "OPIPROXY" },
    { 400, "GLOBALPARAMETERSIFD" },
    { 401, "PROFILETYPE" },
    { 402, "FAXPROFILE" },
    { 403, "CODINGMETHODS" },
    { 404, "VERSIONYEAR" },
    { 405, "MODENUMBER" },
    { 433, "DECODE" },
    { 434, "IMAGEBASECOLOR" },
    { 435, "T82OPTIONS" },
    { 512, "JPEGPROC" },
    { 513, "JPEGIFOFFSET" },
    { 514, "JPEGIFBYTECOUNT" },
    { 515, "JPEGRESTARTINTERVAL" },
    { 517, "JPEGLOSSLESSPREDICTORS" },
    { 518, "JPEGPOINTTRANSFORM" },
    { 519, "JPEGQTABLES" },
    { 520, "JPEGDCTABLES" },
    { 521, "JPEGACTABLES" },
    { 529, "YCBCRCOEFFICIENTS" },
    { 530, "YCBCRSUBSAMPLING" },
    { 531, "YCBCRPOSITIONING" },
    { 532, "REFERENCEBLACKWHITE" },
    { 559, "STRIPROWCOUNTS" },
    { 700, "XMLPACKET" },
    { 32781, "OPIIMAGEID" },
    { 32932, "TIFFANNOTATIONDATA" },
    { 32953, "REFPTS" },
    { 32954, "REGIONTACKPOINT" },
    { 32955, "REGIONWARPCORNERS" },
    { 32956, "REGIONAFFINE" },
    { 32995, "MATTEING" },
    { 32996, "DATATYPE" },
    { 32997, "IMAGEDEPTH" },
    { 32998, "TILEDEPTH" },
    { 33300, "PIXAR_IMAGEFULLWIDTH" },
    { 33301, "PIXAR_IMAGEFULLLENGTH" },
    { 33302, "PIXAR_TEXTUREFORMAT" },
    { 33303, "PIXAR_WRAPMODES" },
    { 33304, "PIXAR_FOVCOT" },
    { 33305, "PIXAR_MATRIX_WORLDTOSCREEN" },
    { 33306, "PIXAR_MATRIX_WORLDTOCAMERA" },
    { 33405, "WRITERSERIALNUMBER" },
    { 33421, "CFAREPEATPATTERNDIM" },
    { 33422, "CFAPATTERN" },
    { 33432, "COPYRIGHT" },
    { 33445, "MD_FILETAG" },
    { 33446, "MD_SCALEPIXEL" },
    { 33447, "MD_COLORTABLE" },
    { 33448, "MD_LABNAME" },
    { 33449, "MD_SAMPLEINFO" },
    { 33450, "MD_PREPDATE" },
    { 33451, "MD_PREPTIME" },
    { 33452, "MD_FILEUNITS" },
    { 33723, "RICHTIFFIPTC" },
    { 33918, "INGR_PACKET_DATA_TAG" },
    { 33919, "INGR_FLAG_REGISTERS" },
    { 33920, "IRASB_TRANSORMATION_MATRIX" },
    { 33922, "MODELTIEPOINTTAG" },
    { 34016, "IT8SITE" },
    { 34017, "IT8COLORSEQUENCE" },
    { 34018, "IT8HEADER" },
    { 34019, "IT8RASTERPADDING" },
    { 34020, "IT8BITSPERRUNLENGTH" },
    { 34021, "IT8BITSPEREXTENDEDR" },
    { 34022, "IT8COLORTABLE" },
    { 34023, "IT8IMAGECOLORINDICATOR" },
    { 34024, "IT8BKGCOLORINDICATOR" },
    { 34025, "IT8IMAGECOLORVALUE" },
    { 34026, "IT8BKGCOLORVALUE" },
    { 34027, "IT8PIXELINTENSITYRANGE" },
    { 34028, "IT8TRANSPARENCYINDICATOR" },
    { 34029, "IT8COLORCHARACTERIZATION" },
    { 34030, "IT8HCUSAGE" },
    { 34031, "IT8TRAPINDICATOR" },
    { 34032, "IT8CMYKEQUIVALENT" },
    { 34232, "FRAMECOUNT" },
    { 34264, "MODELTRANSFORMATIONTAG" },
    { 34377, "PHOTOSHOP" },
    { 34665, "EXIFIFD" },
    { 34675, "ICCPROFILE" },
    { 34732, "IMAGELAYER" },
    { 34750, "JBIGOPTIONS" },
    { 34853, "GPSIFD" },
    { 34908, "FAXRECVPARAMS" },
    { 34909, "FAXSUBADDRESS" },
    { 34910, "FAXRECVTIME" },
    { 34911, "FAXDCS" },
    { 37439, "STONITS" },
    { 34929, "FEDEX_EDR" },
    { 37724, "IMAGESOURCEDATA" },
    { 40965, "INTEROPERABILITYIFD" },
    { 42112, "GDAL_METADATA" },
    { 42113, "GDAL_NODATA" },
    { 50215, "OCE_SCANJOB_DESCRIPTION" },
    { 50216, "OCE_APPLICATION_SELECTOR" },
    { 50217, "OCE_IDENTIFICATION_NUMBER" },
    { 50218, "OCE_IMAGELOGIC_CHARACTERISTICS" },
    { 50674, "LERC_PARAMETERS" },
    { 50706, "DNGVERSION" },
    { 50707, "DNGBACKWARDVERSION" },
    { 50708, "UNIQUECAMERAMODEL" },
    { 50709, "LOCALIZEDCAMERAMODEL" },
    { 50710, "CFAPLANECOLOR" },
    { 50711, "CFALAYOUT" },
    { 50712, "LINEARIZATIONTABLE" },
    { 50713, "BLACKLEVELREPEATDIM" },
    { 50714, "BLACKLEVEL" },
    { 50715, "BLACKLEVELDELTAH" },
    { 50716, "BLACKLEVELDELTAV" },
    { 50717, "WHITELEVEL" },
    { 50718, "DEFAULTSCALE" },
    { 50719, "DEFAULTCROPORIGIN" },
    { 50720, "DEFAULTCROPSIZE" },
    { 50721, "COLORMATRIX1" },
    { 50722, "COLORMATRIX2" },
    { 50723, "CAMERACALIBRATION1" },
    { 50724, "CAMERACALIBRATION2" },
    { 50725, "REDUCTIONMATRIX1" },
    { 50726, "REDUCTIONMATRIX2" },
    { 50727, "ANALOGBALANCE" },
    { 50728, "ASSHOTNEUTRAL" },
    { 50729, "ASSHOTWHITEXY" },
    { 50730, "BASELINEEXPOSURE" },
    { 50731, "BASELINENOISE" },
    { 50732, "BASELINESHARPNESS" },
    { 50733, "BAYERGREENSPLIT" },
    { 50734, "LINEARRESPONSELIMIT" },
    { 50735, "CAMERASERIALNUMBER" },
    { 50736, "LENSINFO" },
    { 50737, "CHROMABLURRADIUS" },
    { 50738, "ANTIALIASSTRENGTH" },
    { 50739, "SHADOWSCALE" },
    { 50740, "DNGPRIVATEDATA" },
    { 50741, "MAKERNOTESAFETY" },
    { 50778, "CALIBRATIONILLUMINANT1" },
    { 50779, "CALIBRATIONILLUMINANT2" },
    { 50780, "BESTQUALITYSCALE" },
    { 50781, "RAWDATAUNIQUEID" },
    { 50827, "ORIGINALRAWFILENAME" },
    { 50828, "ORIGINALRAWFILEDATA" },
    { 50829, "ACTIVEAREA" },
    { 50830, "MASKEDAREAS" },
    { 50831, "ASSHOTICCPROFILE" },
    { 50832, "ASSHOTPREPROFILEMATRIX" },
    { 50833, "CURRENTICCPROFILE" },
    { 50834, "CURRENTPREPROFILEMATRIX" },
    { 50844, "RPCCOEFFICIENT" },
    { 50784, "ALIAS_LAYER_METADATA" },
    { 50908, "TIFF_RSID" },
    { 50909, "GEO_METADATA" },
    { 50933, "EXTRACAMERAPROFILES" },
    { 65535, "DCSHUESHIFTVALUES" },

    // TAGS missing from libtiff
    // GeoTIFF
    { 33550, "MODELPIXELSCALETAG" },
    { 34735, "GEOKEYDIRECTORYTAG" },
    { 34736, "GEODOUBLEPARAMSTAG" },
    { 34737, "GEOASCIIPARAMSTAG" },
};

const static QMap<int, QByteArray> g_compressionNames = {
    { 1, "NONE" },          { 2, "CCITTRLE" },      { 3, "CCITTFAX3" },
    { 3, "CCITT_T4" },      { 4, "CCITTFAX4" },     { 4, "CCITT_T6" },
    { 5, "LZW" },           { 6, "OJPEG" },         { 7, "JPEG" },
    { 9, "T85" },           { 10, "T43" },          { 32766, "NEXT" },
    { 32771, "CCITTRLEW" }, { 32773, "PACKBITS" },  { 32809, "THUNDERSCAN" },
    { 32895, "IT8CTPAD" },  { 32896, "IT8LW" },     { 32897, "IT8MP" },
    { 32898, "IT8BL" },     { 32908, "PIXARFILM" }, { 32909, "PIXARLOG" },
    { 32946, "DEFLATE" },   { 8, "ADOBE_DEFLATE" }, { 32947, "DCS" },
    { 34661, "JBIG" },      { 34676, "SGILOG" },    { 34677, "SGILOG24" },
    { 34712, "JP2000" },    { 34887, "LERC" },      { 34925, "LZMA" },
    { 50000, "ZSTD" },      { 50001, "WEBP" },      { 50002, "JXL" }
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

class TiffIfdEntryPrivate : public QSharedData
{
public:
    TiffIfdEntryPrivate() {}
    TiffIfdEntryPrivate(const TiffIfdEntryPrivate &other)
        : QSharedData(other)
        , tag(other.tag)
        , type(other.type)
        , count(other.count)
        , valueOrOffset(other.valueOrOffset)
    {
    }
    ~TiffIfdEntryPrivate() {}

    int typeSize()
    {
        switch (type) {
        case TiffIfdEntry::DT_Byte:
        case TiffIfdEntry::DT_SByte:
        case TiffIfdEntry::DT_Ascii:
        case TiffIfdEntry::DT_Undefined:
            return 1;
        case TiffIfdEntry::DT_Short:
        case TiffIfdEntry::DT_SShort:
            return 2;
        case TiffIfdEntry::DT_Long:
        case TiffIfdEntry::DT_SLong:
        case TiffIfdEntry::DT_Ifd:
        case TiffIfdEntry::DT_Float:
            return 4;

        case TiffIfdEntry::DT_Rational:
        case TiffIfdEntry::DT_SRational:
        case TiffIfdEntry::DT_Long8:
        case TiffIfdEntry::DT_SLong8:
        case TiffIfdEntry::DT_Ifd8:
        case TiffIfdEntry::DT_Double:
            return 8;
        default:
            return 0;
        }
    }

    void parserValues(const char *bytes, TiffFile::ByteOrder byteOrder);

    quint16 tag;
    quint16 type;
    quint64 count{ 0 };
    QByteArray valueOrOffset; // 12 bytes for tiff or 20 bytes for bigTiff
    QVariantList values;
};

void TiffIfdEntryPrivate::parserValues(const char *bytes, TiffFile::ByteOrder byteOrder)
{
    if (type == TiffIfdEntry::DT_Ascii) {
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

    if (type == TiffIfdEntry::DT_Undefined) {
        values.append(QByteArray(bytes, count));
        return;
    }

    // To make things simple, save normal integer as qint32 or quint32 here.
    for (int i = 0; i < count; ++i) {
        switch (type) {
        case TiffIfdEntry::DT_Byte:
            values.append(static_cast<quint32>(bytes[i]));
            break;
        case TiffIfdEntry::DT_SByte:
            values.append(static_cast<qint32>(bytes[i]));
            break;
        case TiffIfdEntry::DT_Short:
            values.append(
                static_cast<quint32>(getValueFromBytes<quint16>(bytes + i * 2, byteOrder)));
            break;
        case TiffIfdEntry::DT_SShort:
            values.append(static_cast<qint32>(getValueFromBytes<qint16>(bytes + i * 2, byteOrder)));
            break;
        case TiffIfdEntry::DT_Long:
        case TiffIfdEntry::DT_Ifd:
            values.append(getValueFromBytes<quint32>(bytes + i * 4, byteOrder));
            break;
        case TiffIfdEntry::DT_SLong:
            values.append(getValueFromBytes<qint32>(bytes + i * 4, byteOrder));
            break;
        case TiffIfdEntry::DT_Float:
            values.append(*(reinterpret_cast<const float *>(bytes + i * 4)));
            break;
        case TiffIfdEntry::DT_Double:
            values.append(*(reinterpret_cast<const double *>(bytes + i * 8)));
            break;
        case TiffIfdEntry::DT_Rational:
            values.append(getValueFromBytes<quint32>(bytes + i * 4, byteOrder));
            values.append(getValueFromBytes<quint32>(bytes + i * 4 + 4, byteOrder));
            break;
        case TiffIfdEntry::DT_SRational:
            values.append(getValueFromBytes<qint32>(bytes + i * 4, byteOrder));
            values.append(getValueFromBytes<qint32>(bytes + i * 4 + 4, byteOrder));
            break;
        case TiffIfdEntry::DT_Long8:
        case TiffIfdEntry::DT_Ifd8:
            values.append(getValueFromBytes<quint64>(bytes + i * 8, byteOrder));
            break;
        case TiffIfdEntry::DT_SLong8:
            values.append(getValueFromBytes<qint64>(bytes + i * 8, byteOrder));
            break;
        default:
            break;
        }
    }
}

/*!
 * \class TiffIfdEntry
 */

TiffIfdEntry::TiffIfdEntry()
    : d(new TiffIfdEntryPrivate)
{
}

TiffIfdEntry::TiffIfdEntry(const TiffIfdEntry &other)
    : d(other.d)
{
}

TiffIfdEntry::~TiffIfdEntry()
{
}

quint16 TiffIfdEntry::tag() const
{
    return d->tag;
}

QString TiffIfdEntry::tagName() const
{
    if (g_tagNames.contains(d->tag))
        return QString::fromLatin1(g_tagNames[d->tag]);

    return QStringLiteral("UNKNOWNTAG(%1)").arg(d->tag);
}

quint16 TiffIfdEntry::type() const
{
    return d->type;
}

QString TiffIfdEntry::typeName() const
{
    if (d->type > 0 && d->type < TiffIfdEntry::DT_Ifd8)
        return g_dataTypeName[d->type];

    return QString();
}

quint64 TiffIfdEntry::count() const
{
    return d->count;
}

QByteArray TiffIfdEntry::valueOrOffset() const
{
    return d->valueOrOffset;
}

QVariantList TiffIfdEntry::values() const
{
    return d->values;
}

QString TiffIfdEntry::valueDescription() const
{
    if (d->tag == T_Compression && d->values.size() == 1) {
        const int v = d->values[0].toInt();

        if (g_compressionNames.contains(v))
            return QString::fromLatin1(g_compressionNames[v]);
    }
    return QString();
}

bool TiffIfdEntry::isValid() const
{
    return d->count;
}

class TiffIfdPrivate : public QSharedData
{
public:
    TiffIfdPrivate() {}
    TiffIfdPrivate(const TiffIfdPrivate &other)
        : QSharedData(other)
        , ifdEntries(other.ifdEntries)
        , subIfds(other.subIfds)
        , nextIfdOffset(other.nextIfdOffset)
    {
    }
    ~TiffIfdPrivate() {}

    bool hasIfdEntry(quint16 tag);
    TiffIfdEntry ifdEntry(quint16 tag);

    QVector<TiffIfdEntry> ifdEntries;
    QVector<TiffIfd> subIfds;
    qint64 nextIfdOffset{ 0 };
};

bool TiffIfdPrivate::hasIfdEntry(quint16 tag)
{
    return ifdEntry(tag).isValid();
}

TiffIfdEntry TiffIfdPrivate::ifdEntry(quint16 tag)
{
    auto it = std::find_if(ifdEntries.cbegin(), ifdEntries.cend(),
                           [tag](const TiffIfdEntry &de) { return tag == de.tag(); });
    if (it == ifdEntries.cend())
        return TiffIfdEntry();
    return *it;
}

/*!
 * \class TiffIfd
 */

TiffIfd::TiffIfd()
    : d(new TiffIfdPrivate)
{
}

TiffIfd::TiffIfd(const TiffIfd &other)
    : d(other.d)
{
}

TiffIfd::~TiffIfd()
{
}

QVector<TiffIfdEntry> TiffIfd::ifdEntries() const
{
    return d->ifdEntries;
}

QVector<TiffIfd> TiffIfd::subIfds() const
{
    return d->subIfds;
}

qint64 TiffIfd::nextIfdOffset() const
{
    return d->nextIfdOffset;
}

bool TiffIfd::isValid() const
{
    return !d->ifdEntries.isEmpty();
}

class TiffFilePrivate
{
public:
    TiffFilePrivate();
    void setError(const QString &errorString);
    bool readHeader();
    bool readIfd(qint64 offset, TiffIfd *parentIfd = nullptr);

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

    QVector<TiffIfd> ifds;

    QFile file;
    QString errorString;
    bool hasError{ false };

    TiffParserOptions parserOptions;
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

bool TiffFilePrivate::readIfd(qint64 offset, TiffIfd *parentIfd)
{
    if (!file.seek(offset)) {
        setError(file.errorString());
        return false;
    }

    TiffIfd ifd;

    if (!header.isBigTiff()) {
        quint16 deCount = getValueFromFile<quint16>();
        for (int i = 0; i < deCount; ++i) {
            TiffIfdEntry ifdEntry;
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
            TiffIfdEntry ifdEntry;
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
        // skip unknown datatype
        if (valueBytesCount == 0)
            continue;
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

    if (parserOptions.parserSubIfds) {
        // Note:
        // SUBIFDs in Tiff with pyramid generated by Adobe Photoshop CS6(Windows) can not be
        // parsered here. Nevertheless, Tiff generated by Adobe Photoshop CC 2018 is OK.
        TiffIfdEntry deSubIfd = ifd.d->ifdEntry(TiffIfdEntry::T_SubIfd);
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
TiffFile::TiffFile(const QString &filePath, const TiffParserOptions &options)
    : d(new TiffFilePrivate)
{
    d->file.setFileName(filePath);
    d->parserOptions = options;
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

QVector<TiffIfd> TiffFile::ifds() const
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
