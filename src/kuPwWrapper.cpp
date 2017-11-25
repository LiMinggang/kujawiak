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

#ifdef ENABLE_PICASAWEBMGR
void kuPwWrapper::ErrorHandler(CURLcode res) {
    wxString self = THREAD_NAME_CURRENT;
    wxLogDebug(wxT("%s kuFiWrapper::ErrorHandler: err=%d"), self.c_str(), res);
    if(self!=THREAD_NAME_MAIN)    return;
    wxMessageBox(wxString::Format(wxT("error code=%d"), res));
}

void kuPwWrapper::GetAllSupportedFiles(wxString& src, wxArrayString* files) {
    wxArrayString formats;
    formats.Add(wxT("BMP"));
    formats.Add(wxT("GIF"));
    formats.Add(wxT("JPG"));
    formats.Add(wxT("JPEG"));
    formats.Add(wxT("PNG"));
    wxDir::GetAllFiles(src, files);
    size_t idx=0;
    while(idx < files->GetCount()) {
        wxFileName fn((*files)[idx]);
        if(formats.Index(fn.GetExt().Upper())==wxNOT_FOUND)    // assume suppoerted input/output formats are the same
            files->RemoveAt(idx);
        else {
            (*files)[idx] = fn.GetFullPath();
            idx += 1;
        }
    }
}

int kuPwWrapper::Debug(CURL* curl, curl_infotype type, char* info, size_t size, wxStringOutputStream* stream) {
    if(type == CURLINFO_DATA_OUT)    stream->Write(info, size);
    return 0;
}

size_t kuPwWrapper::WriteData(void* buffer, size_t size, size_t nmemb, wxStringOutputStream* stream) {
    return stream->Write(buffer, size*nmemb).LastWrite();
}

wxString kuPwWrapper::GetLink(wxXmlNode* entry, wxString name) {
    wxString link = wxEmptyString;
    wxXmlNode* node = entry->GetChildren();
    while(node) {
        if(node->GetName()==wxT("link") && node->GetPropVal(wxT("rel"),wxEmptyString)==name) {
            link = node->GetPropVal(wxT("href"), wxEmptyString);
            break;
        }
        node = node->GetNext();
    }
    return link;
}

bool kuPwWrapper::GetEntries(wxString url, kuXmlNodeArray* entries, wxString token) {
    bool success = false;
    CURL* curl;
    CURLcode res;
    wxString data;
    wxStringOutputStream recv(&data);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.mb_str(wxConvUTF8));
        struct curl_slist* headerlist = NULL;
        headerlist = curl_slist_append(headerlist, "Content-Type: text/html; charset=utf-8");
        if(token!=wxEmptyString) {
            wxString str = wxString::Format(wxT("Authorization: GoogleLogin auth=%s"), token);
            headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv);
        //curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");

        res = curl_easy_perform(curl);
        if(res == CURLE_OK)    success = true;
        else    ErrorHandler(res);

        curl_slist_free_all(headerlist);
        curl_easy_cleanup(curl);
    }

    if(success && data!=wxEmptyString) {
        wxXmlDocument doc;
        wxStringInputStream input(data);
        if(doc.Load(input)) {
            if(doc.GetRoot()->GetName() == wxT("feed")) {
                wxXmlNode* entry = doc.GetRoot()->GetChildren();
                WX_CLEAR_ARRAY(*entries);
                while(entry) {
                    if(entry->GetName() == wxT("entry")) {
                        wxXmlNode* node = new wxXmlNode(*entry);
                        wxXmlProperty* property = doc.GetRoot()->GetProperties();
                        while(property) {    // copy properties from root
                            node->AddProperty(property->GetName(), property->GetValue());
                            property = property->GetNext();
                        }
                        entries->Add(node);
                    }
                    entry = entry->GetNext();
                }
            }
        }
    }
    return success;
}

wxXmlNode* kuPwWrapper::GetChild(wxXmlNode* entry, wxString name) {
    wxXmlNode* child = NULL;
    wxXmlNode* node = entry->GetChildren();
    while(node) {
        if(node->GetName() == name) {
            child = node;
            break;
        }
        node = node->GetNext();
    }
    return child;
}

bool kuPwWrapper::GetFile(wxString url, wxString filename, wxString token) {
    bool success = false;
    CURL* curl;
    CURLcode res;
    wxFileOutputStream recv(filename);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.mb_str(wxConvUTF8));
        struct curl_slist* headerlist = NULL;
        headerlist = curl_slist_append(headerlist, "Content-Type: text/html; charset=utf-8");
        if(token!=wxEmptyString) {
            wxString str = wxString::Format(wxT("Authorization: GoogleLogin auth=%s"), token);
            headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv);
        //curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");

        res = curl_easy_perform(curl);
        if(res == CURLE_OK)    success = true;
        else    ErrorHandler(res);

        curl_slist_free_all(headerlist);
        curl_easy_cleanup(curl);
    }
    return success;
}

