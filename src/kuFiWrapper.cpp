// Copyright (C) 2012  Augustino (augustino@users.sourceforge.net)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "ku.h"

// -------- kuFiWrapper --------
bool kuFiWrapper::Initialize() {
    FreeImage_Initialise();
    FreeImage_SetOutputMessage(kuFiWrapper::ErrorHandler);
    return true;
}

bool kuFiWrapper::Finalize() {
    FreeImage_DeInitialise();
    return true;
}

void kuFiWrapper::ErrorHandler(FREE_IMAGE_FORMAT fif, const char *message) {
    wxString self = THREAD_NAME_CURRENT;
    wxString msg(message, wxConvUTF8);
    wxLogDebug(wxT("%s kuFiWrapper::ErrorHandler: %s"), self.c_str(), msg.c_str());
    if(self!=THREAD_NAME_MAIN)    return;
    if(msg.BeforeFirst(':')==wxT("Warning") || msg.BeforeFirst(':')==wxT("Exif")) {
        wxLogStatus(msg);
    } else {
        wxMessageBox(msg);
        if(wxGetApp().SetInterrupt(true) && wxGetApp().mFrame) {
            wxGetApp().mFrame->GetMenuBar()->Enable(kuID_INTERRUPT,false);
            wxGetApp().mFrame->GetToolBar()->EnableTool(kuID_INTERRUPT,false);
        }
    }
}

wxString kuFiWrapper::GetSupportedExtensions() {
    wxString exts;
    for(int i=0; i<FreeImage_GetFIFCount(); i++) {
        wxString list = wxString(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i), wxConvUTF8);
        list.Replace(wxT(","), wxT(";*."));
        exts = exts + wxT("*.") + list + wxT(";");
    }
    exts.RemoveLast();
    #ifdef __WXMSW__
    return wxT("|") + exts;
    #else
    return wxT("|") + exts+wxT(";")+exts.Upper();
    #endif
}

void kuFiWrapper::GetSupportedExtensions(wxArrayString* exts, bool upper) {
    for(int i=0; i<FreeImage_GetFIFCount(); i++) {
        wxStringTokenizer tokenzr(wxString(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i),wxConvUTF8), wxT(","));
        while(tokenzr.HasMoreTokens()) {
            wxString token = tokenzr.GetNextToken();
            if(exts->Index(token)==wxNOT_FOUND) {    // filter duplicated exts
                if(upper)    exts->Add(token.Upper());
                else    exts->Add(token.Lower());
            }
        }
    }
}

void kuFiWrapper::GetAllSupportedFiles(wxString& src, wxArrayString* files, wxArrayString* formats) {
    bool clear = false;
    if(!formats) {
        formats = new wxArrayString();
        kuFiWrapper::GetSupportedExtensions(formats, true);
        clear = true;
    }
    wxDir::GetAllFiles(src, files);
    size_t idx=0;
    while(idx < files->GetCount()) {
        wxFileName fn((*files)[idx]);
        if(formats->Index(fn.GetExt().Upper())==wxNOT_FOUND)    // assume suppoerted input/output formats are the same
            files->RemoveAt(idx);
        else {
            (*files)[idx] = fn.GetFullPath();
            idx += 1;
        }
    }
    if(clear)    delete formats;
}

bool kuFiWrapper::IsSupportedExtension(wxString ext) {
    wxArrayString exts;
    for(int i=0; i<FreeImage_GetFIFCount(); i++) {
        wxStringTokenizer tokenzr(wxString(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i),wxConvUTF8), wxT(","));
        while(tokenzr.HasMoreTokens())
            exts.Add(tokenzr.GetNextToken());
    }
    if(exts.Index(ext)==wxNOT_FOUND)    return false;
    return true;
}

wxString kuFiWrapper::GetMdModelString(FREE_IMAGE_MDTYPE type) {
    switch (type) {
        case FIMD_COMMENTS:
            return    STRING_METADATA_COMMENTS;
        case FIMD_EXIF_MAIN:
            return    STRING_METADATA_MAIN;
        case FIMD_EXIF_EXIF:
            return    STRING_METADATA_EXIF;
        case FIMD_EXIF_GPS:
            return    STRING_METADATA_GPS;
        case FIMD_EXIF_MAKERNOTE:
            return    STRING_METADATA_MAKERNOTE;
        case FIMD_EXIF_INTEROP:
            return    STRING_METADATA_INTEROP;
        case FIMD_IPTC:
            return    STRING_METADATA_IPTC;
        case FIMD_XMP:
            return    STRING_METADATA_XMP;
        case FIMD_GEOTIFF:
            return    STRING_METADATA_GEOTIFF;
        case FIMD_ANIMATION:
            return    STRING_METADATA_ANIMATION;
        default:
            return wxEmptyString;
    }
}

