/*
 * nirio_interface.h
 *
 *  Created on: Mar 20, 2013
 *      Author: ashish
 */


#ifndef NIRIO_INTERFACE_H_
#define NIRIO_INTERFACE_H_

#include <uhd/transport/nirio/RioConstants.h>
#include <uhd/transport/nirio/nirio_transport.h>
#include <boost/smart_ptr.hpp>
#include <cstring>
#include <glob.h>
#include <vector>

namespace nirio_interface
{
    class niriok_proxy {
    public:
        typedef boost::shared_ptr<niriok_proxy> sptr;

        enum version_t { CURRENT, OLDEST_COMPATIBLE };

    	niriok_proxy();
    	virtual ~niriok_proxy();

    	//File operations
    	nirio_status open(const char* filename);
        void close(void);
        bool is_open(void) const;

        nirio_status reset();

        inline uint32_t get_interface_num() { return _interface_num; }

        nirio_status get_cached_session(
    		uint32_t& session);

        nirio_status get_version(
    		version_t type,
    		uint32_t& major,
    		uint32_t& upgrade,
    		uint32_t& maintenance,
    		char& phase,
    		uint32_t& build);

        nirio_status sync_operation(
            const void *writeBuffer,
            size_t writeBufferLength,
            void *readBuffer,
            size_t readBufferLength);

        nirio_status get_attribute(
            const tRioDeviceAttribute32 attribute,
            uint32_t& attrValue);

        nirio_status get_attribute(
            const tRioDeviceAttributeStr  attribute,
            char* buf,
            const uint32_t bufLen,
            uint32_t& stringLen);

        nirio_status set_attribute(
            const tRioDeviceAttribute32 attribute,
            const uint32_t value);

        nirio_status set_attribute(
            const tRioDeviceAttributeStr attribute,
            const char* const buffer);

        template<typename data_t>
        nirio_status peek(uint32_t offset, data_t& value);

        template<typename data_t>
        nirio_status poke(uint32_t offset, const data_t& value);

        nirio_status map_fifo_memory(
        	uint32_t fifo_instance,
        	size_t size,
        	nirio_transport::rio_memory_map& map);

        nirio_status unmap_fifo_memory(
        	nirio_transport::rio_memory_map& map);

        nirio_status download_fpga(
    		tRioDeviceDownloadAttribute destination,
    		const uint8_t* const bitstream,
    		const uint32_t size);

          //Use autogenerated copy-ctor and assignment operator

    	static const uint32_t UNCOMPRESSED_FPGA_IMAGE_SIZE = 15877916;

    private:	//Members
        static uint8_t _reverse(uint8_t b);

        int			_device_handle;
        uint32_t 	_interface_num;
    };

    typedef std::vector<niriok_proxy> niriok_proxy_vtr;
    typedef std::vector<niriok_proxy>::iterator niriok_proxy_vtr_it;

    class niriok_proxy_factory {
    public:
    	static nirio_status get_by_interface_num(uint32_t interface_num, niriok_proxy& proxy);
    	static nirio_status get_by_device_id(uint32_t device_id, niriok_proxy_vtr& proxy_vtr);

    private:	//Functions
    	static nirio_status _get_proxy_vtr(
    		uint32_t filter_value,
    		bool (*filter)(niriok_proxy& proxy, uint32_t filter_value),
    		niriok_proxy_vtr& proxy_vtr);

        static bool _filter_by_interface_num(niriok_proxy& proxy, uint32_t filter_value);
    	static bool _filter_by_device_id(niriok_proxy& proxy, uint32_t filter_value);
};
}

#endif /* NIRIO_INTERFACE_H_ */
