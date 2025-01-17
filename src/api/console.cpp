/*
 *  Copyright (c) 2019 by flomesh.io
 *
 *  Unless prior written consent has been obtained from the copyright
 *  owner, the following shall not be allowed.
 *
 *  1. The distribution of any source codes, header files, make files,
 *     or libraries of the software.
 *
 *  2. Disclosure of any source codes pertaining to the software to any
 *     additional parties.
 *
 *  3. Alteration or removal of any notices in or on the software or
 *     within the documentation included within the software.
 *
 *  ALL SOURCE CODE AS WELL AS ALL DOCUMENTATION INCLUDED WITH THIS
 *  SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION, WITHOUT WARRANTY OF ANY
 *  KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "console.hpp"
#include "data.hpp"
#include "json.hpp"
#include "log.hpp"

#include <sstream>

namespace pipy {

void Console::log(const pjs::Value *values, int count) {
  static Data::Producer s_dp("Console::log");
  Data buf;
  Data::Builder db(buf, &s_dp);
  char str[100];
  db.push(str, Log::format_header(Log::INFO, str, sizeof(str)));
  for (int i = 0; i < count; i++) {
    if (i > 0) db.push(' ');
    auto &v = values[i];
    auto s = v.to_string();
    db.push(s->str());
    s->release();
    if (v.is_object()) {
      if (auto *o = v.o()) {
        if (auto *obj = o->dump()) {
          db.push(' ');
          JSON::encode(obj, nullptr, 0, db);
        }
      }
    }
  }
  db.flush();
  Log::write(buf);
}

} // namespace pipy

namespace pjs {

using namespace pipy;

template<> void ClassDef<Console>::init() {
  ctor();

  // console.log
  method("log", [](Context &ctx, Object *, Value &result) {
    Console::log(&ctx.arg(0), ctx.argc());
  });

}

} // namespace pjs
