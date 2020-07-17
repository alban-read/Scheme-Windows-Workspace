#include "stdafx.h"
#include <Windows.h>
#include "GdiPlus.h"
#include "scheme.h"
#include <WinCrypt.h>
#pragma comment(lib, "crypt32.lib")


using namespace Gdiplus;
using namespace Gdiplus::DllExports;

int get_encoder_clsid(WCHAR *format, CLSID *pClsid)
{
	unsigned int num = 0, size = 0;

	GetImageEncodersSize(&num, &size);
	if (size == 0) return -1;

	auto*p_image_codec_info = static_cast<ImageCodecInfo *>(malloc(size));
	if (p_image_codec_info == nullptr) return -1;

	GetImageEncoders(num, size, p_image_codec_info);
	for (unsigned int j = 0; j < num; ++j)
	{
		if (wcscmp(p_image_codec_info[j].MimeType, format) == 0) {
			*pClsid = p_image_codec_info[j].Clsid;
			free(p_image_codec_info);
			return j;
		}
	}
	free(p_image_codec_info);
	return -1;
}




Gdiplus::Bitmap* ResizeClone(Bitmap *bmp, INT width, INT height)
{
	const auto o_height = bmp->GetHeight();
	const auto o_width = bmp->GetWidth();
	auto n_width = width;
	auto n_height = height;
	const auto ratio = static_cast<double>(o_width) / static_cast<double>(o_height);
	if (o_width > o_height)
	{
		// Resize down by width
		n_height = static_cast<UINT>(static_cast<double>(n_width) / ratio);
	}
	else
	{
		n_width = static_cast<UINT>(n_height * ratio);
	}
	const auto new_bitmap = new Gdiplus::Bitmap(n_width, n_height, bmp->GetPixelFormat());
	Gdiplus::Graphics graphics(new_bitmap);
	graphics.DrawImage(bmp, 0, 0, n_width, n_height);
	return new_bitmap;
}

Gdiplus::Status h_bitmap_to_bitmap(const HBITMAP source, const Gdiplus::PixelFormat pixel_format, Gdiplus::Bitmap** result_out)
{
	BITMAP source_info = { 0 };
	if (!::GetObject(source, sizeof(source_info), &source_info))
		return Gdiplus::GenericError;

	Gdiplus::Status s;

	std::auto_ptr< Gdiplus::Bitmap > target(new Gdiplus::Bitmap(source_info.bmWidth, source_info.bmHeight, pixel_format));
	if (!target.get())
		return Gdiplus::OutOfMemory;
	if ((s = target->GetLastStatus()) != Gdiplus::Ok)
		return s;

	Gdiplus::BitmapData target_info{};
	Gdiplus::Rect rect(0, 0, source_info.bmWidth, source_info.bmHeight);

	s = target->LockBits(&rect, Gdiplus::ImageLockModeWrite, pixel_format, &target_info);
	if (s != Gdiplus::Ok)
		return s;

	if (target_info.Stride != source_info.bmWidthBytes)
		return Gdiplus::InvalidParameter; // pixel_format is wrong! 

	CopyMemory(target_info.Scan0, source_info.bmBits, source_info.bmWidthBytes * source_info.bmHeight);

	s = target->UnlockBits(&target_info);
	if (s != Gdiplus::Ok)
		return s;

	*result_out = target.release();

	return Gdiplus::Ok;
}

int to_base64_crypto(const char* p_src, const int n_len_src, char* p_dst, const int n_len_dst)
{
	DWORD n_len_out = n_len_dst;
	const auto f_ret = CryptBinaryToStringA(
		reinterpret_cast<const BYTE*>(p_src), n_len_src,
		CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
		p_dst, &n_len_out
	);
	if (!f_ret) n_len_out = 0;  // failed
	return(n_len_out);
}
// screen shot function

