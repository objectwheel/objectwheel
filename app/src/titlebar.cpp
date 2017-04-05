#include <fit.h>
#include <titlebar.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyleOption>
#include <QPainter>

using namespace Fit;

const uchar TitleBar::m_SettingsIconData[] = { "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D"
                                               "\x49\x48\x44\x52\x00\x00\x00\x80\x00\x00\x00\x80"
                                               "\x08\x04\x00\x00\x00\x69\x37\xA9\x40\x00\x00\x00"
                                               "\x04\x67\x41\x4D\x41\x00\x00\xB1\x8F\x0B\xFC\x61"
                                               "\x05\x00\x00\x00\x20\x63\x48\x52\x4D\x00\x00\x7A"
                                               "\x25\x00\x00\x80\x83\x00\x00\xF9\xFF\x00\x00\x80"
                                               "\xE9\x00\x00\x75\x30\x00\x00\xEA\x60\x00\x00\x3A"
                                               "\x98\x00\x00\x17\x6F\x92\x5F\xC5\x46\x00\x00\x00"
                                               "\x02\x62\x4B\x47\x44\x00\xFF\x87\x8F\xCC\xBF\x00"
                                               "\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0B\x13\x00"
                                               "\x00\x0B\x13\x01\x00\x9A\x9C\x18\x00\x00\x00\x07"
                                               "\x74\x49\x4D\x45\x07\xE1\x04\x04\x03\x25\x0C\x88"
                                               "\x2D\x71\x85\x00\x00\x0C\x83\x49\x44\x41\x54\x78"
                                               "\xDA\xED\x9D\x4D\xAC\x25\x45\x15\xC7\x7F\x55\x5D"
                                               "\xDD\x7D\xDF\x63\x06\x10\x1D\x33\x8C\xC3\x87\x1F"
                                               "\x03\xCC\x44\x74\x41\x82\x4A\x0C\x10\x0C\x26\x44"
                                               "\x17\xEA\x62\x50\x4C\x34\x31\x2C\xFC\x04\xE3\xCE"
                                               "\xF8\xB1\x31\x71\xE5\x46\x13\x13\x13\x42\x8C\x92"
                                               "\x88\x0B\x8C\x09\x10\x12\x48\x40\x23\x89\x81\xB0"
                                               "\x10\x63\x08\x08\x61\x02\xC4\x80\xC1\xD1\x84\x71"
                                               "\x60\xEE\xED\xAF\x72\x51\xA7\xBB\xAB\xBF\xEE\xED"
                                               "\x19\xDE\x7B\xD7\x77\xDF\xFD\xCF\x66\xDE\xED\xD3"
                                               "\x55\xE7\x9C\xAA\x3A\xF5\x71\xCE\xA9\x86\x35\xD6"
                                               "\x58\x63\x8D\x35\xD6\x58\x63\x8D\x35\xF6\x26\x94"
                                               "\xFF\x87\x5D\x36\x37\x4B\x10\xDA\xFB\xFF\x5E\x11"
                                               "\xBF\x29\xBA\x6E\x89\xAF\x08\xAA\x27\x81\x3C\x0A"
                                               "\x84\x4A\x57\xCF\x02\x79\x36\x8F\x5A\x2D\xA0\x56"
                                               "\x03\xD4\xDA\xA3\xD6\xDB\x44\xED\x41\x37\xC4\xD7"
                                               "\xC4\x28\x2C\x10\x12\x61\x01\x45\x8C\xA6\x00\x0C"
                                               "\x11\x60\x51\x44\x04\x14\x83\xD4\xEE\x59\x40\x2C"
                                               "\xC5\x46\x98\x5E\x6A\x2B\xD4\xB6\x45\x1D\x56\xD4"
                                               "\x75\xBD\x6F\x97\xBA\xCD\xA5\xEE\x28\x40\x34\x65"
                                               "\xC8\xC8\x80\x18\x43\x42\x81\x66\x03\x4B\x2A\x2F"
                                               "\xA7\xE4\xA2\x90\x14\x4B\xC0\x84\xBC\x45\x3D\x01"
                                               "\x12\xC0\x10\x93\x91\xA3\x98\xA0\x49\x1A\xD4\x11"
                                               "\xA1\x47\x9D\x62\x1B\xD4\x41\x45\x5D\x90\x02\x11"
                                               "\x11\x29\x05\x8A\xB8\x45\x0D\x31\x41\xC5\x49\xE1"
                                               "\x71\xE9\xA8\x55\x45\xDD\xE6\x32\xEC\x0E\x04\xD7"
                                               "\x03\x34\x54\x55\xCD\xB0\x04\xC4\x64\x24\xD2\x8A"
                                               "\x53\x0A\x34\x31\x05\x09\x16\x43\x44\x42\x26\x55"
                                               "\x4D\x3B\xD4\x33\x11\xC8\x32\x6B\x50\x47\x68\x66"
                                               "\x22\xBE\xA3\x0E\x09\x85\xBA\xAC\xD7\x10\x91\x92"
                                               "\x02\xF1\x08\xEA\x80\xB8\x43\x1D\x53\x30\x13\xEA"
                                               "\x92\xCB\x9A\x3A\x21\xAB\x45\xF7\x15\xE0\xFE\x6E"
                                               "\x33\x3D\x8E\x8D\x73\x65\x3A\x22\x68\x31\x6D\x88"
                                               "\x7A\xA8\xB7\xB6\x21\x18\x52\x80\x26\x26\x3F\x07"
                                               "\xA6\xFB\x34\x1D\x31\xA3\x90\x4E\x9C\x78\x25\x81"
                                               "\x26\x92\xA1\x16\x7A\x25\xE5\xA4\x40\x40\x28\xD4"
                                               "\x11\x4A\x4A\x30\xA4\x62\x7B\xB4\x47\x3D\xBE\x21"
                                               "\xBA\xD4\x8A\xA2\x4F\x01\x8A\x50\x46\x7C\x53\xD3"
                                               "\x65\xBB\x8C\xD7\x74\x80\x21\xE2\x03\x5C\xC3\x51"
                                               "\x0E\x8A\x31\x92\x6A\x01\x8D\xED\xFC\x02\x85\xFC"
                                               "\xD2\x7C\x96\xF1\x1F\x9E\xE5\x29\x9E\x25\xAD\xAC"
                                               "\x49\xDD\xE5\x83\x16\x97\xFD\x5D\x3E\x6D\xC9\x64"
                                               "\xC8\xC9\x4B\xD1\x4D\xC3\x1E\x64\x14\xA2\xE9\xE9"
                                               "\x40\x27\x9E\x60\x99\x76\x34\x3D\x6D\x68\x3A\xE0"
                                               "\x7C\x6E\xE0\xCB\x5C\xC7\x81\xE6\x42\xEB\x1C\x61"
                                               "\x39\xC9\x93\xFC\x9A\x07\x3B\xD6\xE4\xCC\x59\xD9"
                                               "\x9E\x92\x4B\x67\x42\xDB\x75\xC8\x3F\x65\x27\x76"
                                               "\x62\x95\xC5\x06\x76\xD3\x86\x16\x8B\x8D\xED\xA6"
                                               "\xD5\x16\xAB\xED\x86\x8D\x2D\x16\x1B\xDA\x4D\x1B"
                                               "\x0C\x50\x1B\x7B\xC4\xDE\x65\xFF\x6B\xB7\x1A\x6F"
                                               "\xDA\xBB\xED\xFB\x6D\x2C\xF5\x6E\x48\xBD\xC6\xE3"
                                               "\x72\x43\xB8\xDC\xB4\xD1\x42\x2E\xDB\xCB\xBE\x4A"
                                               "\xFC\x52\x44\x63\x37\xAD\x91\x97\x37\xAA\x97\x5D"
                                               "\xC1\x51\x55\xF0\x86\x8D\x2B\x36\x1C\x75\x68\xAF"
                                               "\xB6\x8F\x6E\xB9\xF0\x25\xFE\x64\x8F\x8D\x6E\x88"
                                               "\xA8\xD3\x6C\x25\x97\x58\xD5\xAF\x00\x65\x23\x79"
                                               "\x39\xF4\x44\x9C\xA7\xE9\x36\x1B\xB1\xBD\xC4\x3E"
                                               "\xB4\x6D\xE2\x5B\x6B\xED\xC3\xF6\x90\x9D\xB4\x1A"
                                               "\x42\xF7\x34\xC4\xBC\xFE\x10\xD9\xA0\x56\x40\x73"
                                               "\x21\x94\xC9\xA8\x36\x4C\xC9\xD1\x4C\x28\x98\x62"
                                               "\x09\x89\x98\x91\xA2\x98\xA0\x98\x52\x10\x30\x21"
                                               "\x65\x26\x0B\x9B\x92\x5A\xF1\x15\x6E\xD9\x82\x51"
                                               "\x3F\x8C\x4F\xF2\x2D\xB1\x34\xCE\x04\xBA\x7A\x73"
                                               "\x66\x15\x97\xB5\xA1\x2E\xB9\x4C\x5A\x32\x59\xDF"
                                               "\x06\xF8\x0A\x28\x28\x64\xED\x56\xBE\x9C\x57\x22"
                                               "\xB6\x8D\x4B\x4C\x42\x2A\x0B\xCE\xBA\xAA\xCB\xF8"
                                               "\xD2\xB6\x8A\x0F\x70\x1B\x47\xA0\xD5\x10\x49\xAB"
                                               "\x21\xAC\x34\x5B\xC9\x65\x53\xA6\xD4\x2F\x4E\x37"
                                               "\x0A\x77\xD3\xC7\x54\x44\x4C\x5B\x9A\xDE\x98\xA3"
                                               "\xE9\x98\x94\x8C\x9B\x78\xEF\xB6\x2B\xE0\x52\x6E"
                                               "\x26\xEC\x6D\x88\x33\x3D\xCD\x96\xA1\x98\x74\x64"
                                               "\xF2\xE6\xA6\xE6\x10\x08\xBD\x2E\x9F\xF4\x74\xF9"
                                               "\xF9\x9A\xD6\x5C\xBB\x25\xD3\xDE\x22\x7C\x84\xA0"
                                               "\xB7\x21\xCA\x59\xBF\xDD\x6C\x6D\x99\x42\x5F\xEA"
                                               "\xE6\x3A\x20\x27\x67\xFE\x82\x73\xDE\xEC\xBB\x9F"
                                               "\x4B\x06\x58\x4E\x38\x75\xD6\xC7\x0D\x8A\xFD\xC4"
                                               "\xBD\x4F\x0E\xA2\x71\xEB\xD1\x31\x8B\xB5\xB4\xB3"
                                               "\xB4\xCB\x7D\x1B\xE0\x2B\xC0\xAD\xCA\xE2\x39\xCB"
                                               "\xA0\x72\x80\x34\x37\x20\x65\xC1\xAA\x35\xA0\x4A"
                                               "\x3C\xCF\x8F\xF9\x2B\x05\x10\x62\xC9\x00\x8D\x21"
                                               "\x93\x65\xAE\x22\x95\xDE\xE7\x18\x0B\x64\x8F\x07"
                                               "\x57\xF3\x7D\xAE\xEA\x29\x2F\x90\xED\xEF\xBC\xF5"
                                               "\xE8\x94\xF6\x6E\xA6\x29\x53\xAF\x02\xE8\x15\x71"
                                               "\xBC\xA6\xFB\x61\xF9\x39\xBF\x02\x20\x44\x49\xE5"
                                               "\xE5\x6A\x2C\xC0\x30\x03\x20\x92\x45\xB8\x92\x0D"
                                               "\x2D\x18\x9E\x61\x93\x5F\xF4\x2A\x35\x26\x9D\xB3"
                                               "\x1E\xAD\x97\xC5\x43\xCD\xA6\xEA\xFE\xD8\x1C\x02"
                                               "\x21\x76\x60\x8F\x55\xEB\x6E\x58\xD3\x21\x7D\xC8"
                                               "\x38\x51\xA9\xB6\x54\x5F\x2A\xFB\x79\x2D\x9B\x23"
                                               "\x37\xD4\x4A\xA6\x8B\x8A\xE9\x97\xC9\x88\x3A\x25"
                                               "\x6A\x31\x73\xFE\xCA\xBF\xBB\x3F\x09\x98\x91\xF7"
                                               "\x0E\x63\x43\x51\x0F\x82\x3E\x1B\xD0\xD5\x9D\xAF"
                                               "\xE9\x60\x40\xD3\x10\x0D\x0C\x01\x0B\x4C\x7A\xB7"
                                               "\x2B\xDD\x71\xEA\x33\x4D\xF7\x00\x4B\x86\xAA\x4F"
                                               "\x3D\xD4\xE5\x87\xB7\xE4\xC5\x7C\x1B\xD0\xDD\x09"
                                               "\x36\x99\xEE\x37\x81\x9A\xA8\x2A\xA1\x3B\x66\xA3"
                                               "\xCE\x86\xB4\xFF\xD4\xA1\xB9\xA9\x52\x3D\xAD\x5F"
                                               "\x36\x55\x34\xD8\x10\x43\xAA\xF5\xCD\xBA\x07\xD3"
                                               "\x29\xB8\x6D\xE5\x0D\x8A\xB7\x84\x36\x93\xC3\x84"
                                               "\x80\x33\x72\xDE\x66\xE5\x59\x40\x42\xD1\xAB\x00"
                                               "\x85\x96\x2A\x03\xA8\x4A\x4A\xC8\xE5\x59\xF9\x4B"
                                               "\x2E\xD6\xC0\x30\xC5\xE2\x36\xCD\x67\x06\x7A\x94"
                                               "\xAE\xC4\xA8\xA9\x6B\x2E\x53\x69\xE1\x40\x7E\x89"
                                               "\x3B\x32\x0D\xD8\x00\x08\x3B\xBA\x9B\x70\x09\x87"
                                               "\x39\xCC\xF9\x18\x74\x25\xBE\x95\xDD\x7B\x40\x2E"
                                               "\x8A\x80\x9C\xB0\x77\x1A\x54\xDC\xC2\x21\x02\x8F"
                                               "\x5A\xA3\x84\x45\x23\x4A\xD3\x5E\xD9\x88\x6A\x02"
                                               "\x52\x8E\xF6\x0E\xAA\xC3\x7C\x55\x54\xE3\x53\x97"
                                               "\x65\x97\x25\x69\x4E\xF1\x12\x27\x38\x49\xC2\x0C"
                                               "\x3C\x99\x8C\xBF\x18\x6E\x9E\x09\xAA\xCA\x06\x24"
                                               "\x64\x4C\xB8\x89\x5B\xF9\x28\x87\xD8\x1C\x18\xDD"
                                               "\xFF\xDF\x28\x38\xC3\x6B\x3C\xC1\xBD\xFC\x81\x19"
                                               "\x93\x6A\x18\x3B\x43\x3F\x70\x26\x58\x8F\xEA\xA3"
                                               "\x7C\x97\xCF\xB0\x7F\xD9\x52\x6C\x01\x4E\xF3\x00"
                                               "\x3F\xE1\x2F\x58\xCF\xF6\x30\xA4\x80\xD2\x7E\xDE"
                                               "\xC0\xCF\xF8\xD0\xB2\x39\xDF\x42\x3C\xC3\x1D\x3C"
                                               "\xC6\x86\x18\x4C\x35\xD4\x03\x42\x59\x65\x5F\xCB"
                                               "\x3D\x5C\xB1\x6C\x9E\xB7\x18\x2F\x72\x3B\x8F\x93"
                                               "\x53\xD9\x80\xAE\x02\x34\x9A\x1C\xCB\x01\xEE\xE5"
                                               "\x13\xCB\xE6\x77\x1B\xF0\x47\x8E\xF3\x2F\x62\xF0"
                                               "\x6D\x80\x6F\xDC\x2C\x19\x16\xCD\x71\x6E\x5C\x36"
                                               "\xAF\xDB\x82\xEB\xB9\x55\x96\xE3\x9E\xC9\x6B\x2A"
                                               "\x00\x42\xDE\xC9\xE7\x06\x56\x60\xBB\x1D\x9A\xE3"
                                               "\x5C\xC0\x14\x3B\x74\x1E\xE0\xB6\x2B\x17\x73\x6C"
                                               "\xD9\x9C\x6E\x1B\xAE\xE2\x30\xB5\xD7\xB8\xA3\x00"
                                               "\x8D\x25\xE1\x10\xEF\x58\x36\x9F\xDB\x86\x0B\xB8"
                                               "\x14\x2D\xDE\xEA\x1E\x05\xB8\xBD\xFA\x66\x6B\x75"
                                               "\xB8\x4A\x08\xD8\x2F\xFE\xA5\x0A\xBE\xB0\xCE\x25"
                                               "\x15\x9D\x6D\xA9\xBB\x0A\x79\xE9\x1A\xED\x53\x80"
                                               "\xDB\x0B\xA4\x2B\x1D\x2B\xE3\x8E\x61\xA8\xF7\x02"
                                               "\x4D\x1B\x80\x9C\xC6\xAC\x2E\x94\x1C\x88\x34\x84"
                                               "\x2E\x51\x1E\x4A\xED\xC4\xC9\xEE\xB2\x60\xC4\xD9"
                                               "\xEE\xFD\xE0\x2B\x60\xF5\x6D\x80\x6D\x9F\x5D\x9A"
                                               "\xD6\x5F\x8A\x6C\xA5\x6D\x80\x3B\x93\x56\x43\x36"
                                               "\x40\x55\xA7\x29\xAB\x0A\x25\xEE\x1F\x4F\xE8\x1A"
                                               "\xA5\x0D\x58\x65\x2C\xB4\x01\x6E\x39\x3C\x0E\xA7"
                                               "\xBD\x39\x55\x73\xDE\x8E\xEE\x20\x72\x4E\x7B\x62"
                                               "\x18\xF6\x8D\x7C\x6F\x84\x0D\xC8\x47\xD9\x80\x37"
                                               "\xB8\x8B\x87\x39\x43\x86\x15\x7F\xC2\x07\xB9\x93"
                                               "\xA3\x3B\x24\xFE\x73\xFC\x94\xA7\x81\x50\x1C\x2C"
                                               "\xFB\xB8\x85\xDB\xB9\x70\x94\xE2\x5A\x36\xA0\x52"
                                               "\x8C\xC5\x6A\x1B\x5A\xEC\x67\x6D\xBA\x30\x4C\xA1"
                                               "\xB0\x3F\xB2\xB1\x04\x1E\x60\x23\x89\xBB\xB8\xD9"
                                               "\x9E\xDC\xD6\xE0\x88\x12\x27\xED\xCD\x52\x6F\x20"
                                               "\x7C\x47\xD6\xD8\x1F\xDA\x62\xE1\x9B\x99\x3D\x6E"
                                               "\xB1\xB1\xD5\xFD\x01\x12\x6E\x2F\x30\x66\x00\x9C"
                                               "\xE6\x31\x0A\xB1\xA8\x9B\x38\xDB\x6A\x78\x9A\xBF"
                                               "\xEF\x48\xFB\x3F\xCF\x13\x68\x36\xE5\x6C\x37\x92"
                                               "\x18\xD4\x47\x38\x35\xE2\x5D\x43\xD8\x5C\xEC\x9D"
                                               "\x9B\x0D\x28\x98\x49\x54\x9F\xEF\x24\x7D\x8B\xE9"
                                               "\x8E\x28\x20\x41\x57\xF1\x9E\xA5\x3B\x85\x66\xDC"
                                               "\xC7\x20\x16\xD8\x80\x40\x0E\xC5\x16\x17\xE3\xDA"
                                               "\xBC\x19\x84\x36\xD9\xA1\x19\x24\xC0\x70\xBA\xE5"
                                               "\x03\x1A\xBB\x83\x75\x36\xC0\xF3\x61\x35\x03\x24"
                                               "\x02\x52\x8A\x11\x85\x19\x14\xBA\x0A\x4F\x98\x54"
                                               "\xAE\xC9\x9D\xD9\x48\x6B\x92\x2A\x40\xA3\x8E\x13"
                                               "\xC9\x46\xD4\xEE\xC2\xE6\x1B\x4E\xDC\xE6\x4B\x29"
                                               "\xF0\x32\x77\x43\xE5\xF8\x72\x1E\x20\x4D\xE1\xF9"
                                               "\x74\x14\x6F\xF1\x06\x61\xC7\x69\x1E\xF0\x00\x2F"
                                               "\x88\xE3\xAA\xE9\x01\x1A\x2A\xA9\xEF\x97\x79\xD4"
                                               "\x4E\x84\x67\x65\x10\xF8\xFE\x46\xCB\xBF\xB9\x87"
                                               "\x0D\xF1\x33\x0D\x95\x54\xF0\x0A\xB1\x44\x26\x54"
                                               "\xAD\x0E\x78\x7E\x81\xA0\x8A\xE3\x75\x91\xB9\x56"
                                               "\xFC\xA9\xCE\x4B\x18\x4A\xF4\xEE\x70\xB8\xAC\x6E"
                                               "\x50\x27\xE2\x06\x77\xE7\xB0\x5A\x22\x89\x20\xC4"
                                               "\x54\x6E\xF0\xD2\x0F\x1C\x09\x75\x24\xC7\x96\x2E"
                                               "\xA6\x33\xED\xA1\x1E\x8A\x5B\xCE\xC5\xF1\x55\xBB"
                                               "\xD8\xAD\x38\xC5\x22\xA9\xB7\x74\xA9\x7A\xA2\x37"
                                               "\x15\x10\x10\x8B\xAB\xDB\x54\x2E\xCD\x32\x98\x41"
                                               "\x13\x8A\xFB\x32\x94\xBE\xD2\x0D\x4C\x86\x48\x5C"
                                               "\xEC\x5D\x6A\x17\xD6\xE0\x92\x29\x02\x29\x3B\xAA"
                                               "\xE6\x92\x50\xCE\x69\xFC\x20\x8A\x92\xBA\x0E\xA2"
                                               "\x28\xA9\x27\x83\xA1\x30\xFD\x21\x17\xAE\x6C\x23"
                                               "\x99\x0E\x73\x83\xA5\x33\xF6\x73\x39\x46\xAC\x81"
                                               "\x6A\x05\xB5\xE4\x28\x0C\x8A\x29\x2F\x92\x78\x41"
                                               "\x14\x65\xA4\xF6\x15\x9C\x27\x29\x0C\x35\xB5\x96"
                                               "\xF6\x0C\xC9\xC9\x50\x12\x02\xE3\xCA\x76\xC7\x2F"
                                               "\x5A\x16\x34\x3E\x75\x19\xC4\xA0\x31\x42\x6D\x24"
                                               "\x05\x22\x60\xC6\xF3\x12\x19\x86\xE7\x62\x8F\x78"
                                               "\x1F\x51\x87\xBA\x59\x6F\xC2\x4B\x9C\x2A\x27\xC2"
                                               "\x3E\x05\x80\xE5\x53\xDC\x8D\x91\x2E\xA1\xAA\xCE"
                                               "\x51\x3B\x94\x15\xF0\x06\xB7\xF1\x74\x95\xE5\x51"
                                               "\x46\x88\x5F\xC0\x2F\xB9\xBE\x5A\x4B\x34\xDF\x9B"
                                               "\xFF\xCB\xD9\x50\x1B\x1E\xE7\xF3\x9C\xE9\x84\x5C"
                                               "\x7C\x98\xFB\xB8\xA8\x1A\xDD\xFD\xB5\x64\x7C\x9D"
                                               "\x07\xCB\x20\xA9\x6E\xB4\xB8\x23\xDC\xC7\x81\x85"
                                               "\xBE\xE0\x50\x3A\x5A\xD3\x14\xA5\xEC\x1F\xB5\x1C"
                                               "\x7D\xBB\xB8\x10\xC3\x44\xEA\x2D\x6D\x0F\x4C\x78"
                                               "\xD7\xC2\xDA\x2D\x1B\xCD\xF0\x88\xB6\x5F\x40\x13"
                                               "\x0E\x84\x39\x34\x51\xC8\x78\xAB\xE3\x86\x42\xA6"
                                               "\xED\xE3\xC6\x6D\x44\xD4\x89\x06\x8C\xAA\xDC\x83"
                                               "\xC5\x7C\xCF\x4D\x9A\x4A\x47\x8B\xA1\x3A\x51\x9A"
                                               "\x7A\xC7\xB6\xD2\xE5\xFA\xA3\x0E\x80\xCD\xDA\x2D"
                                               "\x3B\x00\x97\x6B\xE6\xA1\xD9\x03\xD2\x51\x85\xC0"
                                               "\x84\xF7\x10\xB4\xA2\x34\x15\x07\x38\xB0\x23\xE2"
                                               "\x5F\xC4\x3E\xF0\x42\xA3\x62\x12\x0A\x2E\x66\x63"
                                               "\xC4\xBB\x2E\x5E\x69\xF0\x40\xA4\x9C\x88\x16\x2B"
                                               "\xE0\x0E\xAE\x94\x69\xAF\x90\x89\xE8\x20\x5F\xE3"
                                               "\xCA\x1D\x51\xC0\x95\x7C\x9B\x83\x12\xB7\x1C\x48"
                                               "\x68\xD4\x55\xDC\x39\x4A\x01\xBA\x4E\x9A\x72\x30"
                                               "\xAD\xC7\x01\xE3\x72\x68\x6F\xE4\xF7\xFC\x99\x37"
                                               "\xAB\x35\x57\xCC\x31\xAE\xDD\xA1\xA5\xB0\xE1\x3B"
                                               "\x7C\x9C\xBF\x91\xA2\xD0\x58\x0A\xF6\xF3\xB1\x91"
                                               "\xF1\x0C\x6E\x8E\xF2\xEC\x45\x33\x4E\xD0\x90\x8C"
                                               "\x1E\x06\x47\x38\xB2\x23\xE2\xF6\xAB\xE0\x3A\xAE"
                                               "\x3B\xA7\x37\x5D\x1F\xF7\xFA\x40\x3B\x61\x62\xD5"
                                               "\x11\x10\x35\xE7\xB9\x6E\xA0\xA4\x59\xE9\x63\xD1"
                                               "\xA0\x3D\x5B\x74\x6D\x80\x1A\x65\x03\x76\x2B\x4A"
                                               "\xDF\xD7\xC0\x79\x80\x91\xFD\xD7\xEA\xA2\x33\xCF"
                                               "\xB5\x6D\xC0\x2A\xB7\x3E\xB8\xD4\xDC\xC6\xE1\xD9"
                                               "\xDE\xB3\x01\xAD\x1E\xDE\x4E\x9A\x0A\xD1\x2B\xDD"
                                               "\x0B\x52\x91\xB3\x57\x01\xEE\x3C\x60\xF5\x6D\x40"
                                               "\x34\xBC\x19\xCA\xBA\x49\xB5\x2B\x06\xBD\xD7\x6D"
                                               "\x80\x69\x1B\xFA\xB6\x0D\x30\x2B\x6E\x03\x32\x09"
                                               "\x03\xE9\x55\x80\xF3\x9C\xAF\xB6\x0D\x70\xC9\x7B"
                                               "\x83\x36\x20\x97\x73\xF8\xD5\x85\x26\x6C\xFA\x86"
                                               "\xBB\x36\x20\x58\x71\x1B\x90\x37\x8F\xCE\xBA\x36"
                                               "\x20\x58\xE9\x3E\x90\x89\xA7\xA8\x57\x01\x6B\x1B"
                                               "\xB0\x27\x6C\x80\xD9\xEB\x36\xA0\x68\x1E\xFB\xB4"
                                               "\x6D\x40\xB0\x97\x6D\x80\xEF\xC8\x5C\x55\xB8\xBC"
                                               "\xF4\xC1\x84\x89\x7C\x64\x8C\xD8\xEE\x85\x5B\xEB"
                                               "\xCE\x3D\x13\xDC\x39\xFF\xCE\x32\xE0\x2E\x0B\x1B"
                                               "\x48\x9A\x82\xF2\xC4\x64\x75\xE1\xFA\xF8\xDC\xA4"
                                               "\xA9\x71\x3E\xB6\xDD\x8A\x32\xF8\xA3\x57\x01\x5A"
                                               "\x32\x07\x57\x19\x6E\x9E\x9B\x9B\x34\xB5\xEA\x36"
                                               "\x80\xF9\x89\x93\xAD\x1B\x37\x57\x0E\x0B\x6D\x80"
                                               "\xDE\x13\x49\x53\x73\x6D\xC0\x74\x44\xA4\xC5\x6E"
                                               "\x45\x41\xB2\x38\x71\xF2\xB5\x51\x41\xC7\xBB\x13"
                                               "\xA7\x78\x1D\x33\xDF\x06\x18\x5E\xE5\xB9\x65\xF3"
                                               "\xB9\x6D\x78\x81\x17\x9B\xE2\x77\x6D\x40\xC8\x3F"
                                               "\xB9\x6F\x45\xAD\x80\xE5\x77\xBC\x4E\x7D\xD5\x77"
                                               "\x47\x01\x1A\x98\x02\xF7\xF1\xC4\xB2\x79\xDD\x16"
                                               "\x3C\xC9\x6F\xA8\xAF\xFA\xEE\x51\x80\x95\xCB\xA8"
                                               "\x4F\xF2\x03\x5E\x59\x36\xB7\x5B\x8E\x7F\xF0\x3D"
                                               "\x5E\xAD\xAE\xE3\x1E\x50\x80\xBB\x9D\x3B\xE3\x51"
                                               "\xBE\xC9\x89\x65\x73\xBC\xA5\x38\xC1\x37\x78\xAC"
                                               "\xBA\x1D\xD1\x43\xD3\x06\x98\xEA\x7E\xAE\x47\xF8"
                                               "\x22\xF7\x4B\xD8\xF1\x6E\x47\xC2\x83\x7C\x81\xFB"
                                               "\xD9\x90\xA0\xDE\xC6\x25\xFB\xCD\x4B\x54\x02\xB9"
                                               "\x45\xC4\x05\x1E\x9E\xCF\xA7\x39\xCE\x35\xBC\x7B"
                                               "\xD7\xA6\xD3\x26\x9C\xE4\x29\x7E\xCB\x43\x9C\x62"
                                               "\xA3\x0A\xEA\xD5\x43\x17\x29\x29\xCA\xFB\xF8\x53"
                                               "\x89\xD3\x57\x04\x5C\xC6\x31\x2E\x67\x13\x85\x9F"
                                               "\x6C\x32\xE6\x7A\xEC\x31\xD4\xF5\xB3\xB3\xA3\x5E"
                                               "\x7C\x51\x77\xC1\x69\x5E\xE5\x39\x5E\x60\x8A\x66"
                                               "\xD2\xCA\x2F\xA0\x3F\x5F\xA0\x9B\x07\xE4\xDC\xC9"
                                               "\xD9\x40\x9C\xBF\x22\xAA\xAE\x40\x53\x72\x98\x36"
                                               "\x9F\xBA\x8E\xDC\x4F\x1A\xD4\x7E\x7E\x81\x9D\x43"
                                               "\xDD\xCD\x46\xB0\x72\xC8\x19\x0B\xB5\x5F\x6F\x2E"
                                               "\xDB\xDF\xAE\x4C\xF4\x45\x8B\x97\xD9\x16\xDD\x3B"
                                               "\xC4\x5D\x96\x88\xF6\x22\xC4\x5D\xDE\x47\x28\xF7"
                                               "\x32\x95\x97\x25\x06\x55\x9A\x83\xBB\xC0\xAE\xBC"
                                               "\x2C\xB1\xA4\x4E\x24\x03\x65\x3E\x75\x52\x31\x6D"
                                               "\xAB\x50\x5C\x24\x3B\xA5\xBC\xF3\xD4\xD5\x3B\x91"
                                               "\x9C\x10\x53\x29\xAB\xE6\x72\xD2\xA0\xF6\x65\x6A"
                                               "\x24\x4E\xB6\x6D\x40\x7E\x0E\xB7\xB5\x6F\xFD\x47"
                                               "\x17\xFA\xA9\xE7\x7D\x46\xE1\x6C\xBE\x5F\x10\xCC"
                                               "\x4F\x98\xD8\xDA\x2F\x45\xB4\x6F\x78\x1C\xC3\xF4"
                                               "\xB9\x5E\x9B\xDF\xBD\xF3\xD4\x7D\xA3\xA0\xEF\x13"
                                               "\x0D\x83\x36\xA0\xF9\xF1\x83\x44\xAE\xDB\xB7\x92"
                                               "\xD8\x14\xCA\x77\x46\xCA\xFB\x98\x34\x51\x95\xAA"
                                               "\x64\x06\xA8\xFD\x34\xA8\x2E\x75\x99\x36\x13\x10"
                                               "\x36\x3E\xBA\xD0\xA6\xEE\xD6\x3B\xC4\xE5\x18\x6A"
                                               "\x86\x14\x50\x06\x1F\xBB\x73\xA1\x62\x6E\xF2\xDC"
                                               "\x10\x35\x8C\x4F\x87\xF3\xA9\xEB\x92\xC6\xA4\xDA"
                                               "\xBD\x3D\x2E\xBB\xB7\xC9\x35\x7A\xC1\x1E\x41\xF7"
                                               "\x32\xB5\x35\xD6\x58\x63\x8D\x35\xD6\x58\x63\x8D"
                                               "\x35\xD6\xD8\x43\xF8\x1F\xE3\x88\xC1\x5D\x1A\x4B"
                                               "\xE2\x7D\x00\x00\x00\x25\x74\x45\x58\x74\x64\x61"
                                               "\x74\x65\x3A\x63\x72\x65\x61\x74\x65\x00\x32\x30"
                                               "\x31\x37\x2D\x30\x34\x2D\x30\x34\x54\x30\x33\x3A"
                                               "\x33\x37\x3A\x31\x32\x2B\x30\x33\x3A\x30\x30\xAD"
                                               "\x8D\x8F\x05\x00\x00\x00\x25\x74\x45\x58\x74\x64"
                                               "\x61\x74\x65\x3A\x6D\x6F\x64\x69\x66\x79\x00\x32"
                                               "\x30\x31\x37\x2D\x30\x34\x2D\x30\x34\x54\x30\x33"
                                               "\x3A\x33\x37\x3A\x31\x32\x2B\x30\x33\x3A\x30\x30"
                                               "\xDC\xD0\x37\xB9\x00\x00\x00\x00\x49\x45\x4E\x44"
                                               "\xAE\x42\x60\x82"};

