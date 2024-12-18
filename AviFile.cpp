#include "stdafx.h"
#include "avifile.h"

HWND g_hwFix=NULL;
long WINAPI FixProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    if(uMsg==WM_USER+111){
        HWND hwDlg = GetActiveWindow();
        HWND hw = GetWindow(hwDlg,GW_CHILD);
        do{
            TCHAR ClassName[267];
            GetClassName(hw,ClassName,267);
            if (!_wcsnicmp(ClassName, L"combobox",8))
                break;
        }while(hw = GetWindow(hw,GW_HWNDNEXT));
        if(hw){
            RECT r;
            GetWindowRect(hw,&r);
            r.right-=r.left;
            r.bottom-=r.top;
            ScreenToClient(hwDlg,(LPPOINT)&r);
            r.bottom=150;
            MoveWindow(hw,r.left,r.top,r.right,r.bottom,TRUE);
        }
    }
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
void ErrorMessage(HRESULT hr, char *szError)
{
    LPTSTR pMsgBuf;

    if (hr == AVIERR_UNSUPPORTED) strcpy(szError, "AVIERR_UNSUPPORTED");
    else if(hr==AVIERR_BADFORMAT) strcpy(szError,"AVIERR_BADFORMAT");
    else if(hr==AVIERR_MEMORY) strcpy(szError,"AVIERR_MEMORY");
    else if(hr==AVIERR_INTERNAL) strcpy(szError,"AVIERR_INTERNAL");
    else if(hr==AVIERR_BADFLAGS) strcpy(szError,"AVIERR_BADFLAGS");
    else if(hr==AVIERR_BADPARAM) strcpy(szError,"AVIERR_BADPARAM");
    else if(hr==AVIERR_BADSIZE) strcpy(szError,"AVIERR_BADSIZE");
    else if(hr==AVIERR_BADHANDLE) strcpy(szError,"AVIERR_BADHANDLE");
    else if(hr==AVIERR_FILEREAD) strcpy(szError,"AVIERR_FILEREAD");
    else if(hr==AVIERR_FILEWRITE) strcpy(szError,"AVIERR_FILEWRITE");
    else if(hr==AVIERR_FILEOPEN) strcpy(szError,"AVIERR_FILEOPEN");
    else if(hr==AVIERR_COMPRESSOR) strcpy(szError,"AVIERR_COMPRESSOR");
    else if(hr==AVIERR_NOCOMPRESSOR) strcpy(szError,"AVIERR_NOCOMPRESSOR");
    else if(hr==AVIERR_READONLY) strcpy(szError,"AVIERR_READONLY");
    else if(hr==AVIERR_NODATA) strcpy(szError,"AVIERR_NODATA");
    else if(hr==AVIERR_BUFFERTOOSMALL) strcpy(szError,"AVIERR_BUFFERTOOSMALL");
    else if(hr==AVIERR_CANTCOMPRESS) strcpy(szError,"AVIERR_CANTCOMPRESS");
    else if(hr==AVIERR_USERABORT) strcpy(szError,"AVIERR_USERABORT");
    else if(hr==AVIERR_ERROR) strcpy(szError,"AVIERR_ERROR");
    else{
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,hr,
                      MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&pMsgBuf,0,NULL);
        if (pMsgBuf)WideCharToMultiByte(CP_ACP, 0, pMsgBuf, 256, szError, 256, NULL, NULL);
        LocalFree(pMsgBuf);
    }
}

CAviFile::CAviFile(char* szFileName, int nBpp, int nFps, int nKeyFrameEvery,int nQuality,char *szCodec)
{
    *m_szError = 0;
    m_nFps = nFps;
    m_nKeyFramesEvery = nKeyFrameEvery;
    m_nQuality = nQuality;
    m_Codec = mmioFOURCC(szCodec[0],szCodec[1],szCodec[2],szCodec[3]);
    m_nBpp = nBpp;

    AVIFileInit();

    MultiByteToWideChar(CP_ACP, 0, szFileName, MAX_PATH, m_szFileName, strlen(szFileName) + 1);
    m_hHeap=NULL;
    m_hAviDC=NULL;
    m_lpBits=NULL;
    m_lSample=NULL;
    m_pAviFile=NULL;
    m_pAviStream=NULL;
    m_pAviCompressedStream=NULL;

    pAppendFrame[0]=&CAviFile::AppendDummy;
    pAppendFrame[1]=&CAviFile::AppendFrameFirstTime;
    pAppendFrame[2]=&CAviFile::AppendFrameUsual;

    nAppendFuncSelector=1;      //0=Dummy   1=FirstTime 2=Usual
}

CAviFile::~CAviFile(void)
{
    ReleaseMemory();

    AVIFileExit();
}

