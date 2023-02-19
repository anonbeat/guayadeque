// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 3, or (at your option)
//    any later version.
//
//    This Program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; see the file LICENSE.  If not, write to
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
const unsigned char guImage_player_highlight_stop[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x1e,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x3b, 0x30, 0xae, 0xa2, 0x00, 0x00, 0x00,
  0x04, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x13,
  0xc6, 0x00, 0x00, 0x13, 0xc6, 0x01, 0xf8, 0x22, 0x00, 0xbf, 0x00, 0x00,
  0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
  0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2e, 0x69, 0x6e, 0x6b, 0x73, 0x63,
  0x61, 0x70, 0x65, 0x2e, 0x6f, 0x72, 0x67, 0x9b, 0xee, 0x3c, 0x1a, 0x00,
  0x00, 0x03, 0x09, 0x49, 0x44, 0x41, 0x54, 0x48, 0x89, 0xbd, 0xd7, 0x4f,
  0x88, 0x55, 0x65, 0x18, 0xc7, 0xf1, 0xcf, 0x73, 0x1c, 0xa2, 0xa2, 0x1c,
  0x61, 0xfe, 0x68, 0x64, 0xff, 0x6b, 0x15, 0x16, 0x51, 0xb6, 0xc9, 0x34,
  0xac, 0x16, 0xd1, 0x1f, 0x6a, 0xd5, 0x54, 0x46, 0x25, 0x15, 0xe5, 0xc2,
  0x90, 0x96, 0x52, 0x2b, 0xdb, 0x16, 0x8d, 0xa5, 0x0b, 0x41, 0x88, 0x22,
  0x6b, 0x55, 0x54, 0x04, 0x81, 0x26, 0xcd, 0x24, 0x6d, 0x6c, 0x13, 0xb4,
  0xa9, 0x50, 0x08, 0x07, 0xc6, 0x71, 0x24, 0x67, 0x28, 0xa5, 0x82, 0x79,
  0x5b, 0xbc, 0xef, 0x9d, 0xb9, 0xde, 0xee, 0xe5, 0x9e, 0x19, 0xaf, 0xf3,
  0x83, 0xc3, 0x81, 0xf7, 0xbc, 0xef, 0xf9, 0x3e, 0xe7, 0xfd, 0xf3, 0x7b,
  0x9e, 0x13, 0x6a, 0x28, 0xa5, 0x14, 0xe8, 0xc7, 0x00, 0x86, 0xb1, 0x16,
  0xab, 0xcb, 0xe3, 0x49, 0x4c, 0x60, 0x0a, 0xd3, 0x98, 0x8d, 0x88, 0xd4,
  0xed, 0x9d, 0xd1, 0x05, 0x58, 0x15, 0xc8, 0x46, 0x6c, 0xc6, 0x06, 0xdc,
  0xd2, 0xa1, 0xfb, 0xaf, 0xf8, 0x1e, 0x07, 0xcb, 0xfd, 0x44, 0x44, 0xcc,
  0x2d, 0x1a, 0x9c, 0x52, 0x1a, 0x2e, 0xc0, 0x6d, 0xb8, 0x0f, 0xff, 0xe0,
  0x47, 0x1c, 0xc5, 0x49, 0x9c, 0x2e, 0x5d, 0x07, 0xe4, 0xaf, 0xbf, 0x0b,
  0x77, 0xe2, 0x12, 0x1c, 0xc6, 0x1e, 0x8c, 0x45, 0xc4, 0xa9, 0xda, 0xe0,
  0x94, 0xd2, 0xf5, 0x78, 0x1d, 0xaf, 0xe0, 0x2c, 0x3e, 0xc4, 0x97, 0xf8,
  0xbb, 0x53, 0xa0, 0x45, 0x97, 0xe2, 0x51, 0x6c, 0xc1, 0xe5, 0x05, 0xfe,
  0x6e, 0x44, 0x1c, 0xeb, 0x0a, 0x4e, 0x29, 0xdd, 0x86, 0xb7, 0xf0, 0x08,
  0xbe, 0xc1, 0x6e, 0xfc, 0xd5, 0x05, 0xd8, 0xaa, 0x2b, 0xb0, 0x1d, 0x0f,
  0xe2, 0x2b, 0xec, 0x8c, 0x88, 0x9f, 0x3a, 0x82, 0x53, 0x4a, 0x37, 0x62,
  0x14, 0x0f, 0x61, 0x1f, 0x3e, 0x59, 0x24, 0xb0, 0x55, 0x23, 0x78, 0x09,
  0x5f, 0x63, 0x7b, 0x44, 0x1c, 0x6f, 0x3c, 0xe8, 0x6b, 0x82, 0x0e, 0x63,
  0x07, 0x1e, 0xee, 0x11, 0x54, 0x79, 0xc7, 0x0a, 0xbc, 0x88, 0x63, 0x29,
  0xa5, 0x5d, 0x8d, 0x35, 0xaf, 0x0a, 0xb4, 0xc2, 0x26, 0x79, 0x4d, 0x0f,
  0xe1, 0x40, 0x0f, 0xa0, 0x0d, 0x7d, 0x2c, 0x6f, 0xb6, 0x6d, 0xd8, 0x98,
  0x52, 0x5a, 0x41, 0x99, 0xea, 0x94, 0xd2, 0xb5, 0xf8, 0x00, 0xeb, 0xf1,
  0x34, 0x66, 0x9b, 0x06, 0xde, 0x5e, 0x02, 0xea, 0xaf, 0x09, 0x9a, 0xc1,
  0x5e, 0x34, 0xaf, 0x69, 0x7f, 0x09, 0xe0, 0x07, 0x6c, 0x8d, 0x88, 0xdf,
  0xfb, 0x8a, 0x39, 0x6c, 0x2a, 0xd7, 0xfe, 0x16, 0x28, 0x79, 0xfa, 0xaf,
  0xab, 0x09, 0x85, 0xab, 0xca, 0x98, 0x17, 0x5a, 0x82, 0xf9, 0x14, 0xcf,
  0x63, 0x43, 0x4a, 0xe9, 0x40, 0x55, 0xa2, 0x79, 0x00, 0x49, 0x3e, 0x32,
  0xad, 0xaa, 0xfb, 0xa5, 0xcd, 0x5a, 0xd5, 0xa6, 0xed, 0x73, 0xcc, 0x15,
  0xd6, 0xca, 0x0a, 0x83, 0xb8, 0x47, 0x9e, 0x9a, 0x99, 0x25, 0x40, 0xea,
  0x6a, 0x16, 0x3f, 0xcb, 0xee, 0x37, 0x58, 0xc9, 0xde, 0x7b, 0x93, 0xec,
  0x48, 0x17, 0x5b, 0x47, 0x71, 0x33, 0x86, 0x2a, 0x5c, 0x53, 0x1a, 0xa7,
  0x96, 0x01, 0x7c, 0x52, 0xde, 0xd0, 0x6b, 0x2b, 0x0c, 0x95, 0xc6, 0xe9,
  0x65, 0x00, 0x37, 0xfc, 0x7d, 0x75, 0x25, 0x6f, 0x2a, 0xba, 0x64, 0xaa,
  0x1e, 0x69, 0x9e, 0x51, 0xa1, 0x91, 0x3d, 0x06, 0x97, 0x01, 0x3c, 0x50,
  0xee, 0x93, 0x15, 0x4e, 0xc8, 0x5f, 0x3d, 0xd4, 0xb9, 0x7f, 0xcf, 0x34,
  0x5c, 0x58, 0x13, 0x95, 0xbc, 0xe0, 0xbf, 0xc9, 0xae, 0x75, 0xb1, 0xb5,
  0x1e, 0xbf, 0x60, 0xaa, 0x92, 0x37, 0xd5, 0x11, 0xac, 0xb3, 0x34, 0xb3,
  0xa8, 0xab, 0x95, 0xb8, 0xb5, 0xb0, 0x4e, 0x57, 0xf2, 0xc1, 0x3e, 0x28,
  0x2f, 0xfc, 0x63, 0x6d, 0x06, 0x2c, 0xc5, 0x54, 0xce, 0xb4, 0x69, 0x7b,
  0xbc, 0xdc, 0x0f, 0x45, 0xc4, 0x4c, 0x5f, 0x44, 0xa4, 0x94, 0xd2, 0xb8,
  0x9c, 0x41, 0x46, 0xe4, 0xc4, 0xfd, 0x47, 0xd3, 0x80, 0xb7, 0xe5, 0x24,
  0xd1, 0xce, 0x06, 0x3b, 0x41, 0xf7, 0xb6, 0xb4, 0xad, 0xc2, 0x93, 0xf8,
  0x16, 0xe3, 0x2c, 0x64, 0xa7, 0x0a, 0x4f, 0xc8, 0xf9, 0x73, 0x0c, 0xbb,
  0x2c, 0x1c, 0xb3, 0x0b, 0x55, 0xe0, 0x4d, 0xd9, 0x2a, 0x47, 0xf0, 0x59,
  0x44, 0xcc, 0x55, 0x50, 0xaa, 0xc1, 0xb1, 0x12, 0xe9, 0x66, 0x39, 0x35,
  0xf6, 0x4a, 0xcf, 0xca, 0xc5, 0xe2, 0x7b, 0xf8, 0xae, 0x51, 0x79, 0xce,
  0x57, 0x20, 0x11, 0x71, 0x2a, 0xa5, 0xf4, 0x0e, 0x6e, 0xc0, 0x56, 0x39,
  0x93, 0x5c, 0x48, 0x41, 0x10, 0x78, 0x0a, 0xcf, 0xe1, 0x0b, 0x8c, 0x46,
  0xc4, 0xbc, 0x3b, 0xf6, 0x9d, 0xd7, 0x33, 0xe2, 0x78, 0x4a, 0x69, 0xa7,
  0x3c, 0xcd, 0x2f, 0x97, 0x20, 0x46, 0xf1, 0xe7, 0x22, 0xa1, 0x57, 0xe2,
  0x35, 0xdc, 0x5f, 0xa0, 0x6f, 0x34, 0xd7, 0x5b, 0x8d, 0xa8, 0xfe, 0xa7,
  0x52, 0xde, 0xee, 0x90, 0xcb, 0x95, 0x73, 0xf8, 0x48, 0xce, 0xd5, 0xe7,
  0xba, 0x00, 0x2f, 0x93, 0x4f, 0xc6, 0x33, 0x72, 0xa9, 0xfb, 0x3e, 0x76,
  0xd7, 0x2a, 0x6f, 0x9b, 0xe0, 0x43, 0x72, 0x41, 0xff, 0xaa, 0xbc, 0xee,
  0xff, 0x5a, 0x28, 0xe8, 0x27, 0x9d, 0x5f, 0xd0, 0xaf, 0xc1, 0xdd, 0xb8,
  0x43, 0x9e, 0xc5, 0xc3, 0x05, 0x3a, 0xde, 0x3c, 0xbd, 0xb5, 0xc0, 0x05,
  0x5e, 0xe1, 0x6a, 0xdc, 0x2b, 0x4f, 0x5b, 0xe3, 0x17, 0xa6, 0x75, 0x5c,
  0x92, 0x1d, 0xe9, 0x88, 0x5c, 0x2c, 0x8e, 0x63, 0x62, 0x49, 0xbf, 0x30,
  0x6d, 0x82, 0xe8, 0x97, 0x13, 0xc9, 0xa0, 0xfc, 0x3f, 0xb5, 0xa6, 0x3c,
  0x9a, 0x94, 0xfd, 0x7e, 0x1a, 0xd3, 0x11, 0x51, 0xcb, 0x70, 0xfe, 0x03,
  0x4c, 0x4f, 0xdf, 0xbf, 0xa2, 0xa2, 0xf3, 0x3f, 0x00, 0x00, 0x00, 0x00,
  0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