FREE_IMAGE_FILTER kuFiWrapper::GetFilterById(int id) {
    FREE_IMAGE_FILTER filter;
    switch (id) {
        case kuID_RESCALE_BOX:
            filter = FILTER_BOX;
            break;
        case kuID_RESCALE_BICUBIC:
            filter = FILTER_BICUBIC;
            break;
        case kuID_RESCALE_BILINEAR:
            filter = FILTER_BILINEAR;
            break;
        case kuID_RESCALE_BSPLINE:
            filter = FILTER_BSPLINE;
            break;
        case kuID_RESCALE_CATMULLROM:
            filter = FILTER_CATMULLROM;
            break;
        case kuID_RESCALE_LANCZOS3:
            filter = FILTER_LANCZOS3;
            break;
        default:
            return FILTER_BILINEAR;
    }
    return filter;
}
int kuFiWrapper::GetIdByFilter(FREE_IMAGE_FILTER filter) {
    int id;
    switch (filter) {
        case FILTER_BOX:
            id = kuID_RESCALE_BOX;
            break;
        case FILTER_BICUBIC:
            id = kuID_RESCALE_BICUBIC;
            break;
        case FILTER_BILINEAR:
            id = kuID_RESCALE_BILINEAR;
            break;
        case FILTER_BSPLINE:
            id = kuID_RESCALE_BSPLINE;
            break;
        case FILTER_CATMULLROM:
            id = kuID_RESCALE_CATMULLROM;
            break;
        case FILTER_LANCZOS3:
            id = kuID_RESCALE_LANCZOS3;
            break;
        default:
            return kuID_RESCALE_BILINEAR;
    }
    return id;
}

unsigned kuFiWrapper::ReadProc(void* buffer, unsigned size, unsigned count, fi_handle handle) {
    //return fread(buffer, size, count, (FILE *)handle);
    wxInputStream* in = (wxInputStream*)handle;
    in->Read(buffer, count*size);
    return (unsigned) in->LastRead();
}
unsigned kuFiWrapper::WriteProc(void* buffer, unsigned size, unsigned count, fi_handle handle) {
    //return fwrite(buffer, size, count, (FILE *)handle);
    return 0;
}
int kuFiWrapper::SeekProc(fi_handle handle, long offset, int origin) {
    //return fseek((FILE *)handle, offset, origin);
    wxInputStream* in = (wxInputStream*)handle;
    switch (origin) {
        case SEEK_SET:
            return (int) in->SeekI(offset, wxFromStart);
        case SEEK_CUR:
            return (int) in->SeekI(offset, wxFromCurrent);
        case SEEK_END:
            return (int) in->SeekI(offset, wxFromEnd);
        default:
            break;
    }
    return 0;
}
long kuFiWrapper::TellProc(fi_handle handle) {
    //return ftell((FILE *)handle);
    wxInputStream* in = (wxInputStream*)handle;
    return (long) in->TellI();
}

wxImage* kuFiWrapper::GetWxImage(wxString filename, bool isurl, wxSize size) {
    if(filename==wxEmptyString)   return NULL;
    FIBITMAP* bmp = GetFiBitmap(filename,isurl,size);
    unsigned int width  = FreeImage_GetWidth(bmp);
    unsigned int height = FreeImage_GetHeight(bmp);
    if(width>size.x && height>size.y) {    // it still be loaded completely for other format
        FIBITMAP* tmp = FreeImage_MakeThumbnail(bmp,wxMax(size.x,size.y));
        if(tmp) {
            FreeImage_Unload(bmp);
            bmp = tmp;
        }
    }
    wxImage* image = new wxImage();
    FiBitmap2WxImage(bmp,image);
    FreeImage_Unload(bmp);
    return image;
}