const uchar TitleBar::m_MenuIconData[] = { "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D"
                                           "\x49\x48\x44\x52\x00\x00\x00\x40\x00\x00\x00\x40"
                                           "\x08\x04\x00\x00\x00\x00\x60\xB9\x55\x00\x00\x00"
                                           "\x04\x67\x41\x4D\x41\x00\x00\xB1\x8F\x0B\xFC\x61"
                                           "\x05\x00\x00\x00\x20\x63\x48\x52\x4D\x00\x00\x7A"
                                           "\x25\x00\x00\x80\x83\x00\x00\xF9\xFF\x00\x00\x80"
                                           "\xE9\x00\x00\x75\x30\x00\x00\xEA\x60\x00\x00\x3A"
                                           "\x98\x00\x00\x17\x6F\x92\x5F\xC5\x46\x00\x00\x00"
                                           "\x02\x62\x4B\x47\x44\x00\xFF\x87\x8F\xCC\xBF\x00"
                                           "\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0B\x13\x00"
                                           "\x00\x0B\x13\x01\x00\x9A\x9C\x18\x00\x00\x00\x07"
                                           "\x74\x49\x4D\x45\x07\xE0\x0B\x05\x02\x33\x39\x32"
                                           "\x74\x49\x57\x00\x00\x01\x36\x49\x44\x41\x54\x68"
                                           "\xDE\xED\x96\xAD\x4E\xC3\x60\x14\x86\x9F\x0E\xB6"
                                           "\x6C\x30\x39\x50\x04\x43\xB8\x00\x10\x98\xC1\x0C"
                                           "\x0A\x85\x83\x1B\x40\xC1\x35\x30\xCD\x0D\x60\xB1"
                                           "\xBB\x02\x90\x78\x90\x24\xB0\x90\xE0\x48\x10\xCC"
                                           "\x20\x1A\x4C\xDB\x1D\x44\x43\xF7\xB1\x2C\x23\x90"
                                           "\x1C\x4A\xE0\x7D\x6A\x4E\xD3\xA4\xEF\xFB\x7D\xFD"
                                           "\xC9\x03\x42\x08\x21\x02\xE6\x18\x62\xCE\xC7\x90"
                                           "\xDD\x0F\x99\xD1\x68\x7C\x66\x21\x1F\x9A\xCC\x3A"
                                           "\xAD\x30\x23\xC6\x20\xA1\x36\xA9\x80\x01\x2C\x71"
                                           "\x48\x87\x86\x53\x81\x84\x6B\x4E\xB9\x87\x55\x1E"
                                           "\xC6\xAF\xBD\x62\xD8\xA2\x5D\x98\x37\x57\xB6\x62"
                                           "\x64\x45\x6E\xE5\x7D\x68\x00\xEC\xB1\xE3\xB4\xF6"
                                           "\x11\x1B\x1C\x04\xB1\xC1\x04\x15\xB6\xDC\xE3\x01"
                                           "\xDA\xCC\x87\xA1\xE1\x5C\xFB\xF2\xCD\xBE\x43\x35"
                                           "\x4C\x0D\x0B\xA4\xDC\xFC\x48\x81\x5B\xE2\x09\x05"
                                           "\x52\x80\x1E\x77\xEE\xF1\x4F\x9C\x61\x56\x9C\x16"
                                           "\x9F\x61\x8B\x01\x40\x9B\x63\xD6\xA9\x3A\x85\x67"
                                           "\xF4\x39\xE1\x1C\x2E\xD9\x1E\x2F\x00\x59\xBE\x1D"
                                           "\x4D\x96\xA9\x3B\x15\x48\x78\xE4\x05\x2C\x78\xF2"
                                           "\x51\x78\x3D\x0E\x5F\x4F\x37\x52\xB7\x0D\x16\x7F"
                                           "\x00\xF9\x00\xF2\x01\xF9\x80\x7C\xA0\x47\xDF\x3D"
                                           "\xFE\x53\x1F\xE8\xB2\x26\x1F\xF0\x45\x3E\x20\xA6"
                                           "\x50\xB2\x0F\x0C\x68\xE5\x43\xC9\x3E\x70\x44\xC7"
                                           "\xF1\x3F\xF0\xDB\x7D\x60\xBF\x5C\x1F\x98\x61\xD3"
                                           "\x3D\x1E\xA6\xF8\x40\x24\x1F\x90\x0F\xC8\x07\x84"
                                           "\x10\xE2\x5F\xF2\x06\xF7\x24\x3C\xCA\x88\x9A\xAD"
                                           "\x97\x00\x00\x00\x25\x74\x45\x58\x74\x64\x61\x74"
                                           "\x65\x3A\x63\x72\x65\x61\x74\x65\x00\x32\x30\x31"
                                           "\x36\x2D\x31\x31\x2D\x30\x35\x54\x30\x32\x3A\x35"
                                           "\x31\x3A\x35\x37\x2B\x30\x33\x3A\x30\x30\xA1\x6F"
                                           "\x8E\x74\x00\x00\x00\x25\x74\x45\x58\x74\x64\x61"
                                           "\x74\x65\x3A\x6D\x6F\x64\x69\x66\x79\x00\x32\x30"
                                           "\x31\x36\x2D\x31\x31\x2D\x30\x35\x54\x30\x32\x3A"
                                           "\x35\x31\x3A\x35\x37\x2B\x30\x33\x3A\x30\x30\xD0"
                                           "\x32\x36\xC8\x00\x00\x00\x00\x49\x45\x4E\x44\xAE"
                                           "\x42\x60\x82"};

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , m_Layout(new QVBoxLayout(this))
    , m_UpperLayout(new QHBoxLayout)
    , m_Title(new QLabel)
    , m_Menu(new QPushButton)
    , m_Settings(new QPushButton)
    , m_ShadowWidgetBackground(new QWidget(this))
    , m_ShadowWidget(new QWidget(m_ShadowWidgetBackground))
{
    connect(m_Menu, SIGNAL(toggled(bool)), this, SIGNAL(MenuToggled(bool)));
    connect(m_Settings, SIGNAL(toggled(bool)), this, SIGNAL(SettingsToggled(bool)));

    setObjectName("titleBar");

    m_UpperLayout->setContentsMargins(fit(10), 0, fit(10), 0);

    m_Title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_Menu->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_Settings->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QPixmap settings;
    settings.loadFromData(m_SettingsIconData, sizeof(m_SettingsIconData));
    m_Settings->setIcon(QIcon(settings));
    m_Settings->setIconSize(fit({24, 24}));
    m_Settings->setMinimumSize(fit({32, 32}));
    m_Settings->setMaximumSize(fit({32, 32}));
    m_Settings->setStyleSheet(QString("QPushButton {border:0px; border-radius: %1px;} QPushButton::checked {background:rgba(0,0,0,40);} QPushButton::pressed {background:rgba(0,0,0,40);}").arg(fit(15)));
    m_Settings->setCheckable(true);
    m_Settings->setCursor(Qt::PointingHandCursor);

    QPixmap menu;
    menu.loadFromData(m_MenuIconData, sizeof(m_MenuIconData));
    m_Menu->setIcon(QIcon(menu));
    m_Menu->setIconSize(fit({24, 24}));
    m_Menu->setMinimumSize(fit({32, 32}));
    m_Menu->setMaximumSize(fit({32, 32}));
    m_Menu->setStyleSheet(QString("QPushButton {border:0px; border-radius: %1px;} QPushButton::checked {background:rgba(0,0,0,40);} QPushButton::pressed {background:rgba(0,0,0,40);}").arg(fit(15)));
    m_Menu->setCheckable(true);
    m_Menu->setCursor(Qt::PointingHandCursor);

    m_Title->setAlignment(Qt::AlignCenter);
    m_Title->setStyleSheet("background:transparent; color:white;");

    m_UpperLayout->addWidget(m_Settings);
    m_UpperLayout->addWidget(m_Title);
    m_UpperLayout->addWidget(m_Menu);

    QVBoxLayout* layout = new QVBoxLayout(m_ShadowWidgetBackground);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_ShadowWidget);

    m_ShadowWidgetBackground->setStyleSheet(QString("background:rgb(%1,%2,%3);").arg(m_ShadowWidgetColor.red())
                                            .arg(m_ShadowWidgetColor.green()).arg(m_ShadowWidgetColor.blue()));
    m_ShadowWidgetBackground->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_ShadowWidgetBackground->setMinimumHeight(fit(5));
    m_ShadowWidgetBackground->setMaximumHeight(fit(5));
    m_ShadowWidget->setStyleSheet("background:qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,stop:0 "
                                  "rgba(0, 0, 0, 100), stop:0.6 rgba(0, 0, 0, 20), stop:1 rgba(0, 0, 0, 0));");

    m_Layout->setSpacing(0);
    m_Layout->setContentsMargins(0, 0, 0, 0);
    m_Layout->addLayout(m_UpperLayout);
    m_Layout->addWidget(m_ShadowWidgetBackground);
}

