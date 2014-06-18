//
// Copyright 2011-2013 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <uhd/utils/thread_priority.hpp>
#include <uhd/convert.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <complex>
#include <cstdlib>

namespace po = boost::program_options;

/***********************************************************************
 * Test result variables
 **********************************************************************/
unsigned long long num_overflows = 0;
unsigned long long num_underflows = 0;
unsigned long long num_rx_samps = 0;
unsigned long long num_tx_samps = 0;
unsigned long long num_dropped_samps = 0;
unsigned long long num_seq_errors = 0;

/***********************************************************************
 * Benchmark RX Rate
 **********************************************************************/
void benchmark_rx_rate(uhd::usrp::multi_usrp::sptr usrp, const std::string &rx_cpu, uhd::rx_streamer::sptr rx_stream, std::string ce_select){
    uhd::set_thread_priority_safe();

    std::cout << "Spawning rx thread..." << std::endl;

    boost::uint32_t sid = rx_stream->get_sid(0);
    boost::uint32_t host_address = (sid >> 16) & 0xFFFF;
    boost::uint32_t ce_address = sid & 0xFFFF;

    std::cout << str(boost::format("RX streamer: SID == 0x%08x") % sid) << std::endl;
    std::cout << str(boost::format("RX streamer: From 0x%04x to 0x%04x") % ce_address % host_address) << std::endl;

    std::cout << "Configuring block..." << std::endl;
    std::string ce = "ce" + ce_select;
    usrp->get_device3()->rfnoc_cmd(
            ce, "poke",
            8, // Register 8: Set SID
	    (1<<16) /* use SID */ | host_address
    );
    usrp->get_device3()->rfnoc_cmd(
            ce, "set_fc",
            20000, // Host has a large buffer
            2 // How often we report FC (every Nth packet)
    );

    //setup variables and allocate buffer
    uhd::rx_metadata_t md;
    //const size_t max_samps_per_packet = rx_stream->get_max_num_samps();
    const size_t max_samps_per_packet = 100;
    std::vector<char> buff(max_samps_per_packet*uhd::convert::get_bytes_per_item(rx_cpu));
    std::vector<void *> buffs;
    for (size_t ch = 0; ch < rx_stream->get_num_channels(); ch++)
        buffs.push_back(&buff.front()); //same buffer for each channel
    bool had_an_overflow = false;
    uhd::time_spec_t last_time;

    uhd::stream_cmd_t cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    //rx_stream->issue_stream_cmd(cmd);

    std::cout << "starting rx loop" << std::endl;

    int packet_count = 0;

    while (not boost::this_thread::interruption_requested()){
        try {
          num_rx_samps += rx_stream->recv(buffs, max_samps_per_packet, md, true);
	  if (num_rx_samps) {
              packet_count += num_rx_samps/100;
		  std::cout << "calced num packets ==" << num_rx_samps/100 << std::endl;
          }
        }
        catch (...) {
          /* apparently, the boost thread interruption can sometimes result in
             throwing exceptions not of type boost::exception, this catch allows
             this thread to still attempt to issue the STREAM_MODE_STOP_CONTINUOUS
          */
          break;
        }

        //handle the error codes
        switch(md.error_code){
        case uhd::rx_metadata_t::ERROR_CODE_NONE:
            if (had_an_overflow){
                had_an_overflow = false;
            }
            {
              if (packet_count % 10 == 0) {
                  uint32_t *data = (uint32_t *) &buff[0];
                  std::cout << str(boost::format("Packet %03d: %04x  %04x  %04x  %04x") % packet_count % data[0] % data[1] % data[2] % data[3]) << std::endl;
              }
            }
            break;

        // ERROR_CODE_OVERFLOW can indicate overflow or sequence error
        case uhd::rx_metadata_t::ERROR_CODE_OVERFLOW:
            last_time = md.time_spec;
            had_an_overflow = true;
            // check out_of_sequence flag to see if it was a sequence error or overflow
            if (!md.out_of_sequence)
                num_overflows++;
            break;

        default:
            std::cerr << "Receiver error: " << md.strerror() << std::endl;
            std::cerr << "Unexpected error on recv, continuing..." << std::endl;
            break;
        }
    }
    //rx_stream->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
}

/***********************************************************************
 * Benchmark TX thread
 **********************************************************************/
