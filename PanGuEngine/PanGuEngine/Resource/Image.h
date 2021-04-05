#pragma once

namespace Resource
{

    enum IMAGE_FILE_FORMAT
    {
        /// Unknown format
        IMAGE_FILE_FORMAT_UNKNOWN = 0,

        /// The image is encoded in JPEG format
        IMAGE_FILE_FORMAT_JPEG,

        /// The image is encoded in PNG format
        IMAGE_FILE_FORMAT_PNG,

        /// The image is encoded in TIFF format
        IMAGE_FILE_FORMAT_TIFF,

        /// DDS file
        IMAGE_FILE_FORMAT_DDS,

        /// KTX file
        IMAGE_FILE_FORMAT_KTX
    };


    struct ImageLoadInfo
    {
        /// Image file format
        IMAGE_FILE_FORMAT Format = IMAGE_FILE_FORMAT_UNKNOWN;
    };

    class Image
    {
    public:
        static IMAGE_FILE_FORMAT GetFileFormat(const UINT8* pData, size_t Size);

    };

}


