#pragma warning(disable : 4458)
#pragma warning(disable : 4244)
#pragma warning(disable : 4061)
#pragma warning(disable : 4062)
#pragma warning(disable : 4365)
#pragma warning(disable : 4464)
#pragma warning(disable : 4514)
#pragma warning(disable : 4668)
#pragma warning(disable : 4820)
#pragma warning(disable : 4625)
#pragma warning(disable : 4710)
#pragma warning(disable : 4626)
#pragma warning(disable : 4582)
#pragma warning(disable : 4623)
#pragma warning(disable : 4060)
#pragma warning(disable : 4068)
#pragma warning(disable : 4201)
#pragma warning(disable : 4127)
#pragma warning(disable : 4191)
#pragma warning(disable : 4505)
#pragma warning(disable : 4100)
#pragma warning(disable : 4324)

#pragma warning(disable : 5026)
#pragma warning(disable : 5027)
#pragma warning(disable : 4577)
#pragma warning(disable : 4711)

#pragma warning(disable : 4189)

#include "engine/utils/platform.h"
#include "engine/common.h"
#include "engine/utils/string.h"

#ifdef OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"
#endif

u16 parse_u16(u8 *buffer, b32 byte_alignment) { // byte_alignment: false
	u8 a = byte_alignment ? buffer[1] : buffer[0];
	u8 b = byte_alignment ? buffer[0] : buffer[1];
	return ((u16)(a) << 8) | b;
}

u32 parse_u32(u8 *buffer, b32 byte_alignment) { // byte_alignment: false
	u8 a = byte_alignment ? buffer[3] : buffer[0];
	u8 b = byte_alignment ? buffer[2] : buffer[1];
	u8 c = byte_alignment ? buffer[1] : buffer[2];
	u8 d = byte_alignment ? buffer[0] : buffer[3];
	return ((u32)(a) << 24) | ((u32)(b) << 16) | ((u32)(c) << 8) | ((u32)(d) << 0);
}

struct IFEntry {
	u16 tag;
	u16 format;
	u32 length;
	u32 data;

	union {
		String string;
		i32 value;
	};
};

IFEntry parseIFEntry(u8 *buffer, u32 offset, u32 base, u32 length, b32 byte_alignment) {
	IFEntry result;

	u8 *at = buffer + offset;

	// check if there even is enough data for IFEntry in the buffer
	if (at + 12 > buffer + length) {
		result.tag = 0xFF;
		return result;
	}

	result.tag = parse_u16(at, byte_alignment);
	result.format = parse_u16(at + 2, byte_alignment);
	result.length = parse_u32(at + 4, byte_alignment);
	result.data = parse_u32(at + 8, byte_alignment);

	// Parse value in specified format
	switch (result.format) {
		case 1:
			// if (!extract_values<uint8_t, alignIntel>(result.val_byte, buffer, base, length, result)) {
			// 	result.tag = 0xFF;
			// }
			break;
		case 2: {
			char *start = (char*)(buffer + base + result.data);
			result.string = make_string(start, result.length);
		} break;
		case 3:
			// if (!extract_values<uint16_t, alignIntel>(result.val_short, buffer, base, length, result)) {
			// 	result.tag = 0xFF;
			// }
			break;
		case 4:
			// if (!extract_values<uint32_t, alignIntel>(result.val_long, buffer, base, length, result)) {
			// 	result.tag = 0xFF;
			// }
			break;
		case 5:
			// if (!extract_values<Rational, alignIntel>(result.val_rational, buffer, base, length, result)) {
			// 	result.tag = 0xFF;
			// }
			break;
		case 7:
		case 9:
		case 10:
			break;
		default:
			result.tag = 0xFF;
	}
	return result;
}


#include <io.h>

