//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// copyright            : (C) 2014 Eran Ifrah
// file name            : json_node.cpp
//
// -------------------------------------------------------------------------
// A
//              _____           _      _     _ _
//             /  __ \         | |    | |   (_) |
//             | /  \/ ___   __| | ___| |    _| |_ ___
//             | |    / _ \ / _  |/ _ \ |   | | __/ _ )
//             | \__/\ (_) | (_| |  __/ |___| | ||  __/
//              \____/\___/ \__,_|\___\_____/_|\__\___|
//
//                                                  F i l e
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "JSON.h"

#include "StringUtils.h"
#include "clFontHelper.h"
#include "fileutils.h"

#include <stdlib.h>
#include <wx/dynarray.h>
#include <wx/ffile.h>
#include <wx/filename.h>

JSON::JSON(const wxString& text)
    : m_json(NULL)
{
    m_json = cJSON_Parse(text.mb_str(wxConvUTF8).data());
}

JSON::JSON(cJSON* json)
    : m_json(json)
{
}

JSON::JSON(JSONItem item)
    : m_json(item.release())
{
}

JSON::JSON(int type)
    : m_json(NULL)
{
    if (type == cJSON_Array)
        m_json = cJSON_CreateArray();
    else if (type == cJSON_NULL)
        m_json = cJSON_CreateNull();
    else
        m_json = cJSON_CreateObject();
}

JSON::JSON(const wxFileName& filename)
    : m_json(NULL)
{
    wxString content;
    if (!FileUtils::ReadFileContent(filename, content)) {
        return;
    }
    m_json = cJSON_Parse(content.mb_str(wxConvUTF8).data());
}

JSON::~JSON()
{
    if (m_json) {
        cJSON_Delete(m_json);
        m_json = NULL;
    }
}

void JSON::save(const wxFileName& fn) const
{
    if (!isOk()) {
        FileUtils::WriteFileContent(fn, "{}");
    } else {
        FileUtils::WriteFileContent(fn, toElement().format(), wxConvUTF8);
    }
}

JSONItem JSON::toElement() const
{
    if (!m_json) {
        return JSONItem(NULL);
    }
    return JSONItem(m_json);
}

wxString JSON::errorString() const { return _errorString; }

JSONItem JSONItem::namedObject(const wxString& name) const
{
    if (!m_json) {
        return JSONItem(NULL);
    }

    cJSON* obj = cJSON_GetObjectItem(m_json, name.mb_str(wxConvUTF8).data());
    if (!obj) {
        return JSONItem(NULL);
    }
    return JSONItem(obj);
}

void JSON::clear()
{
    int type = cJSON_Object;
    if (m_json) {
        type = m_json->type;
        cJSON_Delete(m_json);
        m_json = NULL;
    }
    if (type == cJSON_Array)
        m_json = cJSON_CreateArray();
    else
        m_json = cJSON_CreateObject();
}

