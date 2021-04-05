#include "pch.h"
#include "Image.h"

namespace Resource
{

	IMAGE_FILE_FORMAT Image::GetFileFormat(const UINT8* pData, size_t Size)
	{
        if (Size >= 3 && pData[0] == 0xFF && pData[1] == 0xD8 && pData[2] == 0xFF)
            return IMAGE_FILE_FORMAT_JPEG;

        if (Size >= 8 &&
            pData[0] == 0x89 && pData[1] == 0x50 && pData[2] == 0x4E && pData[3] == 0x47 &&
            pData[4] == 0x0D && pData[5] == 0x0A && pData[6] == 0x1A && pData[7] == 0x0A)
            return IMAGE_FILE_FORMAT_PNG;

        if (Size >= 4 &&
            ((pData[0] == 0x49 && pData[1] == 0x20 && pData[2] == 0x49) ||
                (pData[0] == 0x49 && pData[1] == 0x49 && pData[2] == 0x2A && pData[3] == 0x00) ||
                (pData[0] == 0x4D && pData[1] == 0x4D && pData[2] == 0x00 && pData[3] == 0x2A) ||
                (pData[0] == 0x4D && pData[1] == 0x4D && pData[2] == 0x00 && pData[3] == 0x2B)))
            return IMAGE_FILE_FORMAT_TIFF;

        if (Size >= 4 && pData[0] == 0x44 && pData[1] == 0x44 && pData[2] == 0x53 && pData[3] == 0x20)
            return IMAGE_FILE_FORMAT_DDS;

        static constexpr UINT8 KTX10FileIdentifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
        static constexpr UINT8 KTX20FileIdentifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
        if (Size >= 12 &&
            (memcmp(pData, KTX10FileIdentifier, sizeof(KTX10FileIdentifier)) == 0 ||
                memcmp(pData, KTX20FileIdentifier, sizeof(KTX20FileIdentifier)) == 0))
            return IMAGE_FILE_FORMAT_KTX;

        return IMAGE_FILE_FORMAT_UNKNOWN;
	}

}

