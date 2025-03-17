#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
#include <regex>
#include <fcntl.h>
#include <io.h>
#include <vector>

std::string Utf8ToShiftJis(const std::string& utf8Str) {
    if (utf8Str.empty()) return "";
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (wideSize == 0) return "";
    std::wstring wideStr(wideSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideStr[0], wideSize);
    int sjisSize = WideCharToMultiByte(932, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
    if (sjisSize == 0) return "";
    std::string sjisStr(sjisSize, 0);
    WideCharToMultiByte(932, 0, wideStr.c_str(), -1, &sjisStr[0], sjisSize, NULL, NULL);
    return sjisStr;
}

std::string ShiftJisToUtf8(const std::string& sjisStr) {
    if (sjisStr.empty()) return "";
    int wideSize = MultiByteToWideChar(932, 0, sjisStr.c_str(), -1, NULL, 0);
    if (wideSize == 0) return "";
    std::wstring wideStr(wideSize, 0);
    MultiByteToWideChar(932, 0, sjisStr.c_str(), -1, &wideStr[0], wideSize);
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size == 0) return "";
    std::string utf8Str(utf8Size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &utf8Str[0], utf8Size, NULL, NULL);
    return utf8Str;
}

std::string EscapeSpecialChars(const std::string& input) {
    std::string output;
    for (unsigned char c : input) {
        switch (c) {
        case '\r': output += "\\r"; break;
        case '\n': output += "\\n"; break;
        case '\t': output += "\\t"; break;
        default: output.push_back(c);
        }
    }
    return output;
}

std::string ReadFileToString(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cout << Utf8ToShiftJis("Error: Could not open file ")
            << Utf8ToShiftJis(filename) << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

std::string Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string JsonEscape(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 2);

    for (unsigned char c : input) {
        switch (c) {
        case '\\': output += "\\\\"; break;
        case '"':  output += "\\\""; break;
        case '\n': output += "\\n";  break;
        case '\r': output += "\\r";  break;
        case '\t': output += "\\t";  break;
        case '\b': output += "\\b";  break;
        case '\f': output += "\\f";  break;
        default:
            if (c < 0x20) {
                char buf[8];
                sprintf_s(buf, "\\u%04x", c);
                output += buf;
            }
            else {
                output.push_back(c);
            }
        }
    }
    return output;
}

struct ChatMessage {
    std::string role;
    std::string content;

    ChatMessage(const std::string& r, const std::string& c) : role(r), content(c) {}
};

std::string BuildJsonRequestBodyWithMessages(const std::string& model,
    const std::vector<ChatMessage>& messages) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"model\":\"" << JsonEscape(model) << "\",";
    oss << "\"messages\":[";

    for (size_t i = 0; i < messages.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"role\":\"" << messages[i].role << "\",\"content\":\""
            << JsonEscape(messages[i].content) << "\"}";
    }

    oss << "],";
    oss << "\"max_tokens\":1024,";
    oss << "\"temperature\":1.0,";
    oss << "\"top_p\":1.0,";
    oss << "\"frequency_penalty\":0.0,";
    oss << "\"presence_penalty\":0.0";
    oss << "}";
    return oss.str();
}

// 完全にシンプルな方法でJSONから content を取り出す
std::string ExtractContentFromJson(const std::string& response) {
    const char* searchStr = "\"content\":";

    // "content": を探す
    size_t pos = response.find(searchStr);
    if (pos == std::string::npos) {
        return "";
    }

    // "content": の後の位置に移動
    pos += strlen(searchStr);

    // 値の開始位置を探す (空白をスキップ)
    while (pos < response.length() && (response[pos] == ' ' || response[pos] == '\t' ||
        response[pos] == '\r' || response[pos] == '\n')) {
        pos++;
    }

    // 開始の " を確認
    if (pos >= response.length() || response[pos] != '"') {
        return "";
    }
    pos++; // " をスキップ

    std::string content;
    bool escaped = false;

    // 終了の " を探す (エスケープされていない ")
    while (pos < response.length()) {
        char c = response[pos];

        if (escaped) {
            // エスケープされた文字は、そのままエスケープシーケンスとして追加
            content += '\\';
            content += c;
            escaped = false;
        }
        else if (c == '\\') {
            // バックスラッシュが見つかったら、エスケープモードをオン
            escaped = true;
        }
        else if (c == '"') {
            // エスケープされていない " が見つかったら終了
            break;
        }
        else {
            // 通常の文字はそのまま追加
            content += c;
        }

        pos++;
    }

    return content;
}

