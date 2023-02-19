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
const unsigned char guImage_pref_commands[] = {
0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20,
0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7a, 0x7a, 0xf4, 0x00, 0x00, 0x00,
0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xae, 0xce, 0x1c, 0xe9, 0x00, 0x00,
0x00, 0x06, 0x62, 0x4b, 0x47, 0x44, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
0xa0, 0xbd, 0xa7, 0x93, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73,
0x00, 0x00, 0x0d, 0xd7, 0x00, 0x00, 0x0d, 0xd7, 0x01, 0x42, 0x28, 0x9b,
0x78, 0x00, 0x00, 0x00, 0x07, 0x74, 0x49, 0x4d, 0x45, 0x07, 0xda, 0x01,
0x05, 0x07, 0x26, 0x0b, 0x24, 0x8c, 0x00, 0x42, 0x00, 0x00, 0x06, 0x30,
0x49, 0x44, 0x41, 0x54, 0x58, 0xc3, 0xed, 0x97, 0xcf, 0x8f, 0x1c, 0x47,
0x15, 0xc7, 0x3f, 0xaf, 0xaa, 0x67, 0x7f, 0xd8, 0x3b, 0xb1, 0x3d, 0xbb,
0x38, 0x38, 0x90, 0x04, 0x89, 0x08, 0x08, 0x42, 0x08, 0x0b, 0x45, 0xe2,
0x82, 0xb8, 0x38, 0x4a, 0x24, 0x14, 0x8e, 0x20, 0x25, 0x37, 0xfe, 0x07,
0xa4, 0x48, 0x89, 0xe0, 0x98, 0x48, 0x86, 0x13, 0x77, 0x6e, 0x20, 0x82,
0x04, 0x09, 0x32, 0x07, 0x0e, 0x28, 0x51, 0x8e, 0xdc, 0x02, 0x57, 0x62,
0x62, 0x27, 0x18, 0x9c, 0xf5, 0x8e, 0x77, 0x77, 0x7e, 0xf4, 0xf4, 0x74,
0xd5, 0x7b, 0x8f, 0x43, 0x75, 0xcf, 0xce, 0x2c, 0x39, 0x70, 0xe0, 0x80,
0x04, 0xad, 0x99, 0xe9, 0xea, 0xaa, 0xee, 0xae, 0xef, 0xfb, 0xf1, 0x7d,
0xef, 0x3b, 0xf0, 0xbf, 0x7e, 0x48, 0x3f, 0xf8, 0xc6, 0x33, 0xcf, 0x30,
0x59, 0x2a, 0x26, 0x6b, 0x93, 0xff, 0xf9, 0x6d, 0x40, 0x84, 0xe3, 0x7b,
0x77, 0xb8, 0xff, 0xe0, 0x08, 0x80, 0xea, 0x85, 0xef, 0x7d, 0x9f, 0x5b,
0x6f, 0xfc, 0x8c, 0x54, 0x0d, 0x5e, 0xfc, 0xea, 0x17, 0xbf, 0xf4, 0xcd,
0xdd, 0x0b, 0xbb, 0x11, 0xc0, 0xbd, 0xff, 0x01, 0xc7, 0x29, 0x9f, 0xb3,
0x73, 0x59, 0x5a, 0xbb, 0x36, 0xe8, 0xef, 0xa6, 0xac, 0x3b, 0xb8, 0xb8,
0x3b, 0x02, 0x58, 0x59, 0xa4, 0x5e, 0xd4, 0xf9, 0xef, 0xbb, 0x5b, 0xef,
0xde, 0x7f, 0x70, 0xf4, 0xc6, 0xb7, 0x9e, 0x7f, 0x7e, 0x05, 0x6d, 0xe7,
0xbb, 0x2f, 0xbe, 0x74, 0xef, 0x47, 0xaf, 0xbe, 0x3c, 0x9a, 0xce, 0x26,
0xe5, 0x66, 0x77, 0xdc, 0x6d, 0xb5, 0x91, 0x79, 0xd9, 0xd4, 0xdd, 0xba,
0xb5, 0xee, 0x9a, 0x72, 0x9f, 0x59, 0x07, 0xb5, 0x3c, 0x8c, 0xb9, 0x83,
0x39, 0x7e, 0x6e, 0x7c, 0xe1, 0xc2, 0x05, 0x6e, 0xfe, 0xe4, 0xa7, 0x27,
0xbf, 0xff, 0xdd, 0xad, 0xab, 0x40, 0xaa, 0x3a, 0x00, 0x5b, 0x8f, 0x7d,
0xf6, 0x9a, 0xd4, 0x8b, 0x39, 0x0f, 0x8e, 0x0e, 0x99, 0xcf, 0xe7, 0x98,
0x19, 0xee, 0xeb, 0x16, 0x7f, 0xd2, 0xb8, 0x03, 0x0a, 0xdd, 0x9c, 0x9f,
0x79, 0xe7, 0xdc, 0x58, 0x42, 0x60, 0x67, 0x67, 0x87, 0xaa, 0xaa, 0x18,
0x3e, 0x32, 0x84, 0xad, 0xfd, 0x5d, 0xda, 0xf1, 0x0a, 0x00, 0x9a, 0x15,
0x77, 0xe7, 0xf0, 0xf0, 0x01, 0x6f, 0xbd, 0x79, 0x8b, 0x94, 0xf3, 0x66,
0x18, 0xbd, 0x84, 0xd2, 0xbb, 0xb0, 0x9c, 0x21, 0x58, 0x1f, 0xae, 0xff,
0x9e, 0x85, 0xd0, 0x1c, 0x06, 0x55, 0xe0, 0xb9, 0xe7, 0x9e, 0x65, 0x74,
0xe5, 0x0a, 0x6a, 0x86, 0x88, 0xe0, 0xc0, 0x0a, 0x80, 0x99, 0xa3, 0xa6,
0x1c, 0x3d, 0x38, 0xe2, 0xf4, 0x74, 0xca, 0xe3, 0x4f, 0x3c, 0xce, 0xc5,
0x8b, 0x17, 0x11, 0x91, 0x0d, 0x10, 0x8e, 0x63, 0xe6, 0x98, 0x1b, 0x6e,
0x56, 0xc6, 0xe6, 0x98, 0x2a, 0xea, 0x86, 0xa9, 0xa1, 0x66, 0x98, 0x69,
0x19, 0xbb, 0xd3, 0x2c, 0x1a, 0x26, 0xd3, 0x29, 0xe3, 0xf1, 0x43, 0x54,
0x15, 0x33, 0x23, 0x84, 0x80, 0x6e, 0x02, 0x30, 0x34, 0x65, 0x62, 0x8c,
0xec, 0xed, 0x5d, 0xe4, 0x73, 0x4f, 0x3e, 0xc9, 0x70, 0xb8, 0x47, 0x08,
0xd2, 0x65, 0x71, 0x71, 0xa7, 0x99, 0xa1, 0xa6, 0xe5, 0x45, 0x6a, 0x64,
0x2d, 0xe3, 0xac, 0x8a, 0x66, 0x45, 0xd5, 0x50, 0xcd, 0xa4, 0xac, 0x58,
0x37, 0x3f, 0x9f, 0xd7, 0x98, 0xb6, 0xc4, 0x18, 0x48, 0x29, 0xe1, 0xee,
0x9f, 0x00, 0xc0, 0xcd, 0x53, 0x4e, 0x04, 0x09, 0x54, 0x83, 0x01, 0x83,
0xc1, 0x80, 0xed, 0xed, 0x01, 0x21, 0x44, 0x42, 0x08, 0xe0, 0x60, 0xa6,
0x64, 0x33, 0x34, 0x67, 0x72, 0x08, 0x64, 0x14, 0xef, 0xf2, 0xd8, 0xdd,
0xf1, 0x58, 0xce, 0x6a, 0x81, 0x20, 0x86, 0x49, 0x01, 0x6f, 0x38, 0x31,
0x56, 0x04, 0x09, 0xa4, 0x94, 0x29, 0xcc, 0x28, 0xcf, 0xad, 0x00, 0x60,
0x5e, 0xe2, 0x1e, 0xa0, 0x8a, 0x81, 0xd9, 0x7c, 0x46, 0x14, 0x08, 0x55,
0x24, 0x86, 0x88, 0xb9, 0xad, 0xac, 0xd3, 0xac, 0x64, 0x2b, 0x16, 0xf7,
0x1e, 0x50, 0x55, 0x72, 0x2e, 0xdf, 0x95, 0x47, 0xba, 0xf3, 0x72, 0xd1,
0x14, 0x23, 0x02, 0xa4, 0x94, 0x30, 0x75, 0xc2, 0x20, 0xac, 0x03, 0x10,
0xd4, 0x0d, 0xcd, 0x8a, 0x10, 0x08, 0xb1, 0x02, 0x73, 0x0c, 0x47, 0xdc,
0x0b, 0x23, 0x70, 0xe8, 0x68, 0xe9, 0x38, 0x58, 0x5f, 0x17, 0x0a, 0xcd,
0xcc, 0x1c, 0xef, 0x98, 0x63, 0xa6, 0x65, 0x4e, 0x6d, 0x35, 0x17, 0x63,
0x44, 0x10, 0x52, 0x4a, 0xe0, 0x65, 0x9f, 0x35, 0x00, 0x01, 0x33, 0x23,
0xa7, 0x84, 0xe0, 0xc4, 0x10, 0x09, 0x31, 0x10, 0x63, 0x2c, 0x0f, 0x0a,
0xc5, 0xd5, 0x22, 0x54, 0x41, 0x4b, 0x62, 0x4a, 0xc6, 0x73, 0x99, 0xaf,
0xfa, 0x04, 0x75, 0x08, 0x64, 0xc4, 0x2b, 0xc4, 0x12, 0x49, 0x95, 0x7a,
0x36, 0xe3, 0x64, 0x32, 0xe5, 0xca, 0xa5, 0x3d, 0x1c, 0xc8, 0x29, 0x63,
0xee, 0x88, 0xfc, 0x0b, 0x80, 0x2e, 0x04, 0x02, 0x12, 0x84, 0xd9, 0x7c,
0xde, 0x55, 0x4e, 0xe9, 0xe2, 0x55, 0xf8, 0x6e, 0x66, 0x05, 0xac, 0x29,
0x96, 0x95, 0xdc, 0x87, 0x45, 0x0d, 0x55, 0x65, 0xb1, 0x68, 0x98, 0xd7,
0x35, 0xb3, 0xf9, 0x9c, 0x66, 0xd1, 0x94, 0x10, 0x2c, 0x5b, 0x46, 0x97,
0x87, 0xc5, 0x03, 0x39, 0x61, 0x66, 0xc4, 0xb8, 0x01, 0x20, 0xe2, 0xaa,
0xa4, 0x9c, 0x70, 0x07, 0x41, 0x68, 0x9a, 0x86, 0x18, 0x23, 0x8e, 0x13,
0x3a, 0xb4, 0xe0, 0xa8, 0x19, 0xae, 0xa5, 0xfa, 0x95, 0x3c, 0x28, 0x1b,
0xa7, 0x94, 0x39, 0x9d, 0x4c, 0x58, 0x36, 0x0d, 0x29, 0x67, 0x72, 0x2e,
0xc9, 0xe6, 0x66, 0x98, 0x5b, 0xe7, 0x72, 0xa7, 0x4d, 0x09, 0x73, 0x27,
0x74, 0xf4, 0x0e, 0x7d, 0x0e, 0x98, 0x3b, 0x39, 0xe5, 0x52, 0x44, 0x04,
0x52, 0xca, 0xa4, 0x94, 0xf8, 0xf2, 0xd3, 0x4f, 0xa3, 0x9a, 0x69, 0xdb,
0x96, 0x65, 0xdb, 0x92, 0xdb, 0x44, 0xca, 0x99, 0x65, 0x4a, 0xa4, 0xb6,
0xcc, 0xcf, 0xeb, 0x9a, 0x87, 0x27, 0xc7, 0x2c, 0xdb, 0x16, 0xed, 0xca,
0xb4, 0x39, 0x5d, 0xf9, 0x2e, 0xb1, 0x09, 0xa1, 0xdf, 0x23, 0x81, 0x19,
0x84, 0x78, 0xce, 0x03, 0x66, 0xe4, 0x9c, 0xc1, 0x0a, 0x45, 0x16, 0x4d,
0x03, 0x22, 0x8c, 0xc7, 0x0f, 0xb9, 0xfe, 0xb5, 0xeb, 0xfc, 0xe1, 0xed,
0xb7, 0xd9, 0xda, 0xda, 0x2a, 0xc9, 0xe6, 0x86, 0x9b, 0xb3, 0x5c, 0xb6,
0x34, 0xcb, 0x25, 0x29, 0x25, 0xd4, 0x0a, 0x03, 0x4c, 0xcf, 0x31, 0x43,
0x4b, 0x98, 0x1c, 0x01, 0x2f, 0x2c, 0x50, 0x77, 0xaa, 0xc0, 0x9a, 0x07,
0x24, 0xa0, 0x66, 0xe4, 0x9c, 0x30, 0x8a, 0xcb, 0xda, 0x65, 0x4b, 0xbb,
0x5c, 0xf2, 0xde, 0x7b, 0x7f, 0xe2, 0xe8, 0xe8, 0x88, 0x67, 0x6f, 0xdc,
0xa0, 0x9e, 0xd7, 0x2c, 0x9b, 0x86, 0xc5, 0x62, 0xc9, 0x64, 0x3a, 0xa5,
0xae, 0xeb, 0x12, 0xd3, 0xae, 0x02, 0xba, 0x19, 0xda, 0x57, 0x46, 0x2b,
0xe1, 0xea, 0x9b, 0x58, 0x10, 0x29, 0x80, 0x52, 0x46, 0xcd, 0x88, 0x1b,
0x1e, 0x08, 0x01, 0x37, 0xf3, 0x94, 0x72, 0x67, 0x21, 0x2c, 0x9a, 0x06,
0x77, 0x67, 0x34, 0x1a, 0xb1, 0x3f, 0xda, 0xe7, 0x9d, 0x77, 0xde, 0x65,
0xfc, 0xf0, 0x18, 0xc3, 0x71, 0xb5, 0xe2, 0x8d, 0x8e, 0xa2, 0x66, 0xb6,
0xb2, 0x7c, 0xa3, 0x3a, 0xe6, 0x42, 0x47, 0xc1, 0x57, 0x4d, 0xac, 0x4d,
0x2d, 0xb8, 0x21, 0x61, 0x3d, 0x09, 0x63, 0xe7, 0x81, 0x94, 0xb0, 0xac,
0x98, 0x1b, 0xb3, 0xe9, 0x8c, 0x9c, 0x33, 0xdf, 0x79, 0xe1, 0xdb, 0xfc,
0xe6, 0xcd, 0xdf, 0x32, 0x1a, 0x8d, 0xb8, 0xf9, 0xfa, 0x6b, 0xa4, 0x94,
0x98, 0x4c, 0x26, 0xbc, 0x7e, 0xf3, 0xc7, 0x2c, 0x16, 0x4d, 0xd9, 0x7c,
0xad, 0x07, 0xf4, 0xb5, 0x5e, 0x55, 0x57, 0x8d, 0xc9, 0xb4, 0xb4, 0x70,
0xb3, 0x0e, 0xa4, 0xe1, 0x7d, 0x8b, 0xa9, 0xfa, 0x10, 0x98, 0x17, 0x1a,
0xaa, 0x95, 0xae, 0xa8, 0xdd, 0xf5, 0xcf, 0x7f, 0xf9, 0x2b, 0x54, 0x95,
0xba, 0x69, 0x78, 0xf9, 0xd5, 0x1f, 0x02, 0xc5, 0xe2, 0x65, 0x9b, 0x40,
0x4a, 0x6d, 0xc0, 0x03, 0x48, 0xd7, 0x03, 0x25, 0x80, 0x40, 0xec, 0x83,
0x0c, 0xa8, 0x94, 0x42, 0x96, 0x73, 0x67, 0xa4, 0x9f, 0x0b, 0x41, 0x28,
0x21, 0x28, 0x65, 0xd2, 0x0c, 0x57, 0xa3, 0xcd, 0x09, 0x17, 0xa1, 0x4d,
0x67, 0x6d, 0xb9, 0x5e, 0x34, 0x1b, 0xfd, 0xdf, 0x7a, 0x9a, 0x75, 0x9d,
0xb4, 0x5f, 0x3b, 0xaf, 0xe9, 0x44, 0x58, 0x95, 0xf1, 0x36, 0x25, 0xdc,
0x0c, 0x09, 0x6b, 0xbd, 0x20, 0x44, 0x29, 0x2f, 0xc9, 0x99, 0xc5, 0x62,
0xc1, 0xc5, 0xbd, 0x3d, 0x06, 0x12, 0x19, 0x20, 0x67, 0xbd, 0xbd, 0x7b,
0x93, 0xcb, 0xba, 0x40, 0xd8, 0x54, 0x7d, 0xde, 0x9f, 0x3b, 0x31, 0xd2,
0xb7, 0xf2, 0x94, 0x95, 0xe1, 0x23, 0x43, 0x96, 0xcb, 0x65, 0x49, 0x74,
0x35, 0x62, 0x55, 0x9d, 0x01, 0x88, 0x2e, 0xa2, 0xae, 0x21, 0xa5, 0xcc,
0xed, 0x3b, 0x1f, 0x72, 0xf0, 0xe8, 0x25, 0xae, 0x7f, 0xfd, 0x2b, 0xec,
0x5f, 0xbe, 0x42, 0x08, 0x72, 0x26, 0xbf, 0xbc, 0xd7, 0x7b, 0x45, 0x81,
0x58, 0xaf, 0xff, 0x7a, 0x3d, 0xe8, 0x46, 0x4a, 0xca, 0xf8, 0xe1, 0x31,
0x8e, 0x30, 0xa8, 0x22, 0x97, 0x2f, 0x0d, 0xf9, 0xc7, 0xe1, 0x18, 0x70,
0xde, 0xbf, 0x7d, 0x87, 0xcf, 0x3c, 0x76, 0x15, 0xc7, 0x83, 0xe7, 0x2c,
0x2b, 0x00, 0x9a, 0xeb, 0xa1, 0x65, 0x95, 0xe5, 0x32, 0x33, 0x78, 0xf4,
0x84, 0xfd, 0x1b, 0x63, 0xee, 0xff, 0x59, 0x38, 0xbc, 0x6d, 0x54, 0x55,
0x67, 0x91, 0xf5, 0x22, 0xa7, 0x6f, 0x48, 0xac, 0x9a, 0xd3, 0x0a, 0x14,
0x30, 0x9b, 0x2f, 0x38, 0xd8, 0xdf, 0x67, 0x7b, 0x7b, 0x8b, 0xc3, 0xa3,
0x31, 0x7f, 0xbb, 0x77, 0x8f, 0xe1, 0xf0, 0x12, 0x9f, 0x3a, 0x18, 0x71,
0xd7, 0x9c, 0x7a, 0xb1, 0xc0, 0x54, 0xc3, 0x74, 0x32, 0x1e, 0x02, 0xa7,
0x15, 0x40, 0x4e, 0x53, 0xa6, 0x93, 0xe9, 0xb6, 0xb9, 0x71, 0x6d, 0xf8,
0x79, 0xaf, 0xef, 0xff, 0x55, 0x1e, 0x91, 0xab, 0x34, 0xbb, 0x3b, 0x5d,
0xf6, 0x3a, 0x16, 0xb4, 0x6c, 0xdc, 0x75, 0x37, 0x77, 0xc7, 0x10, 0xe8,
0x04, 0x69, 0xaf, 0x94, 0x06, 0x83, 0x01, 0x87, 0x87, 0x87, 0x5d, 0xef,
0x10, 0x62, 0x8c, 0x9c, 0x9c, 0x1c, 0x73, 0x7a, 0x72, 0x8c, 0x61, 0x1e,
0x24, 0x4a, 0x56, 0xab, 0x66, 0xa7, 0x1f, 0xfb, 0x19, 0x0b, 0xcc, 0x4f,
0xee, 0x7e, 0xf0, 0xe1, 0x0f, 0x8e, 0x4f, 0x4e, 0x5e, 0x39, 0x38, 0xd8,
0xbf, 0x56, 0xfd, 0xb1, 0x02, 0x3e, 0x46, 0xf5, 0xde, 0x9a, 0x1c, 0x94,
0x95, 0x2c, 0xdf, 0x90, 0xea, 0x0e, 0x8e, 0x75, 0x6a, 0xc9, 0x59, 0xbb,
0xa1, 0xa3, 0x68, 0x91, 0x2c, 0x2e, 0xa0, 0x59, 0x65, 0x36, 0x9d, 0x7e,
0x54, 0x2f, 0xea, 0xd7, 0x52, 0xca, 0xa7, 0x6b, 0x2c, 0x88, 0xf5, 0xdd,
0x0f, 0xfe, 0xf2, 0xeb, 0x27, 0xe4, 0xa9, 0x2f, 0x08, 0xfe, 0x92, 0x88,
0x20, 0xe5, 0x58, 0x65, 0xf1, 0x7a, 0x6a, 0x3b, 0x20, 0xeb, 0x6a, 0x78,
0xa5, 0x8a, 0xd9, 0x50, 0xc2, 0xb6, 0xa1, 0x9a, 0x4b, 0xb2, 0xcc, 0x67,
0xf5, 0x2f, 0x3e, 0xba, 0x73, 0xfb, 0xad, 0x18, 0xe3, 0x5c, 0x55, 0x37,
0x09, 0x73, 0xf0, 0xe9, 0x6b, 0x57, 0xb6, 0x76, 0x76, 0x9f, 0xd2, 0x9c,
0xb7, 0xcd, 0x74, 0xcb, 0xdd, 0xc3, 0xbf, 0xfd, 0x9f, 0x47, 0xd6, 0x79,
0x70, 0x6e, 0x5d, 0xc4, 0x42, 0x8c, 0x6d, 0x15, 0xab, 0x66, 0xb1, 0x98,
0xbe, 0x3f, 0xbe, 0x7f, 0x78, 0xc2, 0xff, 0x8f, 0xff, 0x96, 0xe3, 0x9f,
0xee, 0x14, 0x87, 0xaa, 0x71, 0x11, 0xb1, 0x9e, 0x00, 0x00, 0x00, 0x00,
0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};