FIBITMAP* kuFiWrapper::GetFiBitmap(wxString filename, bool isurl, wxSize size, int fast) {
    if(filename==wxEmptyString)   return NULL;
    wxString self = THREAD_NAME_CURRENT;

    // set scale
    int flags = 0;
    int scale = size.x>size.y ? size.x : size.y;
    if(fast) {
        if(scale)    scale = scale/fast;
        else    scale = fast;    // assign it directly since cannot use size to calculate
    }

    // open file
    FIBITMAP* bmp;
    FREE_IMAGE_FORMAT fif;
    if(isurl) {
        wxFileSystem* fileSystem = new wxFileSystem();
        wxFSFile* file=fileSystem->OpenFile(filename);
        if(file) {
            FreeImageIO io;
            io.read_proc  = ReadProc;
            io.write_proc = WriteProc;
            io.seek_proc  = SeekProc;
            io.tell_proc  = TellProc;
            fif = FreeImage_GetFileTypeFromHandle(&io, (fi_handle)file->GetStream(), 0);
            if(fif == FIF_UNKNOWN) {
                wxString name = filename.AfterLast(':');    // ex: "archive.zip#zip:filename", "archive.tar.gz#gzip:#tar:filename"
                if(name==wxEmptyString)
                    name = filename.BeforeFirst('#').BeforeLast('.');   // ex: "document.ps.gz#gzip:"
                fif = GetImageFormat(name, false);
                if(fif == FIF_UNKNOWN)    return NULL;
            }
            // set flags
            if(fif==FIF_JPEG) {
                flags |= JPEG_DEFAULT|JPEG_EXIFROTATE;
                if(scale)    flags |= scale <<16;
            } else if(fif==FIF_RAW) {
                if(scale)    flags |= RAW_PREVIEW;
                else    flags |= RAW_DISPLAY;
            }
            bmp = FreeImage_LoadFromHandle(fif, &io, (fi_handle)file->GetStream(), flags);
            delete file;
        } else    return NULL;
        delete fileSystem;
    } else {
        fif = GetImageFormat(filename);
        if(fif == FIF_UNKNOWN)    return NULL;
        // set flags
        if(fif==FIF_JPEG) {
            flags |= JPEG_DEFAULT|JPEG_EXIFROTATE;
            if(scale)    flags |= scale <<16;
        } else if(fif==FIF_RAW) {
            if(scale)    flags |= RAW_PREVIEW;
            else    flags |= RAW_DISPLAY;
        }
        #ifdef __WXMSW__
        bmp = FreeImage_LoadU(fif,filename.wc_str(wxConvFile),flags);
        #else
        bmp = FreeImage_Load(fif,filename.mb_str(wxConvFile),flags);
        #endif
    }
    wxLogDebug(wxT("%s kuFiWrapper::GetFiBitmap: format = %d"), self.c_str(), (int)fif);
    FreeImage_FlipVertical(bmp);

    // scale bmp
    FIBITMAP* thumb;
    unsigned int width  = FreeImage_GetWidth(bmp);
    unsigned int height = FreeImage_GetHeight(bmp);
    wxSize origSize = fif==FIF_JPEG ? GetOriginalJPEGSize(bmp) : wxSize(width, height);
    /* origSize may be wxSize(0,0) for JPEG even if size!=wxSize(0,0) */
    if(fif==FIF_JPEG && origSize==wxSize(0,0) && (size==wxSize(0,0) || size.x>=width && size.y>=height)    // JPEG is fully loaded
       || fif!=FIF_JPEG) {    // other is fully loaded
        thumb = bmp;
    } else if(size == wxSize(0,0)) {    // load completely
        thumb = FreeImage_Rescale(bmp, origSize.x, origSize.y, FILTER_BOX);
    } else {    // fast loading or normal case
        if((double)width/(double)height > (double)size.x/(double)size.y) {
            double ratio = origSize.x==0 ? (double)height/(double)width : (double)origSize.y/(double)origSize.x;
            int target = origSize.x==0 ? size.x : wxMin(origSize.x, size.x);
            thumb = FreeImage_Rescale(bmp, target, (int)(target*ratio+0.5), fast?FILTER_BOX:FILTER_BILINEAR);
        } else {
            double ratio = origSize.y==0 ? (double)width/(double)height : (double)origSize.x/(double)origSize.y;
            int target = origSize.y==0 ? size.y : wxMin(origSize.y, size.y);
            thumb = FreeImage_Rescale(bmp, (int)(target*ratio+0.5), target, fast?FILTER_BOX:FILTER_BILINEAR);
        }
    }
    /* FreeImage_FUNC will clone metadata automatically after 3.11.0
    // set metadata to thumb
    #ifndef __WXMSW__
    FITAG* tag = NULL;
    FIMETADATA* mdhandle = NULL;
    for(size_t i=0; i<(int)FIMD_CUSTOM; i++) {
        if(!FreeImage_GetMetadataCount((FREE_IMAGE_MDMODEL)i, bmp))    continue;    // no data for this model
        mdhandle = FreeImage_FindFirstMetadata((FREE_IMAGE_MDMODEL)i, bmp, &tag);
        if(mdhandle) {
            do {
                FreeImage_SetMetadata((FREE_IMAGE_MDMODEL)i, thumb, FreeImage_GetTagKey(tag), tag);
            } while(FreeImage_FindNextMetadata(mdhandle, &tag));
        }
        FreeImage_FindCloseMetadata(mdhandle);
    }
    #else
    FreeImage_CloneMetadata(thumb, bmp);
    #endif
    */
    if(bmp != thumb) {
        FreeImage_Unload(bmp);
        if(fif==FIF_JPEG && origSize==wxSize(0,0))    SetOriginalJPEGSize(thumb, wxSize(width,height));    // when fully loaded but be rescaled
    }
    wxLogDebug(wxT("%s kuFiWrapper::GetFiBitmap: orig=%dx%d, size=%dx%d, bmp=%dx%d, result=%dx%d, fast=%d, scale=%d"),
                                       self.c_str(), origSize.x, origSize.y, size.x, size.y, width, height, FreeImage_GetWidth(thumb), FreeImage_GetHeight(thumb), fast, scale);
    return thumb;
}

