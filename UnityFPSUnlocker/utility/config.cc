#include "config.hh"

#include <fstream>
#include <string>

#include "logger.hh"

using absl::StatusOr;
using namespace rapidjson;

namespace Utility {
    StatusOr<Document> LoadJsonFromFile(const char* FilePath) {
        Document doc;
        std::ifstream file(FilePath);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                (std::istreambuf_iterator<char>()));
            if (doc.Parse(content.c_str()).HasParseError()) {
                if (content.size() > 2) {
                    content[doc.GetErrorOffset() - 2] = '-';
                    content[doc.GetErrorOffset() - 1] = '>';
                }
                LOG("[%s] %s #%d\nFile: %s %s %zu %s", __FUNCTION__, __FILE__, __LINE__,
                       FilePath, GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), content.c_str());
                return absl::InternalError("Parse error");
            }
        } else {
            LOG("[%s] %s #%d\nCould not read file: %s.", __FUNCTION__, __FILE__, __LINE__, FilePath);
            return absl::InternalError("Could not read file.");
        }
        return doc;
    }
} // namespace Utility