void CAviFile::ReleaseMemory()
{
    nAppendFuncSelector=0;      //Point to DummyFunction
    HRESULT hr;

    if(m_hAviDC)
    {
        DeleteDC(m_hAviDC);
        m_hAviDC=NULL;
    }
    if(m_pAviCompressedStream)
    {
        hr = AVIStreamEndStreaming(m_pAviCompressedStream);
        hr = AVIStreamRelease(m_pAviCompressedStream);
        m_pAviCompressedStream=NULL;
    }
    if(m_pAviStream)
    {
        hr = AVIStreamEndStreaming(m_pAviStream);
        hr = AVIStreamRelease(m_pAviStream);
        m_pAviStream=NULL;
    }
    if(m_pAviFile)
    {
        hr = AVIFileRelease(m_pAviFile);
        m_pAviFile=NULL;
    }
    if(m_lpBits)
    {
        HeapFree(m_hHeap,HEAP_NO_SERIALIZE,m_lpBits);
        m_lpBits=NULL;
    }
    if(m_hHeap)
    {
        HeapDestroy(m_hHeap);
        m_hHeap=NULL;
    }
}

HRESULT CAviFile::AppendFrameFirstTime(HBITMAP  hBitmap)
{
    HRESULT hr;
    int nMaxWidth=GetSystemMetrics(SM_CXSCREEN),nMaxHeight=GetSystemMetrics(SM_CYSCREEN);

    BITMAPINFO  bmpInfo;

    m_hAviDC=CreateCompatibleDC(NULL);
    if(m_hAviDC==NULL)  
    {
        strcpy(m_szError,"Unable to Create Compatible DC");
        goto TerminateInit;
    }

    ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    
    GetDIBits(m_hAviDC,hBitmap,0,0,NULL,&bmpInfo,DIB_RGB_COLORS);

    bmpInfo.bmiHeader.biCompression=BI_RGB;
    bmpInfo.bmiHeader.biBitCount = m_nBpp;
    bmpInfo.bmiHeader.biSizeImage = ((((bmpInfo.bmiHeader.biWidth*(DWORD)bmpInfo.bmiHeader.biBitCount)+31)&~31) >> 3)*bmpInfo.bmiHeader.biHeight;
    
    if(bmpInfo.bmiHeader.biHeight>nMaxHeight)   nMaxHeight=bmpInfo.bmiHeader.biHeight;
    if(bmpInfo.bmiHeader.biWidth>nMaxWidth) nMaxWidth=bmpInfo.bmiHeader.biWidth;

    m_hHeap=HeapCreate(HEAP_NO_SERIALIZE,nMaxWidth*nMaxHeight*4,0);
    if(m_hHeap==NULL)
    {
        strcpy(m_szError,"Unable to Allocate Memory");
        goto TerminateInit;
    }
    
    m_lpBits=HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY|HEAP_NO_SERIALIZE,nMaxWidth*nMaxHeight*4);
    if(m_lpBits==NULL)  
    {   
        strcpy(m_szError,"Unable to Allocate Memory");
        goto TerminateInit;
    }

    if(FAILED(AVIFileOpen(&m_pAviFile,m_szFileName,OF_CREATE|OF_WRITE,NULL)))
    {
        strcpy(m_szError,"Unable to Create the Movie File");
        goto TerminateInit;
    }

    ZeroMemory(&m_AviStreamInfo,sizeof(AVISTREAMINFO));
    m_AviStreamInfo.fccType=streamtypeVIDEO;
    m_AviStreamInfo.fccHandler=m_Codec;
    m_AviStreamInfo.dwScale=1;
    m_AviStreamInfo.dwRate=m_nFps;      //15fps //Ändrad
    m_AviStreamInfo.dwQuality=-1;   //Default Quality
    m_AviStreamInfo.dwSuggestedBufferSize=nMaxWidth*nMaxHeight*4;
    SetRect(&m_AviStreamInfo.rcFrame,0,0,bmpInfo.bmiHeader.biWidth,bmpInfo.bmiHeader.biHeight);
    wcscpy(m_AviStreamInfo.szName,L"Video Stream");

    if(FAILED(AVIFileCreateStream(m_pAviFile,&m_pAviStream,&m_AviStreamInfo)))
    {
        strcpy(m_szError,"Unable to Create Stream");
        goto TerminateInit;

    }

    ZeroMemory(&m_AviCompressOptions,sizeof(AVICOMPRESSOPTIONS));
    m_AviCompressOptions.fccType=streamtypeVIDEO;
    m_AviCompressOptions.fccHandler=m_AviStreamInfo.fccHandler;
    m_AviCompressOptions.dwFlags=0;//AVICOMPRESSF_KEYFRAMES|AVICOMPRESSF_VALID;//|AVICOMPRESSF_DATARATE;
    m_AviCompressOptions.dwKeyFrameEvery=m_nKeyFramesEvery; // Ändrad
    //m_AviCompressOptions.dwBytesPerSecond=1000/8;
    m_AviCompressOptions.dwQuality=m_nQuality;

    hr = AVIMakeCompressedStream(&m_pAviCompressedStream,m_pAviStream,&m_AviCompressOptions,NULL);

    if(FAILED(hr))
    {
        if(hr==AVIERR_NOCOMPRESSOR)
            strcpy(m_szError,"A suitable compressor cannot be found.");
        else if(hr==AVIERR_MEMORY)
            strcpy(m_szError,"There is not enough memory to complete the operation.");
        else if(hr==AVIERR_UNSUPPORTED)
            strcpy(m_szError,"Compression is not supported for this type of data.");
        else
            strcpy(m_szError,"Unable to Create Compressed Stream");
        goto TerminateInit;
    }

    hr = AVIStreamSetFormat(m_pAviCompressedStream,0,(LPVOID)&bmpInfo,bmpInfo.bmiHeader.biSize);
    if(FAILED(hr))
    {
        if(hr==AVIERR_NOCOMPRESSOR)
            strcpy(m_szError,"A suitable compressor cannot be found.");
        else if(hr==AVIERR_MEMORY)
            strcpy(m_szError,"There is not enough memory to complete the operation.");
        else if(hr==AVIERR_UNSUPPORTED)
            strcpy(m_szError,"Compression is not supported for this type of data.");
        else
            ErrorMessage(hr,m_szError);
        goto TerminateInit;
    }

    nAppendFuncSelector=2;      //Point to UsualAppend Function

    return AppendFrameUsual(hBitmap);