wxString kuPwWrapper::GetProperty(wxXmlNode* entry, wxString name, wxString prop) {
    wxString value = wxEmptyString;
    wxXmlNode* node = entry->GetChildren();
    while(node) {
        if(node->GetName() == name) {
            if(prop==wxEmptyString)    value = node->GetNodeContent();
            else    value = node->GetPropVal(prop, wxEmptyString);
            break;
        }
        node = node->GetNext();
    }
    return value;
}

bool kuPwWrapper::SetProperty(wxXmlNode* entry, wxString name, wxString value) {
    bool success = false;
    wxXmlNode* node = entry->GetChildren();
    while(node) {
        if(node->GetName() == name) {
            if(!node->GetChildren()) {
                node->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxEmptyString));
            }
            node->GetChildren()->SetContent(value);    // set the content of first child
            success = true;
            break;
        }
        node = node->GetNext();
    }
    return success;
}

wxString kuPwWrapper::Login(wxString userid, wxString passwd) {
    /* POST https://www.google.com/accounts/ClientLogin for getting token */
    bool success = false;
    CURL* curl;
    CURLcode res;
    wxStringOutputStream recv;

    curl = curl_easy_init();
    if(curl) {
        wxString str = wxString::Format(wxT("accountType=GOOGLE&Email=%s@gmail.com&Passwd=%s&service=lh2&source=kuView-1.7pre"),
                                        userid, passwd);
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/accounts/ClientLogin");
        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, str.ToAscii());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv);
        //curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");

        res = curl_easy_perform(curl);
        if(res == CURLE_OK)    success = true;
        else    ErrorHandler(res);

        curl_easy_cleanup(curl);
    }
    if(success) {
        int index = recv.GetString().Find(wxT("Auth="));
        if(index != wxNOT_FOUND)    return recv.GetString().Mid(index+5).Trim();
    }
    return wxEmptyString;
}

bool kuPwWrapper::GetAlbums(wxString userid, kuXmlNodeArray* albums, wxString token) {
    /* GET http://picasaweb.google.com/data/feed/api/user/userID */
    wxString url = wxString::Format(wxT("http://picasaweb.google.com/data/feed/api/user/%s"), userid);
    return GetEntries(url, albums, token);
}

wxXmlNode* kuPwWrapper::AddAlbum(wxString userid, wxString token, wxString title, wxString summary, wxString location, wxString access) {
    /* POST http://picasaweb.google.com/data/feed/api/user/userID with entry */
    bool success = false;
    CURL* curl;
    CURLcode res;
    wxString data;
    wxStringOutputStream recv(&data);

    curl = curl_easy_init();
    if(curl) {
        wxString str = wxString::Format(wxT("http://picasaweb.google.com/data/feed/api/user/%s"), userid);
        curl_easy_setopt(curl, CURLOPT_URL, str.mb_str(wxConvUTF8));
        str = wxString::Format(wxT("<entry xmlns='http://www.w3.org/2005/Atom' xmlns:gphoto='http://schemas.google.com/photos/2007'><category scheme='http://schemas.google.com/g/2005#kind' term='http://schemas.google.com/photos/2007#album' /><title type='text'>%s</title><summary type='text'>%s</summary><gphoto:location>%s</gphoto:location><gphoto:access>%s</gphoto:access><gphoto:commentingEnabled>true</gphoto:commentingEnabled></entry>"),
                               title, summary, location, access);
        curl_off_t length = strlen(str.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, length);
        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, str.mb_str(wxConvUTF8));
        struct curl_slist* headerlist = NULL;
        headerlist = curl_slist_append(headerlist, "Content-Type: application/atom+xml; charset=utf-8");
        str = wxString::Format(wxT("Content-Length: %d"), length);
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        str = wxString::Format(wxT("Authorization: GoogleLogin auth=%s"), token);
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv);
        //curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");

        res = curl_easy_perform(curl);
        if(res == CURLE_OK)    success = true;
        else    ErrorHandler(res);

        curl_slist_free_all(headerlist);
        curl_easy_cleanup(curl);
    }

    wxXmlNode* node = NULL;
    if(success && data!=wxEmptyString) {
        wxXmlDocument doc;
        wxStringInputStream input(data);
        if(doc.Load(input)) {
            if(doc.GetRoot()->GetName() == wxT("entry")) {
                node = new wxXmlNode(*(doc.GetRoot()));
            }
        }
    }
    return node;
}

