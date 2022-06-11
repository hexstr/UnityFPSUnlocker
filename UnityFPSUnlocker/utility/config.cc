#include "config.hh"

#include "logger.hh"

using absl::StatusOr;
using namespace rapidjson;

namespace Utility {
    StatusOr<Document> LoadJsonFromFile(const char* FilePath) {
        Document doc;
        std::FILE* fp = std::fopen(FilePath, "rb");
        if (fp) {
            std::fseek(fp, 0, SEEK_END);
            size_t len = std::ftell(fp);
            char* buf = (char*)std::malloc(len + 1);
            buf[len] = 0;
            if (buf == nullptr) {
                std::fclose(fp);
                return absl::InternalError("Could not allocate memory");
            }
            std::rewind(fp);
            size_t readb = std::fread(buf, 1, len, fp);
            std::fclose(fp);
            if (readb != len) {
                std::free(buf);
                return absl::InternalError("Could not read the data");
            }
            if (doc.Parse(buf).HasParseError()) {
                logger("[%s] %s #%d\nFile: %s %s %zu %s", __FUNCTION__, __FILE__, __LINE__,
                       FilePath, GetParseError_En(doc.GetParseError()), doc.GetErrorOffset(), buf);
                return absl::InternalError("Parse error");
            }
        } else {
            logger("[%s] %s #%d\nCould not read file: %s Cause: %s.", __FUNCTION__, __FILE__, __LINE__, FilePath, strerror(errno));
            return absl::InternalError("Could not read file.");
        }
        return doc;
    }
} // namespace Utility