#include <uhd/transport/nirio/nirio_driver_iface.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace nirio_driver_iface {

nirio_status rio_open(
    const char* device_path,
    rio_dev_handle_t& device_handle)
{
    device_handle = ::open(device_path, O_RDWR | O_CLOEXEC);
    return (device_handle < 0) ? NiRio_Status_InvalidParameter : NiRio_Status_Success;
}

void rio_close(rio_dev_handle_t& device_handle)
{
    ::close(device_handle);
    device_handle = -1;
}

bool rio_isopen(rio_dev_handle_t device_handle)
{
    return (device_handle >= 0);
}

nirio_status rio_ioctl(
    rio_dev_handle_t device_handle,
	uint32_t ioctl_code,
	const void *write_buf,
	size_t write_buf_len,
	void *read_buf,
	size_t read_buf_len)
{
	nNIRIOSRV200::tRioIoctlBlock ioctl_block = {0,0,0,0,0,0};

	// two-casts necessary to prevent pointer sign-extension
	ioctl_block.inBuf        = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(write_buf));
	ioctl_block.inBufLength  = write_buf_len;
	ioctl_block.outBuf       = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(read_buf));
	ioctl_block.outBufLength = read_buf_len;

	return ::ioctl(device_handle, ioctl_code, &ioctl_block);
}

nirio_status rio_mmap(
    rio_dev_handle_t device_handle,
	uint16_t memory_type,
	size_t size,
	int access_mode,
	rio_mmap_t &map)
{
	if (access_mode == PROT_WRITE) access_mode |= PROT_READ;	//Write-only mode not supported
	map.addr = ::mmap(NULL, size, access_mode, MAP_SHARED, device_handle, (off_t) memory_type * sysconf(_SC_PAGESIZE));
	map.size = size;

	if (map.addr == MAP_FAILED)	{
		map.addr = NULL;
		map.size = 0;
		if (errno == EINVAL)
		    return NiRio_Status_InvalidParameter;
		else if (errno == ENOMEM)
		    return NiRio_Status_MemoryFull;
		else
		    return NiRio_Status_SoftwareFault;
	}
	return 0;
}

nirio_status rio_munmap(rio_mmap_t &map)
{
	nirio_status status = 0;
	if (map.addr != NULL) {
		status = ::munmap(map.addr, map.size);

		map.addr = NULL;
		map.size = 0;
	}
	return status;
}

}
