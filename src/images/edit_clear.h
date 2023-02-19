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
const unsigned char guImage_edit_clear[] = {
0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49,
0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x08, 0x06,
0x00, 0x00, 0x00, 0xe0, 0x77, 0x3d, 0xf8, 0x00, 0x00, 0x00, 0x06, 0x62, 0x4b,
0x47, 0x44, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xa0, 0xbd, 0xa7, 0x93, 0x00,
0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00,
0x00, 0x48, 0x00, 0x46, 0xc9, 0x6b, 0x3e, 0x00, 0x00, 0x04, 0x74, 0x49, 0x44,
0x41, 0x54, 0x48, 0xc7, 0xbd, 0x94, 0x6d, 0x4c, 0x95, 0x65, 0x18, 0xc7, 0x7f,
0xcf, 0xfb, 0x79, 0x81, 0x73, 0x0e, 0xc7, 0x10, 0x05, 0xcc, 0x89, 0xa1, 0x80,
0x66, 0xa0, 0x66, 0x53, 0x73, 0xda, 0x9b, 0xca, 0xd4, 0x2d, 0x37, 0xbf, 0xc8,
0x87, 0xd6, 0x07, 0xa3, 0xb6, 0x74, 0xd6, 0x97, 0x6a, 0x73, 0x38, 0x1a, 0x6d,
0x69, 0xae, 0x51, 0x6d, 0x65, 0xa1, 0x33, 0xb3, 0xd5, 0x56, 0xf6, 0xa2, 0xd3,
0x66, 0xb5, 0xb0, 0x89, 0xf9, 0x02, 0xa8, 0x01, 0x02, 0x22, 0xa8, 0x13, 0xcc,
0x37, 0xe4, 0xe0, 0x39, 0xc0, 0x39, 0x9c, 0xe7, 0x79, 0xce, 0x79, 0xee, 0x3e,
0x78, 0x74, 0xcd, 0x0d, 0xa7, 0x66, 0x5d, 0xdb, 0x7f, 0xf7, 0xee, 0xdd, 0xdb,
0xff, 0x77, 0xdd, 0xff, 0x6b, 0xf7, 0x0d, 0xff, 0x71, 0x49, 0x00, 0xd5, 0x2b,
0x65, 0x3f, 0xb0, 0x4a, 0x96, 0xa4, 0xd7, 0x84, 0x24, 0x05, 0x24, 0xe1, 0x7c,
0xb4, 0xf6, 0x6b, 0x67, 0xdd, 0x83, 0x00, 0x28, 0x00, 0xa5, 0xd3, 0xa4, 0xca,
0xec, 0xf1, 0x45, 0xeb, 0x66, 0xcc, 0x5f, 0x1e, 0xcc, 0xcb, 0x9f, 0xa2, 0xf7,
0x5e, 0xe9, 0x99, 0xfe, 0x74, 0xbe, 0x79, 0xf6, 0x97, 0x56, 0xd1, 0xf6, 0x6f,
0x01, 0x32, 0x80, 0xa2, 0x68, 0x2f, 0x15, 0xcd, 0x5c, 0x60, 0xc4, 0xce, 0xd7,
0x62, 0x87, 0xcf, 0x52, 0xfc, 0xc4, 0x42, 0xaf, 0x2c, 0x51, 0x5d, 0xbd, 0x52,
0xd6, 0x1e, 0x08, 0xc0, 0x49, 0xda, 0x96, 0x93, 0x48, 0x20, 0x2b, 0x3a, 0x43,
0x97, 0x5b, 0xf0, 0x7a, 0x3d, 0x64, 0xe5, 0x4c, 0xf4, 0x03, 0x2f, 0x6f, 0x5d,
0xcf, 0xb2, 0xed, 0x55, 0xfa, 0xe1, 0xcf, 0xca, 0xb9, 0x2f, 0x98, 0x9c, 0x5a,
0xb7, 0x9f, 0x6c, 0xf8, 0x75, 0xd8, 0x97, 0xb7, 0x00, 0x6f, 0xd6, 0x14, 0x06,
0x7a, 0x1a, 0x28, 0x28, 0x99, 0xe7, 0x31, 0x82, 0x6c, 0xd2, 0x5d, 0xe9, 0xdf,
0x08, 0xc1, 0x0c, 0x29, 0x9b, 0x09, 0xf7, 0x3d, 0x83, 0x45, 0x53, 0xa5, 0xfd,
0xf1, 0x68, 0xd8, 0x7b, 0xa9, 0xe7, 0xcc, 0xe3, 0x0f, 0x17, 0xce, 0x55, 0x15,
0x5d, 0xe7, 0x42, 0xff, 0x09, 0xd4, 0x0c, 0x59, 0x5d, 0x56, 0xb6, 0x5a, 0x0b,
0xf7, 0x5d, 0x8d, 0x0d, 0x84, 0xae, 0x1d, 0xdd, 0x53, 0x47, 0xfb, 0xbd, 0x02,
0xa4, 0x7f, 0x6e, 0xaa, 0x57, 0xca, 0x2b, 0x54, 0x4d, 0xdd, 0x31, 0x6a, 0x72,
0x9a, 0xdb, 0x97, 0xe9, 0x65, 0xf6, 0x33, 0xa5, 0xb8, 0xf4, 0x34, 0x9a, 0x5e,
0xdf, 0x4b, 0xff, 0xee, 0xae, 0xa4, 0x94, 0x94, 0x36, 0x2f, 0x75, 0x9c, 0x35,
0xf7, 0x13, 0x11, 0x00, 0xfe, 0xc9, 0xce, 0x09, 0xf7, 0x04, 0x2b, 0xf2, 0x50,
0xae, 0x21, 0x4a, 0x66, 0x4f, 0x61, 0xf8, 0xc2, 0x71, 0x1a, 0x16, 0x7f, 0x0e,
0x07, 0x7a, 0x29, 0x1a, 0x93, 0xab, 0x24, 0x65, 0xf1, 0x4a, 0x4d, 0x05, 0x2f,
0xd4, 0x54, 0xdc, 0xfd, 0x3c, 0x6e, 0x01, 0xb6, 0x56, 0x30, 0x57, 0xc0, 0x89,
0xa2, 0xe2, 0x47, 0x32, 0x0b, 0x8b, 0x0b, 0xa4, 0x70, 0x63, 0x27, 0xad, 0xcf,
0xb7, 0x10, 0x0c, 0x69, 0xe4, 0x7a, 0x03, 0x74, 0x5c, 0xbb, 0x44, 0xa0, 0xbc,
0x40, 0xcd, 0x1e, 0x37, 0xee, 0x13, 0x45, 0x95, 0xba, 0x6b, 0x2a, 0x58, 0x71,
0x4f, 0x11, 0x6d, 0xab, 0xe4, 0x27, 0x21, 0x69, 0x0b, 0x03, 0x41, 0x7f, 0xcc,
0xd3, 0x66, 0x79, 0x9c, 0x2f, 0xa2, 0x6a, 0x41, 0x66, 0x0e, 0x71, 0xdb, 0xe2,
0x5c, 0xa4, 0x8f, 0xf1, 0x1b, 0xf2, 0xc9, 0x7a, 0x2e, 0x17, 0x77, 0xfa, 0x4c,
0x22, 0xfd, 0x57, 0x68, 0xac, 0xfb, 0x39, 0x16, 0xe9, 0xbf, 0xda, 0x66, 0xdb,
0x4e, 0x59, 0x79, 0x15, 0x67, 0xee, 0x6a, 0x06, 0x5b, 0x2a, 0xc9, 0xc5, 0xa1,
0x70, 0xcc, 0x7b, 0xf2, 0xb7, 0x93, 0x82, 0x63, 0x02, 0x21, 0x33, 0x4a, 0x48,
0x35, 0x99, 0xf0, 0xa1, 0xc0, 0xf3, 0xb0, 0x82, 0x61, 0x78, 0xd0, 0x74, 0x2f,
0xae, 0xb4, 0xe9, 0xa8, 0x9e, 0x12, 0x2e, 0x9e, 0x3f, 0x25, 0x8e, 0xd4, 0xee,
0x8d, 0x59, 0xe6, 0xd0, 0xbb, 0x8e, 0x23, 0x36, 0x96, 0x57, 0x91, 0xb8, 0x23,
0xe0, 0x66, 0xed, 0x95, 0xe5, 0x8a, 0xa4, 0x22, 0x2a, 0xd3, 0xe7, 0xe7, 0x49,
0xd9, 0x6f, 0x74, 0x4b, 0xb2, 0x4b, 0xa0, 0xaa, 0x06, 0x86, 0xe1, 0x41, 0x37,
0x3c, 0xa8, 0xaa, 0x81, 0xa6, 0xa6, 0xa1, 0x79, 0x67, 0x60, 0x8b, 0x3c, 0xfe,
0xac, 0xdf, 0x3f, 0x7c, 0xb6, 0xf5, 0x60, 0xd8, 0xb2, 0xcc, 0xb7, 0x81, 0xed,
0xe5, 0x55, 0x98, 0x77, 0x04, 0x00, 0x6c, 0x59, 0x8f, 0xb9, 0xa4, 0xec, 0x55,
0x3d, 0xda, 0xf7, 0x25, 0xc2, 0x89, 0xa2, 0xaa, 0x3a, 0x86, 0xe1, 0x45, 0x37,
0xbc, 0x68, 0x5a, 0x10, 0x84, 0x89, 0xdb, 0xed, 0x41, 0xd1, 0x02, 0x60, 0x94,
0x30, 0x30, 0xe8, 0xa5, 0xb9, 0x7e, 0x5f, 0xec, 0xdc, 0xa9, 0x46, 0x5b, 0x08,
0xe7, 0x7d, 0xc7, 0x61, 0x53, 0x79, 0x15, 0x71, 0x79, 0x24, 0x80, 0xac, 0xf0,
0x57, 0x6c, 0x28, 0x84, 0xa2, 0x8f, 0x06, 0x40, 0x08, 0x81, 0x40, 0xdc, 0x38,
0x93, 0xe3, 0xa8, 0x46, 0x2e, 0xc3, 0x51, 0x13, 0x91, 0xb4, 0x90, 0xcc, 0x63,
0xf8, 0x5d, 0xcd, 0x3c, 0xf9, 0xec, 0x12, 0xcf, 0xd2, 0xb2, 0xb5, 0x7e, 0x21,
0xc9, 0xb7, 0x3e, 0xca, 0x11, 0x01, 0xc2, 0x61, 0x57, 0xdf, 0xe5, 0x1e, 0x47,
0x77, 0xe5, 0x03, 0x82, 0x84, 0x9d, 0x40, 0x24, 0x1d, 0xc0, 0x20, 0x99, 0x48,
0xa2, 0x29, 0x51, 0x34, 0xcf, 0x64, 0xc2, 0xfd, 0x03, 0x38, 0x42, 0x07, 0x04,
0xb2, 0x92, 0xc6, 0x99, 0xb6, 0x7a, 0x53, 0x95, 0xf9, 0xaa, 0xbc, 0x8a, 0xf8,
0x1d, 0x01, 0x8e, 0xc3, 0xf7, 0x5d, 0xed, 0x4d, 0x31, 0xcd, 0x3d, 0x09, 0x49,
0x72, 0x21, 0x84, 0x83, 0x65, 0x25, 0x10, 0x42, 0x90, 0x74, 0xd2, 0x19, 0x8e,
0x86, 0x71, 0xb9, 0x65, 0x5c, 0xbe, 0x22, 0x7a, 0x2f, 0x47, 0x48, 0x6a, 0xf3,
0xe8, 0x68, 0x3e, 0x92, 0xe8, 0x3c, 0x59, 0xdf, 0x63, 0xdb, 0xce, 0xea, 0x9b,
0x3e, 0xca, 0x48, 0x09, 0xed, 0xa9, 0x23, 0x54, 0x3a, 0xc7, 0x5e, 0x96, 0x31,
0x2a, 0x38, 0xd6, 0xeb, 0x0b, 0x48, 0x8e, 0x7d, 0x11, 0x27, 0xa1, 0x20, 0xe3,
0xe0, 0xf1, 0xe5, 0x11, 0x1f, 0x1a, 0x40, 0x72, 0x06, 0xf1, 0x04, 0x67, 0x91,
0x70, 0x72, 0x68, 0x39, 0xfa, 0xa3, 0xdd, 0xd4, 0x58, 0xd7, 0x79, 0x3d, 0x92,
0x5c, 0xb0, 0x66, 0x23, 0xe1, 0x91, 0x00, 0x32, 0x90, 0x0e, 0x64, 0x00, 0x41,
0x9f, 0x97, 0x6e, 0x9f, 0xde, 0xbf, 0x74, 0xd2, 0xb4, 0xc5, 0x6a, 0xc2, 0xec,
0x46, 0xd5, 0xfd, 0xc4, 0x87, 0x06, 0xd1, 0x75, 0x9b, 0xb4, 0x51, 0xb3, 0xe9,
0xbb, 0x12, 0x42, 0xd6, 0xf3, 0x38, 0xfc, 0xfb, 0x6e, 0xb3, 0xa3, 0xfd, 0x74,
0xfd, 0x3b, 0x35, 0x89, 0x17, 0xbf, 0xfb, 0x8d, 0x78, 0xca, 0xc7, 0xba, 0x1d,
0x20, 0x01, 0x69, 0x29, 0xa5, 0x03, 0xe9, 0x27, 0xbb, 0xb0, 0xe6, 0x4c, 0x1b,
0x2e, 0xd6, 0xe4, 0xc8, 0xe8, 0xec, 0x89, 0x8b, 0x64, 0x2b, 0xd6, 0x81, 0xe1,
0xce, 0x25, 0x36, 0x68, 0x11, 0xc8, 0x9a, 0x8f, 0x65, 0x09, 0x6a, 0xf7, 0xec,
0xb0, 0x5a, 0xda, 0xaf, 0xee, 0x7b, 0xeb, 0x83, 0xc4, 0x86, 0xc8, 0x10, 0xc9,
0x94, 0x8f, 0x48, 0xc9, 0xba, 0xfd, 0x06, 0x3a, 0x60, 0xa4, 0x56, 0x1d, 0xd0,
0xdb, 0xcf, 0xd1, 0x5c, 0x90, 0x13, 0x7a, 0xca, 0xef, 0xf7, 0x79, 0x32, 0x46,
0x17, 0x4a, 0x2e, 0xdf, 0xa3, 0x98, 0x31, 0x9b, 0x53, 0x4d, 0xb5, 0xc9, 0xc3,
0x07, 0x0e, 0x45, 0x77, 0xd5, 0x9a, 0xd5, 0x9f, 0xee, 0x14, 0xbb, 0x05, 0x98,
0xdc, 0x90, 0x95, 0xd2, 0x20, 0xe0, 0xdc, 0x0e, 0xb0, 0x80, 0x44, 0xaa, 0x8b,
0x24, 0x90, 0x18, 0x88, 0x32, 0xd4, 0x1b, 0xe2, 0x8f, 0xb1, 0x69, 0xdd, 0xa5,
0x99, 0xd9, 0xf9, 0x7a, 0x7f, 0x6f, 0x07, 0xf5, 0x07, 0x0f, 0x59, 0xa7, 0x3b,
0xc3, 0x75, 0x1b, 0xb7, 0x89, 0x37, 0x8f, 0xb5, 0xd3, 0x0a, 0x44, 0x80, 0x81,
0x94, 0x69, 0x08, 0xb8, 0x9e, 0xf2, 0x19, 0xf9, 0xa1, 0xa5, 0x72, 0xd4, 0x01,
0x17, 0xa0, 0xaf, 0x5a, 0xce, 0x63, 0xb3, 0xa6, 0xf2, 0x83, 0x9d, 0xe4, 0x78,
0x6b, 0x17, 0x1f, 0x6f, 0xde, 0x49, 0x03, 0x60, 0xa7, 0x9a, 0x8a, 0xa7, 0xba,
0x4f, 0xa4, 0xa2, 0xf9, 0xff, 0xea, 0x6f, 0xb8, 0xdf, 0xdb, 0x4c, 0x7f, 0xa4,
0x27, 0xc0, 0x00, 0x00, 0x00, 0x22, 0x7a, 0x54, 0x58, 0x74, 0x53, 0x6f, 0x66,
0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x00, 0x78, 0xda, 0x2b, 0x2f, 0x2f, 0xd7,
0xcb, 0xcc, 0xcb, 0x2e, 0x4e, 0x4e, 0x2c, 0x48, 0xd5, 0xcb, 0x2f, 0x4a, 0x07,
0x00, 0x36, 0xd8, 0x06, 0x58, 0x10, 0x53, 0xca, 0x5c, 0x00, 0x00, 0x00, 0x00,
0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