void parse() {
	u32 buffer_size = 32*MB;
	u8 *buffer = (u8*)malloc(buffer_size);
	u8 *filestart = buffer;
	DWORD length = 0;

	HANDLE file = CreateFile("test.jpg", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);
	BOOL success = ReadFile(file, buffer, buffer_size, &length, 0);
	ASSERT(success, "Could not read file!");

	CloseHandle(file);

	u32 filelength = length;

	// Sanity check: all JPEG files start with 0xFFD8.
	if (buffer[0] != 0xFF || buffer[1] != 0xD8) {
		ASSERT(false, "Missing jpeg header. Not a jpeg file?");
		return;
	};

	// Scan for EXIF header (bytes 0xFF 0xE1) and do a sanity check by
	// looking for bytes "Exif\0\0". The marker length data is in Motorola
	// byte order, which results in the 'false' parameter to parse16().
	// The marker has to contain at least the TIFF header, otherwise the
	// EXIF data is corrupt. So the minimum length specified here has to be:
	//	 2 bytes: section size
	//	 6 bytes: "Exif\0\0" string
	//	 2 bytes: TIFF header (either "II" or "MM" string)
	//	 2 bytes: TIFF magic (short 0x2a00 in Motorola byte order)
	//	 4 bytes: Offset to first IFD
	// =========
	//	16 bytes
	u32 offset = 0;	// current offset into buffer
	for (offset = 0; offset < length - 1; offset++) {
		if (buffer[offset] == 0xFF && buffer[offset + 1] == 0xE1) break;
	}

	if (offset + 4 > length) { 
		ASSERT(false, "Found no exif metadata!");
		return;
	}

	offset += 2;

	u16 section_length = parse_u16(buffer + offset, false);
	if (offset + section_length > length || section_length < 16) {
		ASSERT(false, "Corrupt exif format!");
		return;
	}
	offset += 2;

	// parseFromEXIFSegment(buffer + offset, length - offset);
	buffer += offset;
	length -= offset;

	//
	// Main parsing function for an EXIF segment.
	//
	// PARAM: 'buffer' start of the EXIF TIFF, which must be the bytes "Exif\0\0".
	// PARAM: 'length' length of buffer
	//
	// int easyexif::EXIFInfo::parseFromEXIFSegment(u8 *buffer, u32 length) {
	bool byte_alignment = true; // byte alignment (defined in EXIF header)
	offset							= 0;		// current offset into buffer

	char exif_marker[] = "Exif\0\0";
	b32 valid = 1;
	for (i32 i = 0; i < sizeof(exif_marker)-1 && valid; ++i) {
		valid = buffer[i] == exif_marker[i];
	}

	ASSERT(valid, "Found no Exif marker!");

	offset += 6;

	// Now parsing the TIFF header. The first two bytes are either "II" or
	// "MM" for Intel or Motorola byte alignment. Sanity check by parsing
	// the u16 that follows, making sure it equals 0x2a. The
	// last 4 bytes are an offset into the first IFD, which are added to
	// the global offset counter. For this block, we expect the following
	// minimum size:
	//	2 bytes: 'II' or 'MM'
	//	2 bytes: 0x002a
	//	4 bytes: offset to first IDF
	// -----------------------------
	//	8 bytes
	if (offset + 8 > length) {
		ASSERT(false, "File too short!");
	}

	u32 tiff_header_start = offset;
	if (buffer[offset] == 'I' && buffer[offset + 1] == 'I') {
		byte_alignment = true;
	} else if (buffer[offset] == 'M' && buffer[offset + 1] == 'M') {
		byte_alignment = false;
	} else {
		ASSERT(false, "Unknown byte alignment!");
	}

	offset += 2;

	u16 tmp = parse_u16(buffer + offset, byte_alignment);
	ASSERT(0x2a == tmp, "Error corrupt");
	offset += 2;

	u32 first_ifd_offset = parse_u32(buffer + offset, byte_alignment);
	offset += first_ifd_offset - 4;
	ASSERT(offset < length, "PARSE_EXIF_ERROR_CORRUPT");

	// Now parsing the first Image File Directory (IFD0, for the main image).
	// An IFD consists of a variable number of 12-byte directory entries. The
	// first two bytes of the IFD section contain the number of directory
	// entries in the section. The last 4 bytes of the IFD contain an offset
	// to the next IFD, which means this IFD must contain exactly 6 + 12 * num
	// bytes of data.
	ASSERT(offset + 2 <= length, "PARSE_EXIF_ERROR_CORRUPT");
	int num_entries = parse_u16(buffer + offset, byte_alignment);
	ASSERT((offset + 6 + 12 * num_entries) <= length, "PARSE_EXIF_ERROR_CORRUPT");
	offset += 2;
	u32 exif_sub_ifd_offset = length;
	u32 gps_sub_ifd_offset = length;

#if 1
	while (--num_entries >= 0) {
		IFEntry entry = parseIFEntry(buffer, offset, tiff_header_start, length, byte_alignment);
		offset += 12;
		switch (entry.tag) {
			case 0x102: { // Bits per sample
				// if (result.format == 3 && result.val_short.size())
				// 	this->BitsPerSample = result.val_short.front();
			} break;
			case 0x10E: { // Image description
				ASSERT(entry.format == 2, "Invalid format for image description!");
				String image_description = entry.string;
				printf("description: %.*s\n", image_description.length, *image_description);
			} break;
			case 0x10F: { // Digicam make
				ASSERT(entry.format == 2, "Invalid format for make!");
				String make = entry.string;
				printf("make: %.*s\n", make.length, *make);
			} break;
			case 0x110: { // Digicam model
				ASSERT(entry.format == 2, "Invalid format for model!");
				String model = entry.string;
				printf("model: %.*s\n", model.length, *model);
			} break;
			case 0x112: { // Orientation of image
				ASSERT(entry.format == 3, "Invalid format for orientation!");
				i32 orientation = entry.value;
				printf("orientation: %d\n", orientation);
			} break;
			case 0x131: { // Software used for image
				ASSERT(entry.format == 2, "Invalid format for software!");
				String software = entry.string;
				printf("software: %.*s\n", software.length, *software);
			} break;
			case 0x132: { // EXIF/TIFF date/time of image modification
				ASSERT(entry.format == 2, "Invalid format for software!");
				String date = entry.string;
				printf("date: %.*s\n", date.length, *date);
			} break;
			case 0x8298: { // Copyright information
				ASSERT(entry.format == 2, "Invalid format for software!");
				String copyright = entry.string;
				printf("copyright: %.*s\n", copyright.length, *copyright);
			} break;
			case 0x8825: { // GPS IFS offset
				gps_sub_ifd_offset = tiff_header_start + entry.data;
			} break;
			case 0x8769: { // EXIF SubIFD offset
				exif_sub_ifd_offset = tiff_header_start + entry.data;
			} break;
		}
	}
#endif

	// Jump to the EXIF SubIFD if it exists and parse all the information
	// there. Note that it's possible that the EXIF SubIFD doesn't exist.
	// The EXIF SubIFD contains most of the interesting information that a
	// typical user might want.
	if (exif_sub_ifd_offset + 4 <= length) {
		offset = exif_sub_ifd_offset;
		num_entries = parse_u16(buffer + offset, byte_alignment);
		ASSERT((offset + 6 + 12 * num_entries) <= length, "PARSE_EXIF_ERROR_CORRUPT");
		offset += 2;
#if 0
		while (--num_entries >= 0) {
			IFEntry result = parseIFEntry(buffer, offset, byte_alignment, tiff_header_start, length);
			switch (ult.tag()) {
				case 0x829a:
					// Exposure time in seconds
					if (result.format == 5 && result.val_rational.size())
						this->ExposureTime = result.val_rational.front();
					break;

				case 0x829d:
					// FNumber
					if (result.format == 5 && result.val_rational.size())
						this->FNumber = result.val_rational.front();
					break;

			case 0x8822:
				// Exposure Program
				if (result.format == 3 && result.val_short.size())
					this->ExposureProgram = result.val_short.front();
				break;

				case 0x8827:
					// ISO Speed Rating
					if (result.format == 3 && result.val_short.size())
						this->ISOSpeedRatings = result.val_short.front();
					break;

				case 0x9003:
					// Original date and time
					if (result.format == 2)
						this->DateTimeOriginal = result.val_string;
					break;

				case 0x9004:
					// Digitization date and time
					if (result.format == 2)
						this->DateTimeDigitized = result.val_string;
					break;

				case 0x9201:
					// Shutter speed value
					if (result.format == 5 && result.val_rational.size())
						this->ShutterSpeedValue = result.val_rational.front();
					break;

				case 0x9204:
					// Exposure bias value
					if (result.format == 5 && result.val_rational.size())
						this->ExposureBiasValue = result.val_rational.front();
					break;

				case 0x9206:
					// Subject distance
					if (result.format == 5 && result.val_rational.size())
						this->SubjectDistance = result.val_rational.front();
					break;

				case 0x9209:
					// Flash used
					if (result.format == 3 && result.val_short.size()) {
						uint16_t data = result.val_short.front();
						
						this->Flash = data & 1;
						this->FlashReturnedLight = (data & 6) >> 1;
						this->FlashMode = (data & 24) >> 3;
					}
					break;

				case 0x920a:
					// Focal length
					if (result.format == 5 && result.val_rational.size())
						this->FocalLength = result.val_rational.front();
					break;

				case 0x9207:
					// Metering mode
					if (result.format == 3 && result.val_short.size())
						this->MeteringMode = result.val_short.front();
					break;

				case 0x9291:
					// Subsecond original time
					if (result.format == 2)
						this->SubSecTimeOriginal = result.val_string;
					break;

				case 0xa002:
					// EXIF Image width
					if (result.format == 4 && result.val_long.size())
						this->ImageWidth = result.val_long.front();
					if (result.format == 3 && result.val_short.size())
						this->ImageWidth = result.val_short.front();
					break;

				case 0xa003:
					// EXIF Image height
					if (result.format == 4 && result.val_long.size())
						this->ImageHeight = result.val_long.front();
					if (result.format == 3 && result.val_short.size())
						this->ImageHeight = result.val_short.front();
					break;

				case 0xa20e:
					// EXIF Focal plane X-resolution
					if (result.format == 5) {
						this->LensInfo.FocalPlaneXResolution = result.val_rational[0];
					}
					break;

				case 0xa20f:
					// EXIF Focal plane Y-resolution
					if (result.format == 5) {
						this->LensInfo.FocalPlaneYResolution = result.val_rational[0];
					}
					break;

				case 0xa210:
						// EXIF Focal plane resolution unit
						if (result.format == 3 && result.val_short.size()) {
								this->LensInfo.FocalPlaneResolutionUnit = result.val_short.front();
						}
						break;

				case 0xa405:
					// Focal length in 35mm film
					if (result.format == 3 && result.val_short.size())
						this->FocalLengthIn35mm = result.val_short.front();
					break;

				case 0xa432:
					// Focal length and FStop.
					if (result.format == 5) {
						int sz = static_cast<u32>(result.val_rational.size());
						if (sz)
							this->LensInfo.FocalLengthMin = result.val_rational[0];
						if (sz > 1)
							this->LensInfo.FocalLengthMax = result.val_rational[1];
						if (sz > 2)
							this->LensInfo.FStopMin = result.val_rational[2];
						if (sz > 3)
							this->LensInfo.FStopMax = result.val_rational[3];
					}
					break;

				case 0xa433:
					// Lens make.
					if (result.format == 2) {
						this->LensInfo.Make = result.val_string;
					}
					break;

				case 0xa434:
					// Lens model.
					if (result.format == 2) {
						this->LensInfo.Model = result.val_string;
					}
					break;
			}
			offset += 12;
		}
#endif
	}

	// Jump to the GPS SubIFD if it exists and parse all the information
	// there. Note that it's possible that the GPS SubIFD doesn't exist.
	if (gps_sub_ifd_offset + 4 <= length) {
		offset = gps_sub_ifd_offset;
		num_entries = parse_u16(buffer + offset, byte_alignment);
		ASSERT((offset + 6 + 12 * num_entries) <= length, "PARSE_EXIF_ERROR_CORRUPT");
		offset += 2;
#if 0
		while (--num_entries >= 0) {
			u16 tag, format;
			u32 length, data;
			parseIFEntryHeader(buffer + offset, byte_alignment, tag, format, length, data);
			switch (tag) {
				case 1:
					// GPS north or south
					this->GeoLocation.LatComponents.direction = *(buffer + offset + 8);
					if (this->GeoLocation.LatComponents.direction == 0) {
						this->GeoLocation.LatComponents.direction = '?';
					}
					if ('S' == this->GeoLocation.LatComponents.direction) {
						this->GeoLocation.Latitude = -this->GeoLocation.Latitude;
					}
					break;

				case 2:
					// GPS latitude
					if ((format == 5 || format == 10) && length == 3) {
						this->GeoLocation.LatComponents.degrees = parse_value<Rational>(
								buffer + data + tiff_header_start, byte_alignment);
						this->GeoLocation.LatComponents.minutes = parse_value<Rational>(
								buffer + data + tiff_header_start + 8, byte_alignment);
						this->GeoLocation.LatComponents.seconds = parse_value<Rational>(
								buffer + data + tiff_header_start + 16, byte_alignment);
						this->GeoLocation.Latitude =
								this->GeoLocation.LatComponents.degrees +
								this->GeoLocation.LatComponents.minutes / 60 +
								this->GeoLocation.LatComponents.seconds / 3600;
						if ('S' == this->GeoLocation.LatComponents.direction) {
							this->GeoLocation.Latitude = -this->GeoLocation.Latitude;
						}
					}
					break;

				case 3:
					// GPS east or west
					this->GeoLocation.LonComponents.direction = *(buffer + offset + 8);
					if (this->GeoLocation.LonComponents.direction == 0) {
						this->GeoLocation.LonComponents.direction = '?';
					}
					if ('W' == this->GeoLocation.LonComponents.direction) {
						this->GeoLocation.Longitude = -this->GeoLocation.Longitude;
					}
					break;

				case 4:
					// GPS longitude
					if ((format == 5 || format == 10) && length == 3) {
						this->GeoLocation.LonComponents.degrees = parse_value<Rational>(
								buffer + data + tiff_header_start, byte_alignment);
						this->GeoLocation.LonComponents.minutes = parse_value<Rational>(
								buffer + data + tiff_header_start + 8, byte_alignment);
						this->GeoLocation.LonComponents.seconds = parse_value<Rational>(
								buffer + data + tiff_header_start + 16, byte_alignment);
						this->GeoLocation.Longitude =
								this->GeoLocation.LonComponents.degrees +
								this->GeoLocation.LonComponents.minutes / 60 +
								this->GeoLocation.LonComponents.seconds / 3600;
						if ('W' == this->GeoLocation.LonComponents.direction)
							this->GeoLocation.Longitude = -this->GeoLocation.Longitude;
					}
					break;

				case 5:
					// GPS altitude reference (below or above sea level)
					this->GeoLocation.AltitudeRef = *(buffer + offset + 8);
					if (1 == this->GeoLocation.AltitudeRef) {
						this->GeoLocation.Altitude = -this->GeoLocation.Altitude;
					}
					break;

				case 6:
					// GPS altitude
					if ((format == 5 || format == 10)) {
						this->GeoLocation.Altitude = parse_value<Rational>(
								buffer + data + tiff_header_start, byte_alignment);
						if (1 == this->GeoLocation.AltitudeRef) {
							this->GeoLocation.Altitude = -this->GeoLocation.Altitude;
						}
					}
					break;

				case 11:
					// GPS degree of precision (DOP)
					if ((format == 5 || format == 10)) {
						this->GeoLocation.DOP = parse_value<Rational>(
								buffer + data + tiff_header_start, byte_alignment);
					}
					break;
			}
			offset += 12;
		}
#endif
	}

	{
		HANDLE outfile = CreateFile("test.jpg", GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		success = WriteFile(outfile, filestart, filelength, 0, 0);
		ASSERT(success, "Could not write file!");

		CloseHandle(outfile);
	}
}

#include <shellapi.h>
int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode) {
	parse();
	return 0;
}