bool kuFiWrapper::FiBitmap2WxImage(FIBITMAP* bmp, wxImage* image) {
    wxString self = THREAD_NAME_CURRENT;
    FREE_IMAGE_TYPE type = FreeImage_GetImageType(bmp);
    FREE_IMAGE_COLOR_TYPE color = FreeImage_GetColorType(bmp);
    wxLogDebug(wxT("%s kuFiWrapper::FiBitmap2WxImage: image type = %d, color type = %d"), self.c_str(), (int)type, (int)color);
    FIBITMAP* tmp;
    tmp = FreeImage_ConvertTo32Bits(bmp);
    if(!tmp) {
        tmp = FreeImage_ConvertTo24Bits(bmp);
        if(!tmp)    return false;   // cannot convert to 32bits
    }
    unsigned int width  = FreeImage_GetWidth(tmp);
    unsigned int height = FreeImage_GetHeight(tmp);
    unsigned int pitch  = FreeImage_GetPitch(tmp);
    unsigned int bpp    = FreeImage_GetBPP(tmp);

    image->Destroy();
    image->Create(width,height);
    image->InitAlpha();
    BYTE* bits = FreeImage_GetBits(tmp);
    for(unsigned int y=0; y<height; y++) {
        BYTE* pixel = (BYTE*)bits;
        for(unsigned int x=0; x<width; x++) {
            image->SetRGB(x,y,
                          pixel[FI_RGBA_RED],
                          pixel[FI_RGBA_GREEN],
                          pixel[FI_RGBA_BLUE]);
            if(bpp==32)    image->SetAlpha(x,y,pixel[FI_RGBA_ALPHA]);
            pixel += bpp/8;
        }
        bits += pitch;
    }
    FreeImage_Unload(tmp);
    return true;
}
wxImage* kuFiWrapper::FiBitmap2WxImage(FIBITMAP* bmp) {
    wxImage* image = new wxImage();
    if(FiBitmap2WxImage(bmp,image))    return image;
    return NULL;
}

wxSize kuFiWrapper::GetOriginalJPEGSize(FIBITMAP* bmp) {
    wxSize size(0,0);
    FITAG* tagw = NULL;
    FITAG* tagh = NULL;
    FreeImage_GetMetadata(FIMD_COMMENTS, bmp, "OriginalJPEGWidth", &tagw);
    FreeImage_GetMetadata(FIMD_COMMENTS, bmp, "OriginalJPEGHeight", &tagh);
    if(tagw && tagh) {
        size.x = atoi(FreeImage_TagToString(FIMD_COMMENTS, tagw));
        size.y = atoi(FreeImage_TagToString(FIMD_COMMENTS, tagh));
        unsigned int width  = FreeImage_GetWidth(bmp);
        unsigned int height = FreeImage_GetHeight(bmp);
        if((width>height && size.x<size.y) || (width<height && size.x>size.y)) {    // rotate by JPEG_EXIFROTATE
            size.Set(size.y, size.x);
        }
    }
    return size;
}