// エスケープされた文字列をデコードする改良版
std::string JsonUnescape(const std::string& input) {
    std::string output;
    output.reserve(input.size());

    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            char next = input[i + 1];
            i += 2; // 基本的に2文字進める

            switch (next) {
            case 'n': output += '\n'; break;
            case 'r': output += '\r'; break;
            case 't': output += '\t'; break;
            case '\\': output += '\\'; break;
            case '/': output += '/'; break;
            case '"': output += '"'; break;
            case 'b': output += '\b'; break;
            case 'f': output += '\f'; break;
            case 'u':
                // \uXXXX 形式のユニコード文字
                if (i + 4 <= input.size()) {
                    // この簡易版では ? で置き換える
                    output += '?';
                    i += 4; // 4桁の16進数をスキップ
                }
                else {
                    output += "\\u"; // 不完全なユニコードシーケンス
                }
                break;
            default:
                output += next;
                break;
            }
        }
        else {
            output += input[i++];
        }
    }

    return output;
}

std::vector<ChatMessage> ReadConversationHistory(const std::string& filename, bool isVerbose) {
    std::vector<ChatMessage> history;

    std::ifstream ifs(filename);
    if (!ifs) {
        if (isVerbose) {
            std::cout << Utf8ToShiftJis("Conversation history file not found: ")
                << Utf8ToShiftJis(filename) << std::endl;
        }
        return history;
    }

    std::string line;
    std::string currentRole = "user";
    std::string currentContent = "";
    bool readingContent = false;

    while (std::getline(ifs, line)) {
        line = Trim(line);

        if (line.empty()) {
            if (!currentContent.empty()) {
                history.push_back(ChatMessage(currentRole, currentContent));
                currentContent = "";
                currentRole = (currentRole == "user") ? "assistant" : "user";
            }
            continue;
        }

        if (!currentContent.empty()) {
            currentContent += "\n";
        }
        currentContent += line;
    }

    if (!currentContent.empty()) {
        history.push_back(ChatMessage(currentRole, currentContent));
    }

    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Loaded ")
            << history.size() << Utf8ToShiftJis(" messages from conversation history.")
            << std::endl;
    }

    return history;
}

void SaveConversationHistory(const std::string& filename,
    const std::vector<ChatMessage>& history,
    bool isVerbose) {
    std::ofstream ofs(filename);
    if (!ofs) {
        if (isVerbose) {
            std::cout << Utf8ToShiftJis("Error: Could not open file for writing: ")
                << Utf8ToShiftJis(filename) << std::endl;
        }
        return;
    }

    for (const auto& message : history) {
        ofs << message.content << std::endl << std::endl;
    }

    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Saved ")
            << history.size() << Utf8ToShiftJis(" messages to conversation history.")
            << std::endl;
    }
}

std::map<std::string, std::string> ParseIniSection(const std::string& filename, const std::string& sectionName) {
    std::map<std::string, std::string> values;
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cout << Utf8ToShiftJis("Error: Could not open file ")
            << Utf8ToShiftJis(filename) << std::endl;
        return values;
    }
    std::string line;
    bool inSection = false;
    std::string sectionHeader = "[" + sectionName + "]";
    while (std::getline(ifs, line)) {
        std::string trimmed = Trim(line);
        if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') {
            continue;
        }
        if (trimmed.front() == '[' && trimmed.back() == ']') {
            inSection = (trimmed == sectionHeader);
            continue;
        }
        if (inSection) {
            size_t pos = trimmed.find('=');
            if (pos != std::string::npos) {
                std::string key = Trim(trimmed.substr(0, pos));
                std::string value = Trim(trimmed.substr(pos + 1));
                values[key] = value;
            }
        }
    }
    return values;
}

bool FileExists(const std::string& filename) {
    std::ifstream ifs(filename);
    return ifs.good();
}