const QColor& TitleBar::Color() const
{
    return m_Color;
}

void TitleBar::setColor(const QColor& Color)
{
    m_Color = Color;
    setStyleSheet(QString("#titleBar{background:rgb(%1,%2,%3);}").arg(m_Color.red()).arg(m_Color.green()).arg(m_Color.blue()));
}

const QString& TitleBar::Text() const
{
    return m_Text;
}

void TitleBar::setText(const QString& Text)
{
    m_Text = Text;
    m_Title->setText(m_Text);
}

void TitleBar::hideButtons()
{
    m_Menu->hide();
    m_Settings->hide();
}

void TitleBar::showButtons()
{
    m_Menu->show();
    m_Settings->show();
}

void TitleBar::setMenuChecked(bool checked)
{
    emit m_Menu->setChecked(checked);
}

void TitleBar::setSettingsChecked(bool checked)
{
    emit m_Settings->setChecked(checked);
}

const QColor& TitleBar::ShadowColor() const
{
    return m_ShadowWidgetColor;
}

void TitleBar::setShadowColor(const QColor& ShadowColor)
{
    m_ShadowWidgetColor = ShadowColor;
    m_ShadowWidgetBackground->setStyleSheet(QString("background:rgb(%1,%2,%3);").arg(m_ShadowWidgetColor.red())
                                            .arg(m_ShadowWidgetColor.green()).arg(m_ShadowWidgetColor.blue()));

}

void TitleBar::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}