cJSON* JSON::release()
{
    cJSON* p = m_json;
    m_json = NULL;
    return p;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
JSONItem::JSONItem(cJSON* json)
    : m_json(json)
{
    if (m_json) {
        m_propertyName = m_json->string ? m_json->string : "";
        m_type = m_json->type;
    }
}

JSONItem::JSONItem(const wxString& name, double val)
    : m_propertyName(name)
    , m_type(cJSON_Number)
    , m_valueNumer(val)
{
}

JSONItem::JSONItem(const wxString& name, const std::string& val)
    : m_propertyName(name)
    , m_type(cJSON_String)
    , m_valueString(val)
{
}

JSONItem::JSONItem(const wxString& name, const char* pval, size_t len)
    : m_propertyName(name)
    , m_type(cJSON_String)
    , m_valueString(pval, len)
{
}

JSONItem::JSONItem(const wxString& name, bool val)
    : m_propertyName(name)
    , m_type(val ? cJSON_True : cJSON_False)
{
}

JSONItem JSONItem::operator[](int index) const
{
    if (isArray()) {
        return arrayItem(index);
    }
    return JSONItem(NULL);
}

std::unordered_map<std::string_view, JSONItem> JSONItem::GetAsMap() const
{
    if (!m_json) {
        return {};
    }

    std::unordered_map<std::string_view, JSONItem> res;
    cJSON* c = m_json->child;
    while (c) {
        res.erase(c->string);
        res.insert({ c->string, JSONItem{ c } });
        c = c->next;
    }
    return res;
}

std::vector<JSONItem> JSONItem::GetAsVector() const
{
    if (!m_json || !isArray()) {
        return {};
    }

    std::vector<JSONItem> res;
    res.reserve(arraySize());
    cJSON* c = m_json->child;
    while (c) {
        res.emplace_back(JSONItem{ c });
        c = c->next;
    }
    return res;
}

JSONItem JSONItem::operator[](const wxString& name) const { return namedObject(name); }

JSONItem JSONItem::arrayItem(int pos) const
{
    if (!m_json) {
        return JSONItem(NULL);
    }

    if (m_json->type != cJSON_Array)
        return JSONItem(NULL);

    int size = cJSON_GetArraySize(m_json);
    if (pos >= size)
        return JSONItem(NULL);

    return JSONItem(cJSON_GetArrayItem(m_json, pos));
}

bool JSONItem::isNull() const
{
    if (!m_json) {
        return false;
    }
    return m_json->type == cJSON_NULL;
}

bool JSONItem::toBool(bool defaultValue) const
{
    if (!m_json) {
        return defaultValue;
    }

    if (!isBool()) {
        return defaultValue;
    }

    return m_json->type == cJSON_True;
}

wxString JSONItem::toString(const wxString& defaultValue) const
{
    if (!m_json) {
        return defaultValue;
    }

    if (m_json->type != cJSON_String) {
        return defaultValue;
    }

    return wxString(m_json->valuestring, wxConvUTF8);
}

bool JSONItem::isBool() const
{
    if (!m_json) {
        return false;
    }

    return m_json->type == cJSON_True || m_json->type == cJSON_False;
}

bool JSONItem::isString() const
{
    if (!m_json) {
        return false;
    }

    return m_json->type == cJSON_String;
}

void JSONItem::append(const JSONItem& element)
{
    if (!m_json) {
        return;
    }

    switch (element.getType()) {
    case cJSON_False:
        cJSON_AddFalseToObject(m_json, element.GetPropertyName().mb_str(wxConvUTF8).data());
        break;

    case cJSON_True:
        cJSON_AddTrueToObject(m_json, element.GetPropertyName().mb_str(wxConvUTF8).data());
        break;

    case cJSON_NULL:
        cJSON_AddNullToObject(m_json, element.GetPropertyName().mb_str(wxConvUTF8).data());
        break;

    case cJSON_Number:
        cJSON_AddNumberToObject(m_json, element.GetPropertyName().mb_str(wxConvUTF8).data(), element.m_valueNumer);
        break;

    case cJSON_String:
        cJSON_AddStringToObject(m_json, element.GetPropertyName().mb_str(wxConvUTF8).data(),
                                element.m_valueString.mb_str(wxConvUTF8).data());
        break;

    case cJSON_Array:
    case cJSON_Object:
        cJSON_AddItemToObject(m_json, element.GetPropertyName().mb_str(wxConvUTF8).data(), element.m_json);
        break;
    }
}

void JSONItem::arrayAppend(const char* value)
{
    if (!m_json) {
        return;
    }
    cJSON* p = cJSON_CreateString(value);
    cJSON_AddItemToArray(m_json, p);
}

void JSONItem::arrayAppend(const std::string& value) { arrayAppend((const char*)value.c_str()); }

void JSONItem::arrayAppend(double number)
{
    if (!m_json) {
        return;
    }
    cJSON* p = cJSON_CreateNumber(number);
    cJSON_AddItemToArray(m_json, p);
}

void JSONItem::arrayAppend(int number) { arrayAppend((double)number); }

void JSONItem::arrayAppend(const wxString& value) { arrayAppend((const char*)value.mb_str(wxConvUTF8).data()); }

void JSONItem::arrayAppend(const JSONItem& element)
{
    if (!m_json) {
        return;
    }

    cJSON* p = NULL;
    switch (element.getType()) {
    case cJSON_False:
        p = cJSON_CreateFalse();
        break;

    case cJSON_True:
        p = cJSON_CreateTrue();
        break;

    case cJSON_NULL:
        p = cJSON_CreateNull();
        break;

    case cJSON_Number:
        p = cJSON_CreateNumber(element.m_valueNumer);
        break;

    case cJSON_String:
        p = cJSON_CreateString(element.m_valueString.mb_str(wxConvUTF8).data());
        break;
    case cJSON_Array:
    case cJSON_Object:
        p = element.m_json;
        break;
    }
    if (p) {
        cJSON_AddItemToArray(m_json, p);
    }
}

JSONItem JSONItem::createArray(const wxString& name)
{
    JSONItem arr(cJSON_CreateArray());
    arr.SetPropertyName(name);
    arr.setType(cJSON_Array);
    return arr;
}

JSONItem JSONItem::createObject(const wxString& name)
{
    JSONItem obj(cJSON_CreateObject());
    obj.SetPropertyName(name);
    obj.setType(cJSON_Object);
    return obj;
}

char* JSONItem::FormatRawString(bool formatted) const
{
    if (!m_json) {
        return NULL;
    }

    if (formatted) {
        return cJSON_Print(m_json);

    } else {
        return cJSON_PrintUnformatted(m_json);
    }
}

wxString JSONItem::format(bool formatted) const
{
    if (!m_json) {
        return wxT("");
    }

    char* p = formatted ? cJSON_Print(m_json) : cJSON_PrintUnformatted(m_json);
    wxString s(p, wxConvUTF8);
    free(p);
    return s;
}

int JSONItem::toInt(int defaultVal) const
{
    if (!m_json) {
        return defaultVal;
    }

    if (m_json->type != cJSON_Number) {
        return defaultVal;
    }

    return m_json->valueint;
}

size_t JSONItem::toSize_t(size_t defaultVal) const
{
    if (!m_json) {
        return defaultVal;
    }

    if (m_json->type != cJSON_Number) {
        return defaultVal;
    }

    return (size_t)m_json->valueint;
}

double JSONItem::toDouble(double defaultVal) const
{
    if (!m_json) {
        return defaultVal;
    }

    if (m_json->type != cJSON_Number) {
        return defaultVal;
    }

    return m_json->valuedouble;
}

int JSONItem::arraySize() const
{
    if (!m_json) {
        return 0;
    }

    if (m_json->type != cJSON_Array)
        return 0;

    return cJSON_GetArraySize(m_json);
}

JSONItem& JSONItem::addProperty(const wxString& name, bool value)
{
    append(JSONItem(name, value));
    return *this;
}

JSONItem& JSONItem::addProperty(const wxString& name, const wxString& value)
{
    const wxCharBuffer cb = value.mb_str(wxConvUTF8);
    append(JSONItem(name, cb.data(), cb.length()));
    return *this;
}

JSONItem& JSONItem::addProperty(const wxString& name, const std::string& value)
{
    append(JSONItem(name, value));
    return *this;
}

JSONItem& JSONItem::addProperty(const wxString& name, const wxChar* value)
{
    return addProperty(name, wxString(value));
}

JSONItem& JSONItem::addProperty(const wxString& name, long value)
{
    append(JSONItem(name, (double)value));
    return *this;
}

JSONItem& JSONItem::addProperty(const wxString& name, const wxArrayString& arr)
{
    JSONItem arrEle = JSONItem::createArray(name);
    for (size_t i = 0; i < arr.size(); i++) {
        arrEle.arrayAppend(arr.Item(i));
    }
    append(arrEle);
    return *this;
}

wxArrayString JSONItem::toArrayString(const wxArrayString& defaultValue) const
{
    if (!m_json) {
        return defaultValue;
    }

    if (m_json->type != cJSON_Array) {
        return defaultValue;
    }

    int arr_size = arraySize();
    if (arr_size == 0) {
        return defaultValue;
    }

    wxArrayString arr;
    arr.reserve(arr_size);
    auto child = m_json->child;
    while (child) {
        arr.push_back(wxString(child->valuestring, wxConvUTF8));
        child = child->next;
    }
    return arr;
}

bool JSONItem::hasNamedObject(const wxString& name) const
{
    if (!m_json) {
        return false;
    }

    cJSON* obj = cJSON_GetObjectItem(m_json, name.mb_str(wxConvUTF8).data());
    return obj != NULL;
}
#if wxUSE_GUI
JSONItem& JSONItem::addProperty(const wxString& name, const wxPoint& pt)
{
    wxString szStr;
    szStr << pt.x << "," << pt.y;
    return addProperty(name, szStr);
}

JSONItem& JSONItem::addProperty(const wxString& name, const wxSize& sz)
{
    wxString szStr;
    szStr << sz.x << "," << sz.y;
    return addProperty(name, szStr);
}

wxPoint JSONItem::toPoint() const
{
    if (!m_json) {
        return wxDefaultPosition;
    }

    if (m_json->type != cJSON_String) {
        return wxDefaultPosition;
    }

    wxString str = m_json->valuestring;
    wxString x = str.BeforeFirst(',');
    wxString y = str.AfterFirst(',');

    long nX(-1), nY(-1);
    if (!x.ToLong(&nX) || !y.ToLong(&nY))
        return wxDefaultPosition;

    return wxPoint(nX, nY);
}

wxColour JSONItem::toColour(const wxColour& defaultColour) const
{
    if (!m_json) {
        return defaultColour;
    }

    if (m_json->type != cJSON_String) {
        return defaultColour;
    }
    return wxColour(m_json->valuestring);
}

wxSize JSONItem::toSize() const
{
    wxPoint pt = toPoint();
    return wxSize(pt.x, pt.y);
}
#endif

JSONItem& JSONItem::addProperty(const wxString& name, const JSONItem& element)
{
    if (!m_json) {
        return *this;
    }
    cJSON_AddItemToObject(m_json, name.mb_str(wxConvUTF8).data(), element.m_json);
    return *this;
}

void JSONItem::removeProperty(const wxString& name)
{
    // delete child property
    if (!m_json) {
        return;
    }
    cJSON_DeleteItemFromObject(m_json, name.mb_str(wxConvUTF8).data());
}
#if wxUSE_GUI
JSONItem& JSONItem::addProperty(const wxString& name, const wxStringMap_t& stringMap)
{
    if (!m_json)
        return *this;

    JSONItem arr = JSONItem::createArray(name);
    for (const auto& [key, value] : stringMap) {
        JSONItem obj = JSONItem::createObject();
        obj.addProperty("key", key);
        obj.addProperty("value", value);
        arr.arrayAppend(obj);
    }
    append(arr);
    return *this;
}
#endif
wxStringMap_t JSONItem::toStringMap() const
{
    wxStringMap_t res;
    if (!m_json) {
        return res;
    }

    if (m_json->type != cJSON_Array) {
        return res;
    }

    for (int i = 0; i < arraySize(); ++i) {
        wxString key = arrayItem(i).namedObject("key").toString();
        wxString val = arrayItem(i).namedObject("value").toString();
        res.insert(std::make_pair(key, val));
    }
    return res;
}
JSONItem& JSONItem::addProperty(const wxString& name, size_t value) { return addProperty(name, (int)value); }

#if wxUSE_GUI
JSONItem& JSONItem::addProperty(const wxString& name, const wxColour& colour)
{
    wxString colourValue;
    if (colour.IsOk()) {
        colourValue = colour.GetAsString(wxC2S_HTML_SYNTAX);
    }
    return addProperty(name, colourValue);
}
#endif
JSONItem JSONItem::firstChild()
{
    m_walker = NULL;
    if (!m_json) {
        return JSONItem(NULL);
    }

    if (!m_json->child) {
        return JSONItem(NULL);
    }

    m_walker = m_json->child;
    return JSONItem(m_walker);
}

JSONItem JSONItem::nextChild()
{
    if (!m_walker) {
        return JSONItem(NULL);
    }

    JSONItem element(m_walker->next);
    m_walker = m_walker->next;
    return element;
}
JSONItem& JSONItem::addProperty(const wxString& name, const char* value, const wxMBConv& conv)
{
    return addProperty(name, wxString(value, conv));
}

#if wxUSE_GUI
JSONItem& JSONItem::addProperty(const wxString& name, const wxFont& font)
{
    return addProperty(name, clFontHelper::ToString(font));
}

wxFont JSONItem::toFont(const wxFont& defaultFont) const
{
    wxString str = toString();
    if (str.IsEmpty())
        return defaultFont;
    wxFont f = clFontHelper::FromString(str);
    return f;
}
#endif

bool JSONItem::isArray() const
{
    if (!m_json) {
        return false;
    }
    return m_json->type == cJSON_Array;
}

bool JSONItem::isObject() const
{
    if (!m_json) {
        return false;
    }
    return m_json->type == cJSON_Object;
}

bool JSONItem::isNumber() const
{
    if (!m_json) {
        return false;
    }
    return m_json->type == cJSON_Number;
}

JSONItem JSONItem::detachProperty(const wxString& name)
{
    if (!m_json) {
        return JSONItem(NULL);
    }
    cJSON* j = cJSON_DetachItemFromObject(m_json, name.c_str());
    return JSONItem(j);
}

wxFileName JSONItem::toFileName() const
{
    if (!m_json) {
        return wxFileName();
    }
    return wxFileName(m_valueString);
}

JSONItem& JSONItem::addProperty(const wxString& name, const wxFileName& filename)
{
    return addProperty(name, filename.GetFullPath());
}

JSONItem JSONItem::AddArray(const wxString& name)
{
    JSONItem json = createArray(name);
    append(json);
    return json;
}

JSONItem JSONItem::AddObject(const wxString& name)
{
    JSONItem json = createObject(name);
    append(json);
    return json;
}

JSONItem& JSONItem::addProperty(const wxString& name, cJSON* pjson)
{
    if (!m_json) {
        return *this;
    }
    cJSON_AddItemToObject(m_json, name.mb_str(wxConvUTF8).data(), pjson);
    return *this;
}

std::vector<double> JSONItem::toDoubleArray(const std::vector<double>& defaultValue) const
{
    if (!m_json) {
        return defaultValue;
    }

    if (m_json->type != cJSON_Array) {
        return defaultValue;
    }

    int arr_size = arraySize();
    if (arr_size == 0) {
        return defaultValue;
    }

    std::vector<double> arr;
    arr.reserve(arr_size);
    auto child = m_json->child;
    while (child) {
        arr.push_back(child->valuedouble);
        child = child->next;
    }
    return arr;
}

std::vector<int> JSONItem::toIntArray(const std::vector<int>& defaultValue) const
{
    if (!m_json) {
        return defaultValue;
    }

    if (m_json->type != cJSON_Array) {
        return defaultValue;
    }

    int arr_size = arraySize();
    if (arr_size == 0) {
        return defaultValue;
    }

    std::vector<int> arr;
    arr.reserve(arr_size);
    auto child = m_json->child;
    while (child) {
        arr.push_back(child->valueint);
        child = child->next;
    }
    return arr;
}

JSONItem& JSONItem::addProperty(const wxString& name, const std::vector<int>& arr_int)
{
    if (!m_json) {
        return *this;
    }

    if (m_type != cJSON_Object) {
        return *this;
    }

    // create array
    JSONItem arr = AddArray(name);
    for (size_t i = 0; i < arr_int.size(); ++i) {
        cJSON_AddItemToArray(arr.m_json, cJSON_CreateNumber(arr_int[i]));
    }
    return *this;
}