TerminateInit:

    ReleaseMemory();

    return E_FAIL;
}

HRESULT CAviFile::AppendFrameUsual(HBITMAP hBitmap)
{
    BITMAPINFO  bmpInfo;

    bmpInfo.bmiHeader.biBitCount=0;
    bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    
    GetDIBits(m_hAviDC,hBitmap,0,0,NULL,&bmpInfo,DIB_RGB_COLORS);

    bmpInfo.bmiHeader.biCompression=BI_RGB; 
    bmpInfo.bmiHeader.biBitCount = m_nBpp;
    bmpInfo.bmiHeader.biSizeImage = ((((bmpInfo.bmiHeader.biWidth*(DWORD)bmpInfo.bmiHeader.biBitCount)+31)&~31) >> 3)*bmpInfo.bmiHeader.biHeight;

    GetDIBits(m_hAviDC,hBitmap,0,bmpInfo.bmiHeader.biHeight,m_lpBits,&bmpInfo,DIB_RGB_COLORS);

    if(FAILED(AVIStreamWrite(m_pAviCompressedStream,m_lSample++,1,m_lpBits,bmpInfo.bmiHeader.biSizeImage,0,NULL,NULL)))
        return E_FAIL;

    return S_OK;
}

HRESULT CAviFile::AppendDummy(HBITMAP)
{
    return E_FAIL;
}

HRESULT CAviFile::AppendNewFrame(HBITMAP hBitmap)
{
    return (this->*pAppendFrame[nAppendFuncSelector])((HBITMAP)hBitmap);
}



int CAviFile::ChooseCodec(HWND hWnd,int nWidth, int nHeight)
{
    BITMAPINFOHEADER bih;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = nWidth;
    bih.biHeight = nHeight;
    bih.biPlanes = 1;
    bih.biBitCount = m_nBpp; 
    bih.biCompression = 0; /* 0 For uncompressed */
    bih.biSizeImage =   ((((bih.biWidth*(DWORD)bih.biBitCount)+31)&~31) >> 3)*bih.biHeight;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    COMPVARS cv;
    memset(&cv, 0, sizeof(cv));
    cv.cbSize = sizeof(cv);
    cv.fccHandler = m_Codec;
    cv.dwFlags = ICMF_COMPVARS_VALID;
    cv.lQ = 8500;
    cv.fccType = ICTYPE_VIDEO;
    cv.lKey = 1;
    cv.lDataRate = m_nFps;
    if(!g_hwFix){
        WNDCLASS wc={0};
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"KALLES FIX FÖR ICCompressorChoose";
        wc.lpfnWndProc = (WNDPROC)FixProc;
        RegisterClass(&wc);
        g_hwFix = CreateWindow(wc.lpszClassName,L"",WS_POPUP,0,0,0,0,NULL,NULL,wc.hInstance,0);
    }
    PostMessage(g_hwFix,WM_USER+111,0,0);
    if(!ICCompressorChoose(hWnd,/*ICMF_CHOOSE_DATARATE | */ICMF_CHOOSE_KEYFRAME,&bih,NULL,&cv,"Choose Codec"))
        return 0;
    m_Codec = cv.fccHandler;
    m_nKeyFramesEvery = cv.lKey;
    m_nQuality = cv.lQ/100;
    return 1;
}
char *CAviFile::GetCodec()
{
    return (char*)&m_Codec;
}
