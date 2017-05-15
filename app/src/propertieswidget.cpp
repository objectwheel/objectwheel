#include <propertieswidget.h>
#include <propertyitem.h>
#include <fit.h>
#include <listwidget.h>
#include <lineedit.h>

#include <QStyleOption>
#include <QPainter>
#include <QQuickItem>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QQmlContext>
#include <QIcon>

using namespace Fit;

static const uchar rawData[] = { "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D"
								 "\x49\x48\x44\x52\x00\x00\x00\x80\x00\x00\x00\x80"
								 "\x08\x06\x00\x00\x00\xC3\x3E\x61\xCB\x00\x00\x00"
								 "\x04\x73\x42\x49\x54\x08\x08\x08\x08\x7C\x08\x64"
								 "\x88\x00\x00\x00\x09\x70\x48\x59\x73\x00\x00\x0B"
								 "\x13\x00\x00\x0B\x13\x01\x00\x9A\x9C\x18\x00\x00"
								 "\x0A\x9E\x49\x44\x41\x54\x78\x9C\xED\x9D\x5F\x6C"
								 "\x1C\xF5\x11\xC7\x67\xE6\xD6\x36\x26\x72\x50\x4D"
								 "\x2C\x2B\x6A\x51\x03\x0D\xA9\x68\x9B\x98\x2A\x22"
								 "\x51\xD3\x07\xC8\x03\xAE\x0B\x08\x9E\xE2\x0A\xA4"
								 "\x22\xE5\x29\x95\xF1\xDE\x3A\x3A\x12\xA9\x90\x72"
								 "\x49\xCA\x1F\x25\x96\x75\xF7\xF3\xD9\x81\xBC\xF4"
								 "\xA1\x08\x21\x1C\xB5\x12\x0D\x90\x14\x08\xA4\x02"
								 "\xA4\x26\xA9\x80\x04\x4A\x49\xA8\x2A\x10\xA5\x4A"
								 "\x00\xB9\x81\x02\xF5\x71\xC7\x4C\x1F\xB2\x6E\x11"
								 "\x8D\x77\xF7\xEE\x76\x7F\xBB\x17\xCD\xE7\x75\x67"
								 "\x6F\xBF\x7B\xF7\xBD\xDF\xEE\xCE\x6F\x7E\xB3\x00"
								 "\x8A\xA2\x28\x8A\xA2\x28\x8A\xA2\x28\x8A\xA2\x28"
								 "\x8A\xA2\x28\x8A\xA2\x28\x8A\xA2\x28\x8A\xA2\x28"
								 "\x8A\xA2\x28\x8A\xA2\x28\x8A\xA2\x28\x8A\xA2\xB4"
								 "\x2B\x98\xB6\x80\x38\x28\x16\x8B\xD4\xD7\xD7\x77"
								 "\x95\x88\xAC\x11\x91\x55\xCC\x7C\x25\x11\x7D\x13"
								 "\x00\xFA\x98\xF9\x12\x22\xEA\x04\x00\x60\xE6\xCF"
								 "\x01\xE0\x2C\x00\x7C\x80\x88\xEF\x00\xC0\x5B\x88"
								 "\x78\x42\x44\x8E\xCE\xCE\xCE\xBE\xB9\x63\xC7\x0E"
								 "\x4E\xF3\x3C\xD2\xA0\x6D\x0D\xB0\x6B\xD7\xAE\x9E"
								 "\xEE\xEE\xEE\x9B\x44\xE4\x16\x44\xBC\x1E\x00\x7A"
								 "\x5B\xFC\xC8\x59\x00\x78\x9A\x99\x1F\x77\x1C\xE7"
								 "\x89\xD1\xD1\xD1\x4F\x62\x90\x99\x79\xDA\xCE\x00"
								 "\xA5\x52\x69\x2D\x11\x8D\x20\xE2\x06\x00\xE8\x4E"
								 "\xE2\x18\xCC\xFC\x19\x00\xCC\x10\xD1\x83\xF9\x7C"
								 "\xFE\x68\x12\xC7\xC8\x0A\x6D\x61\x00\x11\xC1\xA9"
								 "\xA9\xA9\x41\x11\xB9\x07\x00\xD6\xD9\x3C\x36\x33"
								 "\xBF\x40\x44\x3B\x5D\xD7\x3D\x84\x88\x62\xF3\xD8"
								 "\x36\xC8\xBC\x01\x26\x27\x27\x07\x00\xA0\x04\x00"
								 "\xEB\xD3\xD4\xC1\xCC\xCF\x02\xC0\xE6\xB1\xB1\xB1"
								 "\xD7\xD3\xD4\x11\x37\x99\x35\xC0\xC4\xC4\x44\xB7"
								 "\xE3\x38\xDB\x11\xF1\x4E\x00\xA0\xB4\xF5\x00\x00"
								 "\x30\xF3\x17\x44\xB4\xBB\xA7\xA7\x67\xE7\xC6\x8D"
								 "\x1B\xE7\xD2\xD6\x13\x07\x99\x34\x40\xA5\x52\x59"
								 "\xC9\xCC\x8F\x22\xE2\x77\xD3\xD6\x72\x3E\x98\xF9"
								 "\x35\xC7\x71\x6E\x1D\x1D\x1D\xFD\x73\xDA\x5A\x5A"
								 "\x25\x13\xFF\xAC\x2F\x53\xA9\x54\x6E\x13\x91\x23"
								 "\x59\xFD\xF1\x01\x00\x88\x68\x65\xBD\x5E\x3F\x6A"
								 "\x8C\x19\x4E\x5B\x4B\xAB\x64\x66\x04\x10\x11\x9C"
								 "\x9C\x9C\xFC\x05\x22\xEE\x68\xE1\x63\x4E\x32\xF3"
								 "\x61\x44\x3C\x8E\x88\x27\xEB\xF5\xFA\xDB\x1D\x1D"
								 "\x1D\x1F\x7D\xF8\xE1\x87\xFF\x02\x00\x58\xB2\x64"
								 "\x49\x4F\xAD\x56\xBB\x84\x88\x2E\x47\xC4\x15\x22"
								 "\x32\x00\x00\xEB\x89\x68\x45\x0B\xC7\xBC\xDB\x75"
								 "\xDD\x07\xDA\xF5\x06\x31\x13\x06\x10\x11\x34\xC6"
								 "\x94\x89\x28\xDF\xC4\xEE\xAF\x00\xC0\xC3\x22\xB2"
								 "\xCF\xF3\xBC\xBF\x37\x73\xFC\xE9\xE9\xE9\xCB\xEA"
								 "\xF5\xFA\x06\x11\xB9\x9D\x88\x06\x1A\xDD\x5F\x44"
								 "\x4A\xF9\x7C\xBE\xD0\x8E\x26\x48\xDD\x00\x2D\xFC"
								 "\xF8\xFB\x45\xE4\x7E\xCF\xF3\xFE\x18\xA7\x1E\x63"
								 "\xCC\x3A\x11\xB9\x8B\x88\x6E\x6C\x64\xBF\x76\x35"
								 "\x41\xEA\x06\x30\xC6\xDC\xD3\xE0\xB0\x7F\x0C\x11"
								 "\x47\x5C\xD7\xFD\x53\x62\xA2\x00\x60\x72\x72\x72"
								 "\x0D\x00\xEC\x01\x80\xD5\x51\xF7\x11\x91\x6D\x9E"
								 "\xE7\xDD\x97\x9C\xAA\xF8\x49\xD5\x00\xFE\x0D\xDF"
								 "\x23\x11\xC3\xAB\x22\x52\x58\xBA\x74\xE9\x43\xC3"
								 "\xC3\xC3\x5F\x24\x2A\xCC\x67\x66\x66\x26\x77\xFA"
								 "\xF4\xE9\x3B\x00\x60\x37\x00\x74\x45\xD9\x07\x11"
								 "\x87\x5D\xD7\xDD\x97\xAC\xB2\xF8\x48\xCD\x00\x95"
								 "\x4A\x65\xA5\x88\x1C\x81\x08\xE9\x5C\x66\x3E\x45"
								 "\x44\xC3\xF9\x7C\xFE\xB8\x05\x69\xFF\x47\xA9\x54"
								 "\xBA\x9A\x88\xF6\x21\xE2\xF2\x08\xE1\x9F\x8A\xC8"
								 "\x1A\xCF\xF3\xDE\x48\x5C\x58\x0C\xA4\x62\x00\x3F"
								 "\xC9\x73\x2C\xE2\xA3\xDE\x8B\x8E\xE3\xDC\x3C\x32"
								 "\x32\xF2\xCF\xC4\x85\x05\x30\x31\x31\xD1\xDB\xD1"
								 "\xD1\xB1\x1F\x22\xA4\xA2\x99\xF9\x38\x22\xAE\xF5"
								 "\x3C\xAF\x6A\x41\x5A\x4B\xA4\x92\x07\xF0\x33\x7C"
								 "\x51\x7E\xFC\xA7\x6A\xB5\xDA\x60\xDA\x3F\x3E\x00"
								 "\x40\xA1\x50\x98\xED\xEA\xEA\xBA\x5E\x44\x7E\x1F"
								 "\x16\x4B\x44\x03\x22\xB2\xCD\x86\xAE\x56\xB1\x3E"
								 "\x02\xF8\xB9\xFD\x97\x21\xDC\x7C\x2F\xD6\x6A\xB5"
								 "\xC1\x42\xA1\xF0\x6F\x0B\xB2\x22\xB3\x77\xEF\xDE"
								 "\x8B\xE7\xE6\xE6\x9E\x45\xC4\x1F\x84\x84\xD6\x45"
								 "\x64\x20\xEB\x97\x02\xAB\x23\x80\x88\x20\x9C\x9B"
								 "\xD8\x09\x3C\x2E\x33\x9F\x72\x1C\xE7\xE6\xAC\xFD"
								 "\xF8\x00\x00\x9B\x36\x6D\xFA\xAC\x5E\xAF\xDF\x24"
								 "\x22\x7F\x0D\x09\x75\x10\xB1\x64\x45\x54\x0B\x58"
								 "\x35\xC0\xD4\xD4\xD4\x20\x84\xCC\xEA\x31\xF3\x1C"
								 "\x00\x6C\xC8\xC2\xB0\xBF\x10\x85\x42\x61\x16\x00"
								 "\x86\x01\xE0\xF3\x90\xD0\x41\x63\x4C\xAA\xB3\x98"
								 "\x61\xD8\x1E\x01\xEE\x09\x8B\x41\xC4\x3B\xC7\xC6"
								 "\xC6\x4E\xD8\xD0\xD3\x0A\x9E\xE7\xBD\x82\x88\x5B"
								 "\xC3\xE2\x10\xB1\x68\x43\x4F\xB3\x58\x33\x40\xA9"
								 "\x54\x5A\x0B\xE1\x77\xD0\xC7\x96\x2E\x5D\xFA\x90"
								 "\x0D\x3D\x71\xD0\xDF\xDF\x3F\x05\xE7\x52\xD1\x41"
								 "\x5C\x5B\x2E\x97\xAF\xB1\xA1\xA7\x19\xAC\x19\x80"
								 "\x88\x46\xC2\x62\x10\x71\xC4\x56\x92\x27\x0E\x86"
								 "\x87\x87\xBF\x10\x91\xD0\xF3\x22\xA2\x9F\xD9\xD0"
								 "\xD3\x0C\x56\x9E\x02\xFC\x02\xCE\x33\x10\x9C\xF4"
								 "\xD9\x9F\xCF\xE7\x6F\xB6\xA1\x27\x6E\xCA\xE5\xF2"
								 "\x53\x44\xF4\xE3\x80\x90\x4F\xAB\xD5\x6A\xFF\x96"
								 "\x2D\x5B\x3E\xB5\x26\x2A\x22\x56\x46\x80\xEE\xEE"
								 "\xEE\x9B\x20\x24\xE3\x27\x22\xF7\xDB\xD0\x92\x04"
								 "\x44\x14\x96\xFF\x5F\xD4\xD9\xD9\xD9\xD0\xE4\x92"
								 "\x2D\xAC\x18\x40\x44\x6E\x09\x09\x79\x35\xEE\x59"
								 "\x3D\x9B\xE4\xF3\xF9\x97\x98\xF9\xB5\xA0\x18\x44"
								 "\xCC\xE4\xE8\x96\xB8\x01\x8A\xC5\x22\x89\xC8\x60"
								 "\x48\xD8\xAF\x93\xD6\x91\x34\x44\xF4\x70\x48\xC8"
								 "\xA0\x9F\x07\xC9\x14\x89\x1B\xA0\xAF\xAF\xEF\x2A"
								 "\x22\xFA\x5A\x50\x8C\x88\xB4\xCD\xEC\xD9\x42\x10"
								 "\xD1\x4C\x48\x48\x5F\xB9\x5C\x6E\xA5\xF2\x28\x11"
								 "\x12\x37\x80\x88\xAC\x09\x09\x39\xD9\x6C\x25\x4F"
								 "\x96\x18\x1D\x1D\x7D\x27\x2C\x3B\x98\xCB\xE5\xD6"
								 "\xDA\xD2\x13\x15\x1B\x06\x58\x15\xB4\x9D\x99\x0F"
								 "\x27\xAD\xC1\x22\xCF\x07\x6D\x0C\xFB\x2E\xD2\x20"
								 "\x71\x03\x30\xF3\x95\x41\xDB\x11\x31\x95\x39\xFE"
								 "\x24\x40\xC4\xC0\x0C\x66\xC4\x7A\x02\xAB\x24\x6E"
								 "\x00\x7F\x95\xEE\x82\x20\xE2\xC9\xA4\x35\xD8\x22"
								 "\xEC\x5C\x98\x79\x99\x25\x29\x91\xB1\xF1\x18\xD8"
								 "\x17\xB4\xB1\x5E\xAF\xBF\x6D\x41\x83\x15\x98\xF9"
								 "\x6F\x21\x21\x81\xDF\x45\x1A\xD8\xB8\x04\x5C\x12"
								 "\xB4\xBD\xA3\xA3\xE3\xA3\xA4\x35\x58\xE4\xE3\xA0"
								 "\x8D\x61\x4F\x43\x69\x60\xE3\x12\xD0\x19\xB4\x7D"
								 "\x7E\xD1\xC6\x05\x42\xA0\x01\x20\x62\x61\xA9\x4D"
								 "\x32\xB7\x34\x4C\xB1\x8B\x8D\x4B\x40\x60\xD1\xC4"
								 "\x92\x25\x4B\x7A\x92\xD6\x60\x91\xC5\x21\xDB\x33"
								 "\x57\x24\x6A\x63\x04\x38\x1B\xB4\xB1\x56\xAB\x05"
								 "\xDE\x23\xB4\x19\x61\x06\x08\xFC\x2E\xD2\xC0\x86"
								 "\x01\x3E\x08\x14\x40\x74\xB9\x05\x0D\x56\x20\xA2"
								 "\x2B\x42\x42\xCE\x58\x11\xD2\x00\x89\x1B\xC0\xEF"
								 "\xC6\x15\xB4\x3D\x73\xF9\xF1\x66\x11\x91\x6F\x87"
								 "\x84\xBC\x6B\x45\x48\x03\xD8\x18\x01\xDE\x0A\xDA"
								 "\xE8\x2F\xD1\xBE\x50\x08\x3C\x17\x11\x39\x65\x4B"
								 "\x48\x54\x6C\x8C\x00\x61\x05\x9E\x99\xAE\x9A\x6D"
								 "\x90\xC0\x73\x89\xF0\x5D\x58\xC7\xC6\x64\x50\x60"
								 "\x9B\x35\x22\x5A\x31\x3D\x3D\x7D\x59\xD2\x3A\x92"
								 "\xA6\x54\x2A\x2D\x03\x80\x6F\x85\x84\x65\xAE\xE5"
								 "\x5C\xE2\x06\x98\x9D\x9D\x7D\x13\xCE\x35\x61\x5C"
								 "\x90\x7A\xBD\xBE\x21\x69\x1D\x49\x93\xCB\xE5\x02"
								 "\xDB\xC5\x30\xF3\xFB\xAE\xEB\x66\x6E\xDE\x23\x71"
								 "\x03\xF8\xED\x57\x9F\x0E\x8A\x11\x91\xDB\x93\xD6"
								 "\x91\x24\x22\x82\x61\xE7\x40\x44\xCF\x64\xB1\x79"
								 "\x84\x95\x4C\x20\x33\x3F\x1E\x28\x82\x68\xC0\x18"
								 "\x63\xB5\x01\x64\x9C\x18\x63\x7E\x18\xB6\xD8\x55"
								 "\x44\x7E\x67\x4B\x4F\x23\x58\x31\x80\xE3\x38\x4F"
								 "\xF8\xED\x57\x17\x44\x44\xEE\xB2\xA1\x25\x09\x10"
								 "\x31\x70\x25\x30\x33\x7F\x72\xD1\x45\x17\x3D\x61"
								 "\x4B\x4F\x23\x58\x31\x80\xDF\x78\x39\xB0\x66\x8E"
								 "\x88\x6E\xF4\xDB\xB2\xB4\x15\xC6\x98\x75\x88\xF8"
								 "\xA3\xA0\x18\x44\x7C\x6C\xD3\xA6\x4D\x81\x7F\x80"
								 "\xB4\xB0\xB9\x32\xE8\xC1\x08\x61\x7B\x66\x66\x66"
								 "\x72\x89\x8B\x89\x89\x62\xB1\xE8\x88\xC8\x9E\x08"
								 "\xA1\xD3\x89\x8B\x69\x12\x6B\x06\xC8\xE7\xF3\x47"
								 "\x99\xF9\x85\x90\xB0\xD5\x7E\x4F\x9E\xB6\xA0\xB7"
								 "\xB7\x37\x1F\xD6\x56\x8E\x99\x9F\xF3\x3C\x2F\x6C"
								 "\xFD\x60\x6A\x58\x9D\x0E\x26\xA2\x9D\x11\xC2\x76"
								 "\x97\x4A\xA5\xAB\x13\x17\xD3\x22\xC6\x98\xD5\x88"
								 "\xF8\x40\x58\x1C\x22\x46\x39\xE7\xD4\xB0\x6A\x00"
								 "\xD7\x75\x0F\xF9\x5D\xB7\x83\xE8\x22\xA2\x7D\x13"
								 "\x13\x13\xAD\xBE\x00\x22\x31\x2A\x95\xCA\xA5\x88"
								 "\x38\x03\x00\x81\xC5\x2E\xCC\x7C\xC0\xF3\xBC\x3F"
								 "\x58\x92\xD5\x14\x56\x0D\xE0\x3F\x07\x6F\x66\xE6"
								 "\xC0\x15\xC0\x88\xB8\xBC\xA3\xA3\x63\xFF\xDE\xBD"
								 "\x7B\x2F\xB6\x24\x2D\x32\xE3\xE3\xE3\x8B\x44\xE4"
								 "\x49\x00\x08\x9C\xF9\x63\xE6\x9A\x88\x14\x2C\xC9"
								 "\x6A\x1A\xEB\x15\x41\x63\x63\x63\xAF\x13\xD1\xEE"
								 "\x08\xA1\xEB\xAA\xD5\xEA\x6F\xB2\x64\x82\xF1\xF1"
								 "\xF1\x45\x5D\x5D\x5D\xBF\x05\x80\x28\x0B\x3C\x76"
								 "\x6D\xDE\xBC\xF9\x2F\x49\x6B\x6A\x95\x54\x4A\xC2"
								 "\x7A\x7A\x7A\x76\x86\x2D\xA6\xF4\x19\xAA\x56\xAB"
								 "\xCF\x64\xE1\x72\x50\xA9\x54\x2E\xED\xEA\xEA\x3A"
								 "\x04\x00\x61\xEB\x1C\x01\x00\xDE\x43\xC4\x7B\x93"
								 "\xD6\x14\x07\xA9\x18\x60\xE3\xC6\x8D\x73\x8E\xE3"
								 "\xDC\x1A\x96\x1C\xF2\x59\xE7\x38\xCE\x11\x63\xCC"
								 "\xF7\x13\x17\xB6\x00\xC6\x98\xD5\xFE\xA4\x56\xD4"
								 "\xA5\x5D\x5F\x07\x80\x9F\x27\x28\x29\x36\x52\x5D"
								 "\xAD\x6A\x8C\x19\x46\xC4\xC7\x22\x86\x7F\x8E\x88"
								 "\x5B\xFB\xFB\xFB\xA7\x6C\x75\x11\x29\x16\x8B\x4E"
								 "\x6F\x6F\x6F\xDE\xBF\xDB\x0F\xBC\xE1\x3B\x1F\x22"
								 "\xB2\xC3\xF3\xBC\xED\xF1\x2B\x8B\x8F\xD4\x97\x2B"
								 "\x1B\x63\xEE\x6E\x64\xB8\x14\x91\x97\x01\xE0\x8E"
								 "\xA4\xFB\x09\xF8\x19\xBE\x69\x00\x68\xE9\x91\x34"
								 "\xEB\x26\x48\xDD\x00\xFE\x8B\x22\x26\x10\x71\x73"
								 "\x23\xFB\x31\xF3\x01\x22\xBA\x2F\x9F\xCF\xBF\x14"
								 "\xA7\x16\x7F\x62\x67\x5B\x58\x7A\xB7\xC1\xCF\xCD"
								 "\xAC\x09\x52\x37\x00\x40\xF3\x26\x00\x38\xF7\xFE"
								 "\x1E\x22\x7A\x98\x88\x66\x46\x47\x47\x03\xEB\x0F"
								 "\x17\xA2\x54\x2A\x2D\xF3\xE7\xF3\x7F\x0A\x00\xDF"
								 "\x6B\xE6\x33\xC2\xC8\xAA\x09\x32\x61\x00\x80\xFF"
								 "\x9A\xE0\xAE\x56\xEE\x9E\xFD\xF5\xF9\xCF\x23\xE2"
								 "\x09\x44\x3C\xE9\xAF\xD5\xFB\x18\xFE\xB7\x62\x67"
								 "\x31\x00\x2C\x26\xA2\x2B\xFC\x02\xCE\x01\x38\x57"
								 "\xC6\x15\x56\xC9\x13\x0B\x59\x34\x41\x66\x0C\x30"
								 "\x8F\x7F\x63\xF8\x2B\x00\x58\x94\xB6\x96\x24\xC8"
								 "\x9A\x09\x32\xB7\x34\xCC\xF3\xBC\x19\xBF\xAB\x48"
								 "\xE6\x0A\x28\xE7\x61\xE6\x7F\x34\xBB\x2F\x22\x16"
								 "\x8D\x31\xDB\x63\x94\xD3\x12\x99\x33\x00\x00\x80"
								 "\xE7\x79\x6F\xF8\x26\xB8\x0F\x00\xEA\x69\xEB\x99"
								 "\x87\x99\x6B\xCC\x7C\x2F\x22\x5E\x21\x22\x4D\xBF"
								 "\xDD\x2C\x4B\x26\xC8\xDC\x25\xE0\xAB\x18\x63\xBE"
								 "\xE3\x77\xDD\x8E\x92\x81\x4B\x0C\x66\x3E\x20\x22"
								 "\x85\x2F\xA7\x77\x8D\x31\xDB\x5B\xE9\x05\x9C\x85"
								 "\xCB\x41\xE6\x0D\x30\x8F\x31\x66\xBD\xFF\x65\x5F"
								 "\x6B\xF3\xB8\xCC\xFC\x5C\x2E\x97\xFB\xA5\xEB\xBA"
								 "\x87\x17\xD0\xD5\xD6\x26\x68\x1B\x03\xCC\x53\x2E"
								 "\x97\xAF\xF1\x7B\xEF\xFE\x04\x12\xBA\x51\x64\xE6"
								 "\x4F\x10\xF1\x31\x22\xDA\xE3\xBA\xEE\xCB\x61\xF1"
								 "\xED\x6C\x82\xB6\x33\xC0\x3C\xE3\xE3\xE3\x8B\x3A"
								 "\x3B\x3B\x6F\xF4\x3B\x70\x0E\x42\x8B\xED\x57\x98"
								 "\xF9\x7D\xBF\x74\xFB\xF1\xCE\xCE\xCE\x27\x1B\xAD"
								 "\xE1\x6B\x57\x13\xB4\xAD\x01\xBE\x8C\x88\x60\xB9"
								 "\x5C\x5E\x91\xCB\xE5\xD6\x8A\xC8\x2A\x44\x5C\xEE"
								 "\x37\x64\xEA\xF3\xDB\xB2\xCC\x77\xE6\xA8\xC2\xB9"
								 "\x25\xDA\x67\x00\xE0\x5D\x11\x39\xE5\x2F\xD7\x3A"
								 "\xEA\xBA\xEE\xC9\x56\xEB\xF6\xDB\xD1\x04\x17\x84"
								 "\x01\xB2\x44\xBB\x99\xA0\x6D\x2A\x70\xDB\x85\x83"
								 "\x07\x0F\x1E\x1E\x1A\x1A\x42\x44\xBC\xAE\x99\xFD"
								 "\x11\xF1\xBA\xA1\xA1\x21\x3C\x78\xF0\xE0\xE1\x78"
								 "\x95\x9D\x1F\x35\x40\x02\xB4\x93\x09\xD4\x00\x09"
								 "\xD1\x2E\x26\x50\x03\x24\x48\x3B\x98\x40\x0D\x90"
								 "\x30\x59\x37\x81\x1A\xC0\x02\x59\x36\x81\x1A\xC0"
								 "\x12\x59\x35\x81\x1A\xC0\x22\x71\x98\xE0\x86\x1B"
								 "\x6E\x38\x7B\xE0\xC0\x81\x23\x71\x69\xD2\x44\x50"
								 "\x0A\xB4\x98\x2C\xAA\x8A\xC8\xF2\xB8\xDE\xB2\x92"
								 "\xC9\x7A\x80\x0B\x1D\xCF\xF3\xB6\xB7\x50\x4F\xD0"
								 "\x85\x88\xB7\xC5\xA5\x45\x0D\x90\x12\xAD\x98\x20"
								 "\xCE\xDE\x8A\x6A\x80\x14\x69\x71\x24\x88\x05\x35"
								 "\x40\xCA\x34\x63\x82\x38\xDF\xB3\xA4\x06\xC8\x00"
								 "\x8D\x98\x80\x99\xE7\x6A\xB5\xDA\x23\x71\x1D\x5B"
								 "\x0D\x90\x11\xA2\x9A\x00\x11\xB7\x16\x0A\x85\xF7"
								 "\xE2\x3A\xAE\x3E\x06\x66\x8C\x4A\xA5\xE2\x89\xC8"
								 "\x2E\xF8\xCA\xEB\x65\x98\x79\x0E\x11\xB7\x7A\x9E"
								 "\x57\x89\xF3\x78\x6A\x80\x0C\x62\x8C\xF9\x86\xFF"
								 "\xA8\xB7\x0A\x00\x40\x44\x5E\xAD\xD7\xEB\x8F\xC6"
								 "\xF9\xCF\x57\x14\x45\x51\x14\x45\x51\x14\x45\x51"
								 "\x14\x45\x51\x14\x45\x51\x14\x45\x51\x14\x45\x51"
								 "\x14\x45\x51\x14\xE5\x42\xE4\x3F\x21\xE7\x39\xD1"
								 "\x00\x29\x5C\x59\x00\x00\x00\x00\x49\x45\x4E\x44"
								 "\xAE\x42\x60\x82"};