bool kuFiWrapper::SetOriginalJPEGSize(FIBITMAP* bmp, wxSize size) {
    // refer to store_size_info() in FreeImage PluginJPEG.cpp
    char buffer[256];
    FITAG *tag = FreeImage_CreateTag();
    if(tag) {
        size_t length = 0;
        // set the original width
        sprintf(buffer, "%d", (int)size.x);
        length = strlen(buffer) + 1;	// include the NULL/0 value
        FreeImage_SetTagKey(tag, "OriginalJPEGWidth");
        FreeImage_SetTagLength(tag, (DWORD)length);
        FreeImage_SetTagCount(tag, (DWORD)length);
        FreeImage_SetTagType(tag, FIDT_ASCII);
        FreeImage_SetTagValue(tag, buffer);
        FreeImage_SetMetadata(FIMD_COMMENTS, bmp, FreeImage_GetTagKey(tag), tag);
        // set the original height
        sprintf(buffer, "%d", (int)size.y);
        length = strlen(buffer) + 1;	// include the NULL/0 value
        FreeImage_SetTagKey(tag, "OriginalJPEGHeight");
        FreeImage_SetTagLength(tag, (DWORD)length);
        FreeImage_SetTagCount(tag, (DWORD)length);
        FreeImage_SetTagType(tag, FIDT_ASCII);
        FreeImage_SetTagValue(tag, buffer);
        FreeImage_SetMetadata(FIMD_COMMENTS, bmp, FreeImage_GetTagKey(tag), tag);
        // destroy the tag
        FreeImage_DeleteTag(tag);
        return true;
    }
	return false;
}

double kuFiWrapper::GetGPSLatitude(FIBITMAP* bmp) {
    double la = INVALID_LATITUDE;
    FITAG* tagv = NULL;
    FITAG* tagr = NULL;
    FreeImage_GetMetadata(FIMD_EXIF_GPS, bmp, "GPSLatitude",     &tagv);
    FreeImage_GetMetadata(FIMD_EXIF_GPS, bmp, "GPSLatitudeRef",  &tagr);
    if(tagv && tagr) {
        // refer to ConvertExifGPSTag() in FreeImage TagConversion.cpp
        unsigned long* pvalue = (unsigned long*)FreeImage_GetTagValue(tagv);
        double ss = 0;
        if(FreeImage_GetTagLength(tagv) == 24) {
            if(pvalue[1])
                ss += ((double)pvalue[0] / (double)pvalue[1]) * 3600;
            if(pvalue[3])
                ss += ((double)pvalue[2] / (double)pvalue[3]) * 60;
            if(pvalue[5])
                ss += ((double)pvalue[4] / (double)pvalue[5]);
            ss /= 3600;
            if(wxString(FreeImage_TagToString(FIMD_EXIF_GPS, tagr), wxConvUTF8).CmpNoCase(wxT("N"))==0)
                la = ss;
            else
                la = -ss;
        }
    }
    return la;
}

double kuFiWrapper::GetGPSLongitude(FIBITMAP* bmp) {
    double lo = INVALID_LONGITUDE;
    FITAG* tagv = NULL;
    FITAG* tagr = NULL;
    FreeImage_GetMetadata(FIMD_EXIF_GPS, bmp, "GPSLongitude",    &tagv);
    FreeImage_GetMetadata(FIMD_EXIF_GPS, bmp, "GPSLongitudeRef", &tagr);
    if(tagv && tagr) {
        // refer to ConvertExifGPSTag() in FreeImage TagConversion.cpp
        unsigned long* pvalue = (unsigned long*)FreeImage_GetTagValue(tagv);
        double ss = 0;
        if(FreeImage_GetTagLength(tagv) == 24) {
            if(pvalue[1])
                ss += ((double)pvalue[0] / (double)pvalue[1]) * 3600;
            if(pvalue[3])
                ss += ((double)pvalue[2] / (double)pvalue[3]) * 60;
            if(pvalue[5])
                ss += ((double)pvalue[4] / (double)pvalue[5]);
            ss /= 3600;
            if(wxString(FreeImage_TagToString(FIMD_EXIF_GPS, tagr), wxConvUTF8).CmpNoCase(wxT("E"))==0)
                lo = ss;
            else
                lo = -ss;
        }
    }
    return lo;
}

FREE_IMAGE_FORMAT kuFiWrapper::GetImageFormat(wxString filename, bool byfile) {
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    if(byfile) {
        #ifdef __WXMSW__
        fif = FreeImage_GetFileTypeU(filename.wc_str(wxConvFile));
        #else
        fif = FreeImage_GetFileType(filename.mb_str(wxConvFile));
        #endif
    }
    if(fif == FIF_UNKNOWN) {
        #ifdef __WXMSW__
        fif = FreeImage_GetFIFFromFilenameU(filename.wc_str(wxConvFile));
        #else
        fif = FreeImage_GetFIFFromFilename(filename.mb_str(wxConvFile));
        #endif
    }
    return fif;
}