wxXmlNode* kuPwWrapper::ModifyAlbum(wxString token, wxXmlNode* entry) {
    /* PUT http://picasaweb.google.com/data/entry/api/user/userID/albumid/albumID/versionNumber */
    bool success = false;
    CURL* curl;
    CURLcode res;
    wxString data;
    wxStringOutputStream recv(&data);
    wxStringOutputStream output;

    wxString link = GetLink(entry, wxT("edit"));
    if(link == wxEmptyString)    return NULL;

    wxXmlDocument doc;
    doc.SetRoot(new wxXmlNode(*entry));
    doc.Save(output);
    wxString str = output.GetString().AfterFirst('>').Trim(false);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, link.mb_str(wxConvUTF8));
        curl_off_t length = strlen(str.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, length);
        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, str.mb_str(wxConvUTF8));
        struct curl_slist* headerlist = NULL;
        headerlist = curl_slist_append(headerlist, "Content-Type: application/atom+xml; charset=utf-8");
        str = wxString::Format(wxT("Content-Length: %d"), length);
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        headerlist = curl_slist_append(headerlist,"Expect:");
        str = wxString::Format(wxT("Authorization: GoogleLogin auth=%s"), token);
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv);
        //curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");

        res = curl_easy_perform(curl);
        if(res == CURLE_OK)    success = true;
        else    ErrorHandler(res);

        curl_slist_free_all(headerlist);
        curl_easy_cleanup(curl);
    }

    wxXmlNode* node = NULL;
    if(success && data!=wxEmptyString) {
        wxXmlDocument doc;
        wxStringInputStream input(data);
        if(doc.Load(input)) {
            if(doc.GetRoot()->GetName() == wxT("entry")) {
                node = new wxXmlNode(*(doc.GetRoot()));
            }
        }
    }
    return node;
}

bool kuPwWrapper::GetPhotos(wxXmlNode* album, kuXmlNodeArray* photos, wxString token) {
    /* GET http://picasaweb.google.com/data/feed/api/user/userID/albumid/albumID */
    wxString userid = GetLink(album, wxT("self")).BeforeLast('/').BeforeLast('/').AfterLast('/');
    wxString albumid = GetProperty(album, wxT("gphoto:id"));
    wxString url = wxString::Format(wxT("http://picasaweb.google.com/data/feed/api/user/%s/albumid/%s"), userid, albumid);
    return GetEntries(url, photos, token);
}

bool kuPwWrapper::AddPhoto(wxXmlNode* album, wxString filename, wxString type, wxString token) {
    /* POST http://picasaweb.google.com/data/feed/api/user/userID/albumid/albumID */
    wxFFile file(filename, wxT("rb"));
    if(!file.IsOpened())    return false;

    bool success = false;
    CURL* curl;
    CURLcode res;
    wxStringOutputStream recv;
    //wxStringOutputStream debug;

    wxString userid = GetLink(album, wxT("self")).BeforeLast('/').BeforeLast('/').AfterLast('/');
    wxString albumid = GetProperty(album, wxT("gphoto:id"));

    curl = curl_easy_init();
    if(curl) {
        wxString str = wxString::Format(wxT("http://picasaweb.google.com/data/feed/api/user/%s/albumid/%s"), userid, albumid);
        curl_easy_setopt(curl, CURLOPT_URL, str.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_INFILE, file.fp());
        curl_off_t length = file.Length();
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, length);
        struct curl_slist* headerlist = NULL;
        str = wxString::Format(wxT("Content-Type: image/%s"), type);
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        str = wxString::Format(wxT("Content-Length: %d"), length);
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        headerlist = curl_slist_append(headerlist,"Expect:");
        str = wxString::Format(wxT("Slug: %s"), wxFileName(filename).GetFullName());
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        str = wxString::Format(wxT("Authorization: GoogleLogin auth=%s"), token);
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv);
        //curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");

        res = curl_easy_perform(curl);
        if(res == CURLE_OK)    success = true;
        else    ErrorHandler(res);

        curl_slist_free_all(headerlist);
        curl_easy_cleanup(curl);
    }
    return success;
}

bool kuPwWrapper::GetMediaContent(wxXmlNode* photo, wxString dir, wxString token) {
    /* GET URL */
    wxXmlNode* group = kuPwWrapper::GetChild(photo, wxT("media:group"));
    wxString url = kuPwWrapper::GetProperty(group, wxT("media:content"), wxT("url"));
    wxFileName filename(dir, url.AfterLast('/'));
    return GetFile(url.BeforeLast('/')+wxT("/d/")+url.AfterLast('/'), filename.GetFullPath(), token);
}

wxString kuPwWrapper::GetAlbumLink(wxXmlNode* album) {
    return GetLink(album, wxT("alternate"));
}

bool kuPwWrapper::RemoveEntry(wxString token, wxXmlNode* entry) {
    /* DELETE http://picasaweb.google.com/data/entry/api/user/userID/albumid/albumID/versionNumber */
    /* DELETE http://picasaweb.google.com/data/entry/api/user/userID/albumid/albumID/photoid/photoID/versionNumber */
    bool success = false;
    CURL* curl;
    CURLcode res;
    wxStringOutputStream recv;

    wxString link = GetLink(entry, wxT("edit"));
    if(link == wxEmptyString)    return false;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, link.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        struct curl_slist* headerlist = NULL;
        wxString str = wxString::Format(wxT("Authorization: GoogleLogin auth=%s"), token);
        headerlist = curl_slist_append(headerlist, str.mb_str(wxConvUTF8));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv);
        //curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");

        res = curl_easy_perform(curl);
        if(res == CURLE_OK)    success = true;
        else    ErrorHandler(res);

        curl_slist_free_all(headerlist);
        curl_easy_cleanup(curl);
    }
    return success;
}
#endif
