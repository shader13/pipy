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

#ifndef LISTENER_HPP
#define LISTENER_HPP

#include "net.hpp"
#include "inbound.hpp"
#include "options.hpp"

#include <functional>
#include <string>
#include <list>
#include <map>

namespace pipy {

class PipelineLayout;

//
// Listener
//

class Listener {
public:
  enum class Protocol {
    TCP,
    UDP,
  };

  struct Options : public Inbound::Options, public pipy::Options {
    Protocol protocol = Protocol::TCP;
    size_t max_packet_size = 16 * 1024;
    int max_connections = -1;
    bool reserved = false;

    Options() {}
    Options(pjs::Object *options);
  };

  static void set_reuse_port(bool reuse);

  static auto get(const std::string &ip, int port, Protocol protocol) -> Listener* {
    if (auto *l = find(port, protocol)) return l;
    return new Listener(protocol, ip, port);
  }

  static bool is_open(int port, Protocol protocol) {
    if (auto *l = find(port, protocol)) return l->is_open();
    return false;
  }

  static void for_each(const std::function<void(Listener*)> &cb) {
    for (const auto &p : s_listeners) {
      for (const auto &q : p.second) {
        cb(q.second);
      }
    }
  }

  auto ip() const -> const std::string& { return m_ip; }
  auto port() const -> int { return m_port; }
  auto protocol() const -> Protocol { return m_protocol; }
  bool is_open() const { return m_pipeline_layout; }
  bool reserved() const { return m_options.reserved; }
  auto pipeline_layout() const -> PipelineLayout* { return m_pipeline_layout; }
  void pipeline_layout(PipelineLayout *layout);
  auto peak_connections() const -> int { return m_peak_connections; }

  void set_options(const Options &options);
  void for_each_inbound(const std::function<void(Inbound*)> &cb);

private:
  Listener(Protocol protocol, const std::string &ip, int port);
  ~Listener();

  //
  // Listener::Acceptor
  //

  class Acceptor : public pjs::RefCount<Acceptor> {
  public:
    virtual auto count() -> size_t const = 0;
    virtual void accept() = 0;
    virtual void cancel() = 0;
    virtual void open(Inbound *inbound) = 0;
    virtual void close(Inbound *inbound) = 0;
    virtual void close() = 0;
    virtual void for_each_inbound(const std::function<void(Inbound*)> &cb) = 0;
  protected:
    virtual ~Acceptor() {}
    friend class pjs::RefCount<Acceptor>;
  };

  //
  // Listener::AcceptorTCP
  //

  class AcceptorTCP : public Acceptor {
  public:
    AcceptorTCP(Listener *listener);
    virtual ~AcceptorTCP();

    bool start(const asio::ip::tcp::endpoint &endpoint);

    virtual auto count() -> size_t const override;
    virtual void accept() override;
    virtual void cancel() override;
    virtual void open(Inbound *inbound) override;
    virtual void close(Inbound *inbound) override;
    virtual void close() override;
    virtual void for_each_inbound(const std::function<void(Inbound*)> &cb) override;

  private:
    Listener* m_listener;
    asio::ip::tcp::acceptor m_acceptor;
    List<InboundTCP> m_inbounds;
  };

  //
  // Listener::AcceptorUDP
  //

  class AcceptorUDP : public Acceptor {
  public:
    AcceptorUDP(Listener *listener);
    virtual ~AcceptorUDP();

    void start(const asio::ip::udp::endpoint &endpoint);
    auto inbound(const asio::ip::udp::endpoint &peer, bool create = false) -> InboundUDP*;

    virtual auto count() -> size_t const override;
    virtual void accept() override;
    virtual void cancel() override;
    virtual void open(Inbound *inbound) override;
    virtual void close(Inbound *inbound) override;
    virtual void close() override;
    virtual void for_each_inbound(const std::function<void(Inbound*)> &cb) override;

  private:
    Listener* m_listener;
    List<InboundUDP> m_inbounds;
    asio::ip::udp::socket m_socket;
    asio::ip::udp::endpoint m_peer;
    std::map<asio::ip::udp::endpoint, InboundUDP*> m_inbound_map;
    bool m_paused = false;

    void receive();
  };

  void start();
  void pause();
  void resume();
  void open(Inbound *inbound);
  void close(Inbound *inbound);
  void close();
  void describe(char *buf, size_t len);
  void set_sock_opts(int sock);

  Options m_options;
  Protocol m_protocol;
  std::string m_ip;
  int m_port;
  int m_peak_connections = 0;
  bool m_paused = false;
  asio::ip::address m_address;
  pjs::Ref<Acceptor> m_acceptor;
  pjs::Ref<PipelineLayout> m_pipeline_layout;

  // static std::list<Listener*> s_all_listeners;
  static std::map<Protocol, std::map<int, Listener*>> s_listeners;
  static bool s_reuse_port;

  static auto find(int port, Protocol protocol) -> Listener*;

  friend class InboundTCP;
  friend class InboundUDP;
  friend class pjs::RefCount<Listener>;
};

} // namespace pipy

#endif // LISTENER_HPP
