#include <iostream>
using namespace std;

#define BOOST_ASIO_SEPARATE_COMPILATION 

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/extensions.hpp>

using namespace libtorrent;

struct test_plugin : libtorrent::plugin {

};

int main (int argc, char **argv) {
  session s(fingerprint("IL", 0, 1, 1, 0), std::make_pair(0xa1a1, 0xa1b1));
  s.stop_dht();
  s.start_lsd();
  s.start_natpmp();
  s.start_upnp();
  


  char a;
  cin >> a;
  
  //  add_torrent_params

}
