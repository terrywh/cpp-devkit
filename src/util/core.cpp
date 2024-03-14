#include "core.h"

namespace util {

core::core() {
    ins_ = this;
}
core::~core() {
    ins_ = nullptr;
}
core* core::ins_ = nullptr;

} // namespace util