ptr snaggit( char *name) {

	ptr s;
	ULONG u_quality = 95;
	const auto h_my_wnd = GetDesktopWindow();
	RECT  r;
	LPBYTE lpCapture;

	CLSID image_clsid;
	Bitmap *p_screen_shot;


	GetWindowRect(h_my_wnd, &r);
	const auto dc = GetWindowDC(h_my_wnd);

	const int w = r.right - r.left;
	const int h = r.bottom - r.top;

	const int nBPP = GetDeviceCaps(dc, BITSPIXEL);
	const HDC hdcCapture = CreateCompatibleDC(dc);


	BITMAPINFO bmiCapture = {
		{sizeof(BITMAPINFOHEADER), w, -h, 1, nBPP, BI_RGB, 0, 0, 0, 0, 0},
	};


	const HBITMAP hbm_capture = CreateDIBSection(dc, &bmiCapture,
		DIB_PAL_COLORS, reinterpret_cast<LPVOID *>(&lpCapture), nullptr, 0);

	// failed to take it 
	if (!hbm_capture)
	{
		DeleteDC(hdcCapture);
		DeleteDC(dc);

		return ptr Sfalse;
	}

	// copy the screenshot buffer 
	const auto n_capture = SaveDC(hdcCapture);
	SelectObject(hdcCapture, hbm_capture);
	BitBlt(hdcCapture, 0, 0, w, h, dc, 0, 0, SRCCOPY);
	RestoreDC(hdcCapture, n_capture);
	DeleteDC(hdcCapture);
	DeleteDC(dc);


	try {

		p_screen_shot = new Bitmap(hbm_capture, static_cast<HPALETTE>(nullptr));

	}
	catch (...) {

		return Sstring("snaggit - out of memory");
	}


	// RESIZE it.
	// screenshots the size of the whole screen are not very viewable.
	auto resized = ResizeClone(p_screen_shot, p_screen_shot->GetWidth()*0.8, p_screen_shot->GetHeight()*0.8);

	delete p_screen_shot;

	// convert to JPEG
	EncoderParameters encoder_params{};
	encoder_params.Count = 1;
	encoder_params.Parameter[0].NumberOfValues = 1;
	encoder_params.Parameter[0].Guid = EncoderQuality;
	encoder_params.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoder_params.Parameter[0].Value = &u_quality;
	get_encoder_clsid(L"image/jpeg", &image_clsid);

	{
		IStream *p_stream = nullptr;
		const LARGE_INTEGER li_zero = {};
		ULARGE_INTEGER pos = {};
		STATSTG stg = {};
		ULONG bytes_read = 0;
		auto hr_ret = S_OK;

		BYTE* buffer = nullptr;
		DWORD dw_buffer_size = 0;


		hr_ret = CreateStreamOnHGlobal(nullptr, TRUE, &p_stream);
		if (hr_ret == E_OUTOFMEMORY || hr_ret == E_INVALIDARG) {
			return Sstring("snaggit - CreateStreamOnHGlobal failed");
		}
		hr_ret = resized->Save(p_stream, &image_clsid, &encoder_params) == 0 ? S_OK : E_FAIL;
		if (hr_ret != S_OK) {
			return Sstring("snaggit - Save failed");
		}
		hr_ret = p_stream->Seek(li_zero, STREAM_SEEK_SET, &pos);
		if (hr_ret != S_OK) {
			return Sstring("snaggit - Seek failed");
		}
		hr_ret = p_stream->Stat(&stg, STATFLAG_NONAME);
		if (hr_ret != S_OK) {
			return Sstring("snaggit - Seek failed");
		}

		buffer = new BYTE[stg.cbSize.LowPart];
		dw_buffer_size = stg.cbSize.LowPart;


		hr_ret = p_stream->Read(buffer, stg.cbSize.LowPart, &bytes_read);
		if (hr_ret != S_OK) {
			return  Sstring("snaggit - Read failed");
		}
		const int nl = strlen(name);
		const int fl = bytes_read;
		const auto skip = (2 * (nl));
		const auto dlen = 24 + (2 * nl + 1) + fl;

		const auto encoded = new(std::nothrow) char[dlen * 2];
		const auto data = new(std::nothrow) char[dlen];

		if (encoded == nullptr || data == nullptr) {
			return Sstring("snaggit - out of memory");
		}

		auto ptr = data;

		struct ip_header {
			long signature;
			long headerlen;
			long version;
			long reserved;
			long flen;
			long nlen;
		};

		ip_header header{};
		header.signature = 1095125447;
		header.headerlen = 20;
		header.version = 1;
		header.reserved = 0;
		header.flen = fl;
		header.nlen = nl;


		memcpy(ptr, &header, 24);

		ptr = ptr + 24;
		memcpy(ptr, name, 2 * (nl + 1));
		ptr += skip;

		memcpy(ptr, buffer, bytes_read);


		to_base64_crypto(data, dlen, encoded, dlen * 2);


		s = Sstring(encoded);

		delete[] encoded;
		delete[] data;
		delete[] buffer;


		if (p_stream)
		{
			p_stream->Release();
		}
	}

	delete resized;

	DeleteObject(hbm_capture);


	//GetBrowserApp().GetFrame()->ShowWindowAsync(SW_SHOWNORMAL);
	Sleep(250);

	return s;

}