/**
*	@file	GdiplusUtil.cpp
*	@brief	Gdi+���g���̂�֗��ɂ���
*/

#include "stdafx.h"
#include "GdiplusUtil.h"

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

class CGdlplusUtil
{
public:
	CGdlplusUtil()
	{
		GdiplusStartupInput	gdiplusStartupInput;
		GdiplusStartup(&m_token, &gdiplusStartupInput, NULL);

		UINT	size;
		GetImageEncodersSize(&m_nEncoders, &size);
		m_pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		GetImageEncoders(m_nEncoders, size, m_pImageCodecInfo);
	}
	~CGdlplusUtil()
	{
		free(m_pImageCodecInfo);
		Gdiplus::GdiplusShutdown(m_token);
	}

	ImageCodecInfo*	GetEncoderByExtension(LPCWSTR extension)
	{
		for (UINT i = 0; i < m_nEncoders; ++i) {
			if (PathMatchSpecW(extension, m_pImageCodecInfo[i].FilenameExtension))
				return &m_pImageCodecInfo[i];
		}
		return nullptr;
	}

	ImageCodecInfo*	GetEncoderByMimeType(LPCWSTR mimetype)
	{
		for (UINT i = 0; i < m_nEncoders; ++i) {
			if (wcscmp(m_pImageCodecInfo[i].MimeType, mimetype) == 0)
				return &m_pImageCodecInfo[i];
		}
		return nullptr;
	}

private:
	ULONG_PTR	m_token;
	UINT		m_nEncoders;
	ImageCodecInfo* m_pImageCodecInfo;

	
};

CGdlplusUtil	GdiplusUtil;	/// GDI+�̏������ƌ�n���̂���




//---------------------------------------
/// �g���q���w�肵�ăG���R�[�_�[���擾����
Gdiplus::ImageCodecInfo*	GetEncoderByExtension(LPCWSTR extension)
{
	return GdiplusUtil.GetEncoderByExtension(extension);
}


//--------------------------------------
/// MIME�^�C�v���w�肵�ăG���R�[�_���擾����
Gdiplus::ImageCodecInfo*	GetEncoderByMimeType(LPCWSTR mimetype)
{
	return GdiplusUtil.GetEncoderByMimeType(mimetype);
}











