#include <iostream>
using namespace std;
#include <unistd.h>

#define BOOST_ASIO_SEPARATE_COMPILATION
//#define TORRENT_DEBUG 1
//#define TORRENT_USE_IOSTREAM 1


#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/extensions.hpp>

//#include <libtorrent/policy.hpp>
#include <libtorrent/peer_connection.hpp>
//#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
//#include <libtorrent/extensions/metadata_transfer.hpp>
//#include <libtorrent/socket.hpp>
#include <libtorrent/peer_info.hpp>


#include "libtorrent/bt_peer_connection.hpp"

#define msg_id 0x99

using namespace libtorrent;


struct talk_peer_plugun : libtorrent::peer_plugin {
	peer_connection *peer;

	virtual ~talk_peer_plugun () {
		cout << "peer plugin deleted\n";
	}
	virtual void add_handshake (entry &e) {
		//e.list().push_back(entry(std::string("derp")));
		entry &message = e["m"];
		message["LT_DERP"] = msg_id;
		//cout << "type: " << e.type() << endl;
	}
	virtual bool on_extension_handshake (const lazy_entry &e) {
		if(e.type() != lazy_entry::dict_t) {
			cout << "not dict\n";
			return false;
		}
		lazy_entry const *message = e.dict_find("m");
		if(!message || message->type() != lazy_entry::dict_t) {
			cout << "2nd not dict\n";
			return false;
		}
		int index = message->dict_find_int_value("LT_DERP", -1);
		if(index == -1) {
			cout << "index fail\n";
			peer->disconnect(errors::invalid_not_interested);
			return false;
		}
		cout << "passed handshake check\n";
		peer->send_interested();
		return true;
	}
	virtual bool on_unknown_message(int length, int msg, buffer::const_interval body) {
		cout << "unknown " << length << endl;
	}


	virtual bool on_extended(int length, int msg, buffer::const_interval body) {
		if(msg != msg_id) return false;
		detail::read_uint32(body.begin);
		cout << "recieved int: " << detail::read_uint32(body.begin) << endl;
		return true;
	}


	virtual void tick() {
		char buf[14];
		char *ptr = buf;

		detail::write_uint32(sizeof(buf), ptr);
		detail::write_uint8(bt_peer_connection::msg_extended, ptr);
		detail::write_uint8(msg_id, ptr);
		detail::write_uint32(0, ptr);
		detail::write_uint32(123123123, ptr);

		//char *buf = "testing123";
		peer->send_buffer(buf, sizeof(buf));
		cout << "sending\n";
		peer->keep_alive();
	}
};

struct talk_torrent_plugin : libtorrent::torrent_plugin {
	virtual ~talk_torrent_plugin () {}
	virtual boost::shared_ptr<peer_plugin> new_connection(peer_connection *con) {
		cout << "new connection \n";
		peer_info ss;
		// this causes a seg fault some times, it appears to happen in the download_queue_time or something
		//con->get_peer_info(ss); //peer_info_struct()->remote();
		//cout << ss.ip.address().to_string() << endl;
		auto ret = new talk_peer_plugun;
		ret->peer = con;
		return boost::shared_ptr<peer_plugin>(ret);
	}
};

struct talk_plugin : libtorrent::plugin {
	virtual ~talk_plugin () {}
	virtual boost::shared_ptr<torrent_plugin> new_torrent(torrent *t, void *user) {
		cout << "new torrent\n";
		return boost::shared_ptr<torrent_plugin>(new talk_torrent_plugin);
	}
};

int main (int argc, char **argv) {
	session s(fingerprint("IL", 0, 1, 1, 0), std::make_pair(0xa1a1, 0xa1b7));
	session_settings settings;// = s.settings();
	settings.user_agent = "i test/0.0.1";
	settings.ssl_listen = 0; // still problem with upnp opening up port 4433
	s.set_settings(settings);
	s.add_extension(&libtorrent::create_ut_pex_plugin);
	//s.add_extension(&libtorrent::create_metadata_plugin);
	boost::shared_ptr<plugin> test_plug(new talk_plugin);
	s.add_extension(test_plug);

	pe_settings con_settings;
	con_settings.out_enc_policy = pe_settings::forced;
	con_settings.in_enc_policy = pe_settings::forced;
	con_settings.allowed_enc_level = pe_settings::rc4;
	s.set_pe_settings(con_settings);

	s.stop_dht();
	s.start_lsd();
	s.start_natpmp();
	s.start_upnp();

	add_torrent_params tor_params;
	tor_params.trackers.push_back("udp://tracker.publicbt.com:80");
	//tor_params.trackers.push_back("udp://tracker.openbittorrent.com:80");
	tor_params.info_hash = sha1_hash();
	*(tor_params.info_hash.begin()) = 0xa1;
	*(tor_params.info_hash.begin()+1) = 0xa1;
	*(tor_params.info_hash.begin()+2) = rrr % 256;
	*(tor_params.info_hash.begin()+3) = ( rrr /256) % 256 ;
	cout << "Random: " << rrr << endl ;
	tor_params.save_path = "/tmp/torrent-test";

	torrent_handle tor = s.add_torrent(tor_params);
	tor.set_max_connections(1000);

	while(1) {
		torrent_status stat = tor.status();
		cout << "loop " << tor.is_valid() << " prog:" << stat.progress << " trac:" << stat.current_tracker << " peers:" << stat.num_peers << " con: " << stat.num_connections << " seed:" << stat.num_seeds << " name:" << tor.name() << endl;



		sleep(1000); // 1sec
	}

	//	add_torrent_params

}
