#ifndef __AVIFILE_H
#define __AVIFILE_H

#include <vfw.h>

class CAviFile
{
    HDC             m_hAviDC;
    HANDLE          m_hHeap;
    LPVOID          m_lpBits;
    LONG            m_lSample;
    TCHAR           m_szFileName[MAX_PATH];
    PAVIFILE        m_pAviFile;
    PAVISTREAM      m_pAviStream;
    PAVISTREAM      m_pAviCompressedStream;
    AVISTREAMINFO   m_AviStreamInfo;
    AVICOMPRESSOPTIONS  m_AviCompressOptions;
    int m_nFps;
    int m_nKeyFramesEvery;
    int m_nQuality;
    DWORD m_Codec;
    char m_szError[256];
    int m_nBpp;

    int     nAppendFuncSelector;        //0=Dummy   1=FirstTime 2=Usual
    HRESULT AppendFrameFirstTime(HBITMAP );
    HRESULT AppendFrameUsual(HBITMAP);
    HRESULT AppendDummy(HBITMAP);
    HRESULT (CAviFile::*pAppendFrame[3])(HBITMAP hBitmap);

    void    ReleaseMemory();
public:
    CAviFile(char *szFileName, int nBpp=24, int nFps=20, int nKeyFrameEvery=1,int nQuality=100, char *szCodec="MSVC");
    ~CAviFile(void);
    HRESULT AppendNewFrame(HBITMAP hBitmap);
    HRESULT AppendNewFrame(int nWidth, int nHeight, LPVOID pBits,int nBitsPerPixel=32);

    int ChooseCodec(HWND hWnd,int nWidth, int nHeight);
    char *GetLastError() { return m_szError; }
    char *GetCodec();
};

#endif