void benchmark_tx_rate(uhd::usrp::multi_usrp::sptr usrp, const std::string &tx_cpu, uhd::tx_streamer::sptr tx_stream, size_t n_packets_to_send, size_t spp){
    uhd::set_thread_priority_safe();

    std::cout << "Spawning tx thread..." << std::endl;
    boost::uint32_t sid = tx_stream->get_sid(0);
    std::cout << str(boost::format("TX streamer: SID == 0x%08x") % sid) << std::endl;

    //setup variables and allocate buffer
    uhd::tx_metadata_t md;
    //const size_t max_samps_per_packet = tx_stream->get_max_num_samps();
    const size_t max_samps_per_packet = spp;
    std::vector<char> buff(max_samps_per_packet*uhd::convert::get_bytes_per_item(tx_cpu), 0xAA);
    std::vector<const void *> buffs;
    for (size_t ch = 0; ch < tx_stream->get_num_channels(); ch++)
        buffs.push_back(&buff.front()); //same buffer for each channel
    md.has_time_spec = (buffs.size() != 1);

    int send_calls = 0;
    while (not boost::this_thread::interruption_requested() and send_calls++ < n_packets_to_send){
        //std::cout << "send!" << std::endl;
        num_tx_samps += tx_stream->send(buffs, max_samps_per_packet, md)*tx_stream->get_num_channels();;
        //std::cout << "num_tx_samps==" << num_tx_samps << std::endl;
        md.has_time_spec = false;
        //boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }

    //send a mini EOB packet
    //md.end_of_burst = true;
    //tx_stream->send(buffs, 0, md);
}

void benchmark_tx_rate_async_helper(uhd::tx_streamer::sptr tx_stream){
    //setup variables and allocate buffer
    uhd::async_metadata_t async_md;

    while (not boost::this_thread::interruption_requested()){

        if (not tx_stream->recv_async_msg(async_md)) continue;

        //handle the error codes
        switch(async_md.event_code){
        case uhd::async_metadata_t::EVENT_CODE_BURST_ACK:
            return;

        case uhd::async_metadata_t::EVENT_CODE_UNDERFLOW:
        case uhd::async_metadata_t::EVENT_CODE_UNDERFLOW_IN_PACKET:
            num_underflows++;
            break;

        case uhd::async_metadata_t::EVENT_CODE_SEQ_ERROR:
        case uhd::async_metadata_t::EVENT_CODE_SEQ_ERROR_IN_BURST:
            num_seq_errors++;
            break;

        default:
            std::cerr << "Event code: " << async_md.event_code << std::endl;
            std::cerr << "Unexpected event on async recv, continuing..." << std::endl;
            break;
        }
    }
}

/***********************************************************************
 * Main code + dispatcher
 **********************************************************************/
int UHD_SAFE_MAIN(int argc, char *argv[]){
    uhd::set_thread_priority_safe();

    //variables to be set by po
    std::string args, ce;
    double duration;
    size_t n_packets, spp;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "single uhd device address args")
        ("ce", po::value<std::string>(&ce)->default_value("0"), "single uhd device address args")
        ("duration", po::value<double>(&duration)->default_value(10.0), "duration for the test in seconds")
        ("npackets", po::value<size_t>(&n_packets)->default_value(0xFFFFFFFF), "max number of packets to send")
        ("spp", po::value<size_t>(&spp)->default_value(100), "samples per packet")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("rfnoc loopback %s") % desc << std::endl;
        return ~0;
    }

    //create a usrp device
    std::cout << std::endl;
    uhd::device_addrs_t device_addrs = uhd::device::find(args);
    if (not device_addrs.empty() and device_addrs.at(0).get("type", "") != "x300"){
        std::cout << "must use x300" << std::endl;
        return ~0;
    }
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);
    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

    boost::thread_group thread_group;

    //spawn the receive test thread
    {
	//create a receive streamer
	uhd::stream_args_t stream_args("sc16", "sc16");
	stream_args.args["src_addr"] = ce;
	stream_args.channels = std::vector<size_t>(1, 0);
	uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);
	thread_group.create_thread(boost::bind(&benchmark_rx_rate, usrp, "sc16", rx_stream, ce));
    }

    boost::this_thread::sleep(boost::posix_time::milliseconds(200));

    //spawn the transmit test thread
    {
        //create a transmit streamer
	uhd::stream_args_t stream_args("sc16", "sc16");
	stream_args.channels = std::vector<size_t>(1, 0);
	stream_args.args["dst_addr"] = ce;
	stream_args.args["spp"] = "180";
	stream_args.args["send_buff_size"] = "2048";
	uhd::tx_streamer::sptr tx_stream = usrp->get_tx_stream(stream_args);
	thread_group.create_thread(boost::bind(&benchmark_tx_rate, usrp, "sc16", tx_stream, n_packets, spp));
	thread_group.create_thread(boost::bind(&benchmark_tx_rate_async_helper, tx_stream));
    }

    //sleep for the required duration
    const long secs = long(duration);
    const long usecs = long((duration - secs)*1e6);
    boost::this_thread::sleep(boost::posix_time::seconds(secs) + boost::posix_time::microseconds(usecs));

    //interrupt and join the threads
    thread_group.interrupt_all();
    thread_group.join_all();

    boost::this_thread::sleep(boost::posix_time::seconds(2));

    std::cout << "num_tx_samps == " << std::dec << num_tx_samps << std::endl;
    std::cout << "num_rx_samps == " << std::dec << num_rx_samps << std::endl;

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
}
