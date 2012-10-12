#ifndef INCLUDED_AD9361_FILTER_TAPS_HPP
#define INCLUDED_AD9361_FILTER_TAPS_HPP


/* A default 128-tap filter that can be used for generic circumstances. */
static uint16_t default_128tap_coeffs[] = {
    0x0001,0xfff1,0xffcf,0xffc0,0xffe8,0x0020,0x001a,0xffe3,
    0xffe1,0x001f,0x0028,0xffdf,0xffcc,0x0024,0x0043,0xffdb,
    0xffac,0x0026,0x0068,0xffdb,0xff80,0x0022,0x009a,0xffe2,
    0xff47,0x0017,0x00db,0xfff3,0xfeff,0xffff,0x012b,0x0013,
    0xfea5,0xffd7,0x0190,0x0046,0xfe35,0xff97,0x020e,0x0095,
    0xfda7,0xff36,0x02ae,0x010d,0xfcf0,0xfea1,0x0383,0x01c6,
    0xfbf3,0xfdb6,0x04b7,0x02f8,0xfa6d,0xfc1a,0x06be,0x0541,
    0xf787,0xf898,0x0b60,0x0b6d,0xee88,0xea40,0x2786,0x7209
};


/* The below pair of filters is optimized for a 10MHz LTE application. */
static uint16_t lte10mhz_rx_coeffs[] = {
    0xffe2,0x0042,0x0024,0x0095,0x0056,0x004d,0xffcf,0xffb7,
    0xffb1,0x0019,0x0059,0x006a,0x0004,0xff9d,0xff72,0xffd4,
    0x0063,0x00b7,0x0062,0xffac,0xff21,0xff59,0x0032,0x0101,
    0x00f8,0x0008,0xfeea,0xfeac,0xffa3,0x0117,0x01b5,0x00d0,
    0xff05,0xfdea,0xfe9e,0x00ba,0x026f,0x0215,0xffb5,0xfd4a,
    0xfd18,0xffa0,0x02de,0x03dc,0x0155,0xfd2a,0xfb0d,0xfd54,
    0x0287,0x062f,0x048a,0xfe37,0xf862,0xf8c1,0x004d,0x0963,
    0x0b88,0x02a4,0xf3e7,0xebdd,0xf5f8,0x1366,0x3830,0x518b
};

static uint16_t lte10mhz_tx_coeffs[] = {
    0xfffb,0x0000,0x0004,0x0017,0x0024,0x0028,0x0013,0xfff3,
    0xffdc,0xffe5,0x000b,0x0030,0x002e,0xfffe,0xffc4,0xffb8,
    0xfff0,0x0045,0x0068,0x002b,0xffb6,0xff72,0xffad,0x0047,
    0x00b8,0x0088,0xffc8,0xff1c,0xff33,0x001a,0x0110,0x0124,
    0x0019,0xfec8,0xfe74,0xff9a,0x0156,0x0208,0x00d3,0xfe9b,
    0xfd68,0xfe96,0x015d,0x033f,0x0236,0xfecd,0xfc00,0xfcb5,
    0x00d7,0x04e5,0x04cc,0xffd5,0xf9fe,0xf8fb,0xfef2,0x078c,
    0x0aae,0x036d,0xf5c0,0xed89,0xf685,0x12af,0x36a4,0x4faa
};


#endif // INCLUDED_AD9361_FILTER_TAPS_HPP
