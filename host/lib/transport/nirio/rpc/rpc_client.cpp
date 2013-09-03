///
// Copyright 2013 Ettus Research LLC
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

#include "rpc_client.hpp"
#include <boost/bind.hpp>

#define CHAIN_BLOCKING_XFER(func, exp, status) \
    if (status) status = (static_cast<size_t>((func)) == exp);

namespace usrprio_rpc {

using boost::asio::ip::tcp;

rpc_client::rpc_client (
    const std::string& server,
    const std::string& port,
    boost::uint32_t process_id,
    boost::uint32_t host_id
) : _socket(_io_service)
{
    //Fill in handshake info
    _hshake_args_client.version = CURRENT_VERSION;
    _hshake_args_client.oldest_comp_version = OLDEST_COMPATIBLE_VERSION;
    _hshake_args_client.pid = process_id;
    _hshake_args_client.hid = host_id;

    try {
        //Synchronous resolve + connect
        tcp::resolver resolver(_io_service);
        tcp::resolver::query query(tcp::v4(), server, port);
        tcp::resolver::iterator iterator = resolver.resolve(query);
        boost::asio::connect(_socket, iterator);

        try {
            //Perform handshake
            bool status = true;
            CHAIN_BLOCKING_XFER(
                boost::asio::write(_socket, boost::asio::buffer(&_hshake_args_client, sizeof(_hshake_args_client))),
                sizeof(_hshake_args_client), status);
            CHAIN_BLOCKING_XFER(
                boost::asio::read(_socket, boost::asio::buffer(&_hshake_args_server, sizeof(_hshake_args_server))),
                sizeof(_hshake_args_server), status);

            _request.header.hshake_id = _hshake_args_server.hshake_id;

            if (_hshake_args_server.version >= _hshake_args_client.oldest_comp_version &&
                _hshake_args_client.version >= _hshake_args_server.oldest_comp_version &&
                status)
            {
                _wait_for_next_response_header();

                //Spawn a thread for the io_service callback handler. This thread will run until rpc_client is destroyed.
                _io_service_thread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &_io_service)));
            } else {
                _exec_err.assign(boost::asio::error::connection_refused, boost::system::system_category());
            }
        } catch (boost::exception& e) {
            _exec_err.assign(boost::asio::error::connection_refused, boost::system::system_category());
        }
    } catch (boost::exception& e) {
        _exec_err.assign(boost::asio::error::connection_aborted, boost::system::system_category());
    }
}

rpc_client::~rpc_client () {
    _stop_io_service();
}

const boost::system::error_code& rpc_client::call(
    func_id_t func_id,
    const func_args_writer_t& in_args,
    func_args_reader_t &out_args,
    boost::posix_time::milliseconds timeout
)
{
    boost::mutex::scoped_lock lock(_mutex);

    if (_io_service_thread.get()) {
        _request.header.func_id = func_id;
        in_args.store(_request.data);
        _request.header.func_args_size = _request.data.size();

        _exec_err.clear();

        //Send function call header and args
        bool status = true;
        try {
            CHAIN_BLOCKING_XFER(
                boost::asio::write(_socket, boost::asio::buffer(&_request.header, sizeof(_request.header))),
                sizeof(_request.header), status);
            CHAIN_BLOCKING_XFER(
                boost::asio::write(_socket, boost::asio::buffer(&(*_request.data.begin()), _request.data.size())),
                _request.data.size(), status);
        } catch (boost::exception& e) {
            status = false;
        }

        //Wait for response using condition variable
        if (status) {
            if (!_exec_gate.timed_wait(lock, timeout)) {
                _exec_err.assign(boost::asio::error::timed_out, boost::system::system_category());
            }
        } else {
            _exec_err.assign(boost::asio::error::connection_aborted, boost::system::system_category());
            _stop_io_service();
        }

        //Verify that we are talking to the correct endpoint
        if ((_request.header.hshake_id != _response.header.hshake_id) && !_exec_err) {
            _exec_err.assign(boost::asio::error::operation_aborted, boost::system::system_category());
        }

        if (!_exec_err) out_args.load(_response.data);
    }

    return _exec_err;
}

void rpc_client::_handle_response_hdr(const boost::system::error_code& err, size_t transferred, size_t expected)
{
    boost::mutex::scoped_lock lock(_mutex);
    _exec_err = err;
    if (!_exec_err) {
        //Response header received. Verify that it is expected
        if (func_args_header_t::match_function(_request.header, _response.header)) {
            _response.data.resize(_response.header.func_args_size);

            //Wait for response data
            boost::asio::async_read(_socket,
                boost::asio::buffer(&(*_response.data.begin()), _response.data.size()),
                boost::bind(&rpc_client::_handle_response_data, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    _response.data.size()));
        } else {
            //Unexpected response. Ignore it.
            _exec_err.assign(boost::asio::error::operation_aborted, boost::system::system_category());

            _wait_for_next_response_header();
        }
    }

    if (_exec_err) _exec_gate.notify_all();
}

void rpc_client::_handle_response_data(const boost::system::error_code& err, size_t transferred, size_t expected)
{
    boost::mutex::scoped_lock lock(_mutex);
    _exec_err = err;
    if (transferred != expected) {
        _exec_err.assign(boost::asio::error::operation_aborted, boost::system::system_category());
    }

    _exec_gate.notify_all();

    _wait_for_next_response_header();
}

void rpc_client::_wait_for_next_response_header() {
    //_mutex must be locked when this call is made
    boost::asio::async_read(
        _socket,
        boost::asio::buffer(&_response.header, sizeof(_response.header)),
        boost::bind(&rpc_client::_handle_response_hdr, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred,
            sizeof(_response.header)));
}

}