int main(int argc, char* argv[]) {
    SetConsoleCP(932);
    SetConsoleOutputCP(932);
    (void)_setmode(_fileno(stdout), _O_TEXT);

    bool isVerbose = false;
    std::string userPrompt = "Enter your prompt here";
    std::string historyFile = "conversation_history.txt";
    std::string customSystemPromptFile = "";
    bool useHistory = false;
    bool resetHistory = false;
    bool useCustomSystemPrompt = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v") {
            isVerbose = true;
        }
        else if (arg == "--history" && i + 1 < argc) {
            historyFile = argv[++i];
            useHistory = true;
        }
        else if (arg == "--system" && i + 1 < argc) {
            customSystemPromptFile = argv[++i];
            useCustomSystemPrompt = true;
        }
        else if (arg == "--reset") {
            resetHistory = true;
        }
        else {
            userPrompt = ShiftJisToUtf8(arg);
        }
    }

    std::string iniFile = "AgentTalk.ini";
    std::string systemPromptFile = "SystemPrompt.ini";

    if (!FileExists(iniFile)) {
        std::cout << Utf8ToShiftJis("Error: INI file not found: ")
            << Utf8ToShiftJis(iniFile) << std::endl;
        return 1;
    }
    if (!FileExists(systemPromptFile)) {
        std::cout << Utf8ToShiftJis("Error: SystemPrompt file not found: ")
            << Utf8ToShiftJis(systemPromptFile) << std::endl;
        return 1;
    }

    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Debug: Found INI file: ")
            << Utf8ToShiftJis(iniFile) << std::endl;
        std::cout << Utf8ToShiftJis("Debug: Found SystemPrompt file: ")
            << Utf8ToShiftJis(systemPromptFile) << std::endl;
        if (useHistory) {
            std::cout << Utf8ToShiftJis("Debug: Using conversation history from: ")
                << Utf8ToShiftJis(historyFile) << std::endl;
        }
    }

    std::string iniContent = ReadFileToString(iniFile);
    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Debug: AgentTalk.ini contents (escaped):\n[")
            << Utf8ToShiftJis(EscapeSpecialChars(iniContent))
            << Utf8ToShiftJis("]\n");
    }
    auto settings = ParseIniSection(iniFile, "settings");

    std::string host = settings.count("host") ? settings["host"] : "api.openai.com";
    std::string endpoint = settings.count("endpoint") ? settings["endpoint"] : "/v1/chat/completions";
    std::string apikey = settings.count("apikey") ? settings["apikey"] : "";
    std::string model = settings.count("model") ? settings["model"] : "gpt-3.5-turbo";

    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Debug: Loaded settings:\n");
        std::cout << Utf8ToShiftJis("  host: [") << Utf8ToShiftJis(host) << Utf8ToShiftJis("]\n");
        std::cout << Utf8ToShiftJis("  endpoint: [") << Utf8ToShiftJis(endpoint) << Utf8ToShiftJis("]\n");
        std::cout << Utf8ToShiftJis("  model: [") << Utf8ToShiftJis(model) << Utf8ToShiftJis("]\n");
        std::cout << Utf8ToShiftJis("  apikey: [")
            << (apikey.empty() ? Utf8ToShiftJis("EMPTY") : Utf8ToShiftJis(apikey))
            << Utf8ToShiftJis("]\n");
    }

    std::string systemPrompt;

    if (useCustomSystemPrompt && !customSystemPromptFile.empty() && FileExists(customSystemPromptFile)) {
        systemPrompt = ReadFileToString(customSystemPromptFile);
        if (isVerbose) {
            std::cout << Utf8ToShiftJis("Debug: Using custom system prompt from: ")
                << Utf8ToShiftJis(customSystemPromptFile) << std::endl;
        }
    }
    else {
        systemPrompt = ReadFileToString(systemPromptFile);
    }

    systemPrompt = Trim(systemPrompt);
    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Debug: systemPrompt (escaped): [")
            << Utf8ToShiftJis(EscapeSpecialChars(systemPrompt))
            << Utf8ToShiftJis("]\n");
    }

    userPrompt = Trim(userPrompt);
    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Debug: userPrompt (escaped): [")
            << Utf8ToShiftJis(EscapeSpecialChars(userPrompt))
            << Utf8ToShiftJis("]\n");
    }

    std::vector<ChatMessage> messages;

    if (!systemPrompt.empty()) {
        messages.push_back(ChatMessage("system", systemPrompt));
    }

    if (useHistory && !resetHistory) {
        std::vector<ChatMessage> history = ReadConversationHistory(historyFile, isVerbose);
        messages.insert(messages.end(), history.begin(), history.end());
    }

    messages.push_back(ChatMessage("user", userPrompt));

    std::string jsonBody = BuildJsonRequestBodyWithMessages(model, messages);
    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Request JSON body (raw): ") << jsonBody << std::endl;
        std::cout << Utf8ToShiftJis("Request JSON body (escaped): ")
            << Utf8ToShiftJis(EscapeSpecialChars(jsonBody)) << std::endl;
    }

    std::string url = "https://" + host + endpoint;
    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Sending request to: [")
            << Utf8ToShiftJis(url) << Utf8ToShiftJis("]\n");
    }

    std::wstring wHost(host.begin(), host.end());
    std::wstring wPath(endpoint.begin(), endpoint.end());

    HINTERNET hSession = WinHttpOpen(L"OpenAIBridge/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        std::cout << Utf8ToShiftJis("Error: WinHttpOpen failed.") << std::endl;
        return 1;
    }
    HINTERNET hConnect = WinHttpConnect(hSession, wHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        std::cout << Utf8ToShiftJis("Error: WinHttpConnect failed.") << std::endl;
        WinHttpCloseHandle(hSession);
        return 1;
    }
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", wPath.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        std::cout << Utf8ToShiftJis("Error: WinHttpOpenRequest failed.") << std::endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    std::wstring authHeader = L"Authorization: Bearer " + std::wstring(apikey.begin(), apikey.end());
    std::wstring contentType = L"Content-Type: application/json";
    BOOL result = WinHttpAddRequestHeaders(hRequest, authHeader.c_str(), -1L, WINHTTP_ADDREQ_FLAG_ADD);
    result = result && WinHttpAddRequestHeaders(hRequest, contentType.c_str(), -1L, WINHTTP_ADDREQ_FLAG_ADD);
    if (!result) {
        std::cout << Utf8ToShiftJis("Error: WinHttpAddRequestHeaders failed.") << std::endl;
    }

    result = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        (LPVOID)jsonBody.c_str(),
        (DWORD)jsonBody.size(),
        (DWORD)jsonBody.size(),
        0);
    if (!result) {
        std::cout << Utf8ToShiftJis("Error: WinHttpSendRequest failed.") << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    result = WinHttpReceiveResponse(hRequest, NULL);
    if (!result) {
        std::cout << Utf8ToShiftJis("Error: WinHttpReceiveResponse failed.") << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    std::string response;
    DWORD dwSize = 0;
    do {
        DWORD dwDownloaded = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            std::cout << Utf8ToShiftJis("Error: WinHttpQueryDataAvailable failed.") << std::endl;
            break;
        }
        if (dwSize == 0) break;
        char* buffer = new char[dwSize + 1];
        ZeroMemory(buffer, dwSize + 1);
        if (!WinHttpReadData(hRequest, (LPVOID)buffer, dwSize, &dwDownloaded)) {
            std::cout << Utf8ToShiftJis("Error: WinHttpReadData failed.") << std::endl;
            delete[] buffer;
            break;
        }
        response.append(buffer, dwDownloaded);
        delete[] buffer;
    } while (dwSize > 0);

    if (isVerbose) {
        std::cout << Utf8ToShiftJis("Response from server: [")
            << Utf8ToShiftJis(response) << Utf8ToShiftJis("]\n");
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // ここ重要: 抽出したContent部分をデバッグ用に表示
    std::string extractedContent = ExtractContentFromJson(response);
    if (isVerbose && !extractedContent.empty()) {
        std::cout << Utf8ToShiftJis("Extracted content (before unescape): [")
            << Utf8ToShiftJis(extractedContent) << Utf8ToShiftJis("]\n");
    }

    if (!extractedContent.empty()) {
        std::string unescapedContent = JsonUnescape(extractedContent);

        if (isVerbose) {
            std::cout << Utf8ToShiftJis("Unescaped content: [")
                << Utf8ToShiftJis(unescapedContent) << Utf8ToShiftJis("]\n");
        }

        std::string sjisContent = Utf8ToShiftJis(unescapedContent);
        std::cout << sjisContent << std::endl;

        if (useHistory) {
            messages.push_back(ChatMessage("assistant", unescapedContent));
            std::vector<ChatMessage> historyToSave;
            for (size_t i = 0; i < messages.size(); i++) {
                if (messages[i].role != "system") {
                    historyToSave.push_back(messages[i]);
                }
            }
            SaveConversationHistory(historyFile, historyToSave, isVerbose);
        }
    }
    else {
        std::cout << Utf8ToShiftJis("No content found in response. Raw response preview:") << std::endl;
        int maxPreviewLength = 500;
        int previewLength = (response.length() < (size_t)maxPreviewLength) ? response.length() : maxPreviewLength;
        std::string preview = response.substr(0, previewLength);
        std::cout << Utf8ToShiftJis(preview) << std::endl;

        if (response.length() > (size_t)maxPreviewLength) {
            std::cout << Utf8ToShiftJis("... (truncated)") << std::endl;
        }
    }

    return 0;
}