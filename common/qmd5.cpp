/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "qmd5.h"

#include <QDataStream>

enum Operation {
  OperationF,
  OperationG,
  OperationH,
  OperationI
};

// these values are taken from RFC1321
static const Q_UINT32 T[64] = {
  0xd76aa478, // 1, 0 based accessed
  0xe8c7b756, // 2
  0x242070db, // 3
  0xc1bdceee, // 4
  0xf57c0faf, // 5
  0x4787c62a, // 6
  0xa8304613, // 7
  0xfd469501, // 8
  0x698098d8, // 9
  0x8b44f7af, // 10
  0xffff5bb1, // 11
  0x895cd7be, // 12
  0x6b901122, // 13
  0xfd987193, // 14
  0xa679438e, // 15
  0x49b40821, // 16
  0xf61e2562, // 17
  0xc040b340, // 18
  0x265e5a51, // 19
  0xe9b6c7aa, // 20
  0xd62f105d, // 21
  0x02441453, // 22
  0xd8a1e681, // 23
  0xe7d3fbc8, // 24
  0x21e1cde6, // 25
  0xc33707d6, // 26
  0xf4d50d87, // 27
  0x455a14ed, // 28
  0xa9e3e905, // 29
  0xfcefa3f8, // 30
  0x676f02d9, // 31
  0x8d2a4c8a, // 32
  0xfffa3942, // 33
  0x8771f681, // 34
  0x6d9d6122, // 35
  0xfde5380c, // 36
  0xa4beea44, // 37
  0x4bdecfa9, // 38
  0xf6bb4b60, // 39
  0xbebfbc70, // 40
  0x289b7ec6, // 41
  0xeaa127fa, // 42
  0xd4ef3085, // 43
  0x04881d05, // 44
  0xd9d4d039, // 45
  0xe6db99e5, // 46
  0x1fa27cf8, // 47
  0xc4ac5665, // 48
  0xf4292244, // 49
  0x432aff97, // 50
  0xab9423a7, // 51
  0xfc93a039, // 52
  0x655b59c3, // 53
  0x8f0ccc92, // 54
  0xffeff47d, // 55
  0x85845dd1, // 56
  0x6fa87e4f, // 57
  0xfe2ce6e0, // 58
  0xa3014314, // 59
  0x4e0811a1, // 60
  0xf7537e82, // 61
  0xbd3af235, // 62
  0x2ad7d2bb, // 63
  0xeb86d391  // 64
};

static void md5Round( Operation func, Q_UINT32 & a, Q_UINT32 & b, Q_UINT32 & c, Q_UINT32 & d, Q_UINT32 k, int s, int i)
{
  Q_UINT32 o = 0, t = 0;
  switch(func)
  {
    case OperationF:
      o = (b & c) | (~b & d);
      break;
    case OperationG:
      o = (b & d) | (c & ~d);
      break;
    case OperationH:
      o = b ^ c ^ d;
      break;
    case OperationI:
      o = c ^ (b | ~d);
      break;
  };

  t = a + o + k + T[i-1];
  t = ((t << s) | (t >> (32 - s)));
  a = b + t;
}


QString QMd5(const QString & message)
{
  QByteArray msg = message.toUtf8();
  return QMd5(msg);
}

