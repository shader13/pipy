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

#ifndef MODULE_HPP
#define MODULE_HPP

#include "pjs/pjs.hpp"
#include "context.hpp"
#include "worker.hpp"
#include "task.hpp"

#include <map>
#include <set>

namespace pipy {

class Context;
class Configuration;
class PipelineLayout;

//
// ModuleBase
//

class ModuleBase : public pjs::RefCount<ModuleBase> {
public:
  auto index() const -> int { return m_index; }
  auto label() const -> const std::string { return m_label; }

  virtual auto new_context(Context *base = nullptr) -> Context* = 0;

  void for_each_pipeline(const std::function<void(PipelineLayout*)> &cb);
  void shutdown();

protected:
  ModuleBase(int index = 0, const std::string &label = std::string())
    : m_index(index)
    , m_label(label) {}

  virtual ~ModuleBase() {}

private:
  int m_index;
  std::string m_label;
  std::list<pjs::Ref<PipelineLayout>> m_pipelines;

  friend class pjs::RefCount<ModuleBase>;
  friend class PipelineLayout;
};

//
// Module
//

class Module : public ModuleBase {
public:
  auto worker() const -> Worker* { return m_worker; }
  auto name() const -> pjs::Str* { return m_name; }
  auto path() const -> const std::string& { return m_path; }
  auto source() const -> const std::string& { return m_source; }

  auto entrance_pipeline() -> PipelineLayout* { return m_entrance_pipeline; }
  auto find_named_pipeline(pjs::Str *name) -> PipelineLayout*;
  auto find_indexed_pipeline(int index) -> PipelineLayout*;

  virtual auto new_context(Context *base = nullptr) -> Context* override;

private:
  bool load(const std::string &path);
  void unload();
  void bind_exports();
  void bind_imports();
  void make_pipelines();
  void bind_pipelines();

  auto new_context_data(pjs::Object *prototype = nullptr) -> pjs::Object* {
    auto obj = new ContextDataBase(m_filename);
    m_context_class->init(obj, prototype);
    return obj;
  }

private:
  Module(Worker *worker, int index);
  ~Module();

  pjs::Ref<Worker> m_worker;
  pjs::Ref<pjs::Str> m_name;
  std::string m_path;
  std::string m_source;
  std::unique_ptr<pjs::Expr> m_script;
  std::unique_ptr<pjs::Expr::Imports> m_imports;
  pjs::Ref<pjs::Str> m_filename;
  pjs::Ref<Configuration> m_configuration;
  pjs::Ref<pjs::Class> m_context_class;
  std::map<pjs::Ref<pjs::Str>, PipelineLayout*> m_named_pipelines;
  std::map<int, PipelineLayout*> m_indexed_pipelines;
  PipelineLayout *m_entrance_pipeline = nullptr;

  friend class pjs::RefCount<Module>;
  friend class Configuration;
  friend class Worker;
};

} // namespace pipy

#endif // MODULE_HPP
