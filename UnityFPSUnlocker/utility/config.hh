#ifndef CONFIG_HEADER
#define CONFIG_HEADER

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <absl/status/status.h>
#include <absl/status/statusor.h>

namespace Utility {
    absl::StatusOr<rapidjson::Document> LoadJsonFromFile(const char*);
}; // namespace Utility

#endif // config.hh