QString QMd5(const QByteArray & message)
{
  // create the data array and input stream for it
  QByteArray data;
  QDataStream dataIN(&data, QIODevice::WriteOnly);
  dataIN.setByteOrder(QDataStream::LittleEndian);

  // add the data to the stream
  Q_UINT32 len = message.size();
  dataIN.writeRawBytes(message.data(), len);
  
  // Calculate the size of the pad required.
  // The pad must be at least 1 bit long and
  // should make the overall data size a multiple
  // of 512 bits minus the size for the 64 bit
  // length value.
  Q_INT32 pad = 64 - (len % 64) - 8;
  if(pad <= 0)
    pad += 64;

  // populate the data array with the pad values
  int i;
  dataIN << (Q_UINT8)0x80;
  for(pad--; pad > 0; pad--)
    dataIN << (Q_UINT8)0x00;

  // create a 64bit count of the number of bits in the data array
  Q_UINT64 bitLen = (Q_UINT64)len << 3;
  dataIN << bitLen;

  // Initialize the MD state variables
  Q_UINT32 stateA = 0x67452301;
  Q_UINT32 stateB = 0xefcdab89;
  Q_UINT32 stateC = 0x98badcfe;
  Q_UINT32 stateD = 0x10325476;

  // setup a data stream for reading the data
  QDataStream dataOUT(&data, QIODevice::ReadOnly);
  dataOUT.setByteOrder(QDataStream::LittleEndian);

  // process the message in 16-word blocks
  Q_UINT32 X[16];
  for(i = 0; i * 64 < data.size(); i++)
  {
    Q_UINT32 a = stateA;
    Q_UINT32 b = stateB;
    Q_UINT32 c = stateC;
    Q_UINT32 d = stateD;

    for(int j = 0; j < 16; j++)
      dataOUT >> X[j];

    // round 1
    md5Round( OperationF, a, b, c, d, X[ 0],  7,  1);
    md5Round( OperationF, d, a, b, c, X[ 1], 12,  2);
    md5Round( OperationF, c, d, a, b, X[ 2], 17,  3);
    md5Round( OperationF, b, c, d, a, X[ 3], 22,  4);
    md5Round( OperationF, a, b, c, d, X[ 4],  7,  5);
    md5Round( OperationF, d, a, b, c, X[ 5], 12,  6);
    md5Round( OperationF, c, d, a, b, X[ 6], 17,  7);
    md5Round( OperationF, b, c, d, a, X[ 7], 22,  8);
    md5Round( OperationF, a, b, c, d, X[ 8],  7,  9);
    md5Round( OperationF, d, a, b, c, X[ 9], 12, 10);
    md5Round( OperationF, c, d, a, b, X[10], 17, 11);
    md5Round( OperationF, b, c, d, a, X[11], 22, 12);
    md5Round( OperationF, a, b, c, d, X[12],  7, 13);
    md5Round( OperationF, d, a, b, c, X[13], 12, 14);
    md5Round( OperationF, c, d, a, b, X[14], 17, 15);
    md5Round( OperationF, b, c, d, a, X[15], 22, 16);

    // round 2
    md5Round( OperationG, a, b, c, d, X[ 1],  5, 17);
    md5Round( OperationG, d, a, b, c, X[ 6],  9, 18);
    md5Round( OperationG, c, d, a, b, X[11], 14, 19);
    md5Round( OperationG, b, c, d, a, X[ 0], 20, 20);
    md5Round( OperationG, a, b, c, d, X[ 5],  5, 21);
    md5Round( OperationG, d, a, b, c, X[10],  9, 22);
    md5Round( OperationG, c, d, a, b, X[15], 14, 23);
    md5Round( OperationG, b, c, d, a, X[ 4], 20, 24);
    md5Round( OperationG, a, b, c, d, X[ 9],  5, 25);
    md5Round( OperationG, d, a, b, c, X[14],  9, 26);
    md5Round( OperationG, c, d, a, b, X[ 3], 14, 27);
    md5Round( OperationG, b, c, d, a, X[ 8], 20, 28);
    md5Round( OperationG, a, b, c, d, X[13],  5, 29);
    md5Round( OperationG, d, a, b, c, X[ 2],  9, 30);
    md5Round( OperationG, c, d, a, b, X[ 7], 14, 31);
    md5Round( OperationG, b, c, d, a, X[12], 20, 32);

    // round 3
    md5Round( OperationH, a, b, c, d, X[ 5],  4, 33);
    md5Round( OperationH, d, a, b, c, X[ 8], 11, 34);
    md5Round( OperationH, c, d, a, b, X[11], 16, 35);
    md5Round( OperationH, b, c, d, a, X[14], 23, 36);
    md5Round( OperationH, a, b, c, d, X[ 1],  4, 37);
    md5Round( OperationH, d, a, b, c, X[ 4], 11, 38);
    md5Round( OperationH, c, d, a, b, X[ 7], 16, 39);
    md5Round( OperationH, b, c, d, a, X[10], 23, 40);
    md5Round( OperationH, a, b, c, d, X[13],  4, 41);
    md5Round( OperationH, d, a, b, c, X[ 0], 11, 42);
    md5Round( OperationH, c, d, a, b, X[ 3], 16, 43);
    md5Round( OperationH, b, c, d, a, X[ 6], 23, 44);
    md5Round( OperationH, a, b, c, d, X[ 9],  4, 45);
    md5Round( OperationH, d, a, b, c, X[12], 11, 46);
    md5Round( OperationH, c, d, a, b, X[15], 16, 47);
    md5Round( OperationH, b, c, d, a, X[ 2], 23, 48);

    // round 4
    md5Round( OperationI, a, b, c, d, X[ 0],  6, 49);
    md5Round( OperationI, d, a, b, c, X[ 7], 10, 50);
    md5Round( OperationI, c, d, a, b, X[14], 15, 51);
    md5Round( OperationI, b, c, d, a, X[ 5], 21, 52);
    md5Round( OperationI, a, b, c, d, X[12],  6, 53);
    md5Round( OperationI, d, a, b, c, X[ 3], 10, 54);
    md5Round( OperationI, c, d, a, b, X[10], 15, 55);
    md5Round( OperationI, b, c, d, a, X[ 1], 21, 56);
    md5Round( OperationI, a, b, c, d, X[ 8],  6, 57);
    md5Round( OperationI, d, a, b, c, X[15], 10, 58);
    md5Round( OperationI, c, d, a, b, X[ 6], 15, 59);
    md5Round( OperationI, b, c, d, a, X[13], 21, 60);
    md5Round( OperationI, a, b, c, d, X[ 4],  6, 61);
    md5Round( OperationI, d, a, b, c, X[11], 10, 62);
    md5Round( OperationI, c, d, a, b, X[ 2], 15, 63);
    md5Round( OperationI, b, c, d, a, X[ 9], 21, 64);

    stateA += a;
    stateB += b;
    stateC += c;
    stateD += d;
  }

  // encode the hash from 4 Q_UINT32 to 16 Q_UINT8
  QByteArray digest;
  QDataStream digestIN(&digest, QIODevice::WriteOnly);
  digestIN.setByteOrder(QDataStream::LittleEndian);
  digestIN << (Q_UINT32)stateA;
  digestIN << (Q_UINT32)stateB;
  digestIN << (Q_UINT32)stateC;
  digestIN << (Q_UINT32)stateD;

  // encode the 16 Q_UINT8 into a HexString
  QString hex;
  for(i = 0; i < digest.size(); i++)
    hex = hex.sprintf("%s%02x", hex.latin1(), (Q_UINT8)digest.at(i));

  return hex;
}