PropertiesWidget::PropertiesWidget(QWidget *parent)
	: QWidget(parent)
	, m_ListWidget(new ListWidget)
	, m_Layout(new QVBoxLayout(this))
	, m_Color(QColor("#566573"))
	, m_LastObject(nullptr)
	, m_SearchEdit(nullptr)
	, m_rootContext(nullptr)
{
	setAutoFillBackground(true);
	QPalette p(palette());
	p.setColor(QPalette::Window, m_Color);
	setPalette(p);

	m_ListWidget->setStyleSheet(QString("QListView::item{background:none;border: 0px solid transparent;}"
										"QListView { border:0px solid white;background:rgba(%1,%2,%3,%4);}")
								.arg(m_Color.red()).arg(m_Color.green())
								.arg(m_Color.blue()).arg(m_Color.alpha()));
	m_ListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_ListWidget->setHorizontalScrollMode(ListWidget::ScrollPerPixel);
	m_ListWidget->setVerticalScrollMode(ListWidget::ScrollPerPixel);
	m_ListWidget->setSelectionBehavior(ListWidget::SelectRows);
	m_ListWidget->setFocusPolicy(Qt::NoFocus);
	m_ListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_Layout->setSpacing(fit(10));
	m_Layout->setContentsMargins(fit(10), fit(5), fit(5), fit(10));


	QPixmap pm;
	pm.loadFromData(rawData, sizeof(rawData));
	pm = pm.scaled(fit(30), fit(30), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	m_SearchEdit = new LineEdit;
	m_SearchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_SearchEdit->setFixedHeight(fit(30));
	m_SearchEdit->setIcon(QIcon(pm));
	m_SearchEdit->setPlaceholderText("Search");
	m_SearchEdit->show();
	connect(m_SearchEdit->lineEdit(), (void(QLineEdit::*)(const QString&))(&QLineEdit::textEdited), [=] (const QString& str){
		if (m_LastObject) {
			refreshList(m_LastObject, str);
		}
	});

	m_Layout->addWidget(m_SearchEdit);
	m_Layout->addWidget(m_ListWidget);
}

const PropertiesWidget::Properties& PropertiesWidget::properties() const
{
	return m_Properties;
}

const QColor& PropertiesWidget::color() const
{
	return m_Color;
}

void PropertiesWidget::setColor(const QColor& color)
{
	m_Color = color;
	QPalette p(palette());
	p.setColor(QPalette::Window, m_Color);
	setPalette(p);
}

QList<QQuickItem*>* PropertiesWidget::itemSource() const
{
	return m_Items;
}

void PropertiesWidget::setItemSource(QList<QQuickItem*>* items)
{
    m_Items = items;
}

QList<QUrl>*PropertiesWidget::urlList() const
{
    return m_UrlList;
}

void PropertiesWidget::setUrlList(QList<QUrl>* urlList)
{
    m_UrlList = urlList;
}

void PropertiesWidget::setRootContext(QQmlContext* const context)
{
	m_rootContext = context;
}

void PropertiesWidget::refreshList(QObject* const selectedItem, const QString& filter)
{
	clearList();

	if (filter.isEmpty() && m_LastObject != selectedItem) {
		m_SearchEdit->lineEdit()->clear();
	}

	/* Get selected item's properties */
	auto meta = selectedItem->metaObject();
	int count = meta->propertyCount();
	for (int i = 0; i < count; i++) {
		auto property = meta->property(i);
		if (!QString(property.name()).startsWith("__") && QString(property.name()).contains(filter, Qt::CaseInsensitive)) {
			m_Properties << Property(property, selectedItem);
		}
	}

	/* Update list widget */
	refreshListWidget(selectedItem);

	setEnabled(true);

	m_LastObject = selectedItem;

	emit listRefreshed();
}

void PropertiesWidget::clearList()
{
	m_ListWidget->clear();
	m_Properties.clear();
	m_LastObject = nullptr;
}

void PropertiesWidget::refreshListWidget(QObject* const selectedItem)
{
	QListWidgetItem* item = new QListWidgetItem;
	PropertyItem* propertyItem = new PropertyItem(selectedItem, m_rootContext);
	propertyItem->setItemSource(m_Items);
    propertyItem->setUrlList(m_UrlList);
	if (!propertyItem->isValid()) {
		delete item;
		propertyItem->deleteLater();
		return;
	}
	connect(propertyItem, &PropertyItem::valueApplied, [&] {
		emit idChanged(m_rootContext->nameForObject(selectedItem));
	});
	propertyItem->resize(m_ListWidget->width() - fit(4), propertyItem->height());
	propertyItem->setFixedWidth(m_ListWidget->width() - fit(4));
	item->setSizeHint(QSize(m_ListWidget->width() - fit(4),propertyItem->sizeHint().height()));
	m_ListWidget->addItem(item);
	m_ListWidget->setItemWidget(item, propertyItem);

	// ***

	for (auto property : m_Properties) {
		QListWidgetItem* item = new QListWidgetItem;
		PropertyItem* propertyItem = new PropertyItem(property, m_rootContext);
		propertyItem->setItemSource(m_Items);
        propertyItem->setUrlList(m_UrlList);
		if (!propertyItem->isValid()) {
			delete item;
			propertyItem->deleteLater();
			continue;
		}
		connect(propertyItem, &PropertyItem::valueApplied, [&] {
			emit propertyChanged(property);
		});
		propertyItem->resize(m_ListWidget->width() - fit(4), propertyItem->height());
		propertyItem->setFixedWidth(m_ListWidget->width() - fit(4));
		item->setSizeHint(QSize(m_ListWidget->width() - fit(4),propertyItem->sizeHint().height()));
		m_ListWidget->addItem(item);
		m_ListWidget->setItemWidget(item, propertyItem);
	}
}

void PropertiesWidget::showEvent(QShowEvent* event)
{
	fixItemsGeometry();
	QWidget::showEvent(event);
}

void PropertiesWidget::fixItemsGeometry()
{
	for (int i=0; i<m_ListWidget->count(); i++) {
		QWidget* propertyItem = m_ListWidget->itemWidget(m_ListWidget->item(i));
		propertyItem->resize(m_ListWidget->width() - fit(4), propertyItem->height());
		propertyItem->setFixedWidth(m_ListWidget->width() - fit(4));
		m_ListWidget->item(i)->setSizeHint(QSize(m_ListWidget->width() - fit(4),propertyItem->sizeHint().height()));
	}
}
